#include "onyx/screen/screen_proxy.h"
#include "keyboard_dialog.h"

namespace text_editor
{

KeyboardDialog::KeyboardDialog(QWidget *parent, const QString & title)
    : OnyxDialog(parent)
    , hbox_(&content_widget_)
    , keyboard_(0)
    , text_edit_("", this)
    , ok_button_(tr("OK"), this)
    , clear_button_(tr("Clear"), this)
{
    createLayout();
    title_text_label_.setText(title);

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

    ok_button_.installEventFilter(this);
    clear_button_.installEventFilter(this);
    text_edit_.installEventFilter(this);
    keyboard_.installEventFilter(this);
}

int KeyboardDialog::popup()
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

    // adjust position
    int x = Shadows::PIXELS;
    if (!keyboard_.isVisible())
    {
        x = parentWidget()->width() - width - Shadows::PIXELS;
    }
    int y = parentWidget()->height() - height() - Shadows::PIXELS - 30;
    move(x, y);
    return exec();
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
    int x = Shadows::PIXELS;
    if (!keyboard_.isVisible())
    {
        x = parentWidget()->width() - width() - Shadows::PIXELS;
    }
    int y = parentWidget()->height() - height() - Shadows::PIXELS - 30;
    move(x, y);
}

void KeyboardDialog::onTimeout()
{
    onyx::screen::instance().updateScreen(onyx::screen::ScreenProxy::GU);
}

void KeyboardDialog::onOKClicked()
{
    shadows_.show(false);
    done(QDialog::Accepted);
}

void KeyboardDialog::onClose()
{
    shadows_.show(false);
    done(QDialog::Rejected);
}

QString KeyboardDialog::inputText() const
{
    return text_edit_.text();
}

bool KeyboardDialog::eventFilter(QObject *obj, QEvent *event)
{
    QWidget * wnd = 0;
    bool changed = false;
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (obj == &text_edit_)
        {
            if (key_event->key() == Qt::Key_Down)
            {
                keyboard_.setFocus();
                changed = true;
            }
            else if (key_event->key() == Qt::Key_Right && ok_button_.isEnabled())
            {
                ok_button_.setFocus();
                changed = true;
            }
        }
        else if (obj == &ok_button_)
        {
            if (key_event->key() == Qt::Key_Down)
            {
                keyboard_.setFocus();
                changed = true;
            }
            else if (key_event->key() == Qt::Key_Right && clear_button_.isEnabled())
            {
                clear_button_.setFocus();
                changed = true;
            }
            else if (key_event->key() == Qt::Key_Left)
            {
                text_edit_.setFocus();
                changed = true;
            }
        }
        else if (obj == &clear_button_)
        {
            if (key_event->key() == Qt::Key_Down)
            {
                keyboard_.setFocus();
                changed = true;
            }
            else if (key_event->key() == Qt::Key_Left && ok_button_.isEnabled())
            {
                ok_button_.setFocus();
                changed = true;
            }
        }
        else if (obj == &keyboard_)
        {
            if (key_event->key() == Qt::Key_Down ||
                key_event->key() == Qt::Key_Up ||
                key_event->key() == Qt::Key_Left ||
                key_event->key() == Qt::Key_Right)
            {
                wnd = ui::moveFocus(this, key_event->key());
                if (wnd)
                {
                    wnd->setFocus();
                }
                changed = true;
            }
        }

        if (changed)
        {
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::DW);
        }
        return changed;

    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

}
