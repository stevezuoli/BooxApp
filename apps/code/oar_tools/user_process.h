#ifndef OAR_USER_PROCESS_H_
#define OAR_USER_PROCESS_H_

#include <QProcess>

/// User application process. Drop all privileges in the child process, and enter
/// a chroot jail.
class UserProcess : public QProcess
{
public:
    UserProcess(QObject *parent = 0);
    ~UserProcess();

public:
    void setChrootJail(const QString & dir) { dir_ = dir; }

protected:
    void setupChildProcess();

private:
    QString dir_;
};


#endif
