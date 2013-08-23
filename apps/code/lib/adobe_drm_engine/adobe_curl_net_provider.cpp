#include <string.h>
#include <stdlib.h> //atoi
#include <curl/curl.h>

#include "adobe_curl_net_provider.h"

namespace adobe_drm
{

#if (LIBCURL_VERSION_NUM >= 0x071200)
#define USE_CURL_PAUSE 
#endif

static size_t CurlStream_header( void *ptr, size_t size, size_t nmemb, void *handle);
static size_t CurlStream_writer( void *ptr, size_t size, size_t nmemb, void *handle);
static size_t CurlStream_reader( void *ptr, size_t size, size_t nmemb, void *handle);

class CurlStream : public dputils::GuardedStream, dpio::StreamClient
{
public:
    CurlStream(const dp::String& method, const dp::String& url, dpio::StreamClient * client, dpio::Stream * data_to_post) :
        method_(method),
        client_(client),
        data_to_post_(data_to_post),
        read_offset_(0),
        write_offset_(0),
        data_read_(NULL),
        data_capacity_(0),
        data_bytes_read_(0),
        state_(0),
        saved_(NULL),
        saved_size_(0)
    {
        char * szHeaders = NULL;
        char szContentTypeHeader[] = "Content-type: ";
        size_t contentTypeHeaderLen = sizeof(szContentTypeHeader)/sizeof(char) - 1;

        // Create our cur_ handle
        cur_ = curl_easy_init();
        headers_ = NULL;
        if (cur_)
        {
            // Now set up all of the cur_ options  
            curl_easy_setopt(cur_, CURLOPT_URL, url.utf8());  
            curl_easy_setopt(cur_, CURLOPT_FOLLOWLOCATION, 1);  
            curl_easy_setopt(cur_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);		

            if( 0 == strcmp(method.utf8(), "POST") )
            {
                curl_easy_setopt(cur_, CURLOPT_POST, true );

                // Read all the data to be posted
                // WARNING: this code works only for the synchronous stream
                if (data_to_post_)
                {
                    data_to_post_->setStreamClient(this);
                    data_to_post_->requestInfo();
                    data_to_post_->requestBytes(0, (size_t)-1);
                }

                //const char *strContentType = contentTypeUTF16.utf16();
                size_t contentTypeLen = content_type_.length();

                size_t headersLen = 0;
                if( contentTypeLen != 0 && data_bytes_read_ != 0 )
                {
                    headersLen = contentTypeLen + contentTypeHeaderLen;
                    szHeaders = new char[headersLen + 1];
                    ::strcpy( szHeaders, szContentTypeHeader );
                    ::strcpy( szHeaders + contentTypeHeaderLen, content_type_.utf8() );

                    headers_ = curl_slist_append (headers_, szHeaders);
                    curl_easy_setopt(cur_, CURLOPT_HTTPHEADER, headers_);
                    curl_easy_setopt(cur_, CURLOPT_READFUNCTION, CurlStream_reader );
                    curl_easy_setopt(cur_, CURLOPT_READDATA, static_cast<void*>(this));
                    curl_easy_setopt(cur_, CURLOPT_POSTFIELDSIZE, data_bytes_read_);
                }
                else
                {
                    curl_easy_setopt(cur_, CURLOPT_HEADER, 0);
                }
            }

            curl_easy_setopt(cur_, CURLOPT_HEADERFUNCTION, CurlStream_header);  
            curl_easy_setopt(cur_, CURLOPT_HEADERDATA, static_cast<void *>(this));
            curl_easy_setopt(cur_, CURLOPT_WRITEFUNCTION, CurlStream_writer);  
            curl_easy_setopt(cur_, CURLOPT_WRITEDATA, static_cast<void *>(this));
        }

        if (szHeaders)
        {
            delete[] szHeaders;
        }

    }

    ~CurlStream()
    {
            // Always cleanup  
            curl_easy_cleanup(cur_);
            if( headers_ )
            {
                curl_slist_free_all( headers_ );
            }
            if (saved_)
                delete[] saved_;
    }

    void deleteThis()
    {
        delete this;
    }

    void setStreamClient( dpio::StreamClient * client )
    {
        client_ = client;
    }

    unsigned int getCapabilities()
    {
        return dpio::SC_SYNCHRONOUS;
    }

    void requestInfo()
    {
        if (state_ == 0)
        {
            state_ = 1; 
            perform();
        }
    }

