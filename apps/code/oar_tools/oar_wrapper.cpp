
#include <QtGui/QtGui>
#include "user_process.h"

int main(int argc, char * argv[])
{
    if (argc <= 1)
    {
        return -1;
    }

    // Explorer passes additional parameters.
    QStringList list;
    int begin = 1;
    int end   = argc - 1;
    for(int i = begin; i < end; ++i)
    {
        list << argv[i];
    }

    QString app_name = argv[end];
    UserProcess run;
    QFileInfo info(app_name);
    run.setChrootJail(info.absolutePath());
    run.execute(app_name, list);
    run.waitForFinished(-1);
    qDebug("run returns %d", run.exitCode());

    return 0;
}

