#ifndef EXPLORER_DATE_DIALOG_H_
#define EXPLORER_DATE_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui.h"
#include "onyx/ui/calendar.h"

using namespace ui;

namespace explorer
{

namespace view
{

/// Date settings widget. Not sure we should place it in the ui library.
/// So far, only explorer use it.
class DateDialog : public ui::OnyxDialog
{
    Q_OBJECT

public:
    DateDialog(QWidget *parent);
    ~DateDialog(void);

public:
    int exec();

protected Q_SLOTS:
    void onCloseClicked(bool);

private:
    void createLayout();
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    bool eventFilter(QObject *obj, QEvent *event);
    bool event(QEvent*);

    void updateBySystemDate();
    void changeSystemDate();

    void onKeyUpOrDown(int);

    OnyxLineEdit* nextLineEdit(const OnyxLineEdit *,int direct);
private Q_SLOTS:
    void onEditFocus(OnyxLineEdit *edit);
    void onOkClicked();

private:
    QVBoxLayout ver_layout_;

    QDate date_;

    QHBoxLayout time_label_layout_;
    QHBoxLayout time_edit_layout_;
    
    OnyxLabel day_label_;
    OnyxLineEdit day_edit_;

    OnyxLabel month_label_;
    OnyxLineEdit month_edit_;

    OnyxLabel year_label_;
    OnyxLineEdit year_edit_;

    OnyxLabel hour_label_;
    OnyxLineEdit hour_edit_;

    OnyxLabel minute_label_;
    OnyxLineEdit minute_edit_;

    OnyxLabel second_label_;
    OnyxLineEdit second_edit_;

    OnyxLineEdit * receiver_;
    
    QIntValidator day_validator_;
    QIntValidator month_validator_;
    QIntValidator year_validator_;


    QIntValidator hour_validator_;
    QIntValidator minute_validator_;
    QIntValidator second_validator_;

    int last_direction_;
};

}   // namespace view

}   // namespace explorer

#endif
