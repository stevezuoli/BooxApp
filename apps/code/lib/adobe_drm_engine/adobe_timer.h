#ifndef ADOBE_TIMER_H_
#define ADOBE_TIMER_H_

#include "adobe_drm_utils.h"

namespace adobe_drm
{

class AdobeTimerPrivate;
class AdobeTimer
{
public:
    AdobeTimer();
    ~AdobeTimer();
    AdobeTimerPrivate* data();

private:
    AdobeTimerPrivate * data_;
};

};

#endif
