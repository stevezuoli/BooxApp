/// Wifi dialog.
#ifndef UI_WIFI_DIALOG_H_
#define UI_WIFI_DIALOG_H_

#include <map>
#include <QtGui/QtGui>
#include "onyx/ui/paginator.h"
#include "ap_item.h"

using namespace sys;

namespace ui
{

class WifiDialog : public QDialog
{
    Q_OBJECT

public:
    WifiDialog(QWidget *parent, SysStatus & sys);
    ~WifiDialog();

public Q_SLOTS:
    void triggerScan();

public:
    void wifiScanResults(WifiProfiles &wifi_profiles) { wifi_profiles = scan_results_; }
    void runBackGround();
    int  popup(bool start_scan = true, bool auto_connect = true);
    void connect(const QString & ssid, const QString &password);
    bool connectToBestAP();
    bool connectToDefaultAP();
    void enableKeyboard(bool enable){ enable_keyboard_ = enable;}
    QString connectingAccessPoint();

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

private Q_SLOTS:
    void onScanTimeout();
    void onConnectionTimeout();

    void scan();
    void onAPItemClicked(WifiProfile & profile);
    void onRefreshClicked();
    void onCustomizedClicked();
    void onCloseClicked();

    void onSdioChanged(bool on);
    void enableChildren(bool enable);

    void onScanReturned();
    void onConnectionChanged(WifiProfile , WpaConnection::ConnectionState state);
    void onNeedPassword(WifiProfile profile);
    void onNoMatchedAP();

    void onComplete();
    void onAPConfig(WifiProfile &);

    void onItemActivated(CatalogView *catalog, ContentView *item, int user_data);
    void onPositionChanged(const int, const int);

private:
    void createLayout();
    void connectAllAPItems(CatalogView &view);
    void arrangeAPItems(WifiProfiles & list);
    void appendStoredAPs(WifiProfiles & list);
    void setupConnections();
    void clear();

    void setPassword(WifiProfile & profile, const QString & password);
    void storeAp(WifiProfile & profile);

    void updateStateLabel(WpaConnection::ConnectionState state);

    void enableAutoConnect(bool e) { auto_connect_to_best_ap_ = e; }
    bool allowAutoConnect() { return auto_connect_to_best_ap_; }

    void scanResults(WifiProfiles &);
    WifiProfiles records(sys::SystemConfig& conf);

    void updateHardwareAddress();
    void showPaginationButtons(bool show_prev = false, bool show_next = false);
    bool showConfigurationDialog(WifiProfile &profile);

    void checkAndRestorePassword(WifiProfile &profile);

    void sort(ODatas &list);

private:
    QVBoxLayout  big_box_;
    QHBoxLayout  title_hbox_;
    QHBoxLayout state_widget_layout_;
    QVBoxLayout content_layout_;
    QVBoxLayout ap_layout_;
    QHBoxLayout buttons_layout_;

    OnyxLabel title_icon_label_;
    OnyxLabel title_text_label_;
    OnyxPushButton close_button_;

    WifiTitleItem state_widget_;
    OnyxPushButton prev_button_;
    OnyxPushButton next_button_;
    ui::CatalogView ap_view_;
    OnyxLabel hardware_address_;

    const SysStatus & sys_;
    WpaConnectionManager& proxy_;
    bool auto_connect_to_best_ap_;      ///< Access points used successfully before.
    bool auto_connect_to_default_ap_;    ///< If we have pre-installed access point.
    bool ap_dialog_visible_;
    bool enable_keyboard_;

    WifiProfiles scan_results_;
    ODatas datas_;
    QString clicked_ssid_;
    bool scanned_once_;

};

}

#endif // UI_WIFI_DIALOG_H_
