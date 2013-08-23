
#include "onyx/base/base.h"
#include "comic_application.h"
#include "onyx/sys/sys.h"

using namespace comic_reader;

int main (int argc, char *argv[])
{
	ComicApplication app(argc, argv);
    ComicApplicationAdaptor adaptor(&app);

    sys::SysStatus::instance().setSystemBusy(false);

    Q_INIT_RESOURCE(vbf_icons);
    Q_INIT_RESOURCE(onyx_ui_images);

    if (app.open(app.currentPath()))
    {
        return app.exec();
    }
    else
    {
        // TODO handle error
    }
	return 0;
}
