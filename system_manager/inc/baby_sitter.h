// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef NABOO_SYSTEM_MANAGER_BABY_SITTER_H_
#define NABOO_SYSTEM_MANAGER_BABY_SITTER_H_

#include <sys/types.h>

#include <QObject>
#include <QProcess>
#include <QString>

// The BabySitter baby-sits a process by launching the process and
// restarting it whenever it is terminated.
class BabySitter : public QObject
{
    Q_OBJECT;
public:
    explicit BabySitter(const QString& cmd);
    virtual ~BabySitter();

public:
    void start();
    void enableRestart(bool e) { should_restart_ = e; }
    bool ensureFinished();

private slots:
    void childTerminated(int exit_code, QProcess::ExitStatus exit_status);

private:
    const QString cmd_;
    QProcess process_;
    bool should_restart_;
};

#endif  // NABOO_SYSTEM_MANAGER_BABY_SITTER_H_
