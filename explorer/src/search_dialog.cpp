#include "onyx/screen/screen_proxy.h"
#include "search_dialog.h"

namespace explorer
{

namespace view
{

SearchContext::SearchContext(void)
    : pattern_()
    , include_(false)
    , stop_(false)
    , node_(0)
{
}

SearchContext::~SearchContext(void)
{
}

void SearchContext::reset()
{
    pattern_.clear();
    include_ = false;
    stop_ = false;
    node_ = 0;
}

void SearchContext::setPattern(const QString &pattern)
{
    pattern_ = pattern;
}


SearchDialog::SearchDialog(QWidget *parent, SearchContext & ctx)
    : OnyxDialog(parent)
    , hbox_(&content_widget_)
    , text_edit_("", this)
    , search_button_(QApplication::tr("Search"), this)
    , clear_button_(QApplication::tr("Clear"), this)
    , sub_dir_(QApplication::tr("Including Sub Directories"), this)
    , keyboard_(0)
    , searching_timer_(this)
    , ctx_(ctx)
    , searching_(false)
    , update_parent_(false)
    , update_whole_widget_(false)
    , update_state_(UPDATE_NONE)
{
    createLayout();

    // Widget attributes.
    setModal(true);
    setFocusPolicy(Qt::NoFocus);

    // Timer.
    connect(&searching_timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));

    text_edit_.installEventFilter(this);
    search_button_.installEventFilter(this);
    clear_button_.installEventFilter(this);
    sub_dir_.installEventFilter(this);
    keyboard_.installEventFilter(this);
}

SearchDialog::~SearchDialog(void)
{
}

void SearchDialog::createLayout()
{
    title_icon_label_.setPixmap(QPixmap(":/images/dictionary_search.png"));

    content_widget_.setFixedHeight(WIDGET_HEIGHT + 4 * SPACING);

    // hbox to layout line edit and buttons.
    hbox_.setContentsMargins(SPACING, SPACING, SPACING, SPACING);
    hbox_.setSpacing(SPACING * 4);

    // Line edit.
    text_edit_.setFixedHeight(WIDGET_HEIGHT);
    text_edit_.setText(ctx_.pattern());
    hbox_.addWidget(&text_edit_, 400);

    // Buttons.
    search_button_.setFixedHeight(WIDGET_HEIGHT);
    clear_button_.setFixedHeight(WIDGET_HEIGHT);
    search_button_.setFocusPolicy(Qt::NoFocus);
    clear_button_.setFocusPolicy(Qt::NoFocus);

    hbox_.addWidget(&search_button_);
    hbox_.addWidget(&clear_button_);

    sub_dir_.setAutoExclusive(false);
    sub_dir_.selectOnClicked(false);
    sub_dir_.setChecked(ctx_.includeSubDirs());
    vbox_.addWidget(&sub_dir_);

    // keyboard.
    keyboard_.attachReceiver(this);
    vbox_.addWidget(&keyboard_);

    // Setup connections.
    connect(&search_button_, SIGNAL(clicked()), this, SLOT(onSearchClicked()), Qt::QueuedConnection);
    connect(&clear_button_, SIGNAL(clicked()), &text_edit_, SLOT(clear()));
    connect(&text_edit_, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
}

/// Update all widget according to the specified parameter.
void SearchDialog::updateChildrenWidgets(bool searching)
{
    search_button_.setEnabled(!searching);
    clear_button_.setEnabled(!searching);
    text_edit_.setEnabled(!searching);
    keyboard_.setVisible(!searching);
    sub_dir_.setEnabled(!searching);
}

void SearchDialog::onSearchClicked()
{
    // Update widget.
    searching_ = true;
    update_parent_ = true;
    updateChildrenWidgets(true);
    ctx_.setPattern(text_edit_.text());
    ctx_.includeSubDirs(sub_dir_.isChecked());
    ctx_.mutable_stop() = false;
    searching_timer_.start(3* 1000);

    // Add * if necessary.
    QString pattern = ctx_.pattern();
    if (!pattern.startsWith("*"))
    {
        pattern.prepend("*");
    }
    if (!pattern.endsWith("*"))
    {
        pattern.append("*");
    }
    BranchNode * node = ctx_.node();
    QStringList filters(pattern);
    if (node->search(filters, ctx_.includeSubDirs(), ctx_.mutable_stop()))
    {
        accept();
    }
    else
    {
        // TODO, should we clear the search result?
        reject();
    }
}

void SearchDialog::onCloseClicked()
{
    // Make sure caller stop searching.
    ctx_.stop(true);
    shadows_.show(false);
    done(QDialog::Rejected);
}

void SearchDialog::onTextChanged(const QString& text)
{
    if ((search_button_.isEnabled() && text.isEmpty()) ||
        (!search_button_.isEnabled() && !text.isEmpty()))
    {
        update_whole_widget_ = true;
    }
    else
    {
        update_whole_widget_ = false;
    }
    search_button_.setEnabled(!text.isEmpty());
    clear_button_.setEnabled(!text.isEmpty());

}

/// This function is called by parent widget to display the search widget.
int SearchDialog::popup(const int bottom_margin)
{
    onyx::screen::instance().enableUpdate(false);

    shadows_.show(true);
    if (isHidden())
    {
        show();
    }
    resize(parentWidget()->width() - 2 * shadows_.PIXELS, height());

    // TODO, need to figure out the height of status bar. Or we can ask parent widget
    // to place the position.
    move(shadows_.PIXELS,
         parentWidget()->height() - height() - shadows_.PIXELS - bottom_margin);

    if (text_edit_.text().isEmpty())
    {
        text_edit_.setText(ctx_.pattern());
    }
    onTextChanged(text_edit_.text());
    updateTitle(QApplication::tr("Search Files..."));
    onyx::screen::instance().enableUpdate(true);
    return exec();
}

void SearchDialog::mouseMoveEvent(QMouseEvent *me)
{
    me->accept();
}

void SearchDialog::mousePressEvent(QMouseEvent *me)
{
    me->accept();
}

void SearchDialog::mouseReleaseEvent(QMouseEvent *me)
{
    me->accept();
}

void SearchDialog::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    int key = ke->key();
    if (key == Qt::Key_Escape || key == ui::Device_Menu_Key)
    {
        onCloseClicked();
        return;
    }
}

/// The keyPressEvent could be sent from virtual keyboard.
void SearchDialog::keyPressEvent(QKeyEvent * ke)
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
    onyx::screen::instance().enableUpdate(false);

