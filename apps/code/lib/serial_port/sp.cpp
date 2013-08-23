#ifndef _WINDOWS
#include <stdio.h>
#include <stdlib.h>
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
#endif

#include "sp.h"

SerialPort::SerialPort(const char * device_name)
: fd_(-1)
{
    open(device_name);
}

SerialPort::~SerialPort()
{
    close();
}

bool SerialPort::open(const char* device_name)
{
#ifndef _WINDOWS
    close();
    fd_ = ::open(device_name, O_RDWR | O_NOCTTY);
    if (fd_ < 0)
    {
        qDebug("Could not open device %s in QFile.", device_name);
        return false;
    }

    // setup.
    struct termios opt;
    tcgetattr(fd_, &opt);

    opt.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    opt.c_iflag = IGNPAR;
    opt.c_oflag = 0;
    opt.c_lflag = 0;

    opt.c_cc[VMIN] = 1;
    opt.c_cc[VTIME] = 0;
    tcflush(fd_, TCIFLUSH);
    tcsetattr(fd_, TCSANOW, &opt);

#endif
    return true;
}

bool SerialPort::isOpened()
{
    return (fd_ >= 0);
}

void SerialPort::close()
{
#ifndef _WINDOWS
    if (isOpened())
    {
        ::close(fd_);
        fd_ = -1;
    }
#endif
}

bool SerialPort::waitForReadyRead(int ms)
{
    if (!isOpened())
    {
        return false;
    }
#ifndef _WINDOWS
    fd_set fileSet;
    FD_ZERO(&fileSet);
    FD_SET(fd_, &fileSet);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = ms * 1000;
    int n = select(fd_ + 1, &fileSet, NULL, &fileSet, &timeout);
    if (!n)
    {
        qDebug("select timeout.");
        return false;
    }

    int queue = 0;
    if (n==-1 || ioctl(fd_, FIONREAD, &queue)==-1)
    {
        qDebug("no queue.");
        return false;
    }
#endif
    return true;
}

int SerialPort::write(const QByteArray & data)
{
    if (!isOpened())
    {
        return 0;
    }
#ifndef _WINDOWS
    return ::write(fd_, data.constData(), data.length());
#endif
    return 0;
}

QByteArray SerialPort::read()
{
    QByteArray data;
    if (!isOpened())
    {
        return data;
    }
#ifndef _WINDOWS
    const int size = 64 * 1000;
    char buffer[size] = {0};
    int len = ::read(fd_, buffer, size - 1);
    data.append(buffer);
#endif
    return data;
}

