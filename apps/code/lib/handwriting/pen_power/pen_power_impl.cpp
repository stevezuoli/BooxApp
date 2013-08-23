
#include "pen_power_impl.h"

namespace handwriting
{

PenPowerPlugin::PenPowerPlugin()
{
}

PenPowerPlugin::~PenPowerPlugin()
{
}

bool PenPowerPlugin::initialize(const QLocale & locale)
{
    hwr_data_.pPrivate = reinterpret_cast<signed char *>(&work_buf_[0]);
    if (PPHWRInit(&hwr_data_) != STATUS_OK)
    {
        qWarning("Could not init library.");
        return false;
    }

    PPHWRSetType(&hwr_data_,static_cast<DWORD>(ALL_TYPE));
    return true;
}

bool PenPowerPlugin::setWorkArea(const QRect & rect)
{
    HWRBOX box;
    box.left = rect.left();
    box.top = rect.top();
    box.right = rect.right();
    box.bottom = rect.bottom();
    return (PPHWRSetBox(&hwr_data_, &box) == STATUS_OK);
}

bool PenPowerPlugin::setCandidateNum(const int number)
{
    code_array_.clear();
    code_array_.resize(number);
    return (PPHWRSetCandidateNum(&hwr_data_, number) == STATUS_OK);
}

void PenPowerPlugin::clearPoints()
{
    raw_points_.clear();
}

void PenPowerPlugin::collectPoint(int x, int y)
{
    POINT_TYPE p;
    p.x = static_cast<short>(x);
    p.y = static_cast<short>(y);
    raw_points_.push_back(p);
}

void PenPowerPlugin::finishStroke()
{
    // Not be used here
}

void PenPowerPlugin::finishCharacter()
{
    // Not be used here
}

bool PenPowerPlugin::recognize(QStringList & result)
{
    result.clear();
    if(PPHWRRecognize(&hwr_data_, reinterpret_cast<WORD *>(&raw_points_[0]), &code_array_[0]) != STATUS_OK)
    {
        qWarning("Could not recognize the data.");
        return false;
    }

    // Need to conver.
    int no_cand = 0;
    while (code_array_[no_cand])
    {
        // TODO.
        result.push_back(QChar(code_array_[no_cand]));
        no_cand++;
    }
    return true;
}

};