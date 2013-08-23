#ifndef SERIAL_PORT_H_
#define SERIAL_PORT_H_

#include <QtCore/QtCore>


class SerialPort
{
public:
    SerialPort(const char * device);
    ~SerialPort();

public:
    bool open(const char * device);
    bool isOpened();
    void close();

    bool waitForReadyRead(int ms);

    int write(const QByteArray & data);
    QByteArray read();

private:
    int fd_;

};


#endif
