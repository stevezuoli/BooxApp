#ifndef TOUCH_SCREEN_MANAGER_H_
#define TOUCH_SCREEN_MANAGER_H_

#include "gpio/gpio.h"

/// Touch screen manager.
class TouchScreenManager
{
public:
    static TouchScreenManager & instance()
    {
        static TouchScreenManager instance_;
        return instance_;
    }
    ~TouchScreenManager();

public:
    bool hasTouchScreen();
    bool enableTouchScreen(bool);

    void turnOn(Gpio &gpio);
    void turnOff(Gpio &gpio);
    void resetTouchScreen(Gpio &gpio);

    void checkDigitizerType();

private:
    TouchScreenManager();
    TouchScreenManager(const TouchScreenManager &){}

private:
    bool map();
    void unmap();

    bool isWaltop() { return is_waltop_; }

private:
    int ts_fd_;         ///< Touch screen device handle.
    bool has_touch_screen_;
    bool is_waltop_;
};

#endif // POWER_MANAGER_H_
