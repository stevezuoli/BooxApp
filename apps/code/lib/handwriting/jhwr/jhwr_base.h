#ifndef JHWR_BASE_H_
#define JHWR_BASE_H_

#include <QtGui/QtGui>
#include "onyx/data/handwriting_interface.h"

namespace handwriting
{

void getSplitPoint(short *stroke_buffer,int start, int size, short * points, int max_size);

class jHWRBase
{
public:
    jHWRBase();
    virtual ~jHWRBase();

public:
    virtual bool initialize(const QLocale & locale) = 0;
    virtual bool deinitialize() = 0;
    virtual bool setWorkArea(const QRect & rect) = 0;
    virtual bool setCandidateNum(const int number) = 0;
    virtual bool setLocale(const QLocale & locale) = 0;
    virtual bool setSpecialRecognizeRange(SpecialRecognizeRange range) = 0;

    virtual void clearPoints();
    virtual void collectPoint(int x, int y);
    virtual void finishStroke();
    virtual void finishCharacter();
    virtual bool recognize(QStringList & results) = 0;

protected:
    std::vector<short>    raw_points_;
    int                   candidates_num_;
    unsigned char         *address_;
};

};
#endif
