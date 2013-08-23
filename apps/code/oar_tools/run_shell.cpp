
#include <QtGui/QtGui>


int main(int argc, char * argv[])
{
    QProcess run;
    run.execute("sh +x /media/sd/run");
    run.waitForFinished(-1);
    qDebug("run returns %d", run.exitCode());

    return 0;
}

