// -*- mode: c++; c-basic-offset: 4; -*-

#include <stdlib.h>
#ifdef _WINDOWS
#include <windows.h>
#else
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/apm_bios.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <linux/rtc.h>
#include <sys/time.h>
#include <time.h>
#endif

#include <QImageReader>

#include "onyx/sys/sys_utils.h"

namespace sys
{

int runScript(const QString &command, const QStringList & parameters)
{
    QProcess script;
    script.setEnvironment(QProcess::systemEnvironment());
    script.start(command, parameters);
    if (!script.waitForStarted())
    {
        qDebug("Could not start %s", qPrintable(command));
        return -1;
    }

    while (script.state() == QProcess::Running)
    {
        QCoreApplication::processEvents();
    }
    return script.exitCode();
}

int runScriptBlock(const QString &command,
                   const QStringList & parameters,
                   const int timeout)
{
    QProcess script;
    script.setEnvironment(QProcess::systemEnvironment());
    script.start(command, parameters);
    if (!script.waitForStarted())
    {
        qDebug("Could not start %s", qPrintable(command));
        return -1;
    }

    if (!script.waitForFinished(timeout))
    {
        qDebug("Failed");
    }
    return script.exitCode();
}

unsigned long long diskSpace(const QString & mount_ponit)
{
#ifdef _WINDOWS
    return 0;
#else
    struct statfs stats;
    statfs (mount_ponit.toLocal8Bit(), &stats);
    return (static_cast<unsigned long long>(stats.f_blocks) *
            static_cast<unsigned long long>(stats.f_bsize));
#endif
}

unsigned long long freeSpace(const QString & mount_ponit)
{
#ifdef _WINDOWS
    return 0;
#else
    struct statfs stats;
    statfs (mount_ponit.toLocal8Bit(), &stats);
    return (static_cast<unsigned long long>(stats.f_bavail) *
            static_cast<unsigned long long>(stats.f_bsize));
#endif
}

unsigned long systemTotalMemory()
{
#ifdef _WINDOWS
    MEMORYSTATUS info;
    GlobalMemoryStatus(&info);
    return info.dwTotalPhys;
#else
    struct sysinfo info;
    sysinfo(&info);
    return info.totalram * info.mem_unit;
#endif
}

// On Linux, we can also use  cat /proc/<pid>/status to get process memory
// usage.
unsigned long systemFreeMemory()
{
#ifdef _WINDOWS
    MEMORYSTATUS info;
    GlobalMemoryStatus(&info);
    return info.dwAvailPhys;
#else
    struct sysinfo info;
    sysinfo(&info);
    return info.freeram * info.mem_unit;
#endif
}

unsigned long safeMemoryLimit()
{
#ifdef _WINDOWS
    return systemFreeMemory() - (60 << 20);
#else
    return 15 * 1024 * 1024;
#endif
}

bool needReleaseMemory()
{
    return systemFreeMemory() <= safeMemoryLimit();
}

QStringList zipFileList(const QString &path, const int ms)
{
    QProcess script;
    script.setEnvironment(QProcess::systemEnvironment());

    QStringList ret;
    QString command("unzip");
    QStringList parameters;
    parameters << "-l";
    parameters << path;
    script.start(command, parameters);
    if (!script.waitForStarted())
    {
        qDebug("Could not start %s", qPrintable(command));
        return ret;
    }

    if (!script.waitForFinished(ms))
    {
        qDebug("Failed");
    }

    QByteArray data = script.readAllStandardOutput();
    QTextStream stream(&data);

    // Ignore the first three title lines.
    stream.readLine();
    QString tmp = stream.readLine();
    stream.readLine();

    int pos = tmp.lastIndexOf("Name", -1, Qt::CaseInsensitive);
    QString name;
    while (!stream.atEnd())
    {
        tmp = stream.readLine();
        name = tmp.mid(pos);
        ret << name;
    }
    if (ret.size() > 2)
    {
        ret.removeLast();
        ret.removeLast();
    }
    return ret;
}


/// Check the suffix is a image suffix or not.
bool isImage(const QString& suffix)
{
    static QList<QString> supported_formats;
    if (supported_formats.size() <= 0)
    {
        QList<QByteArray> list = QImageReader::supportedImageFormats();
        for(QList<QByteArray>::iterator it = list.begin(); it != list.end(); ++it)
        {
            QString ext(*it);
            ext = ext.toLower();
            supported_formats.push_back(ext);
        }
    }
    return supported_formats.contains(suffix.toLower());
}


bool isImageZip(const QString &path, const int threshold)
{
    QStringList lst = zipFileList(path, 10 * 1000);
    if (lst.size() <= 0)
    {
        return false;
    }

    int count = 0;
    foreach(QString i, lst)
    {
        int pos = i.lastIndexOf(".");
        if (pos > 0)
        {
            QString suffix = i.mid(pos);
            if (isImage(suffix))
            {
                ++count;
            }
        }
    }

    if (count * 100 / lst.size() >= threshold)
    {
        return true;
    }
    return false;
}

bool writeString(const char *dev, const char *str)
{
#ifdef BUILD_FOR_ARM
    int fd = open(dev, O_RDWR);
    write(fd, str, strlen(str));
    close(fd);
#endif
    return true;
}

QByteArray readString(const char *dev, int size)
{
    QByteArray data;
    data.resize(size);
#ifdef BUILD_FOR_ARM
    int fd = open(dev, O_RDONLY);
    read(fd, data.data(), data.size());
    close(fd);
#endif
    return data;
}


}
