#include "onyx/base/device.h"
#include "onyx/ui/status_bar_item_clock.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/platform.h"
#include "onyx/ui/ui_utils.h"

namespace ui
{

const QString StatusBarItemClock::DATE_FORMAT = "hh:mm";

StatusBarItemClock::StatusBarItemClock(QWidget *parent)
    : StatusBarItem(CLOCK, parent)
    , start_(QDateTime::currentDateTime())
{
    QFont font;
    font.setPointSize(20);
    if(ui::isHD() && sys::isIRTouch())
    {
        font.setPointSize(28);
    }

    font.setBold(true);
    setFont(font);
    metrics_.reset(new QFontMetrics(font));
    setTimeText();

    createLayout();
}

StatusBarItemClock::~StatusBarItemClock(void)
{
}

QSize StatusBarItemClock::sizeHint() const
{
    int w = metrics_->width(time_text_);
    int h = static_cast<int> (metrics_->height());
#ifndef Q_WS_QWS
    w += 25;
#endif
    return QSize(w, h);
}

QSize StatusBarItemClock::minimumSizeHint() const
{
    return sizeHint();
}

void StatusBarItemClock::createLayout()
{

}

// return true if new time text is set, return false otherwise.
bool StatusBarItemClock::setTimeText()
{
    QDateTime current(QDateTime::currentDateTime());
    QString new_time_text = current.toString(DATE_FORMAT);

    if (new_time_text == time_text_)
    {
        return false;
    }

    bool update_layout = (metrics_->width(time_text_) != metrics_->width(new_time_text));
    time_text_ = new_time_text;
    if (update_layout)
    {
        updateGeometry();
    }
    return true;
}

void StatusBarItemClock::paintEvent(QPaintEvent *pe)
{
    QPainter painter(this);

    painter.setPen(Qt::white);
    setTimeText();
    painter.drawText(rect(), Qt::AlignCenter, time_text_);
}

void StatusBarItemClock::mousePressEvent(QMouseEvent *me)
{
    me->accept();
}

void StatusBarItemClock::mouseReleaseEvent(QMouseEvent *me)
{
    me->accept();
    bool update = setTimeText();
    if (update)
    {
        onyx::screen::instance().enableUpdate(false);
        repaint();
        onyx::screen::instance().updateWidget(this,
                onyx::screen::ScreenProxy::GU);
        onyx::screen::instance().enableUpdate(true);
    }
    emit clicked();
}

}
