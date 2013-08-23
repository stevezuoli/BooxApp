
#include <QtGui/QtGui>
#include "onyx/ui/line_edit.h"
#include "onyx/screen/screen_update_watcher.h"

const QString LINE_EDIT_STYLE = "       \
QLineEdit                               \
{                                       \
    border: 2px solid dark;             \
    background: white;                  \
    selection-background-color: black;  \
    selection-color: white;             \
    font: 24px bold;                    \
    color: black;                       \
    border-width: 2px;                  \
    border-style: solid;                \
    border-radius: 0;                   \
    padding: 0px;                       \
    min-height: 32px;                   \
}                                       \
QLineEdit:disabled                      \
{                                       \
    border: 2px solid dark;             \
    background: white;                  \
    selection-background-color: black;  \
    selection-color: white;             \
    font: 24px bold;                    \
    color: dark;                       \
    border-width: 2px;                  \
    border-style: solid;                \
    border-radius: 0;                   \
    padding: 0px;                       \
    min-height: 32px;                   \
}";

namespace ui
{

OnyxLineEdit::OnyxLineEdit(QWidget *parent)
: QLineEdit(parent)
, out_of_range_(false)
{
    setStyleSheet(LINE_EDIT_STYLE);
    QApplication::setCursorFlashTime(0);
}

OnyxLineEdit::OnyxLineEdit(const QString & text, QWidget *parent)
: QLineEdit(text, parent)
, out_of_range_(false)
{
    setStyleSheet(LINE_EDIT_STYLE);
    QApplication::setCursorFlashTime(0);
}

OnyxLineEdit::~OnyxLineEdit()
{
}

void OnyxLineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    emit getFocus(this);
}

void OnyxLineEdit::keyReleaseEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Escape)
    {
        ke->ignore();
        return;
    }

    if (out_of_range_ || (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down))
    {
        out_of_range_ = false;
        qDebug("broadcast out of range signal.");
        ke->ignore();
        emit outOfRange(ke);
        return;
    }

    QLineEdit::keyReleaseEvent(ke);
    ke->accept();
}

void OnyxLineEdit::keyPressEvent(QKeyEvent * ke)
{
    if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
    {
        ke->ignore();
        return;
    }

    if ((ke->key() == Qt::Key_Left && cursorPosition() <= 0) ||
        (ke->key() == Qt::Key_Right && cursorPosition() >= text().size()))
    {
        out_of_range_ = true;
    }
    QLineEdit::keyPressEvent(ke);
    ke->accept();
    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
}

void OnyxLineEdit::mouseReleaseEvent(QMouseEvent * event)
{
    event->accept();
    emit getFocus(this);
    emit setCheckByMouse(this);
}

}
