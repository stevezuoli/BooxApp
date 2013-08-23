
#include <stdio.h>
#include <stdlib.h>

#ifndef _WINDOWS
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include "i2c-dev.h"
#endif


#include "i2c.h"

#define I2C_RETRIES    0x0701
#define I2C_TIMEOUT    0x0702
#define I2C_SLAVE      0x0703

// http://blog.chinaunix.net/u1/57747/showart_1355953.html

I2C::I2C(int adapter, int address)
: adapter_(adapter)
, address_(address)
, fd_(-1)
{
    char name[100] = {0};
    sprintf(name, "/dev/i2c-%d", adapter);
    open(name);
}

I2C::~I2C()
{
    close();
}

bool I2C::open(const char *name)
{
#ifndef _WINDOWS
    fd_ = ::open(name, O_RDWR);
    if (fd_ < 0)
    {
        printf("Could not open device %s.\n", name);
        return false;
    }
    printf("Open i2c device %s done address %d.\n", name, address_);
    // Have to use I2C_SLAVE_FORCE in order to make kernel ioctl work.
    // The address is the chipset address
    int res = ioctl(fd_, I2C_SLAVE_FORCE, address_);
    if (res < 0)
    {
        close();
        printf("Error occured inside IOCTL %d.\n", res);
        return false;
    }
/*
    res = ioctl(fd_, I2C_PEC, 1);
    if (res < 0)
    {
        printf("Error occured inside IOCTL with I2C_PEC %d.\n", res);
        return false;
    }
*/
#endif
    return true;
}

void I2C::close()
{
#ifndef _WINDOWS
    ::close(fd_);
    fd_ = -1;
    printf("closed\n");
#endif
}

bool I2C::isOpened()
{
    return (fd_ >= 0);
}

bool I2C::writeInt(unsigned char reg, int value)
{
    if (!isOpened())
    {
        return false;
    }

#ifndef _WINDOWS
    i2c_smbus_write_byte_data(fd_, reg, value);
#endif
    return true;
}

bool I2C::writeShort(unsigned char reg, unsigned short value)
{
    if (!isOpened())
    {
        return false;
    }

#ifndef _WINDOWS
    ::write(fd_, &reg, sizeof(reg));
    ::write(fd_, &value, sizeof(value));
#endif
    return true;
}

bool I2C::writeByte(unsigned char reg, unsigned char value)
{
    if (!isOpened())
    {
        return false;
    }

#ifndef _WINDOWS
    i2c_smbus_write_byte_data(fd_, reg, value);
#endif
    return true;
}

bool I2C::read(unsigned char reg, unsigned char & value)
{
    if (!isOpened())
    {
        return false;
    }

#ifndef _WINDOWS
    value = i2c_smbus_read_byte_data(fd_, reg);

#endif
    return true;
}
