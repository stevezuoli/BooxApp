#ifndef ONYX_I2C_H_
#define ONYX_I2C_H_


class I2C
{
public:
    I2C(int adapter, int address);
    ~I2C();

public:
    bool writeInt(unsigned char reg, int value);
    bool writeShort(unsigned char reg, unsigned short value);
    bool writeByte(unsigned char reg, unsigned char value);

    bool read(unsigned char reg, unsigned char & value);

private:
    bool isOpened();
    bool open(const char *name);
    void close();

private:
    int adapter_;
    int address_;
    int fd_;

};

#endif
