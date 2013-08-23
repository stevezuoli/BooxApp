
#include "i2c/i2c.h"
#include <QtCore/QtCore>

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage i2c_man adapter address read/write reg value");
        return 0;
    }

    int adapter = QString::fromLocal8Bit(argv[1]).toInt();
    int address = QString::fromLocal8Bit(argv[2]).toInt();
    QString type = QString::fromLocal8Bit(argv[3]);
    int reg = QString::fromLocal8Bit(argv[4]).toInt();
    unsigned char value = 0xff;
    if (type.compare("write", Qt::CaseInsensitive) == 0)
    {
        value = QString::fromLocal8Bit(argv[5]).toInt();
    }

    I2C i2c(adapter, address);

    if (type == "read")
    {
        for(int r = 0; r < 0xff; ++r)
        {
            i2c.read(r, value);
            qDebug("value from %d: %d", r, value);
        }
    }
    else
    {
        i2c.writeInt(reg, value);
    }
    return 0;
}
