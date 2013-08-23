

#include <stdlib.h>
#include "sys_conf.h"
#include "dict_conf.h"
#include "pm_conf.h"
#include "volume_conf.h"
#include "page_turning_conf.h"
#include "onyx/base/device.h"

#ifdef BUILD_FOR_ARM
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace sys
{

/// AKA SysConfig stored in hidden flash sector.
struct DeviceConfig{
    char version[16];              // Software version including build time.
    unsigned int program_time;      // Since 1970.
    char serial_no[16];             // Serial number of device.
    char dev_id[16];                // Device id.
    unsigned int screen_size;       // Screen size 60, 80, 97
    unsigned int x_pixels;
    unsigned int y_pixels;
    unsigned int grayscale;
};

SystemConfig::SystemConfig()
{
    open();

    DictConfig::makeSureTableExist(*database_);
    PMConfig::makeSureTableExist(*database_);
    LocaleConfig::makeSureTableExist(*database_);
    ServiceConfig::makeSureTableExist(*database_);
    VolumeConfig::makeSureTableExist(*database_);
    WifiConfig::makeSureTableExist(*database_);
    DialupConfig::makeSureTableExist(*database_);
    PageTurningConfig::makeSureTableExist(*database_);
    UserConfig::makeSureTableExist(*database_);
}

SystemConfig::~SystemConfig()
{
    close();
}

void SystemConfig::loadAllServices(Services &services)
{
    ServiceConfig::loadAllServices(*database_, services);
}

/// Retrieve the calibration service.
bool SystemConfig::calibrationService(Service & service)
{
    return ServiceConfig::calibrationService(*database_, service);
}

bool SystemConfig::musicService(Service & service)
{
    return ServiceConfig::musicService(*database_, service);
}

bool SystemConfig::networkService(Service &service)
{
    return ServiceConfig::networkService(*database_, service);
}

bool SystemConfig::webBrowserService(Service &service)
{
    return ServiceConfig::webBrowserService(*database_, service);
}

bool SystemConfig::registerService(const Service &service,
                                   const QString &path)
{
    return ServiceConfig::registerService(*database_, service, path);
}

bool SystemConfig::unRegisterService(const Service &service)
{
    return ServiceConfig::unRegisterService(*database_, service);
}

/// The database is in home directory.
bool SystemConfig::open()
{
    if (!database_)
    {
        database_.reset(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "system_config")));
        database_->setDatabaseName(QDir::home().filePath("system_config.db"));
    }
    return database_->open();
}

bool SystemConfig::close()
{
    if (database_)
    {
        database_->close();
        database_.reset(0);
        QSqlDatabase::removeDatabase("system_config");
        return true;
    }
    return false;
}

// Locale
QLocale SystemConfig::locale()
{
    return LocaleConfig::locale(*database_);
}

bool SystemConfig::setLocale(const QLocale & locale)
{
    return LocaleConfig::setLocale(*database_, locale);
}

bool SystemConfig::dictionaryRoots(QStringList & dirs)
{
    return DictConfig::dictionaryRoots(*database_, dirs);
}

/// Read the dictionary name that user selected.
QString SystemConfig::selectedDictionary()
{
    return DictConfig::selectedDictionary(*database_);
}

/// Save the dictionary name as the selected.
bool SystemConfig::selectDictionary(const QString & name)
{
    return DictConfig::selectDictionary(*database_, name);
}

static const QString ZONE_PREFIX = "/usr/share/zoneinfo/";
/// See /usr/share/zoneinfo for the list of valid names.
/// The name should contain all relative path like Asia/Chongqing.
bool SystemConfig::setTimezone(const QString & name)
{
    QString format("%1%2");
    format = format.arg(ZONE_PREFIX).arg(name);

    if (QFile::exists(format))
    {
        QStringList args;
        args << format;
        QProcess::startDetached("update_timezone.sh", args);
        return true;
    }
    return false;
}

QString SystemConfig::currentTimezone()
{
    QFileInfo info("/etc/localtime");
    if (info.exists() && info.isSymLink())
    {
        QString path = info.symLinkTarget();
        return path.remove(ZONE_PREFIX);
    }
    return QString();
}

/// Change system date.
/// The steps should be:
/// - Change system date by using date -s 2009.04.27-12:10:20
/// - Change the hardware clock by using hwclock -w
void SystemConfig::setDate(int year, int month, int day,
                           int hour, int minute, int second)
{
    QString format("%1.%2.%3-%4:%5:%6");
    format = format.arg(year).arg(month).arg(day).arg(hour).arg(minute).arg(second);

    QStringList args;
    args << format;
    QProcess::startDetached("update_date.sh", args);
}

bool SystemConfig::isMusicPlayerAvailable()
{
    return QFile::exists("/opt/onyx/arm/bin/music_player");
}

bool SystemConfig::isUpdateSplashEnabled()
{
    return QFile::exists("/opt/onyx/arm/bin/update_splash");
}

