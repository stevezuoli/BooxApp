/// gpio.h onyx internation inc.

#ifndef __NABOO_GPIO_FREESCALE_H__
#define __NABOO_GPIO_FREESCALE_H__

#include <cassert>

/// GPIO class.
class Gpio
{
public:
    Gpio( void );
    ~Gpio( void );

public:
    void setValue(int group, int pin, int value);
    int  value(int group, int pin);
    static void mySleep(int usec);

    void remap();

private:
    void open();
    void close();
    void dump();

    bool setDir(int group, int pin, int val );
    int dir(int group, int pin);

private:
    int fd_;                        ///< The memory device.
    static const int GROUPS = 3;
    static volatile unsigned int * GPIO_GDIR[GROUPS];
    static volatile unsigned int * GPIO_DR[GROUPS];
    static volatile unsigned int * GPIO_IMR[GROUPS];
    static volatile unsigned int * GPIO_PSR[GROUPS];
    static unsigned int * map_[GROUPS];
};

#endif // __NABOO_GPIO_FREESCALE_H__

// end of file
