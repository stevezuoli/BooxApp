#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtGui/QWidget>

#include "onyx/base/dbus.h"
#include "onyx/ui/zoom_setting_actions.h"
#include "onyx/ui/font_actions.h"
#include "onyx/ui/view_actions.h"
#include "onyx/ui/system_actions.h"

#include "onyx/ui/reading_tools_actions.h"
#include "onyx/ui/zoom_setting_actions.h"

#include "onyx_office_view.h"

class QPoint;
class QVBoxLayout;

namespace ui {
class StatusBar;
}

using namespace ui;
using namespace vbf;
namespace onyx {

class MainWidget :  public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent);
    bool open(const QString &path);
    StatusBar & statusBar();

protected:
    bool event(QEvent* event);
   // void resizeEvent(QResizeEvent * event);

private:
    void updateScreen();

private slots:
    void createLayout();

    void onPagebarClicked(const int percentage, const int value);
    void onScreenSizeChanged(int);
    void onFullScreen(bool);

    QSize clientSize();

signals:
    void  PageChanged(const int, const int);

private:
    QVBoxLayout  layout_;
    OfficeView view_;
    StatusBar status_bar_;

    QPoint pos_;
}; //MainWidget


class MainWidgetAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.office_viewer");
public:
    MainWidgetAdaptor(MainWidget *MW)
            : QDBusAbstractAdaptor(MW)
            , app_(MW) {
        QDBusConnection::systemBus().registerService("com.onyx.service.office_viewer");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/office_viewer", app_);
    }

public Q_SLOTS:

    bool open(const QString & path) {
        app_->open(path);
        return true;
    }

private:
    MainWidget *app_;


};  // MainWidgetAdaptor
};

#endif // MainWidget_H

