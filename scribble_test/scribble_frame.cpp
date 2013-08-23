#include "onyx/ui/languages.h"
#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"
#include <algorithm>
#include <math.h>

#include "scribble_frame.h"

using namespace std;

ScribbleWidget::ScribbleWidget(QWidget *parent)
#ifndef Q_WS_QWS
    : QWidget(parent)
#else
    : QWidget(0, Qt::FramelessWindowHint)
#endif
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

#ifndef Q_WS_QWS
    resize(600, 800);
#else
    resize(qApp->desktop()->screenGeometry().size());
#endif
}

ScribbleWidget::~ScribbleWidget()
{
}

void ScribbleWidget::mousePressEvent( QMouseEvent *me )
{
    scribble_points_.clear();
    scribble_points_.push_back(me->pos());
}

void ScribbleWidget::mouseReleaseEvent( QMouseEvent *me )
{
    scribble_points_.push_back(me->pos());
}

void ScribbleWidget::mouseMoveEvent( QMouseEvent *me )
{
    scribble_points_.push_back(me->pos());
}

void ScribbleWidget::keyReleaseEvent( QKeyEvent *ke )
{
    switch(ke->key())
    {
        case ui::Device_Menu_Key:
        case Qt::Key_F10:
        {
            update();
        }
        break;
    default:
        break;
    }
}

void ScribbleWidget::paintEvent( QPaintEvent *pe )
{
    if (!scribble_points_.isEmpty())
    {
        QPainter painter(this);
        QPoint p1, p2;
        for (int i = 0; i < scribble_points_.size() - 1; ++i)
        {
            p1 = scribble_points_[i];
            p2 = scribble_points_[i + 1];
#ifdef BRESHAM
            drawLine(p1, p2, painter);
#else
            painter.drawLine(p1, p2);
#endif
        }
    }
}

void ScribbleWidget::resizeEvent( QResizeEvent *re )
{
}

void ScribbleWidget::drawLine(const QPoint & p1,
                              const QPoint & p2,
                              QPainter & painter)
{
    // set color and size of brush
    int brush_color = 0x00;
    int point_size = 2;
    QBrush brush(QColor(brush_color, brush_color, brush_color));

    // Use bresham algorithm to draw the line
    int rad = point_size / 2;
    int px1 = p1.x() - rad;
    int py1 = p1.y() - rad;
    int px2 = p2.x() - rad;
    int py2 = p2.y() - rad;

    // draw the end point
    //painter.drawRect(px2, py2, point_size, point_size);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(px2, py2, point_size, point_size, brush);

    bool is_steep = abs(py2 - py1) > abs(px2 - px1);
    if (is_steep)
    {
        swap(px1, py1);
        swap(px2, py2);
    }

    // setup line draw
    int deltax   = abs(px2 - px1);
    int deltaerr = abs(py2 - py1);
    int error = 0;
    int x = px1;
    int y = py1;

    // setup step increment
    int xstep = (px1 < px2) ? 1 : -1;
    int ystep = (py1 < py2) ? 1 : -1;

    if (is_steep)
    {
        // draw swapped line
        for (int numpix = 0; numpix < deltax; numpix++)
        {
            x += xstep;
            error += deltaerr;

            if (2 * error > deltax)
            {
                y += ystep;
                error -= deltax;
            }

            // draw the interposed point
            //painter.drawRect(y, x, point_size, point_size);
            painter.fillRect(y, x, point_size, point_size, brush);
        }
    }
    else
    {
        // draw normal line
        for (int numpix = 0; numpix < deltax; numpix++)
        {
            x += xstep;
            error += deltaerr;

            if (2 * error > deltax)
            {
                y += ystep;
                error -= deltax;
            }

            // draw the interposed point
            //painter.drawRect(x, y, point_size, point_size);
            painter.fillRect(x, y, point_size, point_size, brush);
        }
    }
}

bool ScribbleWidget::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    //qDebug("main window event type %d", event->type());
    if (event->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled() &&
        isActiveWindow())
    {
        if (sys::SysStatus::instance().isSystemBusy())
        {
            sys::SysStatus::instance().setSystemBusy(false);
        }
        static int count = 0;
        qDebug("Update request %d", ++count);
        onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
        event->accept();
        return true;
    }
    return ret;
}
