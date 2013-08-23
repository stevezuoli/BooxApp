
#include "onyx/ui/ui_utils.h"
#include "onyx/sys/sys_status.h"
#include "onyx/sys/platform.h"

namespace ui
{

QRect screenGeometry()
{
    QRect rc = QApplication::desktop()->geometry();
    int def_rotation = sys::defaultRotation();
    int r1 = (def_rotation + 90) % 360;
    int r2 = (def_rotation + 270) % 360;
    if (sys::SysStatus::instance().screenTransformation() == r1 ||
        sys::SysStatus::instance().screenTransformation() == r2)
    {
        int w = rc.width();
        rc.setWidth(rc.height());
        rc.setHeight(w);
    }
    return rc;
}

bool dockWidget(QWidget *target, QWidget * container, Qt::Alignment align)
{
    return true;
}

int statusBarHeight()
{
    if (sys::isIRTouch())
    {
        if(isHD())
        {
            return 60;
        }
        return 45;
    }
    return 35;
}

int defaultFontPointSize()
{
    return 20;
}

QPoint globalTopLeft(QWidget *wnd)
{
    return wnd->mapToGlobal(QPoint());
}

QPoint globalCenter(QWidget *wnd)
{
    return QRect(wnd->mapToGlobal(QPoint()), wnd->size()).center();
}

/// Return the distance between first and second widget.
int distance(QWidget * first, QWidget *second)
{
    QPoint first_pt = globalCenter(first);
    QPoint second_pt = globalCenter(second);
    return distance(first_pt, second_pt);
}

int distance(QPoint first_pt, QPoint second_pt)
{
    int x = (second_pt.x() - first_pt.x()) * (second_pt.x() - first_pt.x());
    int y = (second_pt.y() - first_pt.y()) * (second_pt.y() - first_pt.y());
    return static_cast<int>(sqrt(static_cast<float>(x) + static_cast<float>(y)));
}

int keyboardKeyHeight()
{
    return 50;
}

int checkBoxViewWidth()
{
    return 18;
}

QWidget * safeParentWidget(QWidget *parent_widget)
{
    QWidget * safe_ptr = parent_widget;
    if (0 == safe_ptr)
    {
        safe_ptr = qApp->desktop();
    }
    return safe_ptr;
}

QSize bestDialogSize()
{
    int width = QApplication::desktop()->screenGeometry().width() * 2 / 3;
    int height = QApplication::desktop()->screenGeometry().height() * 4 / 5;
    return QSize(width, height);
}

QString sizeString(int size)
{
    QString string;
    if (size > 1024 * 1024 * 1024)
    {
        QString tmp("%1.%2 %3");
        int gb = size >> 30;
        int left = ((size - (gb << 30)) * 10) >> 30;
        string = tmp.arg(gb).arg(left).arg(QApplication::tr("GB"));
    }
    else if (size > 1024 * 1024)
    {
        QString tmp("%1.%2 %3");
        int mb = size >> 20;
        int left = ((size - (mb << 20)) * 10) >> 20;
        string = tmp.arg(mb).arg(left).arg(QApplication::tr("MB"));
    }
    else if (size > 1024)
    {
        QString tmp("%1 %2");
        string = tmp.arg(size >> 10).arg(QApplication::tr("KB"));
    }
    else
    {
        QString tmp("%1 %2");
        string = tmp.arg(size).arg(QApplication::tr("Bytes"));
    }
    return string;
}

bool is97inch()
{
    int w = qgetenv("SCREEN_WIDTH").toInt();
    int h = qgetenv("SCREEN_HEIGHT").toInt();
    return (w >= 1200 || h >= 1200);
}

bool isLandscapeVolumeMapping()
{
    return qgetenv("LANDSCAPE_VOLUME_MAPPING").toInt() > 0;
}

bool isHD()
{
    int w = qgetenv("SCREEN_WIDTH").toInt();
    int h = qgetenv("SCREEN_HEIGHT").toInt();
    return (w == 1024 || h == 1024);
}

}


