

#include <cassert>
#include <cstdio>
#include <error.h>
#ifndef _WINDOWS
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif
#include "marvell_gpio.h"


// 0xD4019000 is the physical address.
static unsigned int* const PHY_GPIO[MarvellGpio::GROUPS]      = { reinterpret_cast<unsigned int *>(0xD4019000),
                                                                  reinterpret_cast<unsigned int *>(0xD4019004),
                                                                  reinterpret_cast<unsigned int *>(0xD4019008),
                                                                  reinterpret_cast<unsigned int *>(0xD4019100)};

static unsigned int* const PHY_GPDR[MarvellGpio::GROUPS]    = { reinterpret_cast<unsigned int *>(0xD4019000 + 0x0c),
                                                                reinterpret_cast<unsigned int *>(0xD4019004 + 0x0c),
                                                                reinterpret_cast<unsigned int *>(0xD4019008 + 0x0c),
                                                                reinterpret_cast<unsigned int *>(0xD4019100 + 0x0c)};

static unsigned int* const PHY_GPSR[MarvellGpio::GROUPS]    = { reinterpret_cast<unsigned int *>(0xD4019000 + 0x18),
                                                                reinterpret_cast<unsigned int *>(0xD4019004 + 0x18),
                                                                reinterpret_cast<unsigned int *>(0xD4019008 + 0x18),
                                                                reinterpret_cast<unsigned int *>(0xD4019100 + 0x18)};

static unsigned int* const PHY_GPCR[MarvellGpio::GROUPS]    = { reinterpret_cast<unsigned int *>(0xD4019000 + 0x24),
                                                                reinterpret_cast<unsigned int *>(0xD4019004 + 0x24),
                                                                reinterpret_cast<unsigned int *>(0xD4019008 + 0x24),
                                                                reinterpret_cast<unsigned int *>(0xD4019100 + 0x24)};

// 0xD401E000

static const int COUNT = 32;
static const size_t MAP_SIZE = 4096;
static const size_t MAP_MASK = MAP_SIZE - 1;
static const int    MUX_CTL_BIT_LEN = 8;

volatile unsigned int * MarvellGpio::DATA[GROUPS];
volatile unsigned int * MarvellGpio::GPDR[GROUPS];
volatile unsigned int * MarvellGpio::GPSR[GROUPS];
volatile unsigned int * MarvellGpio::GPCR[GROUPS];
unsigned int * MarvellGpio::map_[GROUPS];

MarvellGpio::MarvellGpio( void )
: fd_(-1)
{
    open();
}

MarvellGpio::~MarvellGpio( void )
{
    close();
}

void MarvellGpio::open( void )
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
        // GPDR
        pgoff = reinterpret_cast<int>(PHY_GPIO[i]) &  ~MAP_MASK;
        p = reinterpret_cast<unsigned int *>(mmap( 0, MAP_SIZE, MAP_PROT, MAP_SHARED, fd_, pgoff ));
        assert ( p != reinterpret_cast<void *>(-1) );
        GPDR[i] = p + ((reinterpret_cast<int>(PHY_GPDR[i]) & MAP_MASK) / 4);

        // GPSR
        GPSR[i] = p + ((reinterpret_cast<int>(PHY_GPSR[i]) & MAP_MASK) / 4);

        // GPCR
        GPCR[i] = p + ((reinterpret_cast<int>(PHY_GPCR[i]) & MAP_MASK) / 4);

        // DATA
        DATA[i] = p + ((reinterpret_cast<int>(PHY_GPIO[i]) & MAP_MASK) / 4);
        map_[i] = p;
    }
#endif
}

void MarvellGpio::dump()
{
    for(int i = 0; i < GROUPS; ++i)
    {
        printf("Base\t%p\n", DATA[i]);
        printf("GPDR\t%p\n", GPDR[i]);
        printf("GPSR\t%p\n", GPSR[i]);
        printf("GPCR\t%p\n", GPCR[i]);
    }
}

void MarvellGpio::close( void )
{
#ifndef _WINDOWS
    for(int i = 0; i < GROUPS; ++i)
    {
        if (map_[i])
        {
            munmap(map_[i], MAP_SIZE );
            map_[i] = 0;
        }
    }
    ::close(fd_);
    fd_ = 0;
#endif
}

/// This function sets the I/O direction of the pin pin to the value.
/// 0 for input and 1 for output.
bool MarvellGpio::setDir(int group, int pin, int val)
{
    if (GPDR[group] == 0)
    {
        return false;
    }

    if( val & 0x1)
    {
        *GPDR[group] |= (1<<pin);
    }
    else
    {
        *GPDR[group] &= ~(1<<pin);
    }
    return true;
}

/// Need to confirm.
int MarvellGpio::dir(int group, int pin)
{
    if (GPDR[group] == 0)
    {
        return -1;
    }

    return ((*GPDR[group] >> pin) & 0x1);
}

void MarvellGpio::setValue(int group, int pin, int value)
{
    if (setDir(group, pin, 1))
    {
        if (value > 0)
        {
            *GPSR[group] = *GPSR[group] | (1 << pin);
        }
        else
        {
            *GPCR[group] = *GPCR[group] | (1 << pin);
        }
    }
}

int MarvellGpio::value(int group, int pin)
{
    if (DATA[group])
    {
        return ((*DATA[group] >> pin) & 1);
    }
    return -1;
}

void MarvellGpio::setValue(int absolute_pin, int value)
{
    setValue(absolute_pin / COUNT, absolute_pin % COUNT, value);
}

int MarvellGpio::value(int absolute_pin)
{
    return value(absolute_pin / COUNT, absolute_pin % COUNT);
}

void MarvellGpio::mySleep(int usec)
{
#ifndef _WINDOWS
    ::usleep(usec);
#endif
}

void MarvellGpio::remap()
{
    close();
    open();
}

// end of file
