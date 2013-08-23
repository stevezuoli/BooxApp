#include "text_edit.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys.h"

const QString TEXT_EDIT_STYLE = "       \
QTextEdit                               \
{                                       \
    background: white;                  \
    selection-background-color: black;  \
    selection-color: white;             \
    font: 24px bold;                    \
    color: black;                       \
    border-width: 0px;                  \
    padding: 0px;                       \
    min-height: 32px;                   \
}";

namespace text_editor
{

TextEdit::TextEdit(QWidget *parent)
: QTextEdit(parent)
{
    setStyleSheet(TEXT_EDIT_STYLE);
    setLineWrapMode(QTextEdit::WidgetWidth);
    QApplication::setCursorFlashTime(0);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::Panel);
}

TextEdit::~TextEdit()
{
}

void TextEdit::keyPressEvent(QKeyEvent * ke)
{
    if (ke->key() == Qt::Key_F21 ||
        ke->key() == Qt::Key_F22 ||
        ke->key() == Qt::Key_F23 ||
        ke->key() == Qt::Key_F24)
    {
        ke->accept();
        return;
    }

    QTextEdit::keyPressEvent(ke);
    ke->accept();
}

void TextEdit::keyReleaseEvent(QKeyEvent * ke)
{
    if (ke->key() == Qt::Key_F21 ||
        ke->key() == Qt::Key_F22 ||
        ke->key() == Qt::Key_F23 ||
        ke->key() == Qt::Key_F24)
    {
        ke->accept();
        return;
    }

    if (ke->key() == Qt::Key_Escape ||
        ke->key() == ui::Device_Menu_Key)
    {
        ke->ignore();
        return;
    }

    QTextEdit::keyReleaseEvent(ke);
    ke->accept();
}

void TextEdit::dropEvent(QDropEvent * de)
{
    qDebug("Drop, GU update");
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, false);
    QTextEdit::dropEvent(de);
}

void TextEdit::mousePressEvent(QMouseEvent *me)
{
    pressed_point_ = me->pos();
    qDebug("Mouse Press, disable update");
    onyx::screen::instance().enableUpdate(false);
    QTextEdit::mousePressEvent(me);
    me->accept();
}

void TextEdit::mouseReleaseEvent(QMouseEvent *me)
{
    qDebug("Mouse Release");
    QTextEdit::mouseReleaseEvent(me);
    me->accept();
    while (QApplication::hasPendingEvents())
    {
        QApplication::processEvents();
    }
    onyx::screen::instance().enableUpdate(true);

    int direction = sys::SystemConfig::direction(pressed_point_, me->pos());
    if (direction == 0)
    {
        qDebug("DW update");
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW, false);
    }
    else
    {
        qDebug("GU update");
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, false);
    }
}

void TextEdit::mouseDoubleClickEvent(QMouseEvent *me)
{
    me->accept();
}

}

