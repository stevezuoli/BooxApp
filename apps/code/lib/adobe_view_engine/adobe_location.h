#ifndef ADOBE_LOCATION_H_
#define ADOBE_LOCATION_H_

#include "adobe_utils.h"

namespace dpdoc
{
    class Location;
    class RangeInfo;
    class ContentIterator;
    struct Matrix;
};

namespace adobe_view
{

class AdobeLocationPrivate;
class AdobeLocation
{
public:
    AdobeLocation( dpdoc::Location *location );
    ~AdobeLocation();

    QString getBookmark();
    double  getPagePosition();
    dpdoc::Location *getData();

private:
    AdobeLocationPrivate *data_;

    friend bool operator <= ( const shared_ptr<AdobeLocation> & a, const shared_ptr<AdobeLocation> & b );
    friend bool operator >= ( const shared_ptr<AdobeLocation> & a, const shared_ptr<AdobeLocation> & b );
    friend bool operator == ( const shared_ptr<AdobeLocation> & a, const shared_ptr<AdobeLocation> & b );
    friend bool operator < ( const shared_ptr<AdobeLocation> & a, const shared_ptr<AdobeLocation> & b );
    friend bool operator > ( const shared_ptr<AdobeLocation> & a, const shared_ptr<AdobeLocation> & b );
};

typedef shared_ptr<AdobeLocation> AdobeLocationPtr;

class AdobeContentIterator
{
public:
    AdobeContentIterator(dpdoc::ContentIterator *iter);
    ~AdobeContentIterator();

    AdobeContentIterator & operator = ( const AdobeContentIterator & right );

    QString next(unsigned int flags);
    QString previous(unsigned int flags);
    AdobeLocationPtr getCurrentPosition();

private:
    dpdoc::ContentIterator *iter_;
};

typedef shared_ptr<AdobeContentIterator> AdobeContentIteratorPtr;

// Range of an annotation
struct Range
{
    AdobeLocationPtr start;
    AdobeLocationPtr end;

    Range() {}
    Range(AdobeLocationPtr s, AdobeLocationPtr e) : start(s), end(e) { normalize(); }
    Range(const Range & right) : start(right.start), end(right.end) {}
    ~Range() {}

    inline bool isValid() const { return start != 0 && end != 0; }
    inline bool isEmpty() const { return ( start == end ); }
    inline void reset() { start = AdobeLocationPtr(); end = AdobeLocationPtr(); }
    inline void normalize();
};

/// Normalize the annotation range
inline void Range::normalize()
{
    if ( start != 0 && end != 0 && start > end )
    {
        AdobeLocationPtr temp = start;
        start = end;
        end = temp;
    }
}

class AdobeRangeInfo
{
public:
    AdobeRangeInfo(dpdoc::RangeInfo *info);
    ~AdobeRangeInfo();

    bool startsBeforeThisScreen();
    bool endsBeforeThisScreen();
    bool startsAfterThisScreen();
    bool endsAfterThisScreen();
    int getBoxCount();
    void getBox( int boxIndex,
                 double * xMin,
                 double * yMin,
                 double * xMax,
                 double * yMax,
                 dpdoc::Matrix * transform = 0 );
private:
    dpdoc::RangeInfo *info_;
};

};

Q_DECLARE_METATYPE(adobe_view::Range);
Q_DECLARE_METATYPE(adobe_view::AdobeLocationPtr);

#endif
