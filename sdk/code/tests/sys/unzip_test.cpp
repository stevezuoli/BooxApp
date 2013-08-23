#include <QtGui/QtGui>
#include <cassert>

#include "onyx/sys/sys_utils.h"


using namespace sys;

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);
    QStringList lst = zipFileList(argv[1], 10 * 1000);
    qDebug() << lst;
    return 0;
}

