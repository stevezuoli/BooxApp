#ifndef EXPLORER_LOCALE_DIALOG_H_
#define EXPLORER_LOCALE_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui.h"

using namespace ui;

namespace explorer
{

namespace view
{

/// Locale widget. Not sure we should place it in the ui library.
/// So far, only explorer use it.
class LocaleDialog : public OnyxDialog
{
    Q_OBJECT

public:
    LocaleDialog(QWidget *parent, sys::SystemConfig & conf);
    ~LocaleDialog(void);

public:
    int exec();

private:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void changeEvent(QEvent *event);
    bool event(QEvent * event);
    void moveEvent(QMoveEvent *e);
    void resizeEvent(QResizeEvent *e);
    void createLayout();

private Q_SLOTS:
    void onReturn();
    void onLanguageButtonClicked(bool);
    void onOkClicked(bool);
    void onCloseClicked();

private:
    sys::SystemConfig & conf;
    QVBoxLayout ver_layout_top_;
    QHBoxLayout hor_layout_top_;
    QVBoxLayout ver_layout_left_;
    QVBoxLayout ver_layout_right_;

    QHBoxLayout hor_layout_ok_;


    typedef OnyxCheckBox * CheckBoxPtr;
    typedef std::vector<CheckBoxPtr> Buttons;
    typedef std::vector<CheckBoxPtr>::iterator ButtonsIter;
    Buttons language_buttons_;
    OnyxPushButton ok_;

};

}   // namespace view

}   // namespace explorer

#endif
