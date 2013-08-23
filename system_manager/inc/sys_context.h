
#ifndef ONYX_SYSTEM_MANAGER_CONTEXT_H_
#define ONYX_SYSTEM_MANAGER_CONTEXT_H_


/// Maintain current system manager context. The context is used
/// to make sure system manager works correctly during deep sleep
/// and wakeup.
struct Context
{
public:
    Context()
        : sleeping_(false)
        , broadcast_(true)
        , sd_inserted_(false)
        , wifi_state_(false)
        , threeg_state_(false)
        , threeg_connected_(false)
        , suspend_count_(0)
    {
    }

public:
    bool sleeping() { return sleeping_; }
    void sleep(bool s) { sleeping_ = s; }
    void toggleSleep() { sleeping_ = !sleeping_ ; }

    int suspend_count() { return suspend_count_; }
    int inc_suspend_count() { return ++suspend_count_; }
    int dec_suspend_count() { return --suspend_count_; }
    int rst_suspend_count() { return suspend_count_ = 0; }

    bool canBroadcast() { return broadcast_; }
    void enableBroadcast(bool e) { broadcast_ = e; }

    bool isWifiEnabled() { return wifi_state_; }
    void storeWifiState(bool e) { wifi_state_ = e; }

    bool is3GEnabled() { return threeg_state_; }
    void store3GEnabled(bool e) { threeg_state_ = e; }

    bool is3GConnected() { return threeg_connected_; }
    void store3GState(bool c) { threeg_connected_ = c; }

    bool is_sd_inserted() { return sd_inserted_; }
    void changeSd(bool i) { sd_inserted_ = i; }

    const QString & usb_state() const { return usb_state_; }
    QString & mutable_usb_state() { return usb_state_; }

private:
    bool sleeping_;           ///< Is sleeping now.
    bool broadcast_;        ///< Can broadcast signal now.
    bool sd_inserted_;
    bool wifi_state_;       ///< Store state before suspend
    bool threeg_state_;     ///< Store state before suspend
    bool threeg_connected_; ///< 3G is connected or not.
    QString usb_state_;     ///< USB state.
    int suspend_count_;
};

#endif  // NABOO_SYSTEM_MANAGER_CONTEXT_H_
