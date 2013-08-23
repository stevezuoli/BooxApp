#include "dp_all.h"
#include "adobe_search_conf.h"
#include "adobe_document.h"

namespace adobe_view
{

AdobeSearchConf::AdobeSearchConf(AdobeDocumentClient *doc_client)
    : BaseTask(dpdoc::PK_SEARCH)
    , document_client_(doc_client)
    , operation_(SEARCH_NEXT)
    , flags_(0)
    , pattern_()
{
}

AdobeSearchConf::AdobeSearchConf(const AdobeSearchConf & right)
    : BaseTask(dpdoc::PK_SEARCH)
    , document_client_(right.document_client_)
    , operation_(right.operation_)
    , search_range_(right.search_range_)
    , flags_(right.flags_)
    , pattern_(right.pattern_)
    , search_results_(right.search_results_)
{
}

AdobeSearchConf::~AdobeSearchConf()
{
}

AdobeSearchConf& AdobeSearchConf::operator = (const AdobeSearchConf & right)
{
    document_client_ = right.document_client_;
    operation_      = right.operation_;
    search_range_   = right.search_range_;
    flags_          = right.flags_;
    pattern_        = right.pattern_;
    search_results_ = right.search_results_;
    return *this;
}

void AdobeSearchConf::exec()
{
    QApplication::processEvents();
    start();
    bool ret = false;
    switch ( operation_ )
    {
    case SEARCH_NEXT:
        ret = searchNext();
        break;
    case SEARCH_ALL:
        ret = searchAll();
        break;
    default:
        break;
    }

    document_client_->handleSearchResult( ret, this );
    abort();
}

bool AdobeSearchConf::searchAllInRange( const Range & range )
{
    if ( !range.isValid() )
    {
        return false;
    }

    search_range_ = range;
    return searchAll();
}

bool AdobeSearchConf::searchNext()
{
    assert(document_client_ != 0);

    Range result;
    bool ret = document_client_->findText( search_range_, flags_, pattern_, result );
    if (ret)
    {
        search_results_.clear();
        search_results_.push_back( result );
    }
    return ret;
}

bool AdobeSearchConf::searchAll()
{
    assert(document_client_ != 0);

    search_results_.clear();
    Range from = search_range_;
    bool ret = true;
    while ( ret )
    {
        Range result;
        ret = document_client_->findText( from, flags_, pattern_, result );
        if ( ret )
        {
            if ( flags_ & dpdoc::SF_BACK )
            {
                if ( result.start < from.start )
                {
                    break;
                }
                search_results_.push_front( result );
                from.end = result.start;
            }
            else
            {
                if ( result.end > from.end )
                {
                    break;
                }
                search_results_.push_back( result );
                from.start = result.end;
            }
        }
    }

    return !search_results_.empty();
}

}
