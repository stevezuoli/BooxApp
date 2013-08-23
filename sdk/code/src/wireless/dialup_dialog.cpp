
#include "onyx/wireless/dialup_dialog.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/sys/sys.h"
#include "onyx/data/network_types.h"
#include "onyx/data/data_tags.h"

namespace ui
{
static const QString TAG_INDEX = "lang_index";
static const QString TITLE_INDEX = "title_index";
static const int ITEM_HEIGHT = 45;
static const QString BUTTON_STYLE =    "\
QPushButton                             \
{                                       \
    background: transparent;            \
    font-size: 14px;                    \
    border-width: 1px;                  \
    border-color: transparent;          \
    border-style: solid;                \
    color: black;                       \
    padding: 0px;                       \
}                                       \
QPushButton:pressed                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:checked                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    color: white;                       \
    border-color: black;                \
    background-color: black;            \
}                                       \
QPushButton:disabled                    \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: dark;                 \
    color: dark;                        \
    background-color: white;            \
}";

struct APN
{
    QString display_name;
    QString username;
    QString password;
    QString apn;
};

/// TODO, we need to add a filter to file system, so that we can
/// display only part of them.
static const APN APNS[] =
{
    {"Mts", "mts", "mts", "mts"},
    {"Megafon", "gdata", "gdata", "megafon"},
    {"Beeline", "beeline", "beeline", "beeline"},
    {"telenor", "telenor", "", "telenor"},
    {"T-Mobile", "", "", "t-mobile"},
    {"Orange", "", "", "orange"},
    {"O2", "", "", "o2"},
    {"movistar.es", "movistar", "movistar", "movistar"},
    {"web.htgprs", "", "", "htgprs"},
    {"Mobilebox", "", "", "mobilebox"},
    {"Plus", "", "", "plus"},
    {"Vodafone", "", "", "vodafone"},
    {"China Unicom", "", "", "unicom"},
    {"China Telecom", "", "", "telecom"}
};
static const int APNS_COUNT = sizeof(APNS)/sizeof(APNS[0]);

static QString address()
{
    QString result;
    QList<QNetworkInterface> all = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface ni, all)
    {
        //qDebug("interface name %s", qPrintable(ni.name()));
        QList<QNetworkAddressEntry> addrs = ni.addressEntries();
        foreach(QNetworkAddressEntry entry, addrs)
        {
            if (ni.name().compare("ppp0", Qt::CaseInsensitive) == 0)
            {
                result = entry.ip().toString();
            }
          //  qDebug("ip address %s", qPrintable(entry.ip().toString()));
        }
    }
    return result;
}

DialUpDialog::DialUpDialog(QWidget *parent, SysStatus & sys)
#ifndef Q_WS_QWS
    : QDialog(parent, 0)
#else
    : QDialog(parent, Qt::FramelessWindowHint)
#endif
    , big_box_(this)
    , title_widget_(this)
    , title_vbox_(&title_widget_)
    , title_hbox_(0)
    , content_layout_(0)
    , state_box_(0)
    , network_label_(0)
    , state_widget_(0)
    , input_layout_(0)
    , top_label_(0)
    , title_icon_label_(this)
    , title_text_label_(tr("3G Connection"), this)
    , close_button_("", this)
    , sys_(sys)
    , connecting_(false)
{
    loadConf();

    setAutoFillBackground(false);
    createLayout();
    onyx::screen::watcher().addWatcher(this);
}

DialUpDialog::~DialUpDialog()
{
    clearDatas(apns_buttons_datas_);
}

void DialUpDialog::loadConf()
{
    sys::SystemConfig conf;

    QString target_profiles = qgetenv("TARGET_3G_PROFILES");

    all_peers_.clear();
    for (int i = 0; i < APNS_COUNT; ++i)
    {
        DialupProfile tmp;
        tmp.setDisplayName(APNS[i].display_name);
        tmp.setUsername(APNS[i].username);
        tmp.setPassword(APNS[i].password);
        tmp.setApn(APNS[i].apn);

        if (!target_profiles.isEmpty() && !target_profiles.contains(APNS[i].display_name))
        {
            // do not add this profile
        }
        else
        {
            all_peers_.push_back(tmp);
        }
    }

    if (all_peers_.size() > 0)
    {
        profile_ = all_peers_.front();
    }

    // Choose default peer.
    for(int i = 0; i < all_peers_.size(); ++i)
    {
        DialupProfile tmp(all_peers_[i]);
        if (DialupProfile::defaultPeer().compare(tmp.apn(), Qt::CaseInsensitive) == 0)
        {
            profile_ = tmp;
            break;
        }
    }

    if (profile_.apn().isEmpty() || !target_profiles.contains(profile_.apn()))
    {
        // By default use the first one.
        DialupProfile tmp(all_peers_[0]);
        profile_ = tmp;
    }
}

