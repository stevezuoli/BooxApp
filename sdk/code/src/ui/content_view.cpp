
#include <QPainter>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QFocusEvent>
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/ui/content_view.h"
#include "onyx/sys/sys_conf.h"
#include "onyx/data/data_tags.h"
#include "onyx/sys/platform.h"

namespace ui
{

static QPoint s_mouse;
static const int MARGIN = 4;

ContentView::ContentView(QWidget *parent)
        : QWidget(parent)
        , data_(0)
        , pressed_(false)
        , checked_(false)
        , repaint_on_mouse_release_(true)
        , pen_width_(3)
        , bk_color_(Qt::white)
        , waveform_(onyx::screen::ScreenProxy::DW)
{
    setAutoFillBackground(false);
}

ContentView::~ContentView()
{
}

void ContentView::setChecked(bool checked)
{
    checked_ = checked;
}

bool ContentView::isChecked()
{
    return checked_;
}

bool ContentView::updateData(OData* data, bool force)
{
    if (data)
    {
        setFocusPolicy(Qt::StrongFocus);
    }
    else
    {
        setFocusPolicy(Qt::NoFocus);
    }

    if (data_ == data && !force)
    {
        return false;
    }
    checked_ = false;
    data_ = data;
    updateView();
    update();
    return true;
}

OData * ContentView::data()
{
    return data_;
}

OData * ContentView::data() const
{
    return data_;
}

bool ContentView::isPressed()
{
    return pressed_;
}

void ContentView::setPressed(bool p)
{
    pressed_ = p;
}

void ContentView::activate(int user_data)
{
    if (data())
    {
        emit activated(this, user_data);
    }
}

void ContentView::repaintAndRefreshScreen()
{
    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
}

void ContentView::setRepaintOnMouseRelease(bool enable)
{
    repaint_on_mouse_release_ = enable;
}

void ContentView::mousePressEvent(QMouseEvent *event)
{
    s_mouse = event->globalPos();
    if (data())
    {
        setPressed(true);
        if (repaint_on_mouse_release_)
        {
            repaintAndRefreshScreen();
        }
    }
    QWidget::mousePressEvent(event);
}

void ContentView::mouseReleaseEvent(QMouseEvent *event)
{
    bool broadcast = false;
    if (isPressed())
    {
        broadcast = true;
    }
    else
    {
        emit mouse(s_mouse, event->globalPos());
    }
    setPressed(false);
    if (data())
    {
        repaintAndRefreshScreen();
    }
    QWidget::mouseReleaseEvent(event);
    if (broadcast)
    {
        activate();
    }
}

void ContentView::mouseMoveEvent(QMouseEvent * e)
{
    if (data())
    {
        if (!rect().contains(e->pos()))
        {
            if (isPressed())
            {
                setPressed(false);
                repaintAndRefreshScreen();
            }
        }
        else
        {
            int direction = sys::SystemConfig::direction(s_mouse, e->globalPos());
            if (direction != 0)
            {
                setPressed(false);
            }
        }
    }
}

void ContentView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return)
    {
        activate();
        e->accept();
        return;
    }
    e->ignore();
    emit keyRelease(this, e);
}

void ContentView::changeEvent(QEvent *event)
{
}

void ContentView::moveEvent(QMoveEvent * event)
{
    // qDebug() << " ContentView::moveEvent";
    QWidget::moveEvent(event);
}

void ContentView::resizeEvent(QResizeEvent * event)
{
    // qDebug() << "ContentView::resizeEvent";
    QWidget::resizeEvent(event);
}

bool ContentView::event(QEvent * e)
{
    switch (e->type())
    {
    case QEvent::HoverMove:
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
        e->accept();
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}


void ContentView::setFocusWaveform(onyx::screen::ScreenProxy::Waveform w)
{
    waveform_ = w;
}

onyx::screen::ScreenProxy::Waveform ContentView::focusWaveform()
{
    return waveform_;
}

void ContentView::focusInEvent(QFocusEvent * e)
{
    QWidget::focusInEvent(e);
    emit focusChanged(this);
}

void ContentView::focusOutEvent(QFocusEvent * e)
{
    QWidget::focusOutEvent(e);
    emit focusChanged(this);
    //onyx::screen::watcher().enqueue(this, focusWaveform(), onyx::screen::ScreenCommand::WAIT_NONE);
}

void ContentView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (isPressed())
        {
            //painter.fillRect(rect(), Qt::darkGray);
        }
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }
    }
}

