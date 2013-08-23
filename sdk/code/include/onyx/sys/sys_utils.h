// Authors: John

/// Public header of the system configuration library.

#ifndef SYSTEM_UTILS_H__
#define SYSTEM_UTILS_H__

#include <QtCore/QtCore>

namespace sys
{

int runScript(const QString &command, const QStringList & parameters = QStringList());

int runScriptBlock(const QString &command, const QStringList & parameters = QStringList(), const int ms = 5000);


/// Retrieve system flash disk space in bytes.
unsigned long long diskSpace(const QString & mount_ponit);

/// Retrieve system free disk space in bytes.
unsigned long long freeSpace(const QString & mount_ponit);

/// Retrieve system total memory in bytes.
unsigned long systemTotalMemory();

/// Retrieve system available physical memory in bytes.
unsigned long systemFreeMemory();

unsigned long safeMemoryLimit();

bool needReleaseMemory();

QStringList zipFileList(const QString &path, const int ms = 5000);

bool isImage(const QString& suffix);

bool isImageZip(const QString &path, int threshold = 80);

bool writeString(const char *dev, const char *str);

QByteArray readString(const char *dev, int size = 0x400);

}

#endif  // SYSTEM_CONF_H__
