#include <QtGui/QtGui>
#include "../inc/power_manager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Gpio gpio;
    PowerManager pm(gpio);
    pm.deepSleep();
    return 0;
}

