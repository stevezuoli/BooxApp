#include "onyx/base/base.h"
#include "text_application.h"

using namespace ui;
using namespace text_editor;

int main(int argc, char** argv)
{
    TextApplication app(argc,argv);
    TextApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(tts_images);

    if (app.open(app.currentPath()))
    {
        return app.exec();
    }
    return 0;
}
