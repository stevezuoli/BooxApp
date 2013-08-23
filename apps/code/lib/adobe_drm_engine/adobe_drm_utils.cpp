#include "dp_all.h"
#include "adobe_drm_utils.h"

namespace adobe_drm
{

QString getDeviceSerialNumber()
{
    QByteArray device_serial = qgetenv("DEVICE_SERIAL");
    if (device_serial.isEmpty())
    {
        device_serial = "11111111";
    }
    return device_serial.constData();
}

QString getVersionInfo(const char* name)
{
    dp::String info = dp::getVersionInfo(name);
    return info.utf8();
}

}