void ContentView::drawTitle(QPainter &painter, QRect rect, int flags)
{
    if (data() && data()->contains(TAG_TITLE))
    {
        QString family = data()->value(TAG_FONT_FAMILY).toString();
        if (family.isEmpty())
        {
            family = QApplication::font().family();
        }

        int size = data()->value(TAG_FONT_SIZE).toInt();
        if (size <= 0)
        {
            size = ui::defaultFontPointSize();
        }
        QFont font(family, size);

        int is_align_left = flags & Qt::AlignLeft;
        if (is_align_left)
        {
            rect.adjust(10, 0, 0, 0);
        }

        int is_align_right = flags & Qt::AlignRight;
        if (is_align_right)
        {
            rect.adjust(0, 0, -10, 0);
        }

        painter.setFont(font);
        painter.drawText(rect, flags, data()->value(TAG_TITLE).toString());
    }
}



CoverView::CoverView(QWidget *parent)
: ContentView(parent)
{
}

CoverView::~CoverView()
{
}

const QString CoverView::type()
{
    return "CoverView";
}

void CoverView::updateView()
{
    update();
}

void CoverView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (isChecked())
        {
             painter.fillRect(rect().adjusted(penWidth(), penWidth(),
                     -penWidth() - 1, -penWidth() - 1), Qt::gray);
        }
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }

        drawCover(painter, rect());
        if (isPressed() || isChecked())
        {
            painter.setPen(Qt::black);
        }
        drawTitle(painter, rect());
    }
}

void CoverView::drawCover(QPainter & painter, QRect rect)
{
    if (data() && data()->contains(TAG_COVER))
    {
        QPixmap pixmap(qVariantValue<QPixmap>(data()->value(TAG_COVER)));
        int x = (rect.width() - pixmap.width()) / 2;
        painter.drawPixmap(x, MARGIN, pixmap);
    }
}

void CoverView::drawTitle(QPainter & painter, QRect rect)
{
    int alignment = Qt::AlignCenter;
    if (data()->contains(TAG_ALIGN))
    {
        bool ok;
        int val = data()->value(TAG_ALIGN).toInt(&ok);
        if (ok)
        {
            alignment = val;
        }
    }
    ContentView::drawTitle(painter, rect, alignment);
}


CheckBoxView::CheckBoxView(QWidget *parent)
: ContentView(parent)
{

}

CheckBoxView::~CheckBoxView()
{
}

const QString CheckBoxView::type()
{
    return "CheckBoxView";
}

void CheckBoxView::updateView()
{
    update();
}

void CheckBoxView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (data()->contains(TAG_CHECKED))
        {
            setChecked(qVariantValue<bool> (data()->value(TAG_CHECKED)));
        }
        if (isPressed() || isChecked())
        {
            painter.fillRect(rect().adjusted(penWidth(), penWidth(), -penWidth() - 1, -penWidth() - 1), Qt::gray);
        }
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }

        QRect check_box_r = drawCheckBox(painter, rect());

        int icon_x = check_box_r.right() + MARGIN+4;
        QRect icon_r = drawCover(painter, QRect(icon_x, rect().y(),
                rect().width()-icon_x, rect().height()));

        if (isPressed() || isChecked())
        {
            painter.setPen(Qt::black);
        }
        int title_x = icon_r.right() + MARGIN;
        drawTitle(painter, QRect(title_x, rect().y(), rect().width()-title_x, rect().height()));
    }
}

QRect CheckBoxView::drawCheckBox(QPainter & painter, QRect rect)
{
    int width = checkBoxViewWidth();
    int height = width;
    int x = rect.x() + 20;
    int y = (rect.height() - height)/2;
    QRect check_box_rect(x, y, width, height);
    if (isChecked())
    {
        painter.setPen(QPen(Qt::white, 2));
        painter.fillRect(check_box_rect, Qt::black);
    }
    else
    {
        painter.setPen(QPen(Qt::black, 2));
        painter.fillRect(check_box_rect, Qt::white);
    }
    painter.drawRect(check_box_rect);
    return check_box_rect;
}

