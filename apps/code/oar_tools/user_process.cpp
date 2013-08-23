
#include "user_process.h"

#if defined Q_OS_UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#endif

UserProcess::UserProcess(QObject * parent)
: QProcess(parent)
{
}

UserProcess::~UserProcess()
{
}

/// TODO, change it later when access priviilege is ready.
void UserProcess::setupChildProcess()
{
#if defined Q_OS_UNIX
    ::setgroups(0, 0);
    ::chroot(dir_.toLocal8Bit().constData());
    ::chdir(dir_.toLocal8Bit().constData());
    // ::setgid(safeGid);
    // ::setuid(safeUid);
    ::umask(0);
#endif
}

