#ifndef PROXY_SETTINGS_DIALOG_H_
#define PROXY_SETTINGS_DIALOG_H_

#include "onyx/ui/ui.h"
#include "onyx/sys/sys.h"

using namespace ui;

namespace network_service
{

/// Proxy settings dialog.
class ProxySettingsDialog : public OnyxDialog
{
    Q_OBJECT

public:
    ProxySettingsDialog(QWidget *parent);
    ~ProxySettingsDialog(void);

public:
    int popup();

protected:
    void mouseMoveEvent(QMouseEvent *me);
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void keyReleaseEvent(QKeyEvent *);
    void keyPressEvent(QKeyEvent * ke);
    bool event(QEvent * event);
    void moveEvent(QMoveEvent *e);
    void resizeEvent(QResizeEvent *e);

private Q_SLOTS:
    void onGetFocus(OnyxLineEdit *edit);
    void onUseProxyStateChanged(int state);
    void onUseExceptionStateChanged(int state);
    void onOkClicked(bool);

private:
    void createLayout();
    void loadFromSettings();
    void saveToSettings();
    void setReceiver(QWidget *receiver) { keyboard_receiver_ = receiver; }
    QWidget * receiver() { return keyboard_receiver_; }

private:
    QVBoxLayout   content_vbox_;
    QGridLayout   grid_layout_;
    QFormLayout   form_layout_;
    QVBoxLayout   exception_layout_;
    QHBoxLayout   hor_layout_;

    OnyxCheckBox  use_proxy_button_;

    OnyxLabel     type_label_;
    OnyxLabel     host_label_;
    OnyxLabel     port_label_;
    OnyxLabel     user_name_label_;
    OnyxLabel     password_label_;
    OnyxLabel     exceptions_label1_;
    OnyxLabel     exceptions_label2_;

    OnyxCheckBox  socks5_button_;
    OnyxCheckBox  http_button_;
    OnyxCheckBox  use_exceptions_button_;

    OnyxLineEdit  host_edit_;
    OnyxLineEdit  port_edit_;
    OnyxLineEdit  user_name_edit_;
    OnyxLineEdit  password_edit_;
    OnyxLineEdit  exceptions_edit_;

    OnyxPushButton ok_button_;

    QButtonGroup  types_button_group_;

    KeyBoard      keyboard_;     ///< Keyboard.
    QTimer        timer_;        ///< Timer to update the screen.
    QIntValidator validator_;

    QWidget       *keyboard_receiver_;
};


};  // namespace ui

#endif  // AP_CONFIG_DIALOG_H_