QRect CheckBoxView::drawCover(QPainter & painter, QRect rect)
{
    QRect icon_rect(rect.topLeft(), rect.topLeft());
    if (data() && data()->contains(TAG_COVER))
    {
        QPixmap pixmap(qVariantValue<QPixmap>(data()->value(TAG_COVER)));
        painter.drawPixmap(rect.left(), (rect.height() - pixmap.height()) / 2,
                pixmap);
        icon_rect.setRight(rect.left()+pixmap.width());
    }
    return icon_rect;
}

void CheckBoxView::drawTitle(QPainter & painter, QRect rect)
{
    ContentView::drawTitle(painter, rect, Qt::AlignVCenter|Qt::AlignLeft);
}




LineEditView::LineEditView(QWidget *parent)
    : ContentView(parent)
    , inner_edit_(this)
    , layout_(this)
    , forward_focus_(true)
{
    createLayout();
}

LineEditView::~LineEditView()
{
}

const QString LineEditView::type()
{
    return "LineEditView";
}

void LineEditView::checkEditByMouse()
{
    QKeyEvent key_event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, "virtual");
    QApplication::sendEvent(this, &key_event);
}

void LineEditView::createLayout()
{
    layout_.setContentsMargins(MARGIN, 0, MARGIN, 0);
    layout_.addWidget(&inner_edit_, 1);
    layout_.addSpacing(10);
    connect(&inner_edit_, SIGNAL(outOfRange(QKeyEvent*)), this, SLOT(onEditOutOfRange(QKeyEvent*)));
    connect(&inner_edit_, SIGNAL(setCheckByMouse(OnyxLineEdit *)),
            this, SLOT(checkEditByMouse()));
}

void LineEditView::updateView()
{
    if (data())
    {
        if (data()->contains(TAG_TITLE))
        {
            QString text = data()->value(TAG_TITLE).toString();
            inner_edit_.setText(text);
        }
    }
}

void LineEditView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (data()->contains(TAG_CHECKED))
        {
            setChecked(qVariantValue<bool> (data()->value(TAG_CHECKED)));
        }

        if (data()->contains(TAG_IS_PASSWD))
        {
            bool is_password = data()->value(TAG_IS_PASSWD).toBool();
            innerEdit()->setEchoMode(
                    (is_password? QLineEdit::Password : QLineEdit::Normal) );
        }

        // set disable property
        if (data()->contains(TAG_DISABLED))
        {
            bool disabled = data()->value(TAG_DISABLED).toBool();
            innerEdit()->setEnabled(!disabled);
            setEnabled(!disabled);
            if (disabled)
            {
                // no more paint since it is disabled.
                return;
            }
        }

        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(0, 0, -penWidth() , -penWidth()), 5, 5);
        }

        QColor color;
        if (isChecked())
        {
            color = Qt::black;
        }
        else
        {
            color = Qt::white;
        }
        painter.setPen(color);
        QBrush brush(color);
        painter.setBrush(brush);
        painter.drawEllipse(rect().width()-10, (rect().height()-5)/2, 6, 6);
    }
}

