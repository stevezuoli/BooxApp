/// marvell_gpio.h onyx internation inc.

#ifndef __ONYX_GPIO_MARVELL_H__
#define __ONYX_GPIO_MARVELL_H__

#include <cassert>

/// GPIO class for marvell platform.
class MarvellGpio
{
public:
    MarvellGpio( void );
    ~MarvellGpio( void );

public:
    void setValue(int group, int pin, int value);
    int  value(int group, int pin);

    void setValue(int absolute_pin, int value);
    int  value(int absolute_pin);

    bool setDir(int group, int pin, int val );
    int dir(int group, int pin);

    static void mySleep(int usec);

    void remap();

public:
    static const int GROUPS = 4;

private:
    void open();
    void close();
    void dump();



private:
    int fd_;                        ///< The memory device.

    static volatile unsigned int * DATA[GROUPS];
    static volatile unsigned int * GPDR[GROUPS];
    static volatile unsigned int * GPSR[GROUPS];
    static volatile unsigned int * GPCR[GROUPS];
    static unsigned int * map_[GROUPS];
};

#endif // __ONYX_GPIO_MARVELL_H__

// end of file