void DialUpDialog::saveConf()
{
    sys::SystemConfig conf;
    DialupProfiles all;
    all.push_back(profile_);
    conf.saveDialupProfiles(all);
}

int  DialUpDialog::popup(bool show_profile)
{
    if (!sys_.isPowerSwitchOn())
    {
        showOffMessage();
    }
    showMaximized();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);

    // connect to default network.
    for(int i = 0; i < APNS_COUNT; ++i)
    {   
        if(qgetenv("CONNECT_TO_DEFAULT_APN").toInt() > 0)
        {
            if (sys_.isPowerSwitchOn())
            {
                connect("", "", "");
                break;
            }
        } 
    }
    return exec();
}

void DialUpDialog::keyPressEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Escape)
    {
        ke->accept();
        return;
    }
    QDialog::keyPressEvent(ke);
}

void DialUpDialog::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    if (ke->key() == Qt::Key_Escape)
    {
        onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GC);
        accept();
        return;
    }

    QDialog::keyPressEvent(ke);
}

bool DialUpDialog::event(QEvent * e)
{
    bool ret = QDialog::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        //onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
        e->accept();
        return true;
    }
    return ret;
}

void DialUpDialog::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPainterPath path;
    path.addRoundedRect(content_layout_.contentsRect().adjusted(-2, -2, 2, 2), 8, 8, Qt::AbsoluteSize);
    painter.fillPath(path, QBrush(QColor(220, 220, 220)));
    painter.setPen(QColor(Qt::black));
    painter.drawPath(path);
}

void DialUpDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
}

void DialUpDialog::mousePressEvent(QMouseEvent *)
{
}

void DialUpDialog::mouseReleaseEvent(QMouseEvent *)
{
}

void DialUpDialog::createLayout()
{
    const int SPACING = 2;
    const int WIDGET_HEIGHT = 50;
    big_box_.setSizeConstraint(QLayout::SetMaximumSize);
    big_box_.setSpacing(SPACING);
    big_box_.setContentsMargins(SPACING, SPACING, SPACING, SPACING);

    // title widget.
    title_widget_.setAutoFillBackground(true);
    title_widget_.setBackgroundRole(QPalette::Dark);
    title_widget_.setContentsMargins(0, 0, 0, 0);
    title_widget_.setFixedHeight(WIDGET_HEIGHT + SPACING * 2);
    big_box_.addWidget(&title_widget_);

    // content layout.
    const int MARGINS = 10;
    big_box_.addLayout(&content_layout_);
    content_layout_.setContentsMargins(MARGINS, 6, MARGINS, 6);

    // Status.
    state_widget_.setWordWrap(true);
    state_widget_.setAlignment(Qt::AlignLeft);
    state_widget_.setContentsMargins(MARGINS, 0, MARGINS, 0);

    network_label_.setWordWrap(true);
    network_label_.setAlignment(Qt::AlignLeft);
    network_label_.setContentsMargins(MARGINS, 0, MARGINS, 0);

    state_box_.addWidget(&state_widget_, 600);
    state_box_.addWidget(&network_label_, 0, static_cast<Qt::AlignmentFlag>(Qt::AlignHCenter|Qt::AlignBottom));
    content_layout_.addLayout(&state_box_);
    content_layout_.addSpacing(MARGINS);

    // top_label_
    title_vbox_.setSpacing(0);
    title_vbox_.setContentsMargins(SPACING, 0, SPACING, 0);
    title_vbox_.addWidget(&top_label_);
    top_label_.setFixedHeight(2);
    top_label_.setFrameShape(QFrame::HLine);
    top_label_.setAutoFillBackground(true);
    top_label_.setBackgroundRole(QPalette::Base);

    // title hbox.
    title_vbox_.addLayout(&title_hbox_);
    title_hbox_.addWidget(&title_icon_label_, 0, Qt::AlignVCenter);
    title_hbox_.addSpacing(SPACING * 2);
    title_hbox_.addWidget(&title_text_label_, 0, Qt::AlignVCenter);
    title_hbox_.addStretch(0);
    title_hbox_.addWidget(&close_button_);
    title_hbox_.setContentsMargins(2 * SPACING, SPACING, 2 * SPACING, SPACING);
    title_icon_label_.setPixmap(QPixmap(":/images/network_connection.png"));
    title_text_label_.setAlignment(Qt::AlignVCenter);
    title_icon_label_.setFixedHeight(WIDGET_HEIGHT);
    title_text_label_.useTitleBarStyle();
    title_text_label_.setFixedHeight(WIDGET_HEIGHT);

    close_button_.setStyleSheet(BUTTON_STYLE);
    QPixmap close_pixmap(":/images/close.png");
    close_button_.setIconSize(close_pixmap.size());
    close_button_.setIcon(QIcon(close_pixmap));
    close_button_.setFocusPolicy(Qt::NoFocus);
    QObject::connect(&close_button_, SIGNAL(clicked()), this, SLOT(onCloseClicked()));

    // TODO: optimize this
    content_layout_.addSpacing(36);

    // ap layout.
    createAPNsButtons();
    qDebug()<<APNS_buttons_.size().height()<<" "<<APNS_buttons_.size().width()<<" "<<APNS_buttons_.sizeHint().height(); 
    content_layout_.addWidget(&APNS_buttons_);

    //Create disconnect item
    createDisconnectButton();
    APNS_buttons_.setNeighbor( &disconnect_button_, CatalogView::RECYCLE_DOWN);
    APNS_buttons_.setNeighbor( &disconnect_button_, CatalogView::DOWN);
    content_layout_.addWidget(&disconnect_button_);

    content_layout_.addStretch(0);

    QObject::connect(&sys_, SIGNAL(pppConnectionChanged(const QString &, int)),
                     this, SLOT(onPppConnectionChanged(const QString &, int)));
    QObject::connect(&sys_, SIGNAL(report3GNetwork(const int, const int, const int)),
                     this, SLOT(onReport3GNetwork(const int, const int, const int)));
}

