#include "onyx/sys/wpa_connection.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    WpaConnection proxy("wlan0");
    proxy.scan();

    // When signal emitted, use scanResults to retrieve scan results.
    return app.exec();
}

