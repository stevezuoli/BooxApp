#ifndef AP_CONFIG_DIALOG_S_H_
#define AP_CONFIG_DIALOG_S_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/ui_global.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/ui/label.h"
#include "onyx/ui/buttons.h"
#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/onyx_keyboard.h"
#include "onyx/ui/line_edit_view_group.h"

namespace ui
{

class ApConfigDialogS: public OnyxDialog
{
    Q_OBJECT

public:
    ApConfigDialogS(QWidget *parent, WifiProfile & profile);
    ~ApConfigDialogS();

public:
    bool popup();
    QString value(int d_index = -1);

protected Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item,
            int user_data);

protected:
    bool event(QEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void onPlainButtonClicked();
    void onWepButtonClicked();
    void onWpaButtonClicked();
    void onWpa2ButtonClicked();

    void onOutOfDown(CatalogView* child, int, int);

private:
    void addLineEditsToGroup();

    void createLayout();
    void createAuthenticationView();
    void createLineEdits(const int &line_edit_width);
    void createSubMenu(const int &sub_menu_width);
    void createShowPlainText();
    void connectWithChildren();

    void updateWidgets(WifiProfile & profile);

    CatalogView * createEditItem(OData *data, int index, ODatas *edit_datas,
            const int &line_edit_width);

    void clearClicked();
    void setEditEchoMode(QLineEdit::EchoMode mode);
    void showPlainTextClicked(bool target_value);

    void keyPressEvent(QKeyEvent *event);

    QString getSsidText();
    QString getPskText();

    void setPlainProfile(WifiProfile & profile, const QString & id);
    void setWepProfile(WifiProfile & profile, const QString &id, const QString & key);
    void setWpaProfile(WifiProfile & profile, const QString &id, const QString & key);
    void setWpa2Profile(WifiProfile & profile, const QString &id, const QString & key);

    bool isSsidEmpty();

    void createInputs(int size);

private:
    QVBoxLayout big_layout_;
    QFormLayout form_layout_;
    QGridLayout auth_hbox_;    ///< Authentication box.
    QGridLayout enc_hbox_;     ///< Encryption box.
    QHBoxLayout *line_edit_layout_;

    OnyxLabel auth_label_;
    OnyxLabel enc_label_;

    QButtonGroup auth_group_;
    OnyxPushButton plain_button_;
    OnyxPushButton wep_button_;
    OnyxPushButton wpa_psk_button_;
    OnyxPushButton wpa2_psk_button_;

    OnyxPushButton enc_tkip_button_;
    OnyxPushButton enc_ccmp_button_;

    CatalogView sub_menu_;
    CatalogView show_plain_text_;
    QVector<CatalogView *> edit_view_list_;
    LineEditViewGroup edit_view_group_;

    ODatas sub_menu_datas_;
    ODatas show_plain_text_datas_;
    QVector<ODatas *> all_line_edit_datas_;

    OnyxKeyboard keyboard_;
    ODatas edit_list_;

    WifiProfile & profile_;

};

};  // namespace ui

#endif  // AP_CONFIG_DIALOG_S_H_