bool SystemConfig::hasHomePage()
{
#ifdef ELISA_SUPPORT
    return true;
#endif
    return false;
}

const QString & SystemConfig::homePageName()
{
    static QString name;
#ifdef ELISA_SUPPORT
    name = "Elisa";
#endif
    return name;
}

bool SystemConfig::userInfo(UserInfo & data)
{
    return UserConfig::info(*database_, data);
}

bool SystemConfig::updateUserInfo(const UserInfo & data)
{
    return UserConfig::update(*database_, data);
}

/// Get volume. We always read the volume from database.
/// It's necessary as this function is called after startuped.
/// At that time, the volume from hardware is undefined.
int SystemConfig::volume()
{
    return VolumeConfig::volume(*database_);
}

bool SystemConfig::setVolume(const int v)
{
    return VolumeConfig::setVolume(*database_, v);
}

bool SystemConfig::mute(bool m)
{
    return VolumeConfig::mute(*database_, m);
}

bool SystemConfig::isMute()
{
    return VolumeConfig::isMute(*database_);
}

int SystemConfig::suspendInterval()
{
    return PMConfig::suspendInterval(*database_);
}

bool SystemConfig::setSuspendInterval(int ms)
{
    return PMConfig::setSuspendInterval(*database_, ms);
}

int SystemConfig::shutdownInterval()
{
    return PMConfig::shutdownInterval(*database_);
}

bool SystemConfig::setShutdownInterval(int ms)
{
    return PMConfig::setShutdownInterval(*database_, ms);
}

bool SystemConfig::clearWifiProfiles()
{
    return WifiConfig::clear(*database_);
}

bool SystemConfig::loadWifiProfiles(WifiProfiles & all)
{
    return WifiConfig::load(*database_, all);
}

bool SystemConfig::saveWifiProfiles(WifiProfiles & all)
{
    return WifiConfig::save(*database_, all);
}

bool SystemConfig::clearDialupProfiles()
{
    return DialupConfig::clear(*database_);
}

bool SystemConfig::loadDialupProfiles(DialupProfiles & all)
{
    return DialupConfig::load(*database_, all);
}

bool SystemConfig::saveDialupProfiles(DialupProfiles & all)
{
    return DialupConfig::save(*database_, all);
}

/// This function returns 1 for next page. It returns -1 for previous page.
/// If the distance is too small, it returns 0.
/// Caller can the direction by using setDirection.
int SystemConfig::direction(const QPoint & old_position, const QPoint & new_position)
{
    return PageTurningConfig::direction(old_position, new_position);
}

void SystemConfig::setDirection(int conf)
{
    PageTurningConfig::setDirection(conf);
}

int SystemConfig::distance()
{
    return PageTurningConfig::distance();
}

/// Return the default download folder. Ensure the folder exist.
QString SystemConfig::downloadFolder()
{
#ifdef Q_WS_QWS
    QString path = (SDMMC_ROOT);
#else
    QString path = QDir::home().path();
#endif
    QDir dir(path);
    QString ret;
    if (!dir.exists())
    {
        return ret;
    }

    static const QString FOLDER_NAME = "downloads";
    if (!dir.exists(FOLDER_NAME))
    {
        if (!dir.mkdir(FOLDER_NAME))
        {
            return ret;
        }
    }

    if (dir.cd(FOLDER_NAME))
    {
        ret = dir.absolutePath();
    }
    return ret;
}

static const DeviceConfig * readFlash()
{
    static QByteArray data;
    if (data.size() > 0)
    {
        return reinterpret_cast<const DeviceConfig *>(data.constData());
    }

#ifdef BUILD_FOR_ARM
    int fd = open("/dev/mtd1", O_RDONLY);
    if (fd == 0)
    {
        printf("Could not open device.\n");
        return 0;
    }

    static const int SIZE = 2048;
    data.resize(SIZE);
    pread(fd, data.data(), SIZE, (1024 - 2) * 1024);
    close(fd);
#endif
    return reinterpret_cast<const DeviceConfig *>(data.constData());
}

QString SystemConfig::serialNumber()
{
    QString number;
    const DeviceConfig * config = readFlash();
    if (config == 0)
    {
        return number;
    }
    number = QString::fromAscii(config->serial_no);
    return number;
}

QString SystemConfig::deviceId()
{
    QString id;
    const DeviceConfig * config = readFlash();
    if (config == 0)
    {
        return id;
    }
    id = QString::fromAscii(config->dev_id);
    return id;
}

QString SystemConfig::version()
{
    QString version("1.0.20090922");
    const DeviceConfig * config = readFlash();
    if (config == 0)
    {
        return version;
    }
    QString result = QString::fromAscii(config->version);
    if (result.isEmpty())
    {
        return version;
    }
    if (result.size() != version.size() ||
        !result.at(0).isDigit())
    {
        return version;
    }
    return result;
}

}

