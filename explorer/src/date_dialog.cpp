
#include "onyx/screen/screen_proxy.h"
#include "date_dialog.h"

namespace explorer
{

namespace view
{

const QString LABEL_STYLE = "           \
QLabel                                  \
{                                       \
     padding: 0px;                      \
     background: black;                 \
     font: 24px ;                       \
     color: white;                      \
 }";

DateDialog::DateDialog(QWidget *parent)
    : OnyxDialog(parent)
    , ver_layout_(&content_widget_)
    , date_(QDate::currentDate())
    , time_label_layout_(0)
    , time_edit_layout_(0)
    , day_label_(QApplication::tr("Day"),0)
    , day_edit_(0)
    , month_label_(QApplication::tr("Month"))
    , month_edit_(0)
    , year_label_(QApplication::tr("Year"))
    , year_edit_(0)
    , hour_label_(QApplication::tr("Hour"))
    , hour_edit_(0)
    , minute_label_(QApplication::tr("Minute"))
    , minute_edit_(0)
    , second_label_(QApplication::tr("Second"))
    , second_edit_(0)
    , receiver_(0)
    , day_validator_(1, 31, 0)
    , month_validator_(1, 12, 0)
    , year_validator_(2000, 2099, 0)
    , hour_validator_(0, 23, 0)
    , minute_validator_(0, 59, 0)
    , second_validator_(0, 59, 0)
    , last_direction_(Qt::Key_Down)
{
    setAutoFillBackground(false);
    setModal(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    createLayout();
    setFixedSize(590, 150);
}

DateDialog::~DateDialog(void)
{
}

int DateDialog::exec()
{
    updateBySystemDate();
    day_edit_.setFocus();
    shadows_.show(true);
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(0, outbounding(parentWidget()), onyx::screen::ScreenProxy::GC, false, onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void DateDialog::keyPressEvent(QKeyEvent *ke)
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
}

void DateDialog::keyReleaseEvent(QKeyEvent *ke)
{
    QWidget *wnd = 0;
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Down:
        break;
    case Qt::Key_Return:
        break;
    case Qt::Key_Escape:
        reject();
        break;
    }
}

bool DateDialog::eventFilter(QObject *obj, QEvent *event)
{
    bool ret = OnyxDialog::eventFilter(obj, event);
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        switch (key_event->key())
        {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            {
                onOkClicked();
                return true;
            }
            break;
        case Qt::Key_PageDown:
        case Qt::Key_Up:
            {
                onKeyUpOrDown(Qt::Key_Up);
                return true;
            }
            break;
        case Qt::Key_PageUp:
        case Qt::Key_Down:
            {
                onKeyUpOrDown(Qt::Key_Down);
                return true;
            }
            break;
        case Qt::Key_Left:
            {
                OnyxLineEdit* wnd = nextLineEdit(qobject_cast<const OnyxLineEdit*>(this->focusWidget()),0);
                if (wnd)
                {
                    wnd->setFocus();
                }
                return true;
            }
            break;
        case Qt::Key_Right:
            {
                OnyxLineEdit* wnd = nextLineEdit(qobject_cast<const OnyxLineEdit*>(this->focusWidget()),1);
                if (wnd)
                {
                    wnd->setFocus();
                }
                return true;
            }
            break;
        default:
            break;
        }
    }
    return ret;
}

void DateDialog::onKeyUpOrDown(int direct)
{
    QWidget *current = this->focusWidget();
    QAbstractButton *btn = qobject_cast<QAbstractButton*>(current);
    if (btn == 0)
    {
        QLineEdit *edit = qobject_cast<QLineEdit*>(current);
        if (edit)
        {
             QString tmp_str;
             int tmp_int;
             if (Qt::Key_Down == direct)
                 tmp_int = edit->text().toInt() - 1;
             else if (Qt::Key_Up == direct)
                 tmp_int = edit->text().toInt() + 1;
             else
                 return;

             if (edit == &hour_edit_ ||
                 edit == &minute_edit_ ||
                 edit == &second_edit_) 
             {
                 // hour minute second
                 if (tmp_int < qobject_cast<const QIntValidator*>(edit->validator())->bottom())
                    tmp_int = qobject_cast<const QIntValidator*>(edit->validator())->top();
                 else if (tmp_int > qobject_cast<const QIntValidator*>(edit->validator())->top())
                    tmp_int = qobject_cast<const QIntValidator*>(edit->validator())->bottom();
             }
             else 
             {
                 // day month year
                 if (edit == &day_edit_)
                 {
                     // day
                     if (tmp_int < qobject_cast<const QIntValidator*>(edit->validator())->bottom())
                         tmp_int = date_.daysInMonth();
                     else if (tmp_int > date_.daysInMonth())
                         tmp_int = qobject_cast<const QIntValidator*>(edit->validator())->bottom();
                     date_.setDate(date_.year(),date_.month(),tmp_int);
                 }
                 else
                 {
                     // month or year
                     if (tmp_int < qobject_cast<const QIntValidator*>(edit->validator())->bottom())
                         tmp_int = qobject_cast<const QIntValidator*>(edit->validator())->top();
                     else if (tmp_int > qobject_cast<const QIntValidator*>(edit->validator())->top())
                         tmp_int = qobject_cast<const QIntValidator*>(edit->validator())->bottom();
                     
                     if (edit == &month_edit_)
                     {
                         if (false == QDate::isValid(date_.year(),tmp_int,date_.day()))
                         {
                             // month change, date invalid
                             QDate tmp_date(date_.year(),tmp_int,1);
                             date_.setDate(date_.year(),tmp_int,tmp_date.daysInMonth());
                             QString tmp_day;
                             tmp_day.setNum(date_.daysInMonth());
                             day_edit_.setText(tmp_day);
                         }
                         else
                             date_.setDate(date_.year(),tmp_int,date_.day());
                     }
                     else if (edit == &year_edit_)
                     {
                         if (false == QDate::isValid(tmp_int,date_.month(),date_.day()))
                         {
                             // year change, date invalid
                             QDate tmp_date(tmp_int,date_.month(),1);
                             date_.setDate(tmp_int,date_.month(),tmp_date.daysInMonth());
                             QString tmp_day;
                             tmp_day.setNum(date_.daysInMonth());
                             day_edit_.setText(tmp_day);
                         }
                         else
                             date_.setDate(tmp_int,date_.month(),date_.day());
                     }

                 }
             }  
             tmp_str.setNum(tmp_int);
             edit->setText(tmp_str);
             onEditFocus(qobject_cast<OnyxLineEdit*>(edit));
        }
    }
}


OnyxLineEdit* DateDialog::nextLineEdit(const OnyxLineEdit *cur_edit,int direct)
{
    if (direct == 0) // to left
    {
        if (cur_edit == &hour_edit_) return &year_edit_;
        else if (cur_edit == &minute_edit_) return &hour_edit_;
        else if (cur_edit == &second_edit_) return &minute_edit_;
        else if (cur_edit == &day_edit_) return &second_edit_;
        else if (cur_edit == &month_edit_) return &day_edit_;
        else if (cur_edit == &year_edit_) return &month_edit_;
    }
    else if (direct == 1) // to right
    {
        if (cur_edit == &hour_edit_) return &minute_edit_;
        else if (cur_edit == &minute_edit_) return &second_edit_;
        else if (cur_edit == &second_edit_) return &day_edit_;
        else if (cur_edit == &day_edit_) return &month_edit_;
        else if (cur_edit == &month_edit_) return &year_edit_;
        else if (cur_edit == &year_edit_) return &hour_edit_;
    }
    return NULL;
}

void DateDialog::createLayout()
{
    // Retrieve the values from system status.
    updateTitle(QApplication::tr("Date Settings"));
    updateTitleIcon(QPixmap(":/images/small/date.png"));

    // content_widget_.setBackgroundRole(QPalette::Button);

    // The big layout.
    ver_layout_.setContentsMargins(SPACING, 0, SPACING, 0);

    ver_layout_.addSpacing(10);
    ver_layout_.addLayout(&time_label_layout_);
    ver_layout_.addLayout(&time_edit_layout_);
    ver_layout_.addSpacing(10);

    // Time layout
    day_label_.setStyleSheet(LABEL_STYLE);
    day_label_.setAlignment(Qt::AlignCenter);

    month_label_.setStyleSheet(LABEL_STYLE);
    month_label_.setAlignment(Qt::AlignCenter);

    year_label_.setStyleSheet(LABEL_STYLE);
    year_label_.setAlignment(Qt::AlignCenter);

    hour_label_.setStyleSheet(LABEL_STYLE);
    hour_label_.setAlignment(Qt::AlignCenter);

    minute_label_.setStyleSheet(LABEL_STYLE);
    minute_label_.setAlignment(Qt::AlignCenter);

    second_label_.setStyleSheet(LABEL_STYLE);
    second_label_.setAlignment(Qt::AlignCenter);

    time_label_layout_.setSpacing(0);
    time_label_layout_.setContentsMargins(0, 0, 0, 0);
    time_label_layout_.addWidget(&day_label_);
    time_label_layout_.addWidget(&month_label_);
    time_label_layout_.addWidget(&year_label_);
    time_label_layout_.addWidget(&hour_label_);
    time_label_layout_.addWidget(&minute_label_);
    time_label_layout_.addWidget(&second_label_);

    day_edit_.setAlignment(Qt::AlignCenter);
    month_edit_.setAlignment(Qt::AlignCenter);
    year_edit_.setAlignment(Qt::AlignCenter);
    hour_edit_.setAlignment(Qt::AlignCenter);
    minute_edit_.setAlignment(Qt::AlignCenter);
    second_edit_.setAlignment(Qt::AlignCenter);
    time_edit_layout_.addWidget(&day_edit_);
    time_edit_layout_.addWidget(&month_edit_);
    time_edit_layout_.addWidget(&year_edit_);
    time_edit_layout_.addWidget(&hour_edit_);
    time_edit_layout_.addWidget(&minute_edit_);
    time_edit_layout_.addWidget(&second_edit_);

    // Edit validator.
    day_edit_.setValidator(&day_validator_);
    month_edit_.setValidator(&month_validator_);
    year_edit_.setValidator(&year_validator_);
    hour_edit_.setValidator(&hour_validator_);
    minute_edit_.setValidator(&minute_validator_);
    second_edit_.setValidator(&second_validator_);

    // Event Filter
    day_edit_.installEventFilter(this);
    month_edit_.installEventFilter(this);
    year_edit_.installEventFilter(this);
    hour_edit_.installEventFilter(this);
    minute_edit_.installEventFilter(this);
    second_edit_.installEventFilter(this);

    // connections.
    connect(&day_edit_, SIGNAL(getFocus(OnyxLineEdit*)), this, SLOT(onEditFocus(OnyxLineEdit*)));
    connect(&month_edit_, SIGNAL(getFocus(OnyxLineEdit*)), this, SLOT(onEditFocus(OnyxLineEdit*)));
    connect(&year_edit_, SIGNAL(getFocus(OnyxLineEdit*)), this, SLOT(onEditFocus(OnyxLineEdit*)));
    connect(&hour_edit_, SIGNAL(getFocus(OnyxLineEdit*)), this, SLOT(onEditFocus(OnyxLineEdit*)));
    connect(&minute_edit_, SIGNAL(getFocus(OnyxLineEdit*)), this, SLOT(onEditFocus(OnyxLineEdit*)));
    connect(&second_edit_, SIGNAL(getFocus(OnyxLineEdit*)), this, SLOT(onEditFocus(OnyxLineEdit*)));
}

bool DateDialog::event(QEvent* qe)
{
    bool ret = OnyxDialog::event(qe);
    if (qe->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled())
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW);
    }
    return ret;
}

