#ifndef ONYX_TIME_INTERVAL_DIALOG_H
#define ONYX_TIME_INTERVAL_DIALOG_H

#include "onyx_dialog.h"
#include "number_widget.h"
#include "line_edit.h"
#include "label.h"
#include "context_dialog_base.h"

namespace ui
{

class TimeIntervalDialog : public OnyxDialog
{
    Q_OBJECT
public:
    explicit TimeIntervalDialog(QWidget *parent);
    ~TimeIntervalDialog();

    /// Set value.
    void setValue(const int value);

    /// Retrieve number from dialog.
    int value() const { return value_ > total_ ? total_ : value_; }

    /// Show modal dialog. The return value can be
    /// accpeted or rejected.
    int popup(const int value, const int total);

protected:
    bool event(QEvent * event);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void onCloseClicked();

private:
    void createLayout();
    void clear();
    void clickFocusButton();
    void focusUpDownChild(bool up);

private Q_SLOTS:
    void onNumberClicked(const int);
    void onBackspaceClicked();
    void onOKClicked();

private:
    int                     value_;          ///< Current value
    int                     total_;          ///< Limitation of the total number
    QIntValidator           validator_;
    OnyxLineEdit            number_edit_;    ///< Number edit.
    OnyxLabel               second_label;    ///<display second 's'
    NumberWidget            number_widget_;  ///< Number widget.

private:
    NO_COPY_AND_ASSIGN(TimeIntervalDialog);
};

}   // namespace ui


#endif // ONYX_TIME_INTERVAL_DIALOG_H
