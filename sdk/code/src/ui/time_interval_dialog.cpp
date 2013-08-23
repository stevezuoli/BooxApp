#include "onyx/ui/keyboard_navigator.h"
#include "onyx/ui/time_interval_dialog.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

namespace ui
{
const int MARGINS = 5;

TimeIntervalDialog::TimeIntervalDialog(QWidget *parent)
    : OnyxDialog(parent)
    , value_(0)
    , total_(0)
    , validator_(this)
    , number_edit_(this)
    , number_widget_(this)
{
    OnyxDialog::updateTitle(QCoreApplication::tr("Slide Show Interval"));
    setModal(true);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Button);

    connect(&number_widget_,
            SIGNAL(numberClicked(const int)),
            this,
            SLOT(onNumberClicked(const int)));
    connect(&number_widget_,
            SIGNAL(okClicked()),
            this,
            SLOT(onOKClicked()));
    connect(&number_widget_,
            SIGNAL(backspaceClicked()),
            this,
            SLOT(onBackspaceClicked()));

    createLayout();
    onyx::screen::watcher().addWatcher(this);
}

TimeIntervalDialog::~TimeIntervalDialog(void)
{
}

/// Set the number.
void TimeIntervalDialog::setValue(const int value)
{
    value_ = value;
    QString text;
    text.setNum(value_);
    number_edit_.setText(text);
}

int TimeIntervalDialog::popup(const int value, const int total)
{
    total_ = total;

    validator_.setRange(1, total);
    number_edit_.setValidator(&validator_);

    onyx::screen::instance().enableUpdate(false);
    shadows_.show(true);
    show();

    number_edit_.selectAll();
    int w = contentsRect().width() - 2 * MARGINS - second_label.size().width();
    number_edit_.setFixedWidth(w);
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
    onyx::screen::instance().enableUpdate(true);
    return QDialog::exec();
}

void TimeIntervalDialog::createLayout()
{
    // Create number label.
    //number_edit_.setFrameShadow(QFrame::Raised);
    number_edit_.setAlignment(Qt::AlignRight);
    number_edit_.setText("3");
    second_label.setMargin(0);
    second_label.setText(QCoreApplication::tr("second (s)"));
    number_widget_.setOkButtonFocus();

    QHBoxLayout *valueLayout = new QHBoxLayout;
    valueLayout->setContentsMargins(0, 0, 0, 0);
    valueLayout->addWidget(&number_edit_);
    valueLayout->addWidget(&second_label);

    // label.
    // setValue(value_);

    // Add to layout.
    vbox_.setSizeConstraint(QLayout::SetDefaultConstraint);
    vbox_.setContentsMargins(MARGINS, MARGINS, MARGINS, MARGINS);
    number_edit_.setFocusPolicy(Qt::NoFocus);
    //vbox_.addWidget(&number_edit_, 0, Qt::AlignRight|Qt::AlignVCenter);
    vbox_.addLayout(valueLayout);
    vbox_.addSpacing(MARGINS);
    vbox_.addWidget(&number_widget_, 0, Qt::AlignCenter);
}

void TimeIntervalDialog::onOKClicked()
{
    value_ = number_edit_.text().toInt();
    if (value_ > 0 && value_ <= total_)
    {
        done(QDialog::Accepted);
    }
    else
    {
        done(QDialog::Rejected);
    }
}

void TimeIntervalDialog::keyPressEvent(QKeyEvent *e)
{
    e->accept();
}

void TimeIntervalDialog::keyReleaseEvent(QKeyEvent *e)
{
    //   copy from QDialog::keyPressEvent
    //   Calls reject() if Escape is pressed. Simulates a button
    //   click for the default button if Enter is pressed. Move focus
    //   for the arrow keys. Ignore the rest.
#ifdef Q_WS_MAC
    if(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period)
    {
        reject();
    }
    else
#endif
    // Check the current selected type.
    e->accept();
    switch (e->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Down:
        {
            QWidget *wnd = ui::moveFocus(this, e->key());
            if (wnd)
            {
                wnd->setFocus();
            }
        }
        break;
    case Qt::Key_Escape:
    case Device_Menu_Key:
        reject();
        break;
    case Qt::Key_Return:
        break;
    }
}

bool TimeIntervalDialog::event(QEvent * event)
{
    bool ret = OnyxDialog::event(event);
    if (event->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled())
    {
        onyx::screen::watcher().enqueue(this,
                                        onyx::screen::ScreenProxy::DW,
                                        onyx::screen::ScreenCommand::WAIT_ALL);
    }
    return ret;
}

void TimeIntervalDialog::onCloseClicked()
{
    onyx::screen::instance().enableUpdate(false);
    reject();
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
}

void TimeIntervalDialog::onNumberClicked(const int number)
{
    // Disable the parent widget to update screen.
    QString text("%1");
    text = text.arg(number);
    QKeyEvent * key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_0 + number, Qt::NoModifier, text);
    QApplication::postEvent(&number_edit_, key_event);

    update();
}

void TimeIntervalDialog::onBackspaceClicked()
{
    QKeyEvent * key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier, "");
    QApplication::postEvent(&number_edit_, key_event);

    update();
}

} //ui
