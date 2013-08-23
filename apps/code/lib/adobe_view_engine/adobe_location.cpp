#include "dp_all.h"
#include "adobe_location.h"

namespace adobe_view
{

class AdobeLocationPrivate
{
public:
    AdobeLocationPrivate( dpdoc::Location* location ): data_(location){}
    ~AdobeLocationPrivate() {}

private:
    dp::ref<dpdoc::Location> data_;
    friend class AdobeLocation;
};

AdobeLocation::AdobeLocation( dpdoc::Location* location )
    : data_(new AdobeLocationPrivate(location))
{
}

AdobeLocation::~AdobeLocation()
{
    if (data_ != 0)
    {
        delete data_;
    }
}

QString AdobeLocation::getBookmark()
{
    return QString(data_->data_->getBookmark().utf8());
}

double AdobeLocation::getPagePosition()
{
    return data_->data_->getPagePosition();
}

dpdoc::Location* AdobeLocation::getData()
{
    return data_->data_;
}

bool operator <= ( const AdobeLocationPtr & a, const AdobeLocationPtr & b )
{
    int ret = a->getData()->compare( b->getData() );
    return ret <= 0;
}

bool operator >= ( const AdobeLocationPtr & a, const AdobeLocationPtr & b )
{
    int ret = a->getData()->compare( b->getData() );
    return ret >= 0;
}

bool operator == ( const AdobeLocationPtr & a, const AdobeLocationPtr & b )
{
    if ( a->getData() == b->getData() )
    {
        return true;
    }

    int ret = a->getData()->compare( b->getData() );
    return ret == 0;
}

bool operator < ( const AdobeLocationPtr & a, const AdobeLocationPtr & b )
{
    int ret = a->getData()->compare( b->getData() );
    return ret < 0;
}

bool operator > ( const AdobeLocationPtr & a, const AdobeLocationPtr & b )
{
    int ret = a->getData()->compare( b->getData() );
    return ret > 0;
}

AdobeContentIterator::AdobeContentIterator(dpdoc::ContentIterator *iter)
: iter_(iter)
{
}

AdobeContentIterator::~AdobeContentIterator()
{
    if (iter_ != 0)
    {
        iter_->release();
    }
}

QString AdobeContentIterator::next(unsigned int flags)
{
    dp::String result = iter_->next(flags);
    return QString(result.utf8());
}

QString AdobeContentIterator::previous(unsigned int flags)
{
    dp::String result = iter_->previous(flags);
    return QString(result.utf8());
}

AdobeContentIterator & AdobeContentIterator::operator = ( const AdobeContentIterator & right )
{
    iter_ = right.iter_;
    return *this;
}

AdobeLocationPtr AdobeContentIterator::getCurrentPosition()
{
    dp::ref<dpdoc::Location> location = iter_->getCurrentPosition();
    AdobeLocationPtr pos(new AdobeLocation(location));
    return pos;
}

AdobeRangeInfo::AdobeRangeInfo(dpdoc::RangeInfo *info)
    : info_(info)
{
}

AdobeRangeInfo::~AdobeRangeInfo()
{
    if (info_ != 0)
    {
        info_->release();
    }
}

bool AdobeRangeInfo::startsBeforeThisScreen()
{
    return info_->startsBeforeThisScreen();
}

bool AdobeRangeInfo::endsBeforeThisScreen()
{
    return info_->endsBeforeThisScreen();
}

bool AdobeRangeInfo::startsAfterThisScreen()
{
    return info_->startsAfterThisScreen();
}

bool AdobeRangeInfo::endsAfterThisScreen()
{
    return info_->endsAfterThisScreen();
}

int AdobeRangeInfo::getBoxCount()
{
    return info_->getBoxCount();
}

void AdobeRangeInfo::getBox( int boxIndex,
                             double * xMin,
                             double * yMin,
                             double * xMax,
                             double * yMax,
                             dpdoc::Matrix * transform )
{
    dpdoc::Rectangle rect;
    info_->getBox(boxIndex, false, &rect);
    *xMin = rect.xMin;
    *yMin = rect.yMin;
    *xMax = rect.xMax;
    *yMax = rect.yMax;
}

}
