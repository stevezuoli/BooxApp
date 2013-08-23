#include "dp_all.h"
#include "adobe_drm_device_info.h"

namespace adobe_drm
{

static const QString DEV_DIR = ".adobe-digital-editions";

AdobeDeviceInfo::AdobeDeviceInfo()
{
}

AdobeDeviceInfo::~AdobeDeviceInfo()
{
}

const QString & AdobeDeviceInfo::fingerprint()
{
    if (fingerprint_.isEmpty())
    {
        dpdev::DeviceProvider *device_provider = dpdev::DeviceProvider::getProvider(0);
        if (device_provider == 0)
        {
            qWarning("No Device Provider Implementation");
            return fingerprint_;
        }

        dpdev::Device *device = device_provider->getDevice(0);
        if( device == 0 )
        {
            qWarning("No Device Implementation");
            return fingerprint_;
        }

        dp::Data fingerprint = device->getFingerprint();
        dp::String fingerprint_str = dp::String::base64Encode(fingerprint);
        fingerprint_ = fingerprint_str.utf8();
        qDebug("Finger Print:%s", fingerprint_.toUtf8().data());
        writeToXml();
    }

    return fingerprint_;
}

QString AdobeDeviceInfo::devieType()
{
    QByteArray device_type = qgetenv("ADEPT_DEVICE_TYPE");
    if (device_type.isEmpty())
    {
        device_type = "mobile";
    }
    return device_type.constData();
}

const QByteArray & AdobeDeviceInfo::activationData()
{
    if (activation_data_.isEmpty())
    {
        QString path;
#ifdef Q_WS_QWS
        path = ("/media/flash");
#else
        path = QDir::home().path();
#endif
        QDir dir(path);
        if (dir.cd(DEV_DIR))
        {
            path = dir.filePath("activation.xml");
            activation_data_.clear();
            QFile file(path);
            if (file.open(QFile::ReadOnly | QFile::Text))
            {
                activation_data_ = file.readAll();
                activation_data_.append('\0');
            }
        }
    }
    return activation_data_;
}

bool AdobeDeviceInfo::writeToXml()
{
    QString path;
#ifdef Q_WS_QWS
    path = ("/media/flash");
#else
    path = QDir::home().path();
#endif
    QDir dir(path);
    if (!dir.exists(DEV_DIR))
    {
        if (!dir.mkdir(DEV_DIR))
        {
            return false;
        }
    }

    if (dir.cd(DEV_DIR))
    {
        // Change folder attribute.
        changeToHidden(dir.absolutePath().toLocal8Bit().constData());

        path = dir.filePath("device.xml");
        QFile file(path);

        if (!file.open(QFile::WriteOnly | QFile::Text))
        {
            return false;
        }

        QXmlStreamWriter writer(&file);

        // start document
        //writer.writeStartDocument();

        // device info
        writer.writeStartElement("deviceInfo");
        writer.writeAttribute("xmlns", "http://ns.adobe.com/adept");

        // device class
        writer.writeTextElement("deviceClass", "Onyx BOOX Series");

        // device serial number
        writer.writeTextElement("deviceSerial", getDeviceSerialNumber());

        // device name
        QByteArray device_name = qgetenv("ADEPT_DEVICE_NAME");
        if (device_name.isEmpty())
        {
            device_name = "OnyxBoox";
        }
        writer.writeTextElement("deviceName", device_name.constData());

        // devie type
        QByteArray device_type = qgetenv("ADEPT_DEVICE_TYPE");
        if (device_type.isEmpty())
        {
            device_type = SysStatus::instance().hasTouchScreen() ? "mobile" : "tethered";
        }
        writer.writeTextElement("deviceType", device_type);

        // version
        QString major = getVersionInfo("hobbes.major");
        QString minor = getVersionInfo("hobbes.minor");
        QString build = getVersionInfo("hobbes.build");
        QString version("%1.%2.%3");
        version = version.arg(major).arg(minor).arg(build);
        writer.writeStartElement("version");
        writer.writeAttribute("name", "hobbes");
        writer.writeAttribute("value", version);
        writer.writeEndElement();

        // finger print
        writer.writeTextElement("fingerprint", fingerprint_.toUtf8().data());

        // end document
        writer.writeEndDocument();
        return true;
    }
    return false;
}

}
