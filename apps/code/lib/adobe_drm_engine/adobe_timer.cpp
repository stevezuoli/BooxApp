#include "dp_all.h"
#include "adobe_timer.h"

namespace adobe_drm
{

class AdobeTimerPrivate : public dptimer::Timer, QTimer
{
public:
    virtual void release();
    virtual void setTimeout( int millisec );
    virtual void cancel();
};

void AdobeTimerPrivate::release()
{
}

void AdobeTimerPrivate::setTimeout( int millisec )
{
    setInterval(millisec);
}

void AdobeTimerPrivate::cancel()
{
    stop();
    setInterval(0);
}

AdobeTimer::AdobeTimer()
    : data_(new AdobeTimerPrivate())
{
}

AdobeTimer::~AdobeTimer()
{
    data_->release();
}

AdobeTimerPrivate* AdobeTimer::data()
{
    return data_;
}

}