    void requestBytes( size_t offset, size_t len )
    {
        switch (state_)
        {
        case 0: // requestInfo has not be called
            state_ = 3;
            perform();
            break;
        case 1: // requestInfo has been called but we are not blocked yet
            state_ = 3;
            break;
        case 2: // we are blocked 
            state_ = 3;
    #ifdef USE_CURL_PAUSE
            curl_easy_pause(cur_, CURLPAUSE_SEND_CONT);
    #endif
            break;
    #ifndef USE_CURL_PAUSE
        case 4: 
            state_ = 3;
            break;
    #endif
        }
    }

    void reportWriteError( const dp::String& error )
    {
        dputils::StreamGuard guard(this);

        if (client_)
            client_->reportError( error );
    }

    virtual void propertyReady( const dp::String& name, const dp::String& value )
    {
        if( ::strcmp(name.utf8(), "Content-Type") == 0) 
            content_type_ = value;
    }

    virtual void propertiesReady()
    {
    }

    virtual void totalLengthReady( size_t length )
    {
        if (!data_read_) {
            data_read_ = new unsigned char[length];
            data_capacity_ = length;
        }
    }

    virtual void bytesReady( size_t offset, const dp::Data& data, bool eof )
    {
        if( !data.isNull() )
        {
            size_t buflen;
            const unsigned char * buf = data.data(&buflen);

            if (offset != data_bytes_read_) 
            {
                reportError("Stream received non-sequentially");
                return;
            }

            if (data_capacity_ < data_bytes_read_ + buflen) 
            {

                data_capacity_ = data_bytes_read_ + buflen;
                unsigned char * tmp = new unsigned char[data_capacity_];
                if (data_read_)
                {
                    ::memcpy(tmp, data_read_, data_bytes_read_);
                    delete[] data_read_;
                }
                data_read_ = tmp;
            }

            memcpy(data_read_ + data_bytes_read_, buf, buflen);
            data_bytes_read_ += buflen;
        }
    }

    virtual void reportError( const dp::String& error )
    {
        dputils::StreamGuard guard(this);

        if (client_)
            client_->reportError (error);
    }

    private:

    void perform ()
    {
        dputils::StreamGuard guard(this);

        int result = curl_easy_perform(cur_);
        if (result != 0) 
        {
            char err[1024];
            sprintf(err, "CURL returned: %d", result);
            if (client_)
                client_->reportError(err);
        }
        if (saved_ != NULL)
        {
            if (client_)
                client_->bytesReady(write_offset_, dp::Data(saved_, saved_size_), true);
            delete[] saved_;
            saved_size_ = 0;
        }
        else
        {
            if (client_)
                client_->bytesReady(write_offset_, dp::Data(), true);
        }
    }

    void append (unsigned char * ptr, size_t bytes)
    {
        if (saved_ == NULL)
        {
            saved_ = new unsigned char[bytes];
            memcpy(saved_, ptr, bytes);
            saved_size_ = bytes;
        } else {
            unsigned char * tmp = saved_;
            saved_ = new unsigned char[saved_size_ + bytes];
            memcpy(saved_, tmp, saved_size_);
            memcpy(saved_ + saved_size_, ptr, bytes);
            saved_size_ += bytes;
            delete[] tmp;
        }
    }

    size_t header_callback( void *ptr, size_t size, size_t nmemb)
    {
        dputils::StreamGuard guard(this);

        if (!client_)
            return 0;

        // What we will return  
        size_t result = 0;  
        size_t bytes = size * nmemb;

        if (state_ > 1)
            return  bytes;


        // we are not guaranteed that ptr is null-terminated
        char *buf = new char[bytes + 1];
        if (ptr != NULL && bytes != 0)
            ::memcpy (buf, ptr, bytes);
        buf[bytes] = (char)0;
        size_t len = ::strlen(buf);
        while (len > 0 && buf[len-1] <= ' ')
        {
            len--;
            buf[len] = 0;
        }

        if (len == 0)
        {
    #ifdef USE_CURL_PAUSE
            curl_easy_pause (cur_, CURLPAUSE_SEND);
            state_ = 2;
    #else
            state_ = 4;
    #endif
            if (client_)
            {
                client_->propertiesReady();
            }
        } 
        else
        {
            char * colon = ::strchr(buf, ':');
            if (colon)
            {
                *colon = (char)0;
                do
                {
                    ++colon;
                } while (*colon == ' ');

                int t = ::strlen(colon);

                if (client_)
                {
                    if (::strcmp(buf,  "Content-Length") == 0)
                    {
                        int len = ::atoi(colon);
                        if (len > 0)
                        {
                            client_->totalLengthReady(len);
                        }
                    }
                    else
                    {
                        client_->propertyReady(buf, colon);
                    }
                }
            }
        }

        delete[] buf;

        if (client_)
            result = bytes;

        return result;  
    }

