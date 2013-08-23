#ifndef SCRIBBLE_WIDGET_
#define SCRIBBLE_WIDGET_

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "onyx/sys/sys.h"

using namespace ui;
using namespace base;

class ScribbleWidget : public QWidget
{
    Q_OBJECT
public:
    ScribbleWidget(QWidget *parent = 0);
    ~ScribbleWidget();

private:
    // GUI event handlers.
    void mousePressEvent( QMouseEvent *me );
    void mouseReleaseEvent( QMouseEvent *me );
    void mouseMoveEvent( QMouseEvent *me );
    void keyReleaseEvent( QKeyEvent *ke );
    void paintEvent( QPaintEvent *pe );
    void resizeEvent( QResizeEvent *re );
    bool event(QEvent * event);

    void drawLine(const QPoint & p1, const QPoint & p2, QPainter & painter);
private:
    QVector<QPoint> scribble_points_;
};

#endif
