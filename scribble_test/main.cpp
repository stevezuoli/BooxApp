#include "onyx/base/base.h"
#include "scribble_application.h"

using namespace ui;

int main(int argc, char** argv)
{
    ScribbleApplication app(argc,argv);
    if (app.open(app.currentPath()))
    {
        return app.exec();
    }
    return 0;
}
