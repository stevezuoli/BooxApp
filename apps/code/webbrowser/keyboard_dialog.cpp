#include "onyx/screen/screen_proxy.h"

#include "keyboard_dialog.h"

namespace webbrowser
{

static const int WIDGET_HEIGHT = 300;

KeyboardDialog::KeyboardDialog(QWidget *parent)
    : OnyxDialog(parent)
    , hbox_(&content_widget_)
    , keyboard_(0)
    , text_edit_("", this)
    , ok_button_(tr("OK"), this)
    , clear_button_(tr("Clear"), this)
{
    createLayout();

    // Widget attributes.
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    setModal(true);
    setFocusPolicy(Qt::NoFocus);
}

KeyboardDialog::~KeyboardDialog(void)
{
}

void KeyboardDialog::createLayout()
{
    title_widget_.hide();
    content_widget_.setFixedHeight(WIDGET_HEIGHT + 4 * SPACING);

    // hbox to layout line edit and buttons.
    hbox_.setContentsMargins(SPACING, SPACING, SPACING, SPACING);
    hbox_.setSpacing(SPACING * 4);

    // Line edit.
    text_edit_.setFixedHeight(WIDGET_HEIGHT);
    hbox_.addWidget(&text_edit_, 400);

    // Buttons.
    ok_button_.setFixedHeight(WIDGET_HEIGHT);
    clear_button_.setFixedHeight(WIDGET_HEIGHT);

    ok_button_.setFocusPolicy(Qt::NoFocus);
    clear_button_.setFocusPolicy(Qt::NoFocus);

    hbox_.addWidget(&ok_button_);
    hbox_.addWidget(&clear_button_);

    // keyboard.
    keyboard_.attachReceiver(this);
    vbox_.addWidget(&keyboard_);

    // Setup connections.
    connect(&ok_button_, SIGNAL(clicked()), this, SLOT(onOKClicked()), Qt::QueuedConnection);
    connect(&clear_button_, SIGNAL(clicked()), &text_edit_, SLOT(clear()));
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

void KeyboardDialog::ensureVisible()
{
    shadows_.show(true);
    if (isHidden())
    {
        show();
    }

    QRect parent_rect = parentWidget()->rect();
    int border = Shadows::PIXELS;
    int width = parent_rect.width() - border * 2;
    if (size().width() != width)
    {
        resize(width, height());
    }

    adjustPosition();
}

QSize KeyboardDialog::sizeHint () const
{
    int height = childrenRect().height();
    if (keyboard_.isHidden())
    {
        return QSize(300, height + 4 * SPACING);
    }
    else
    {
        return QSize(parentWidget()->width(), parentWidget()->height());
    }
}

QSize KeyboardDialog::minimumSize () const
{
    return sizeHint();
}

QSize KeyboardDialog::maximumSize () const
{
    return sizeHint();
}

void KeyboardDialog::mouseMoveEvent(QMouseEvent *me)
{
    me->accept();
}

void KeyboardDialog::mousePressEvent(QMouseEvent *me)
{
    me->accept();
}

void KeyboardDialog::mouseReleaseEvent(QMouseEvent *me)
{
    me->accept();
    qDebug("Mouse Release on KeyboardDialog");
    if (!rect().contains(me->pos()))
    {
        reject();
    }
}

void KeyboardDialog::keyReleaseEvent(QKeyEvent *ke)
{
    int key = ke->key();
    if (key == Qt::Key_Escape || key == ui::Device_Menu_Key)
    {
        onClose();
        return;
    }
}

/// The keyPressEvent could be sent from virtual keyboard.
void KeyboardDialog::keyPressEvent(QKeyEvent * ke)
{
    ke->accept();
    if (ke->key() == Qt::Key_Enter)
    {
        return;
    }
    else if (ke->key() == Qt::Key_Shift || ke->key() == Qt::Key_CapsLock)
    {
        return;
    }

    // Disable the parent widget to update screen.
    QKeyEvent * key_event = new QKeyEvent(ke->type(), ke->key(), ke->modifiers(), ke->text());
    onyx::screen::instance().enableUpdate(false);

    QApplication::postEvent(&text_edit_, key_event);
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);

    // Update the line edit.
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW, true);
}

void KeyboardDialog::adjustPosition()
{
    int x = Shadows::PIXELS;
    if (!keyboard_.isVisible())
    {
        x = parentWidget()->width() - width() - Shadows::PIXELS;
    }
    int y = parentWidget()->height() - height() - Shadows::PIXELS;
    move(x, y);
}

bool KeyboardDialog::event(QEvent * event)
{
    bool ret = OnyxDialog::event(event);
    if (event->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled())
    {
        onyx::screen::instance().sync(&shadows_.hor_shadow());
        onyx::screen::instance().sync(&shadows_.ver_shadow());
        onyx::screen::instance().updateWidget(parentWidget(), onyx::screen::ScreenProxy::GC);
        event->accept();
        return true;
    }
    return ret;
}

void KeyboardDialog::moveEvent(QMoveEvent *e)
{
    OnyxDialog::moveEvent(e);
}

void KeyboardDialog::resizeEvent(QResizeEvent *e)
{
    OnyxDialog::resizeEvent(e);
}

void KeyboardDialog::clearText()
{
    text_edit_.clear();
}

void KeyboardDialog::onTimeout()
{
    onyx::screen::instance().updateScreen(onyx::screen::ScreenProxy::GU);
}

void KeyboardDialog::onOKClicked()
{
    emit textFinsihed(text_edit_.text());
}

void KeyboardDialog::onClose()
{
    text_edit_.clear();
    shadows_.show(false);
    done(QDialog::Rejected);
}

}
