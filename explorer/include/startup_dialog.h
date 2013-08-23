#ifndef UI_START_UP_DIALOG_H_
#define UI_START_UP_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/onyx_dialog.h"

using namespace ui;

namespace explorer
{

namespace view
{

/// Use this dialog to change the startup setting
class StartupDialog : public OnyxDialog
{
    Q_OBJECT

public:
    typedef enum {
        OPEN_MOST_RECENT_DOC=0,
        START_AT_HOME_SCREEN=1
    } StartupSettingType;

public:
    StartupDialog(QWidget *parent, sys::SystemConfig & ref);
    ~StartupDialog(void);

public:
    int exec();
    static StartupSettingType getStartupSettingType(const QString type);
    static QString getKeyForMiscConf();

private:
    void createLayout();
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    bool event(QEvent*);
    void itemSelected();
    void setStartupType();

private Q_SLOTS:
    void onReturn();
    void onStartupButtonChanged(bool);
    void onOkClicked(bool);

private:
    typedef OnyxCheckBox * CheckBoxPtr;
    typedef std::vector<CheckBoxPtr> Buttons;
    typedef std::vector<CheckBoxPtr>::iterator ButtonsIter;

    sys::SystemConfig & conf;
    QVBoxLayout ver_layout_;

    QVBoxLayout startup_layout_;
    QButtonGroup startup_group_;
    OnyxLabel startup_text_label_;  ///< for the description of startup setting
    Buttons startup_buttons_;

    QHBoxLayout hor_layout_;
    OnyxPushButton ok_;

    StartupSettingType sys_startup_type_;
    StartupSettingType startup_type_;
};

}   // namespace view

}   // namespace explorer

#endif      // UI_START_UP_DIALOG_H_