void DialUpDialog::createAPNsButtons()
{
    APNS_buttons_.setSubItemType(ui::CheckBoxView::type());
    APNS_buttons_.setPreferItemSize(QSize(0, ITEM_HEIGHT));

    for(int row = 0; row < all_peers_.size(); ++row)
    {
        OData * item = new OData;
        item->insert(TAG_TITLE, all_peers_[row].displayName());
        item->insert(TAG_INDEX, row);

        if (all_peers_[row].apn().compare(profile_.apn(), Qt::CaseInsensitive) == 0)
        {
            item->insert(TAG_CHECKED, true);
            selected_ = item;
        }
        apns_buttons_datas_.push_back(item);
    }
    
    
    APNS_buttons_.setData(apns_buttons_datas_, true);
    APNS_buttons_.setFixedHeight(all_peers_.size()*(ITEM_HEIGHT+APNS_buttons_.spacing()));
    APNS_buttons_.setFixedGrid(apns_buttons_datas_.size(), 1);

    QObject::connect(&APNS_buttons_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView*, ContentView*, int)));

}

void DialUpDialog::createDisconnectButton()
{
    disconnect_button_.setSubItemType(ui::CoverView::type());
    disconnect_button_.setPreferItemSize(QSize(0, ITEM_HEIGHT));
    ODatas d;

    OData * item = new OData;
    item->insert(TAG_TITLE, tr("Disconnect"));
    item->insert(TITLE_INDEX, 1);
    d.push_back(item);


    disconnect_button_.setData(d, true);
    disconnect_button_.setMinimumHeight( ITEM_HEIGHT );

    QObject::connect(&disconnect_button_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onDisconnectClicked(void)));
}

void DialUpDialog::clear()
{
}

void DialUpDialog::connect(const QString & peer,
                           const QString & username,
                           const QString & password)
{
    profile_.setApn(peer);
    profile_.setUsername(username);
    profile_.setPassword(password);
    if (!sys_.connect3g(peer, username, password))
    {
        if (!sys_.isPowerSwitchOn())
        {
            showOffMessage();
        }
    }
}

void DialUpDialog::onTimeout()
{
}

void DialUpDialog::onConnectClicked(bool)
{
    updateStatus(tr("Establishing 3G-connection... Please wait"));
}

