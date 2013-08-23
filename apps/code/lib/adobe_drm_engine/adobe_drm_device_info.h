#ifndef ADOBE_DRM_DEVICE_INFO_H_
#define ADOBE_DRM_DEVICE_INFO_H_

#include "adobe_drm_utils.h"

using namespace base;

namespace adobe_drm
{

class AdobeDeviceInfo
{
public:
    AdobeDeviceInfo();
    ~AdobeDeviceInfo();

    const QString & fingerprint();
    QString devieType();
    const QByteArray & activationData();
    inline void setActivationData(const char * data);

private:
    AdobeDeviceInfo(const AdobeDeviceInfo & right);
    bool writeToXml();

private:
    QByteArray               activation_data_;
    QString                  fingerprint_;
};

inline void AdobeDeviceInfo::setActivationData(const char * data)
{
    activation_data_ = data;
}

};

#endif  // ADOBE_DRM_DEVICE_INFO_H_
