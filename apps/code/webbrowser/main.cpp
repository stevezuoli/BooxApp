#include <QtGui>
#include "web_application.h"
#include "view.h"

using namespace webbrowser;

int main(int argc, char * argv[])
{
    WebApplication app(argc, argv);
    WebApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(wifi_images);
    Q_INIT_RESOURCE(network_service_res);

    QString path;
    if (argc >= 2)
    {
        path = QString::fromLocal8Bit(argv[1]);
    }
    if(qgetenv("CONNECT_TO_DEFAULT_APN").toInt() > 0 &&
            sys::SysStatus::instance().isPowerSwitchOn())
    {
        adaptor.configNetwork();
    }
    adaptor.open(path);
    return app.exec();
}
