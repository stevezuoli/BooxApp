// -*- mode: c++; c-basic-offset: 4; -*-

#include "baby_sitter.h"

#include <QString>
#include <QProcess>

BabySitter::BabySitter(const QString& cmd)
        : QObject(NULL), cmd_(cmd), process_(NULL), should_restart_(true)
{
}

BabySitter::~BabySitter()
{
}

void BabySitter::start()
{
    connect(&process_, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(childTerminated(int, QProcess::ExitStatus)));
    process_.start(cmd_);
}

bool BabySitter::ensureFinished()
{
    // TODO: Seems there is still some deadlock waiting between it and its child process.
    // So far, just use a very short time, fix it later. Child process should store all
    // options before calling shutdown. So it's ok to kill the process.
    enableRestart(false);
    return process_.waitForFinished(500);
}

void BabySitter::childTerminated(int exit_code,
                                 QProcess::ExitStatus exit_status)
{
    if (should_restart_ && exit_status == QProcess::CrashExit)
    {
        qDebug("Restart shell process %s.", qPrintable(cmd_));
        process_.start(cmd_);
    }
}
