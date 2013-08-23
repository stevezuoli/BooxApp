#include "battery_test.h"
#include "screen_manager/screen_manager.h"

MessageWidget::MessageWidget(PowerManager & watcher)
: QWidget(0, Qt::FramelessWindowHint)
, watcher_(watcher)
, timer_(0)
, count_(0)
{
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer_.start(3000);
    font_.setPointSize(20);
}

MessageWidget::~MessageWidget()
{
}

void MessageWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setFont(font_);
    QString information("count %1 volatge %2 threshold %3");
    information = information.arg(count_).arg(watcher_.voltage()).arg(3400);
    p.fillRect(rect(), QBrush(Qt::white));
    p.drawText(QPoint(0, 100), information);
}

bool MessageWidget::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::UpdateRequest)
    {
        screen::instance().updateWidget(this, screen::SCREEN_UPDATE_FULL);
    }
    return ret;
}

void MessageWidget::onTimeout()
{
    ++count_;
    update();
}

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);
    Gpio gpio;
    PowerManager pm(gpio);
    MessageWidget wnd(pm);
    wnd.showMaximized();
    return app.exec();
}
