
#include "onyx/base/base.h"
#include "sys_keyboard_filter.h"


static const int TIMEOUT = 1000;        ///< TODO, if necessary, we can make it configurable.

SysKeyboardFilter::SysKeyboardFilter()
: is_pressed_(false)
, previous_key_(0)
, enabled_(true)
, enable_standby_(true)
, usb_connected_(false)
, stylus_inserted_(false)
, power_on_(false)
, count_(0)
, timer_(this)
{
    // TODO, need to query the wifi and stylus state.
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

SysKeyboardFilter::~SysKeyboardFilter()
{
}

/// Check the stylus is inserted or not.
bool SysKeyboardFilter::isStylusInserted()
{
    return true;
}

/// Check the wifi switch is on or not.
bool SysKeyboardFilter::isWifiOn()
{
    return true;
}

/// Set or reset the count of keyboard event that will be ignored.
void SysKeyboardFilter::ignoreKeyboardEvent(int count)
{
    count_ = count;
}

/// This function check the key should be passed to qws client or not.
/// When it returns false, the other application can receive the key event.
/// Otherwise, the key is consumed by this function.
bool SysKeyboardFilter::filter(int unicode,
                               int keycode,
                               int modifiers,
                               bool isPress,
                               bool autoRepeat)
{
    // Ignore or not.
    if (count_ > 0 && is_pressed_ && !isPress)
    {
        --count_;
        qDebug("Inore the key.");
        return true;
    }

    // Record the key state.
    is_pressed_ = isPress;

    // By default continue and do not ignore it.
    bool ret = false;

    // Need to record the previous key
    // qDebug("SysKeyboardFilter::filter received unicode %d keycode %d prssed %d" , unicode, keycode, isPress);
    switch (keycode)
    {
    case Qt::Key_F35:
        if (!isPress && isStandbyEnabled())
        {
            emit standbyPressed();
        }
        return true;
    case Qt::Key_Sleep:
        emit shutdownPressed();
        return true;
    case Qt::Key_Print:
        emit printScreenSignal();
        return true;
    case Qt::Key_F21:
        onUSBEvent(isPress);
        return false;
    case Qt::Key_F23:
        emit stylusDistanceChanged(isPress);
        return false;
    case Qt::Key_F24:
        onStylusEvent(isPress);
        return true;
    case Qt::Key_VolumeUp:
        if (!isPress)
        {
            emit volumeUpPressed();
        }
        return false;
    case Qt::Key_VolumeDown:
        if (!isPress)
        {
            emit volumeDownPressed();
        }
        return false;
    case Qt::Key_Stop:
        emit forceQuitPressed();
        return true;
    case Qt::Key_OpenUrl:
        emit onlineServicePressed();
        return true;
    case Qt::Key_F20:
        onPowerSwitchEvent(isPress);
        return true;
    default:
        break;
    }

    if (!isEnabled())
    {
        qDebug("Keyboard is disabled.");
        return true;
    }
    return ret;
}

/// For some hardware reason, when user inserts USB cable, device will receive
/// several keyboard event, so we have to add a filter here by using timer.
void SysKeyboardFilter::onUSBEvent(bool isPress)
{
    usb_connected_ = isPress;
    previous_key_ = Qt::Key_F21;
    timer_.stop();
    timer_.start(700);
}

void SysKeyboardFilter::onStylusEvent(bool isPress)
{
    previous_key_ = Qt::Key_F24;
    stylus_inserted_ = isPress;
    timer_.stop();
    timer_.start(700);
}

void SysKeyboardFilter::onPowerSwitchEvent(bool isPress)
{
    // qDebug("Received 3G power switch changed event. %d\n", isPress);
    previous_key_ = Qt::Key_F20;
    power_on_ = isPress;
    timer_.stop();
    timer_.start(700);
}

/// Called when it's timeout. It's used to detect long press.
void SysKeyboardFilter::onTimeout()
{
    // Check the previous key now to see which singal should emit.
    if (previous_key_ == 0)
    {
        return;
    }

    if (previous_key_ ==  Qt::Key_F21)
    {
        emit usbCableChanged(usb_connected_);
    }
    else if (previous_key_ ==  Qt::Key_F24)
    {
        emit stylusChanged(stylus_inserted_);
    }
    else if (previous_key_ ==  Qt::Key_F20)
    {
        qDebug("emit power switch changed event now %d", power_on_);
        emit powerSwitchChanged(power_on_);
    }
    previous_key_ = 0;
}