void DateDialog::updateBySystemDate()
{
    QTime time = QDateTime::currentDateTime().time();
    QDate date = QDateTime::currentDateTime().date();
    QString message;

    // time.
    message.setNum(time.hour());
    hour_edit_.setText(message);

    message.setNum(time.minute());
    minute_edit_.setText(message);

    message.setNum(time.second());
    second_edit_.setText(message);
    
    // day.
    message.setNum(date.day());
    day_edit_.setText(message);

    message.setNum(date.month());
    month_edit_.setText(message);

    message.setNum(date.year());
    year_edit_.setText(message);
}

/// Change system date by using the value from edit entries.
void DateDialog::changeSystemDate()
{
    // Use local time finally. The hardware clock is always kept in UTC time
    QDateTime new_date( 
                        QDate(year_edit_.text().toInt(), month_edit_.text().toInt(), day_edit_.text().toInt()),
                        QTime(hour_edit_.text().toInt(), minute_edit_.text().toInt(), second_edit_.text().toInt()));

    qDebug("New date %s ", qPrintable(new_date.toString("yyyy-MM-dd hh:mm")));
    sys::SystemConfig::setDate(
        new_date.date().year(),
        new_date.date().month(),
        new_date.date().day(),
        new_date.time().hour(),
        new_date.time().minute(),
        new_date.time().second());
}

void DateDialog::onEditFocus(OnyxLineEdit *edit)
{
    edit->selectAll();
    receiver_ = edit;
}

void DateDialog::onOkClicked()
{
    changeSystemDate();
    accept();
}

void DateDialog::onCloseClicked(bool)
{
    onyx::screen::instance().enableUpdate(false);
    reject();
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GC);
}

}   // namespace view

}   // namespace explorer

