#ifndef ADOBE_SEARCH_CONF_H_
#define ADOBE_SEARCH_CONF_H_

#include "adobe_utils.h"
#include "adobe_location.h"

using namespace vbf;

namespace adobe_view
{

typedef QList<Range> SearchResults;

class AdobeDocumentClient;
class AdobeSearchConf : public BaseTask
{
public:
    AdobeSearchConf(AdobeDocumentClient *doc_client);
    AdobeSearchConf(const AdobeSearchConf & right);
    ~AdobeSearchConf();

    AdobeSearchConf& operator = (const AdobeSearchConf & right);

    void exec();
    bool searchAllInRange( const Range & range );

    inline void setStart( AdobeLocationPtr s );
    inline AdobeLocationPtr start() const;
    inline void setEnd( AdobeLocationPtr e );
    inline AdobeLocationPtr end() const;
    inline void setFlags( unsigned int f );
    inline unsigned int flags() const;
    inline void setPattern( const QString & p );
    inline const QString & pattern() const;
    inline const SearchResults & results() const;
    inline void clearSearchResults();
    inline bool clearSearchResultsButKeepCurrent();

private:
    bool searchNext();
    bool searchAll();

private:
    AdobeDocumentClient     *document_client_;
    AdobeSearchOperation    operation_;
    Range                   search_range_;
    unsigned int            flags_;
    QString                 pattern_;
    SearchResults           search_results_;
};

void AdobeSearchConf::setStart( AdobeLocationPtr s )
{
    search_range_.start = s;
}

AdobeLocationPtr AdobeSearchConf::start() const
{
    return search_range_.start;
}

void AdobeSearchConf::setEnd( AdobeLocationPtr e )
{
    search_range_.end = e;
}

AdobeLocationPtr AdobeSearchConf::end() const
{
    return search_range_.end;
}

void AdobeSearchConf::setFlags( unsigned int f )
{
    flags_ = f;
}

unsigned int AdobeSearchConf::flags() const
{
    return flags_;
}

void AdobeSearchConf::setPattern( const QString & p )
{
    pattern_ = p;
}

const QString & AdobeSearchConf::pattern() const
{
    return pattern_;
}

const SearchResults & AdobeSearchConf::results() const
{
    return search_results_;
}

void AdobeSearchConf::clearSearchResults()
{
    search_results_.clear();
}

bool AdobeSearchConf::clearSearchResultsButKeepCurrent()
{
    if ( search_results_.empty() ) return false;
    Range current = search_results_.front();
    search_results_.clear();
    search_results_.push_back( current );
    return true;
}

};

#endif
