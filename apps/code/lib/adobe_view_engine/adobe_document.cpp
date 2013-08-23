#include "dp_all.h"
#include "adobe_document.h"

//#define SAVE_PASSWORD 1

#define JPEG_LIB_VERSION  62    /* Version 6b */
#define CONV_INT_TO_STR_X_(a) #a
#define CONV_INT_TO_STR(a) CONV_INT_TO_STR_X_(a)

Q_DECLARE_METATYPE(dpdoc::TOCItem*);

namespace adobe_view
{

bool initializePlatform()
{
    int code = dp::platformInit( dp::PI_DEFAULT );
    if( code != dp::IS_OK )
    {
        return false;
    }

    dp::setVersionInfo( "product", "boox" );

    // zlib and png version is set inside Hobbes through API calls
    // jpeg has to be done here because there is only #define, no version API
    dp::setVersionInfo( "jpeg", CONV_INT_TO_STR(JPEG_LIB_VERSION) );

    // register the primary device
    dp::cryptRegisterOpenSSL();
    dp::deviceRegisterPrimary();
    dp::deviceRegisterExternal();
    dp::deviceMountRemovablePartitions();
    dp::documentRegisterPDF();
    dp::documentRegisterEPUB();
    return true;
}

// URL-encode UTF8 string, adding file:/// and replacing \ to /
static bool urlEncodeFileName( const char * str, QByteArray & result )
{
    size_t len = 0;
    const char * p = str;
    while( true )
    {
        char c = *(p++);
        if( c == '\0' )
        {
            break;
        }
        len++;
        if( c < ' ' || (unsigned char)c > '~' || c == '%' || c == '+' )
        {
            len += 2;
        }
    }

    bool winShare = str[0] == '\\' && str[1] == '\\';
    bool unixAbs = !winShare && (str[0] == '/' || str[1] == '\\');
    char * url = new char[len+( winShare? 6 : ( unixAbs ? 8 : 9 ) )];
    if( winShare )
    {
        ::strcpy( url, "file:" );
    }
    else if( unixAbs )
    {
        ::strcpy( url, "file://" );
    }
    else
    {
        ::strcpy( url, "file:///" );
    }

    char * t = url + ::strlen(url);
    p = str;

    while( true )
    {
        char c = *(p++);
        if( c == '\0' )
            break;
        if( c < ' ' || (unsigned char)c > '~' || c == '%' || c == '+' )
        {
            sprintf( t, "%%%02X", (unsigned char)c );
            t += 3;
        }
        else if( c == ' ' )
        {
            *(t++) = '+';
        }
        else if( c == '\\' )
        {
            *(t++) = '/';
        }
        else
        {
            *(t++) = c;
        }
    }
    *(t++) = '\0';

    result = url;
    delete [] url;
    return true;
}

static bool getFileMimeType( const QString & file_name, QString & mime_type )
{
    if (file_name.endsWith(".psf", Qt::CaseInsensitive) ||
        file_name.endsWith(".ocf", Qt::CaseInsensitive))
    {
        mime_type = "application/psf";
        return true;
    }

    if (file_name.endsWith(".epub", Qt::CaseInsensitive))
    {
        mime_type = "application/epub+zip";
        return true;
    }

    if (file_name.endsWith(".pdf", Qt::CaseInsensitive))
    {
        mime_type = "application/pdf";
        return true;
    }

    if (file_name.endsWith(".svg", Qt::CaseInsensitive))
    {
        mime_type = "image/svg+xml";
        return true;
    }

    if (file_name.endsWith(".etd", Qt::CaseInsensitive))
    {
        mime_type = "application/x-ebx";
        return true;
    }
    return false;
}

static int getDocumentType( const QString & mime_type )
{
    if (mime_type == "application/pdf")
    {
        return FIX_PAGE_DOCUMENT;
    }
    return REFLOWABLE_DOCUMENT;
}

static dp::String initializeResourceURL()
{
    dp::String result;
    QByteArray res_folder = getenv( "ADOBE_RESOURCE_FOLDER" );
    if (res_folder.isEmpty())
    {
        return result;
    }

    QByteArray res_url;
    if ( urlEncodeFileName( res_folder.constData(), res_url ) )
    {
        result = res_url.constData();
    }
    return result;
}

class AdobeResProvider : public dpres::ResourceProvider 
{
public:
    AdobeResProvider( dp::String res_folder, bool verbose );
    virtual ~AdobeResProvider();

