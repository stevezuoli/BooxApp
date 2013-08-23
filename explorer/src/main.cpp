#include "explorer/include/main_window.h"
#include "onyx/ui/languages.h"

using namespace explorer;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("explorer");

    // Resource.
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(wifi_images);

    // Need a translator map.
    ui::loadTranslator(QLocale::system().name());
    
    // Set Font
    sys::SystemConfig conf;
    if (!conf.defaultFontFamily().isEmpty())
    {
        QApplication::setFont(conf.defaultFontFamily());
    }
    conf.close();

    // Model and main window.
    model::ModelTree model;
    view::MainWindow main_wnd(model);
    main_wnd.updateAll();

    return app.exec();
}
