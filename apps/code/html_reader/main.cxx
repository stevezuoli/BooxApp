#include <QtGui>
#include "application.h"
#include "onyx/ui/languages.h"

using namespace reader;

int main(int argc, char * argv[])
{
    ReaderApplication app(argc, argv);
    ReaderApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(vbf_icons);
    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(dictionary_images);
    ui::loadTranslator(QLocale::system().name());

    if (argc >= 2)
    {
        // Should change to local8bit.
        if (adaptor.open(QString::fromLocal8Bit(argv[1])))
        {
            return app.exec();
        }
    }
    return -1;
}