    // This is the writer call back function used by cur_  
    size_t writer_callback( void *ptr, size_t size, size_t nmemb)
    {
        dputils::StreamGuard guard(this);

        if (!client_)
            return 0;

        // What we will return  
        size_t result = 0;  
        size_t bytes = size * nmemb;
        switch (state_)
        {
        case 0: //  broken state;
            return 0;
        case 1: // unexpected, but let's recover
    #ifdef USE_CURL_PAUSE
            curl_easy_pause (cur_, CURLPAUSE_SEND);
            state_ = 2;
    #else
            state_ = 4;
    #endif
            append ((unsigned char *)ptr, bytes);
            if (client_)
            {
                client_->propertiesReady();
            }
            break;
        case 2:
            {
                //it's the first time after requestBytes was called
                append((unsigned char *)ptr, bytes);
                dp::Data data (saved_, saved_size_);
                state_ = 3;
                if (client_)
                {
                    size_t oldOffset = write_offset_;
                    write_offset_ += bytes;
                    client_->bytesReady(oldOffset, data, bytes == 0);
                }
            }
            break;
        case 3:
            {
                dp::Data data (static_cast<unsigned char *>(ptr), bytes);
                if (client_)
                {
                    size_t oldOffset = write_offset_;
                    write_offset_ += bytes;
                    client_->bytesReady(oldOffset, data, bytes == 0);
                }
            }
            break;

    #ifndef USE_CURL_PAUSE
        case 4:
            {
                append ((unsigned char *)ptr, bytes);
            }
            break;
    #endif
        }
        if (client_)
            result = bytes;

        return result;
    }

    // This is the reader call back function used by cur_  
    size_t reader_callback ( void *ptr, size_t size, size_t nmemb)
    {
        // What we will return  
        size_t result = 0;
        size_t requestedBytes = size * nmemb;
        size_t actualBytes = data_bytes_read_ - read_offset_;

        if( actualBytes == 0 )
            return 0;

        if( actualBytes <= requestedBytes )
        {
            memcpy( ptr, data_read_ + read_offset_, actualBytes );
            result = actualBytes;
        }
        else
        {
            memcpy(ptr, data_read_ + read_offset_, requestedBytes);
            result = requestedBytes;
        }

        read_offset_ += result;
    	
        return result;  
    }

private:
    dp::String          method_;
    dpio::StreamClient  *client_;
    dpio::Stream        *data_to_post_;

    size_t              write_offset_;
    size_t              read_offset_;

    CURL *cur_;  
    struct curl_slist   *headers_;
    dp::String          content_type_;
    unsigned char       *data_read_;
    size_t              data_capacity_;
    size_t              data_bytes_read_;

    int                 state_;
    unsigned char       *saved_;
    size_t              saved_size_;

    friend size_t CurlStream_header( void *ptr, size_t size, size_t nmemb, void *buffer);
    friend size_t CurlStream_writer( void *ptr, size_t size, size_t nmemb, void *buffer);
    friend size_t CurlStream_reader( void *ptr, size_t size, size_t nmemb, void *buffer);
};


CurlNetProvider::CurlNetProvider()
{
}

CurlNetProvider::~CurlNetProvider()
{
}

// This is the header writer call back function used by cur_  
static size_t CurlStream_header( void *ptr, size_t size, size_t nmemb, void *handle)
{
    // What we will return  
    size_t result = 0;  
    if( handle != NULL )
    {
        CurlStream * stream = reinterpret_cast<CurlStream *>(handle);
        result = stream->header_callback (ptr, size, nmemb);
    }

    return result;
}

// This is the writer call back function used by cur_  
static size_t CurlStream_writer( void *ptr, size_t size, size_t nmemb, void *handle)
{
    // What we will return  
    size_t result = 0;  
    if( handle != NULL )
    {
        CurlStream * stream = reinterpret_cast<CurlStream *>(handle);
        result = stream->writer_callback (ptr, size, nmemb);
    }

    return result;
}

// This is the reader call back function used by cur_  
static size_t CurlStream_reader( void *ptr, size_t size, size_t nmemb, void *handle)
{
    // What we will return  
    size_t result = 0;  
    if( handle != NULL )
    {
        CurlStream * stream = reinterpret_cast<CurlStream *>(handle);
        result = stream->reader_callback (ptr, size, nmemb);
    }

    return result;
}

dpio::Stream* CurlNetProvider::open( const dp::String& method,
                                     const dp::String& url,
                                     dpio::StreamClient * client,
                                     unsigned int cap,
                                     dpio::Stream * data_to_post)
{
    return new CurlStream(method, url, client, data_to_post);
}

}
