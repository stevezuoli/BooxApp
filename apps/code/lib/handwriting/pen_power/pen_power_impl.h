#ifndef PEN_POWER_PLUGIN_IMPL_H_
#define PEN_POWER_PLUGIN_IMPL_H_

#define CPP_SOURCE
#include "HWR_API.H"


#include "../handwriting_interface.h"

namespace handwriting
{

class PenPowerPlugin : public HandwritingInterface
{
    Q_OBJECT
public:
    PenPowerPlugin();
    virtual ~PenPowerPlugin();

public:
    virtual bool initialize(const QLocale & locale);
    virtual bool setWorkArea(const QRect & rect);
    virtual bool setCandidateNum(const int number);

    virtual void clearPoints();
    virtual void collectPoint(int x, int y);
    virtual void finishStroke();
    virtual void finishCharacter();
    virtual bool recognize(QStringList & results);

private:
    HWRData hwr_data_;
    std::vector<POINT_TYPE> raw_points_;
    std::vector<unsigned short> code_array_;
    unsigned char work_buf_[8192];

};

};  // namespace handwriting

#endif