    /**
     *  Request a global resource download from a given url with a Stream with at least
     *  given capabilities. Security considerations are responsibilities of the host.
     *  If NULL is returned, request is considered to be failed.
     */
    virtual dpio::Stream * getResourceStream( const dp::String& url_in, unsigned int capabilities );

private:
    dp::String res_folder_;
    bool verbose_;
};

AdobeResProvider::AdobeResProvider( dp::String res_folder, bool verbose )
    : res_folder_(res_folder), verbose_(verbose)
{
}

AdobeResProvider::~AdobeResProvider()
{
}

dpio::Stream * AdobeResProvider::getResourceStream( const dp::String& url_in, unsigned int capabilities )
{
    dp::String url = url_in;
    if( verbose_ )
    {
        qDebug( "Loading %s\n", url.utf8() );
    }

    if( ::strncmp( url.utf8(), "data:", 5 ) == 0 )
    {
        return dpio::Stream::createDataURLStream( url, NULL, NULL );
    }

    // resources: user stylesheet, fonts, hyphenation dictionaries and resources they references
    if( ::strncmp( url.utf8(), "res:///", 7 ) == 0 && url.length() < 1024 && !res_folder_.isNull() )
    {
        char tmp[2048];
        ::strcpy( tmp, res_folder_.utf8() );
        ::strcat( tmp, url.utf8()+7 );
        url = dp::String( tmp );
    }

    dpio::Partition * partition = dpio::Partition::findPartitionForURL( url );
    if( partition != NULL )
    {
        return partition->readFile( url, NULL, capabilities );
    }
    return NULL;
}

class AdobeDocumentClientPrivate : public dpdoc::DocumentClient
{
public:
    AdobeDocumentClientPrivate(AdobeDocumentClient *host);
    virtual ~AdobeDocumentClientPrivate();

    virtual void * getOptionalInterface(const char * name);
    virtual int getInterfaceVersion();
    virtual dpio::Stream* getResourceStream(const dp::String &url, unsigned int caps);
    virtual bool canContinueProcessing(int kind);
    virtual void reportLoadingState(int state);
    virtual void reportDocumentError(const dp::String &errorString);
    virtual void reportErrorListChange();
    virtual void requestLicense(const dp::String &type, const dp::String &resourceId, const dp::Data &requestData);
    virtual void documentSerialized();
    virtual void requestDocumentPassword();

    inline dpdoc::Document *document() { return document_; }

