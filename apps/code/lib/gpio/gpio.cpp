

#include <cassert>
#include <cstdio>
#include <error.h>
#ifndef _WINDOWS
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif
#include "gpio.h"

// 0x53f00000 is the physical address.
static unsigned int* const PHY_GPIO_DR[3]      = { reinterpret_cast<unsigned int *>(0x53FCC000),
                                                   reinterpret_cast<unsigned int *>(0x53FD0000),
                                                   reinterpret_cast<unsigned int *>(0x53FA4000) };
static unsigned int* const PHY_GPIO_GDIR[3]    = { reinterpret_cast<unsigned int *>(0x53FCC004),
                                                   reinterpret_cast<unsigned int *>(0x53FD0004),
                                                   reinterpret_cast<unsigned int *>(0x53FA4004) };
static unsigned int* const PHY_GPIO_IMR[3]     = { reinterpret_cast<unsigned int *>(0x53FCC014),
                                                   reinterpret_cast<unsigned int *>(0x53FD0014),
                                                   reinterpret_cast<unsigned int *>(0x53FA4014) };

static unsigned int* const PHY_GPIO_PSR[3]     = { reinterpret_cast<unsigned int *>(0x53FCC008),
                                                   reinterpret_cast<unsigned int *>(0x53FD0008),
                                                   reinterpret_cast<unsigned int *>(0x53FA4008) };

static const size_t MAP_SIZE = 4096;
static const size_t MAP_MASK = MAP_SIZE - 1;
static const int    MUX_CTL_BIT_LEN = 8;

volatile unsigned int * Gpio::GPIO_GDIR[GROUPS];
volatile unsigned int * Gpio::GPIO_DR[GROUPS];
volatile unsigned int * Gpio::GPIO_IMR[GROUPS];
volatile unsigned int * Gpio::GPIO_PSR[GROUPS];
unsigned int * Gpio::map_[GROUPS];

Gpio::Gpio( void )
: fd_(-1)
{
    open();
}

Gpio::~Gpio( void )
{
    close();
}

void Gpio::open( void )
{
#ifndef _WINDOWS
    static const int MAP_PROT = PROT_READ | PROT_WRITE;
    fd_ = ::open( "/dev/mem", O_RDWR | O_SYNC );
    if (fd_ <= 0)
    {
        return;
    }

    // Init address one by one. Not sure which one to use.
    size_t pgoff = 0;
    unsigned int *p = 0;
    for(int i = 0; i < GROUPS; ++i)
    {
        // DR
        pgoff = reinterpret_cast<int>(PHY_GPIO_DR[i]) &  ~MAP_MASK;
        p = reinterpret_cast<unsigned int *>(mmap( 0, MAP_SIZE, MAP_PROT, MAP_SHARED, fd_, pgoff ));
        assert ( p != reinterpret_cast<void *>(-1) );
        GPIO_DR[i] = p + ((reinterpret_cast<int>(PHY_GPIO_DR[i]) & MAP_MASK) / 4);

        // GDIR
        GPIO_GDIR[i] = p + ((reinterpret_cast<int>(PHY_GPIO_GDIR[i]) & MAP_MASK) / 4);

        // IMR
        GPIO_IMR[i] = p + ((reinterpret_cast<int>(PHY_GPIO_IMR[i]) & MAP_MASK) / 4);

        // PSR
        GPIO_PSR[i] =  p + ((reinterpret_cast<int>(PHY_GPIO_PSR[i]) & MAP_MASK) / 4);
        map_[i] = p;
    }
#endif
}

void Gpio::dump()
{
    for(int i = 0; i < GROUPS; ++i)
    {
        printf("GPIO_DR\t%p\n", GPIO_DR[i]);
        printf("GPIO_GDIR\t%p\n", GPIO_GDIR[i]);
        printf("GPIO_IMR\t%p\n", GPIO_IMR[i]);
        printf("GPIO_PSR\t%p\n", GPIO_PSR[i]);
    }
}

void Gpio::close( void )
{
#ifndef _WINDOWS
    for(int i = 0; i < GROUPS; ++i)
    {
        if (map_[i])
        {
            munmap( map_[i], MAP_SIZE );
            map_[i] = 0;
        }
    }
    ::close(fd_);
    fd_ = 0;
#endif
}

/// This function sets the I/O direction of the pin pin to the value.
/// 0 for input and 1 for output.
bool Gpio::setDir(int group, int pin, int val)
{
    if (GPIO_GDIR[group] == 0)
    {
        return false;
    }

    if( val & 0x1)
    {
        *GPIO_GDIR[group] |= (1<<pin);
    }
    else
    {
        *GPIO_GDIR[group] &= ~(1<<pin);
    }
    return true;

    // Don't change imr register. Sometimes, if we changed this
    // we can not receive interrupt from kernel.
    // But I'm not sure the others can work correctly without touching
    // imr register. TODO: need more test.
    // *GPIO_IMR[group] &= ~(1<<pin);
}

/// Need to confirm.
int Gpio::dir(int group, int pin)
{
    if (GPIO_GDIR[group] == 0)
    {
        return -1;
    }

    return ((*GPIO_GDIR[group] >> pin) & 0x1);
}

void Gpio::setValue(int group, int pin, int value)
{
    if (setDir(group, pin, 1))
    {
        *GPIO_DR[group] = *GPIO_DR[group] & ~(1 << pin) | (value << pin);
    }
}

int Gpio::value(int group, int pin)
{
    if (setDir(group, pin, 0))
    {
        return ((*GPIO_DR[group] >> pin) & 1);
    }
    return -1;
}

void Gpio::mySleep(int usec)
{
#ifndef _WINDOWS
    ::usleep(usec);
#endif
}

void Gpio::remap()
{
    close();
    open();
}

// end of file
