#include "image_application.h"
#include "image_model.h"
#include "image_view.h"

using namespace image;

int main(int argc,char **argv)
{
    ImageApplication app(argc,argv);
    ImageApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(onyx_ui_images);
    sys::SysStatus::instance().setSystemBusy( false );
    return app.exec();
}