    bool open(const QByteArray & url, const QString & mime_type);
    bool close();

private:
    dpdoc::Document     *document_;
    AdobeDocumentClient *host_;
};

AdobeDocumentClientPrivate::AdobeDocumentClientPrivate(AdobeDocumentClient *host)
    : document_(0)
    , host_(host)
{
}

AdobeDocumentClientPrivate::~AdobeDocumentClientPrivate()
{
    close();
}

bool AdobeDocumentClientPrivate::open(const QByteArray & url, const QString & mime_type)
{
    close();

    // initialize resource folder
    dp::String res_folder_url = initializeResourceURL();
    AdobeResProvider * res_provider = new AdobeResProvider( res_folder_url, false );
    dpres::ResourceProvider::setProvider( res_provider );

    document_ = dpdoc::Document::createDocument(this, dp::String(mime_type.toUtf8().constData()));
    if (document_ == 0)
    {
        return false;
    }

    // intialize the annotations manager
    dp::String url_str(url.constData());
    document_->setURL(url_str);
    return true;
}

/// Close this document
bool AdobeDocumentClientPrivate::close()
{
    if (document_ != 0)
    {
        document_->release();
        document_ = 0;
    }
    return true;
}

int AdobeDocumentClientPrivate::getInterfaceVersion()
{
    return 1;
}

void * AdobeDocumentClientPrivate::getOptionalInterface( const char * name )
{
    // TODO. Implement Me
    return 0;
}

dpio::Stream * AdobeDocumentClientPrivate::getResourceStream( const dp::String &url,
                                                              unsigned int capabilities )
{
    if( ::strncmp( url.utf8(), "data:", 5 ) == 0 )
    {
        return dpio::Stream::createDataURLStream( url, 0, 0 );
    }

    dpio::Partition * partition = dpio::Partition::findPartitionForURL( url );
    if( partition != 0 )
    {
        return partition->readFile( url, 0, capabilities );
    }
    return 0;
}

bool AdobeDocumentClientPrivate::canContinueProcessing(int kind)
{
    if (kind == dpdoc::PK_SEARCH)
    {
        return true;
    }

    BaseTask *current_task = host_->tasksHandler()->currentTask();

    // Handle GUI events
    QApplication::processEvents();
    if ( current_task != 0 && current_task->status() != TASK_RUN && current_task->type() == kind )
    {
        qDebug("Current Task: %d is canceled, status:%d", current_task->type(), current_task->status());
        return false;
    }
    return kind != dpdoc::PK_BACKGROUND;
}

void AdobeDocumentClientPrivate::reportLoadingState(int state)
{
    switch (state)
    {
    case dpdoc::LS_ERROR:
        {
            host_->notifyDocumentLoadError(QCoreApplication::tr("Load Error"));
        }
    case dpdoc::LS_INITIAL:
        {
            qDebug("\n\n LOG: Continue Loading! \n\n");
        }
        break;
    case dpdoc::LS_INCOMPLETE:
    case dpdoc::LS_COMPLETE:
        {
            host_->notifyDocumentReady();
        }
        break;
    default:
        qDebug("Unknown loading state:%d", state);
        break;
    }
}

void AdobeDocumentClientPrivate::reportDocumentError(const dp::String &errorString)
{
    qWarning("Error in document navigation:%s", errorString.utf8());
    host_->notifyDocumentLoadError(errorString.utf8());
}

void AdobeDocumentClientPrivate::reportErrorListChange()
{
}

void AdobeDocumentClientPrivate::requestLicense(const dp::String &type,
                                                const dp::String &resourceId,
                                                const dp::Data &requestData)
{
    QString type_str(type.utf8());
    QString resource_id_str(resourceId.utf8());
    QByteArray request_data((const char *)(requestData.data()));
    qWarning("Request License, Type:%s, Resource ID:%s, Request Data:%s",
             qPrintable(type_str), qPrintable(resource_id_str), request_data.constData());

    host_->notifyRequestLicense(type_str, resource_id_str, request_data);
    document_->setLicense(type, resourceId, dp::Data());
}

void AdobeDocumentClientPrivate::documentSerialized()
{
}

void AdobeDocumentClientPrivate::requestDocumentPassword()
{
    host_->notifyRequestPassword();
}

// AdobeDocumentClient
AdobeDocumentClient::AdobeDocumentClient()
    : doc_private_(new AdobeDocumentClientPrivate(this))
    , type_(INVALID_DOCUMENT)
    , toc_root_(0)
    , toc_model_(0)
    , search_conf_(this)
    , is_ready_(false)
    , options_loaded_(false)
    , error_found_(false)
    , tried_existing_password_(false)
    , license_required_(false)
{
}

AdobeDocumentClient::~AdobeDocumentClient()
{
}

bool AdobeDocumentClient::openCMSByPath(const QString & path)
{
    path_ = path;
    if (!openCMS())
    {
        qWarning("Open CMS Fails. The document cannot be opened");
        return false;
    }
    return true;
}

bool AdobeDocumentClient::open(const QString & path)
{
    if (!openCMSByPath(path))
    {
        return false;
    }

    if (loadFulfillmentItemFromDB() && fulfillment_item_.has_returned)
    {
        notifyDocumentLoadError(QCoreApplication::tr("The book has been returned."));
        return false;
    }

    if (!getFileMimeType(path, mime_type_))
    {
        return false;
    }

    type_ = getDocumentType(mime_type_);
    if ( !urlEncodeFileName( path.toUtf8().constData(), url_ ) )
    {
        return false;
    }
    return doc_private_->open(url_, mime_type_);
}

bool AdobeDocumentClient::isTheDocument(const QString &path)
{
    return ( path_ == path );
}

/// Close this document
bool AdobeDocumentClient::close()
{
    // clear all of the tasks before closing.
    tasks_handler_.clearTasks();
    if (!is_ready_)
    {
        return true;
    }

    emit documentCloseSignal();
    toc_model_.reset(0);
    database_.close();
    if (!toc_items_.empty())
    {
        TOCItems::iterator iter = toc_items_.begin();
        for (; iter != toc_items_.end(); ++iter)
        {
            if (*iter != 0)
            {
                (*iter)->release();
            }
        }
        toc_items_.clear();
    }

    if (toc_root_ != 0)
    {
        toc_root_->release();
    }

    //doc_private_->close();
    is_ready_    = false;
    error_found_ = false;
    return true;
}

bool AdobeDocumentClient::loadFulfillmentItemFromDocument()
{
    if (fulfillment_item_.isValid())
    {
        return true;
    }

    if (!is_ready_)
    {
        return false;
    }

    dpdoc::Document * document = doc_private_->document();
    dp::ref<dpdrm::Rights> rights = document->getRights();
    dp::list<dpdrm::License> licenses = rights->getLicenses();
    for (size_t i = 0; i < licenses.length(); ++i)
    {
        dp::String fulfillment_id = licenses[i]->getFulfillmentID();
        fulfillment_item_.fulfillment_id = fulfillment_id.utf8();

        unsigned long long expire_time = 0;
        dp::list<dpdrm::Permission> permissions = licenses[i]->getPermissions("display");
        for (size_t m = 0; m < permissions.length(); ++m)
        {
            expire_time = permissions[m]->getExpiration();
        }
        if (expire_time != 0)
        {
            fulfillment_item_.is_returnable = !fulfillment_item_.fulfillment_id.isEmpty();
            fulfillment_item_.has_returned  = false;
            fulfillment_item_.expiration_date = expire_time;

            qDebug("Loading... Fulfillment ID:%s from Document", qPrintable(fulfillment_item_.fulfillment_id));
            uint expiration_date_in_secs = fulfillment_item_.expiration_date / 1000;
            if (expiration_date_in_secs != 0)
            {
                QDateTime date_time = QDateTime::fromTime_t(expiration_date_in_secs);
                qDebug("Is Returnable:%d, Has Returned:%d, Expire:%s",
                       fulfillment_item_.is_returnable,
                       fulfillment_item_.has_returned,
                       qPrintable(date_time.toString()));
            }
        }
    }
    return true;
}

bool AdobeDocumentClient::loadFulfillmentItemFromDB()
{
    if (fulfillment_item_.isValid())
    {
        return true;
    }

    if (path_.isEmpty())
    {
        qWarning("Document Path of fulfillment item is empty");
        return false;
    }

    fulfillment_item_.is_returnable = false;
    fulfillment_item_.has_returned  = false;

    cms::ContentNode node;
    QFileInfo info(path_);
    node.mutable_name() = info.fileName();
    node.mutable_location() = info.path();
    node.mutable_size() = info.size();
    if (database_.getContentNode(node))
    {
        QVariantMap attributes;
        node.attributes(attributes);
        fulfillment_item_.fulfillment_id = attributes[CMS_FULFILLMENT_ID].toString();
        if (attributes.contains(CMS_IS_RETURNABLE))
        {
            fulfillment_item_.is_returnable  = attributes[CMS_IS_RETURNABLE].toBool();
        }
        if (attributes.contains(CMS_HAS_RETURNED))
        {
            fulfillment_item_.has_returned   = attributes[CMS_HAS_RETURNED].toBool();
        }
        if (attributes.contains(CMS_EXPIRED_DATE))
        {
            fulfillment_item_.expiration_date = attributes[CMS_EXPIRED_DATE].toULongLong();
        }

        qDebug("Loading... Fulfillment ID:%s", qPrintable(fulfillment_item_.fulfillment_id));
        uint expiration_date_in_secs = fulfillment_item_.expiration_date / 1000;
        if (expiration_date_in_secs != 0)
        {
            QDateTime date_time = QDateTime::fromTime_t(expiration_date_in_secs);
            qDebug("Is Returnable:%d, Has Returned:%d, Expire:%s",
                   fulfillment_item_.is_returnable,
                   fulfillment_item_.has_returned,
                   qPrintable(date_time.toString()));
        }
        return true;
    }
    return false;
}

bool AdobeDocumentClient::saveMetadata()
{
    QString attrs;
    conf_.info.mutable_title() = metaData("DC.title", attrs);
    qDebug("Title:%s", conf_.info.title().toUtf8().constData());

    conf_.info.mutable_authors() = metaData("DC.creator", attrs);
    qDebug("Authors:%s", conf_.info.authors().toUtf8().constData());

    conf_.info.mutable_publisher() = metaData("DC.publisher", attrs);
    if (conf_.info.publisher().isEmpty())
    {
        conf_.info.mutable_publisher() = metaData("Publisher", attrs);
    }
    qDebug("Publisher:%s", conf_.info.publisher().toUtf8().constData());
    return true;
}

QString AdobeDocumentClient::metaData(const QString & name, QString & attrs)
{
    dp::String request_name(name.toUtf8().constData());

    dp::String ns; // TODO. Fill this value
    dp::String attribute_name; // TODO. Fill this value

    QString ret;
    dp::String meta_data;
    dp::String additional_attrs;
    int index = 0;
    const int max = 100;
    do
    {
        dp::ref<dpdoc::MetadataItem> metadata_item = document()->getMetadata(request_name, index++);
        if (metadata_item != 0)
        {
            meta_data = metadata_item->getValue();
            //additional_attrs = metadata_item->getAttribute(ns, attribute_name);    //TODO. Get attributes
        }
    } while (meta_data.isNull() && index < max);

    if (!meta_data.isNull())
    {
        ret = QString::fromUtf8(meta_data.utf8());
    }

    if (!additional_attrs.isNull())
    {
        attrs = QString::fromUtf8(additional_attrs.utf8());
    }
    return ret;
}

void AdobeDocumentClient::setPassword(const QString & password)
{
#ifdef SAVE_PASSWORD
    if (loadOptions())
    {
        QVariantMap attributes;
        conf_.info.attributes(attributes);
        attributes[CMS_PASSWORD] = password;
        conf_.info.setAttributes(attributes);
    }
    tried_existing_password_ = true;
#endif
    document()->setDocumentPassword(dp::String(password.toUtf8().constData()));
}

void AdobeDocumentClient::setSearchBusy(bool busy)
{
    BaseTask *current_task = tasksHandler()->currentTask();
    if (current_task != 0 && current_task->type() == dpdoc::PK_SEARCH)
    {
        if (!busy)
        {
            current_task->abort();
        }
        else
        {
            current_task->start();
        }
    }
}

int AdobeDocumentClient::type()
{
    return type_;
}

bool AdobeDocumentClient::hasToc()
{
    return ( toc_root_ != 0 && toc_root_->getChildCount() > 0 );
}

void AdobeDocumentClient::loadTOCItem( QStandardItem * parent, dpdoc::TOCItem * cur_item )
{
    int childs_count = cur_item->getChildCount();
    for ( int i = 0; i < childs_count; ++i )
    {
        dpdoc::TOCItem *child = cur_item->getChild( i );
        toc_items_.push_back( child );

        // create new standard item for this item
        QString title = QString::fromUtf8( child->getTitle().utf8() ).trimmed();
        QStandardItem *model_item = new QStandardItem( title );
        model_item->setData( QVariant::fromValue(child), TOC_ITEM );

        dp::ref<dpdoc::Location> location = child->getLocation();
        QStandardItem *page_item = 0;
        if (location != 0)
        {
            double pos = location->getPagePosition();
            location->release();

            QString page_str;
            page_str.setNum( static_cast<int>(pos) + 1 );
            page_item = new QStandardItem( page_str );
            page_item->setTextAlignment( Qt::AlignCenter );
            page_item->setData( QVariant::fromValue(child), TOC_ITEM );
        }

        int row_count = parent->rowCount();
        parent->appendRow( model_item );
        if (page_item != 0)
        {
            parent->setChild( row_count, 1, page_item );
        }
        loadTOCItem( model_item, child );
    }
}

QStandardItemModel* AdobeDocumentClient::getTOCModel()
{
    if (toc_model_ != 0)
    {
        return toc_model_.get();
    }

    if (toc_root_ != 0)
    {
        toc_model_.reset( new QStandardItemModel() );
        toc_items_.clear();
        QStandardItem *root = toc_model_->invisibleRootItem();
        loadTOCItem( root, toc_root_ );

        // set header data
        toc_model_->setHeaderData(0, Qt::Horizontal, QVariant::fromValue(tr("Title")), Qt::DisplayRole);
        toc_model_->setHeaderData(1, Qt::Horizontal, QVariant::fromValue(tr("Page")), Qt::DisplayRole);
        return toc_model_.get();
    }
    return 0;
}

AdobeLocationPtr AdobeDocumentClient::getPositionByTOCIndex( const QModelIndex& index )
{
    if (toc_model_ == 0)
    {
        return AdobeLocationPtr();
    }

    QStandardItem *item = toc_model_->itemFromIndex( index );
    dpdoc::TOCItem *toc = item->data(TOC_ITEM).value<dpdoc::TOCItem*>();

    AdobeLocationPtr ret( new AdobeLocation(toc->getLocation()) );
    return ret;
}

AdobeLocationPtr AdobeDocumentClient::searchString( const AdobeLocationPtr & start,
                                                    const AdobeLocationPtr & end,
                                                    const QString & pattern,
                                                    const bool forward )
{
    if ( start >= end )
    {
        return AdobeLocationPtr();
    }

    unsigned int flags = dpdoc::SF_MATCH_CASE |
                         dpdoc::SF_WHOLE_WORD |
                         ( forward ? 0 : dpdoc::SF_BACK );
    Range result;
    Range from(start, end);
    if (findText(from, flags, pattern, result))
    {
        if (!forward)
        {
            return result.end >= start ? result.end : AdobeLocationPtr();
        }
        return result.start <= end ? result.start : AdobeLocationPtr();
    }
    return AdobeLocationPtr();
}

bool AdobeDocumentClient::findText( const Range & from,
                                    unsigned int flags,
                                    const QString & pattern,
                                    Range & result )
{
    dp::String string_to_find(pattern.toUtf8().constData());
    dpdoc::Location *from_start = from.start->getData();
    dpdoc::Location *from_end = from.end->getData();
    if (flags & dpdoc::SF_BACK)
    {
        from_start = from.end->getData();
        from_end = from.start->getData();
    }

    dpdoc::Range range;
    bool ret = document()->findText(from_start,
                                    from_end,
                                    flags,
                                    string_to_find,
                                    &range);
    if (ret)
    {
        result.start.reset(new AdobeLocation(range.beginning));
        result.end.reset(new AdobeLocation(range.end));
    }
    return ret;
}

void AdobeDocumentClient::handleSearchResult( bool found, const AdobeSearchConf * search_conf )
{
    if (found)
    {
        search_conf_ = *search_conf;
        emit searchSucceeded();
        return;
    }
    emit searchNoMoreResults();
}

void AdobeDocumentClient::sendSearchRequest( AdobeSearchConf * search_conf )
{
    if (document() != 0)
    {
        AdobeSearchConf *new_request = new AdobeSearchConf( *search_conf );
        tasksHandler()->addTask( new_request, false );
    }
}

void AdobeDocumentClient::notifyDocumentLoadError(const QString & error)
{
    error_found_ = true;
    emit documentErrorSignal(error);
}

void AdobeDocumentClient::notifyRequestLicense(const QString & type,
                                               const QString & resource_id,
                                               const QByteArray & request_data)
{
    license_required_ = true;
    emit documentRequestLicense(type, resource_id, request_data);
}

void AdobeDocumentClient::notifyDocumentReady()
{
    is_ready_ = true;

#ifdef SAVE_PASSWORD
    tried_existing_password_ = false;
#endif

    loadFulfillmentItemFromDocument();
    toc_root_ = doc_private_->document()->getTocRoot();
    emit documentReadySignal();
}

void AdobeDocumentClient::notifyRequestPassword()
{
#ifdef SAVE_PASSWORD
    license_required_ = true;
    QString password = retrievePasswordFromDB();
    if (password.isEmpty() || tried_existing_password_)
    {
        emit requestPassword();
        return;
    }
    tried_existing_password_ = true;
    document()->setDocumentPassword(dp::String(password.toUtf8().constData()));
#else
    emit requestPassword();
#endif
}

AdobeLocationPtr AdobeDocumentClient::getBeginning()
{
    dp::ref<dpdoc::Location> loc = document()->getBeginning();
    AdobeLocationPtr beginning(new AdobeLocation(loc));
    return beginning;
}

AdobeLocationPtr AdobeDocumentClient::getEnd()
{
    dp::ref<dpdoc::Location> loc = document()->getEnd();
    AdobeLocationPtr end(new AdobeLocation(loc));
    return end;
}

AdobeLocationPtr AdobeDocumentClient::getLocationFromBookmark(const QString & bookmark)
{
    dp::String bkmk_str(bookmark.toUtf8().constData());
    dp::ref<dpdoc::Location> loc = document()->getLocationFromBookmark(bkmk_str);
    AdobeLocationPtr loc_ptr(new AdobeLocation(loc));
    return loc_ptr;
}

AdobeLocationPtr AdobeDocumentClient::getLocationFromPagePosition(double pos)
{
    dp::ref<dpdoc::Location> loc = document()->getLocationFromPagePosition(pos);
    AdobeLocationPtr loc_ptr(new AdobeLocation(loc));
    return loc_ptr;
}

double AdobeDocumentClient::getPagePositionFromLocation(const AdobeLocationPtr & location)
{
    double position = location->getPagePosition();
    return position;
}

QString AdobeDocumentClient::getText(const Range & range)
{
    dp::String title = doc_private_->document()->getText(range.start->getData(),
                                                         range.end->getData());
    QString title_str = QString::fromUtf8(title.utf8());
    return title_str;
}

AdobeContentIteratorPtr AdobeDocumentClient::getContentIterator(int variety, const AdobeLocationPtr & location)
{
    dpdoc::ContentIterator *iter = doc_private_->document()->getContentIterator(variety, location->getData());
    AdobeContentIteratorPtr iter_ptr(new AdobeContentIterator(iter));
    return iter_ptr;
}

bool AdobeDocumentClient::saveOptions(bool add_to_history)
{
    bool ret = openCMS();
    if (ret)
    {
        if (fulfillment_item_.isValid())
        {
            QVariantMap attributes;
            conf_.info.attributes(attributes);
            attributes[CMS_FULFILLMENT_ID] = fulfillment_item_.fulfillment_id;
            attributes[CMS_IS_RETURNABLE]  = fulfillment_item_.is_returnable;
            attributes[CMS_HAS_RETURNED]   = fulfillment_item_.has_returned;
            if (fulfillment_item_.expiration_date != 0)
            {
                attributes[CMS_EXPIRED_DATE] = fulfillment_item_.expiration_date;
            }

            qDebug("Save fulfillment item, expiration:%lld", fulfillment_item_.expiration_date);
            conf_.info.setAttributes(attributes);
        }
        saveMetadata();
        ret = vbf::saveDocumentOptions(database_, path_, conf_, add_to_history);
    }
    return ret;
}

bool AdobeDocumentClient::openCMS()
{
    if (!database_.isOpen())
    {
        vbf::openDatabase(path_, database_);
    }
    return database_.isOpen();
}

QString AdobeDocumentClient::retrievePasswordFromDB()
{
    QString password;
    if (loadOptions())
    {
        QVariantMap attributes;
        conf_.info.attributes(attributes);
        if (attributes.contains(CMS_PASSWORD))
        {
            password = attributes[CMS_PASSWORD].toString();
        }
    }
    return password;
}

bool AdobeDocumentClient::loadOptions()
{
    bool ret = openCMS();
    if (ret && !options_loaded_)
    {
        ret = vbf::loadDocumentOptions(database_, path_, conf_);
        options_loaded_ = ret;
    }
    return ret;
}

double AdobeDocumentClient::getPageCount()
{
    return document()->getPageCount();
}

dpdoc::Document* AdobeDocumentClient::document()
{
    return doc_private_->document();
}

}
