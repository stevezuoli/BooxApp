#include "onyx/base/base.h"
#include "webapp_application.h"

using namespace ui;
using namespace webapp;

int main(int argc, char** argv)
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

    adaptor.open(path);
    return app.exec();
}
