#ifndef AUTOSAVER_H
#define AUTOSAVER_H

#include "ns_utils.h"

namespace network_service
{

/*
    This class will call the save() slot on the parent object when the parent changes.
    It will wait several seconds after changed() to combining multiple changes and
    prevent continuous writing to disk.
  */
class AutoSaver : public QObject
{
    Q_OBJECT
public:
    AutoSaver(QObject *parent);
    ~AutoSaver();
    void saveIfNeccessary();

public slots:
    void changeOccurred();

protected:
    void timerEvent(QTimerEvent *event);

private:
    QBasicTimer timer_;
    QTime first_change_;
};

};

#endif // AUTOSAVER_H

