#include <QtGui/QApplication>
#include "main_widget.h"

// Onyx SDK
#include "onyx/base/base.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys_status.h"
#include "onyx/ui/languages.h"
#include "onyx/ui/ui.h"

using namespace onyx;

int main (int argc, char **argv)
{
    QApplication app (argc, argv);
    if (argc < 1)
    {
        return 0;
    }

    // Make sure you load translator before main widget created.
    ui::loadTranslator (QLocale::system().name());

    // In order to show boot splash. Make sure main widget process
    // all screen update requests.
    MainWidget main_widget (0);
    MainWidgetAdaptor adaptor (&main_widget);
    main_widget.showMaximized();
    qApp->processEvents();
    sys::SysStatus::instance().setSystemBusy(false);

    // Open document now.
    QString p = QString::fromLocal8Bit (argv[1]);
    if (main_widget.open (p))
    {
        return app.exec();
    }
    return 0;
}