void DialUpDialog::updateStatus(QString status)
{
    state_widget_.setText(status);
    update();
    onyx::screen::instance().updateWidget(&state_widget_, onyx::screen::ScreenProxy::GU);
}

void DialUpDialog::onDialogAccept()
{
    accept();
    onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GC);
}

void DialUpDialog::onPppConnectionChanged(const QString &message, int status)
{
    if (status == TG_CHECKING_NETWORK)
    {
        updateStatus(tr("Searching Network..."));
    }
    else if (status == TG_CONNECTING)
    {
        updateStatus(tr("Establishing 3G-connection... Please wait"));
    }
    else if (status == TG_CONNECTED)
    {
        QString result(tr("Connected. Address: %1"));
        result = result.arg(qPrintable(address()));
        updateStatus(result);
        saveConf();
        QTimer::singleShot(1500, this, SLOT(onDialogAccept()));
    }
    else if (status == TG_DISCONNECTED)
    {
        if (message.isEmpty())
        {
            if (!sys_.isPowerSwitchOn())
            {
                showOffMessage();
            }
            else
            {
                updateStatus(tr("Communication is not established."));
            }
        }
        else
        {
            if (message.compare("Sim-card Error", Qt::CaseInsensitive) == 0)
            {
                updateStatus(tr("Sim-card Error"));
            }
            else if (message.compare("Modem error", Qt::CaseInsensitive) == 0)
            {
                updateStatus(tr("Modem error"));
            }
            else if (message.compare("EXIT_CONNECT_FAILED", Qt::CaseInsensitive) == 0)
            {
                updateStatus(tr("EXIT_CONNECT_FAILED"));
            }
            else if (message.compare("EXIT_HANGUP", Qt::CaseInsensitive) == 0)
            {
                updateStatus(tr("Communication is not established."));
            }
            else
            {
                updateStatus(message);
            }
        }
    }
}

void DialUpDialog::showDNSResult(QHostInfo info)
{
    if (info.addresses().size() > 0)
    {
        updateStatus(tr("Finished."));
    }
    else
    {
        updateStatus(info.errorString());
    }
}

static QString networkType(const int n)
{
    switch (n)
    {
    case NO_SERVICE:
        return "NO_SERVICE";
    case GSM_MODE:
        return "GSM";
    case GPRS_MODE:
        return "GPRS";
    case EDGE_MODE:
        return "EDGE";
    case WCDMA_MODE:
    case HSDPA_MODE:
    case HSUPA_MODE:
    case HSDPA_MODE_AND_HSUPA_MODE:
    case TD_SCDMA_MODE:
    case HSPA_PLUS_MODE:
        return "3G";
    }
    return "";
}

void DialUpDialog::onReport3GNetwork(const int signal,
                                     const int total,
                                     const int network)
{
    if (signal >= 0 && total > 0)
    {
        QString t("%1 %2%");
        t = t.arg(networkType(network)).arg(signal * 100 / total);
        network_label_.setText(t);
    }
    else
    {
        showOffMessage();
    }
}

void DialUpDialog::showOffMessage()
{
    QString t(":/images/signal_3g_off.png");
    QPixmap pixmap(t);
    network_label_.setPixmap(pixmap);
    updateStatus(tr("3G Connection is off. Please turn 3G switch on."));
}

void DialUpDialog::onDisconnectClicked(void)
{
    disconnect_button_.setFocus();
    disconnect_button_.setFocusTo(0, 0);
    sys::SysStatus::instance().disconnect3g();
}

void DialUpDialog::onGetFocus(OnyxLineEdit *object)
{
}

void DialUpDialog::onCloseClicked()
{
    reject();
    onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GC);
}

void DialUpDialog::onItemActivated(CatalogView *catalog, ContentView *item, int user_data)
{
    if (!item || !item->data())
    {
        return;
    }
    int index = item->data()->value(TAG_INDEX).toInt();
    
    onDisconnectClicked();
    selected_->insert(TAG_CHECKED, false);
    onConnectClicked(true);
    connect(all_peers_[index].apn(), all_peers_[index].username().toLocal8Bit().constData(), all_peers_[index].password().toLocal8Bit().constData());
    selected_ = item->data();
    selected_->insert(TAG_CHECKED, true);
 
    update();
    onyx::screen::watcher().enqueue(catalog, onyx::screen::ScreenProxy::GC);
}

} //namespace ui

///
/// \example wifi/dialup_main.cpp
/// This is an example of how to use the DialUpDialog class.
///
