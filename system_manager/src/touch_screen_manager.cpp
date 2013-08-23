
#include "onyx/base/device.h"
#include "touch_screen_manager.h"
#include <stdio.h>
#include <QtCore/QtCore>

#ifdef BUILD_FOR_ARM
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#endif

static const int TOUCH_GRP_NO = 0;
static const int TOUCH_SLEEP_PIN_NO = 1;
static const int TOUCH_RESET_PIN_NO = 2;

TouchScreenManager::TouchScreenManager()
: ts_fd_(-1)
, has_touch_screen_(true)
, is_waltop_(false)
{
    map();

    has_touch_screen_ = (qgetenv("NO_TOUCH").toInt() != 1);
}

TouchScreenManager::~TouchScreenManager()
{
    unmap();
}

/// Check the device has touch screen or not.
/// If it has the touch screen, this function returns true.
/// Otherwise it returns false.
bool TouchScreenManager::hasTouchScreen()
{
#ifdef _WINDOWS
    return true;
#endif
    return has_touch_screen_;
}

void TouchScreenManager::resetTouchScreen(Gpio &gpio)
{
    // Reset the touch screen.
    gpio.setValue(TOUCH_GRP_NO, TOUCH_RESET_PIN_NO, 0);
    gpio.mySleep(1000 * 200);
    gpio.setValue(TOUCH_GRP_NO, TOUCH_RESET_PIN_NO, 1);

    // Ensure the sleep pin is low.
    gpio.setValue(TOUCH_GRP_NO, TOUCH_SLEEP_PIN_NO, 0);
}

void TouchScreenManager::checkDigitizerType()
{
#ifdef BUILD_FOR_ARM
    is_waltop_ = false;
    unsigned char value = 0x2a;
    int written = write(ts_fd_, &value, sizeof(value));
    if (written < 0)
    {
        printf("Could not write data to digitizer.\n");
        return;
    }

    fd_set fileSet;
    FD_ZERO(&fileSet);
    FD_SET(ts_fd_, &fileSet);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000 * 1000;
    int n = select(ts_fd_ + 1, &fileSet, NULL, &fileSet, &timeout);
    if (!n)
    {
        printf("select timeout.\n");
        return ;
    }

    int queue = 0;
    if (n==-1 || ioctl(ts_fd_, FIONREAD, &queue)==-1)
    {
        printf("no queue.\n");
        return ;
    }
    value = 0;
    read(ts_fd_, &value, sizeof(value));
    if (value > 0)
    {
        is_waltop_ = true;
    }
    printf("value from digitizer %d\n", value);
#endif
}

/// Enable or disalbe the touch screen. When it's disabled, Qt does not
/// receive any message from touch screen.
bool TouchScreenManager::enableTouchScreen(bool enable)
{
    return true;
    if (ts_fd_ == -1)
    {
        printf("Touch screen device can not be opened.");
        return false;
    }
    int written = 0;

#ifdef BUILD_FOR_ARM
    printf("Enable WACOM touch screen  %d !\n", enable);
    if (enable)
    {
        written = write(ts_fd_, "1", 1);
    }
    else
    {
        written = write(ts_fd_, "0", 1);
    }
#endif
    if (written <= 0)
    {
        printf("Could not write data to touch screen device.");
        return false;
    }
    return true;
}

/// Actually need to use reset.
void TouchScreenManager::turnOn(Gpio &gpio)
{
    // Should change GPIO direction to receive keyboard message.
    gpio.setValue(0, 1, 0);
    /*
    gpio.value(0, 1);
    if (!isWaltop())
    {
        enableTouchScreen(true);
    }
    */
}

void TouchScreenManager::turnOff(Gpio &gpio)
{
    /*
    if (!isWaltop())
    {
        enableTouchScreen(false);
    }
    */
    gpio.setValue(0, 1, 1);
}

bool TouchScreenManager::map()
{
#ifdef BUILD_FOR_ARM
    // Try to open touch screen device.
    ts_fd_ = open(TOUCH_SCREEN_DEVICE, O_RDWR);
    if (ts_fd_ < 0)
    {
        has_touch_screen_ = false;
        return false;
    }
    has_touch_screen_ = true;

#endif
    return true;
}

void TouchScreenManager::unmap()
{
#ifdef BUILD_FOR_ARM
    close(ts_fd_);
    ts_fd_ = -1;
#endif
}



