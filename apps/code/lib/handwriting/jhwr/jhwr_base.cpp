#include "jhwr_base.h"

namespace handwriting
{

void getSplitPoint(short *stroke_buffer,int start, int size, short * points, int max_size)
{
    int i = 0;
    for(i = 0; i < size; i++)
    {
        points[i] = stroke_buffer[i + start];
        if(i == max_size - 2)
        {
            break;
        }
    }
    points[i]   = -1;
    points[i + 1] = -1;
    return;
}

jHWRBase::jHWRBase()
: address_(0)
, candidates_num_(10)
{
}

jHWRBase::~jHWRBase()
{
}

void jHWRBase::clearPoints()
{
    raw_points_.clear();
}

void jHWRBase::collectPoint(int x, int y)
{
    qDebug("Collect Point(%d, %d)", x, y);
    raw_points_.push_back(x);
    raw_points_.push_back(y);
}

void jHWRBase::finishStroke()
{
    if (!raw_points_.empty())
    {
        qDebug("Finish stroke");
        raw_points_.push_back(-1);
        raw_points_.push_back(0);
    }
}

void jHWRBase::finishCharacter()
{
    if (!raw_points_.empty())
    {
        qDebug("Finish character");
        raw_points_.push_back(-1);
        raw_points_.push_back(-1);
    }
}

}