void LineEditView::focusInEvent(QFocusEvent * event)
{
    if (forward_focus_)
    {
        inner_edit_.setFocus();
        onyx::screen::watcher().enqueue(&inner_edit_, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
    }
}

void LineEditView::focusOutEvent(QFocusEvent * event)
{
    forward_focus_ = true;
}

void LineEditView::onEditOutOfRange(QKeyEvent *ke)
{
    forward_focus_ = false;
    setFocus();
    emit keyRelease(this, ke);
    onyx::screen::watcher().enqueue(&inner_edit_, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
}

void LineEditView::keyPressEvent(QKeyEvent * src)
{
    if (Qt::Key_Return == src->key() || Qt::Key_Enter == src->key())
    {
        emit checkStateChanged(this);
        src->accept();
        update();
        onyx::screen::watcher().enqueue(this,
                onyx::screen::ScreenProxy::DW,
                onyx::screen::ScreenCommand::WAIT_NONE);
    }
    else
    {
        QKeyEvent key(src->type(), src->key(), src->modifiers(), src->text());
        QApplication::sendEvent(&inner_edit_, &key);
    }
}

void LineEditView::keyReleaseEvent(QKeyEvent * event)
{
    if (Qt::Key_Escape == event->key())
    {
        event->ignore();
        return;
    }
    event->accept();
}


ClockView::ClockView(QWidget *parent)
: ContentView(parent)
{
}

ClockView::~ClockView()
{
}

const QString ClockView::type()
{
    return "ClockView";
}

void ClockView::updateView()
{
}

void ClockView::drawDigitalClock(QPainter &painter)
{
    int size = ui::defaultFontPointSize();
    QFont font(QApplication::font());
    font.setPointSize(size);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter|Qt::TextWordWrap , QDateTime::currentDateTime().toString());
}

void ClockView::paintEvent(QPaintEvent * event)
{
     const double PI = 3.14159265;
     const double LEN = 75;


    static const QPoint hourHand[3] = {
         QPoint(3, 4),
         QPoint(-4, 4),
         QPoint(0, -40)
     };
     static const QPoint minuteHand[3] = {
         QPoint(3, 4),
         QPoint(-3, 4),
         QPoint(0, -70)
     };



     QColor hourColor(Qt::black);
     QColor minuteColor(Qt::black);

     int side = qMin(width() - 2 * penWidth(), height() - 2 * penWidth());
     QTime time = QTime::currentTime();

     QPainter painter(this);
     painter.setRenderHint(QPainter::Antialiasing);
     painter.fillRect(rect(), bkColor());
     painter.setRenderHint(QPainter::Antialiasing);

     if (hasFocus())
     {
         QPen pen;
         pen.setWidth(penWidth());
         painter.setPen(pen);
         painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
     }

     // drawDigitalClock(painter);
     // return;

     painter.translate(width() / 2, height() / 2);
     painter.scale(side / 200.0, side / 200.0);

     painter.setPen(Qt::NoPen);
     painter.setBrush(hourColor);

     painter.save();
     painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
     painter.drawConvexPolygon(hourHand, 3);
     painter.restore();

     painter.setPen(hourColor);

     QFont font;
     font.setPointSize(14);
     if(ui::isHD() && sys::isIRTouch())
     {
         font.setPointSize(18);
     }
     font.setBold(true);
     painter.setFont(font);
     QFontMetrics fm = painter.fontMetrics();
     QString s;
     for (int i = 0; i < 12; ++i) {
         double x = cos(i * 30.0 * PI / 180.0) * LEN;
         double y = sin(i * 30.0 * PI / 180.0) * LEN;
         int d = (i + 3) % 12;
         if (d <= 0)
         {
             d = 12;
         }
         s = QString::number(d);
         QRect rc = fm.boundingRect(s);
         rc.moveCenter(QPoint(x, y));
         painter.drawText(rc, Qt::AlignCenter, s);
     }

     for (int i = 0; i < 12; ++i) {
         painter.drawLine(88, 0, 98, 0);
         painter.rotate(30.0);
     }

     painter.setPen(Qt::NoPen);
     painter.setBrush(minuteColor);

     painter.save();
     painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
     painter.drawConvexPolygon(minuteHand, 3);
     painter.restore();

     painter.setPen(minuteColor);
}

ButtonView::ButtonView(QWidget *parent)
    : ContentView(parent)
{
}

ButtonView::~ButtonView()
{
}

const QString ButtonView::type()
{
    return "ButtonView";
}

void ButtonView::updateView()
{
    update();
}


void ButtonView::keyPressEvent(QKeyEvent *e)
{
    ContentView::keyPressEvent(e);
}

void ButtonView::keyReleaseEvent(QKeyEvent *e)
{
    ContentView::keyReleaseEvent(e);
}

void ButtonView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }
        else
        {
            QPen pen;
            pen.setWidth(2);
            pen.setColor(Qt::black);
            painter.setPen(pen);
            painter.drawRect(rect());
        }

    }

    drawTitle(painter, rect());
}

void ButtonView::drawTitle(QPainter & painter, QRect rect)
{
    int alignment = Qt::AlignCenter;
    if (data()->contains(TAG_ALIGN))
    {
        bool ok;
        int val = data()->value(TAG_ALIGN).toInt(&ok);
        if (ok)
        {
            alignment = val;
        }
    }
    ContentView::drawTitle(painter, rect, alignment);
}

}

