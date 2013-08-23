#include "adobe_drm_engine/adobe_drm_application.h"

using namespace adobe_drm;

int main(int argc, char* argv[])
{
    AdobeDRMApplication app(argc, argv);
    AdobeDRMApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(vbf_icons);
    Q_INIT_RESOURCE(onyx_ui_images);

    if (app.execute())
    {
        return app.exec();
    }
    return 0;
}
