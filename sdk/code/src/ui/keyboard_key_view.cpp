#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/keyboard_key_view.h"
#include "onyx/ui/keyboard_data.h"

namespace ui
{

static const int KEYBOARD_KEY_MARGIN = 2;

KeyboardKeyView::KeyboardKeyView(QWidget *parent)
    : ContentView(parent)
{
    setRepaintOnMouseRelease(false);
}

KeyboardKeyView::~KeyboardKeyView()
{
}

void KeyboardKeyView::updateView()
{
    update();
}

const QString KeyboardKeyView::type()
{
    return "KeyboardKeyView";
}

void KeyboardKeyView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    if (data())
    {
        if (isPressed())
        {
            // painter.fillRect(rect(), Qt::darkGray);
        }
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }

        drawIcon(painter, rect());
        drawText(painter, rect());
    }
}

void KeyboardKeyView::drawIcon(QPainter & painter, QRect rect)
{
    if (data() && data()->contains(TAG_COVER))
    {
        QPixmap pixmap(qVariantValue<QPixmap> (data()->value(TAG_COVER)));
        int x = (rect.width() - pixmap.width()) / 2;
        int y = (rect.height() - pixmap.height()) / 2;
        painter.drawPixmap(x, y, pixmap);
    }
}

void KeyboardKeyView::drawText(QPainter & painter, QRect rect)
{
    if (data() && data()->contains(TAG_TITLE))
    {
        QFont font;
        font.setPointSize(24);
        if (data()->contains(TAG_FONT_SIZE))
        {
            int font_size = qVariantValue<int>(data()->value(TAG_FONT_SIZE));
            font.setPointSize(font_size);
        }
        painter.setFont(font);
        painter.drawText(rect, Qt::AlignCenter, data()->value(
                TAG_TITLE).toString());
    }
}

}   // namespace ui
