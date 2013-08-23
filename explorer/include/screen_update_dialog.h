
#ifndef UI_SCREEN_UPDATE_DIALOG_H_
#define UI_SCREEN_UPDATE_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/onyx_dialog.h"

using namespace ui;

namespace explorer
{

namespace view
{

// Dialog to change screen update setting
class ScreenUpdateDialog : public OnyxDialog
{
    Q_OBJECT

public:
    struct SettingType
    {
        const char * title;
        int type;
    };

private:
    typedef enum {
        ALWAYS = 1,
        EVERY_THREE_PAGES = 3,
        EVERY_FIVE_PAGES = 5,
        EVERY_SEVEN_PAGES = 7,
        EVERY_NIGHT_PAGES = 9,
    } ScreenUpdateSettingType;

    typedef enum {
        SPEED_FIRST = 8,
        QUALITY_FIRST = 16,
    } GrayScaleSettingType;

    // for screen update
    static const QString SCREEN_UPDATE_KEY;
    static const char* SCREEN_UPDATE_SCOPE;
    static const int SCREEN_UPDATE_COUNT;

    // for gray scale
    static const QString GRAY_SCALE_KEY;
    static const char* GRAY_SCALE_SCOPE;
    static const int GRAY_SCALE_COUNT;

public:
    ScreenUpdateDialog(QWidget *parent, sys::SystemConfig & ref);
    ~ScreenUpdateDialog();

public:
    int exec();
    static QString getKeyForMiscConf();
    QString getGrayScaleSettingKey();

private:
    static ScreenUpdateSettingType getScreenUpdateSettingType(const QString type);
    static GrayScaleSettingType getGrayScaleSettingType(const QString type);

    void createLayout();
    void createScreenUpdateLayout();
    void createGrayScaleLayout();
    void setScreenUpdateSetting();
    void setGrayscaleSetting();
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    bool event(QEvent*);

private Q_SLOTS:
    void onReturn();
    void onScreenUpdateButtonChanged(bool);
    void onGrayScaleButtonChanged(bool);
    void onOkClicked(bool);

private:
    typedef OnyxCheckBox * CheckBoxPtr;
    typedef std::vector<CheckBoxPtr> Buttons;
    typedef std::vector<CheckBoxPtr>::iterator ButtonsIter;

    sys::SystemConfig & conf;
    QVBoxLayout ver_layout_;

    // for screen update setting
    QVBoxLayout screen_update_layout_;
    OnyxLabel screen_update_text_label_;  ///< for the description of screen full update
    QButtonGroup screen_update_group_;
    Buttons screen_update_buttons_;

    // for gray scale setting
    QVBoxLayout gray_scale_layout_;
    OnyxLabel gray_scale_text_label_;  ///< for the description of screen full update
    QButtonGroup gray_scale_group_;
    Buttons gray_scale_buttons_;

    QHBoxLayout hor_layout_;
    OnyxPushButton ok_;

    ScreenUpdateSettingType sys_screen_update_setting_;
    ScreenUpdateSettingType screen_update_setting_;

    GrayScaleSettingType sys_gray_scale_setting_;
    GrayScaleSettingType gray_scale_setting_;

};

}   // namespace view

}   // namespace explorer

#endif      // UI_SCREEN_UPDATE_DIALOG_H_