    if (text_edit_.hasFocus() ||
        (ke->key() != Qt::Key_Down &&
         ke->key() != Qt::Key_Up &&
         ke->key() != Qt::Key_Left &&
         ke->key() != Qt::Key_Right))
    {
        QKeyEvent * key_event = new QKeyEvent(ke->type(), ke->key(), ke->modifiers(), ke->text());
        QApplication::postEvent(&text_edit_, key_event);
    }

    while (QApplication::hasPendingEvents())
    {
        QApplication::processEvents();
    }

    onyx::screen::instance().enableUpdate(true);

    if (update_whole_widget_)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
    }
    else
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW, true, onyx::screen::ScreenCommand::WAIT_ALL);
    }
}

void SearchDialog::updateTitle(const QString &message)
{
    if (message.isEmpty())
    {
        OnyxDialog::updateTitle(QApplication::tr("Search..."));
    }
    else
    {
        OnyxDialog::updateTitle(message);
    }
}

bool SearchDialog::event(QEvent * event)
{
    bool ret = OnyxDialog::event(event);
    if (event->type() == QEvent::UpdateRequest)
    {
        if (update_state_ == UPDATE_NONE)
        {
            QTimer::singleShot(0, this, SLOT(forceUpdate()));
        }

        if (onyx::screen::instance().isUpdateEnabled())
        {
            onyx::screen::instance().sync(&shadows_.hor_shadow());
            onyx::screen::instance().sync(&shadows_.ver_shadow());
            if (UPDATE_INITIAL == update_state_)
            {
                onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GC);
                update_state_ = UPDATE_NORMAL;
            }
            else if (update_state_ == UPDATE_NORMAL)
            {
                if (update_parent_)
                {
                    update_parent_ = false;
                    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GC);
                }
                else
                {
                    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
                }
            }
        }
    }
    return ret;
}

bool SearchDialog::eventFilter(QObject *obj, QEvent *event)
{
     QWidget * wnd = 0;
     if (event->type() == QEvent::KeyRelease)
     {
         QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
         if (obj == &text_edit_)
         {
             if (key_event->key() == Qt::Key_Down)
             {
                 sub_dir_.setFocus();
                 return true;
             }
             else if (key_event->key() == Qt::Key_Right && search_button_.isEnabled())
             {
                 search_button_.setFocus();
                 return true;
             }
         }
         else if (obj == &search_button_)
         {
             if (key_event->key() == Qt::Key_Down)
             {
                 sub_dir_.setFocus();
                 return true;
             }
             else if (key_event->key() == Qt::Key_Right && clear_button_.isEnabled())
             {
                 clear_button_.setFocus();
                 return true;
             }
             else if (key_event->key() == Qt::Key_Left)
             {
                 text_edit_.setFocus();
                 return true;
             }
         }
         else if (obj == &clear_button_)
         {
             if (key_event->key() == Qt::Key_Down)
             {
                 sub_dir_.setFocus();
                 return true;
             }
             else if (key_event->key() == Qt::Key_Left && search_button_.isEnabled())
             {
                 search_button_.setFocus();
                 return true;
             }
         }
         else if (obj == &sub_dir_)
         {
             if (key_event->key() == Qt::Key_Down)
             {
                 wnd = ui::moveFocus(this, key_event->key());
                 if (wnd)
                 {
                     wnd->setFocus();
                 }
                 return true;
             }
             else if (key_event->key() == Qt::Key_Up)
             {
                 text_edit_.setFocus();
                 return true;
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
                 return true;
             }
        }
     }
     // standard event processing
     return QObject::eventFilter(obj, event);
}

void SearchDialog::moveEvent(QMoveEvent *e)
{
    OnyxDialog::moveEvent(e);
}

void SearchDialog::resizeEvent(QResizeEvent *e)
{
    OnyxDialog::resizeEvent(e);
}

void SearchDialog::forceUpdate()
{
    update_state_ = UPDATE_INITIAL;
    update();
}

void SearchDialog::onTimeout()
{
    // Some simple animation. Using the fast update if possible.
}

}   // namespace view

}   // namespace explorer

