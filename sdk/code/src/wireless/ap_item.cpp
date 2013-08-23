#include "onyx/wireless/ap_item.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/data/data_tags.h"

namespace ui
{

const QString TYPE_LABEL_STYLE = "      \
QLabel                                  \
{                                       \
     padding: 0px;                      \
     background: transparent;           \
     font: 24px ;                       \
     color: black;                      \
 }";

/// Compiler issue.
#ifndef _WINDOWS
const int WifiAPItem::SIGNAL_ICONS;
#endif

static const int ID_CUSTOMIZE = 0;
static const int ID_REFRESH = 1;


static const int SPACING = 20;
static const int ICON_SIZE = 44 + SPACING;
static const QColor TITLE_BK_COLOR = QColor(210, 210, 210);
static const QColor NORMAL_BK_COLOR = QColor(255, 255, 255);
static const QColor SELECTED_BK_COLOR = QColor(164, 164, 164);


WifiTitleItem::WifiTitleItem(QWidget *parent)
: QWidget(parent)
, layout_(this)
, title_label_(tr("Starting Wifi Device..."), 0)
, dash_board_(0, 0)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setFixedHeight(80);
    createLayout();
}

WifiTitleItem::~WifiTitleItem()
{
}

void WifiTitleItem::createLayout()
{
    layout_.setContentsMargins(SPACING, SPACING / 5, SPACING, SPACING / 5);
    layout_.setSpacing(10);

    layout_.addWidget(&title_label_, 0, Qt::AlignVCenter);
    layout_.addStretch(0);


    // dash board
    dash_board_.setFixedWidth(75 * 2);
    layout_.addWidget(&dash_board_);
    createDashBoard();
    connect(&dash_board_,
            SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this,
            SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

void WifiTitleItem::createDashBoard()
{
    ODatas datas;
    ODataPtr customize (new OData);
    customize->insert(TAG_COVER, QPixmap(":/images/customize.png"));
    customize->insert(TAG_ID, ID_CUSTOMIZE);
    datas.push_back(customize);

    ODataPtr refresh (new OData);
    refresh->insert(TAG_COVER, QPixmap(":/images/refresh.png"));
    refresh->insert(TAG_ID, ID_REFRESH);

    datas.push_back(refresh);

    dash_board_.setFixedGrid(1, datas.size());
    dash_board_.setData(datas);
    dash_board_.setSubItemBkColor(Qt::transparent);
}

void WifiTitleItem::setState(const QString & state)
{
    title_label_.setText(state);
}

void WifiTitleItem::onItemActivated(CatalogView *catalog, ContentView *item, int user_data)
{
    if (!item || !item->data())
    {
        return;
    }

    int id = item->data()->value(TAG_ID).toInt();
    if (id == ID_CUSTOMIZE)
    {
        emit customizedClicked();
    }
    else if (id == ID_REFRESH)
    {
        emit refreshClicked();
    }
}

bool WifiTitleItem::event(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::MouseMove:
    case QEvent::HoverMove:
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
        e->accept();
        return true;

    default:
        break;
    }
    return QWidget::event(e);
}

static QByteArray selected_bssid;
scoped_ptr<QPixmap> WifiAPItem::selected_pixmap_;
WifiAPItem *WifiAPItem::selected_item_ = 0;
static WifiAPItem *previous_selected_item_ = 0;

WifiAPItem::WifiAPItem(QWidget *parent)
    : ui::ContentView(parent)
    , hor_layout_(this)
    , ssid_label_(0)
    , config_button_(QPixmap(":/images/customize.png"), "", 0)
    , lock_icon_label_(0)
{
    createLayout();
    updateByProfile(profile_);
}

WifiAPItem::~WifiAPItem(void)
{
}

void WifiAPItem::updateView()
{
    // convert OData to profile
    if (data())
    {
        WifiProfile * d = static_cast<WifiProfile *>(data());
        profile_ = *d;
    }
    else
    {
        profile_.clear();
    }
    updateByProfile(profile_);
    selected_item_ = 0;
    previous_selected_item_ = 0;
    selected_bssid.clear();
}

void WifiAPItem::setProfile(WifiProfile & profile)
{
    if (!(profile_ == profile))
    {
        profile_ = profile;
        updateByProfile(profile_);
    }
    selected_item_ = 0;
    previous_selected_item_ = 0;
    selected_bssid.clear();
}

WifiProfile & WifiAPItem::profile()
{
    return profile_;
}

bool WifiAPItem::event(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::MouseMove:
    case QEvent::HoverMove:
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
        e->accept();
        return true;

    default:
        break;
    }
    return QWidget::event(e);
}

void WifiAPItem::paintEvent(QPaintEvent *e)
{
    if (profile_.bssid().isEmpty() && profile_.isPresent())
    {
        return;
    }

    QPainter painter(this);
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8, Qt::AbsoluteSize);

    if (isSelected() || isAPChecked())
    {
        painter.fillPath(path, QBrush(SELECTED_BK_COLOR));
    }
    else
    {
        painter.fillPath(path, QBrush(NORMAL_BK_COLOR));
    }

    if (isSelected())
    {
        if (!selected_pixmap_)
        {
            selected_pixmap_.reset(new QPixmap(":/images/selected.png"));
        }
        painter.drawPixmap(SPACING, (height() - selected_pixmap_->height()) / 2, *selected_pixmap_);
    }

    QPen pen;
    pen.setWidth(penWidth());

    if (hasFocus())
    {
        pen.setColor(Qt::black);
        painter.setPen(pen);
        painter.drawPath(path);
    }
    else
    {
        pen.setColor(Qt::white);
        painter.setPen(pen);
        painter.drawPath(path);
    }
}

void WifiAPItem::mousePressEvent(QMouseEvent *e)
{
    ContentView::mousePressEvent(e);
}

void WifiAPItem::activateItem()
{
    if (!profile_.bssid().isEmpty() && profile_.isPresent())
    {
        previous_selected_item_ = selected_item_;
        selected_item_ = this;
        selected_bssid = profile_.bssid();
        if (previous_selected_item_)
        {
            previous_selected_item_->repaint();
            onyx::screen::watcher().enqueue(previous_selected_item_, onyx::screen::ScreenProxy::GU);
        }
        repaint();
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
    }
}

void WifiAPItem::mouseReleaseEvent(QMouseEvent *e)
{
    activateItem();
    activate(0);
}

void WifiAPItem::focusInEvent(QFocusEvent * e)
{
    ContentView::focusInEvent(e);
}

void WifiAPItem::focusOutEvent(QFocusEvent * e)
{
    ContentView::focusOutEvent(e);
}

void WifiAPItem::createLayout()
{
    hor_layout_.setContentsMargins(ICON_SIZE + SPACING, SPACING / 2, SPACING, SPACING / 2);
    hor_layout_.setSpacing(0);

    ssid_label_.setContentsMargins(0, 0, 0, 0);
    hor_layout_.addWidget(&ssid_label_, 0, Qt::AlignVCenter);
    hor_layout_.addStretch(0);
    hor_layout_.addWidget(&lock_icon_label_, 0, Qt::AlignBottom);
    for(int i = 0; i < SIGNAL_ICONS; ++i)
    {
        hor_layout_.addWidget(&signal_labels_[i], 0, Qt::AlignBottom);
    }

    hor_layout_.addSpacing(10);
    config_button_.setFocusPolicy(Qt::NoFocus);
    hor_layout_.addWidget(&config_button_, 0, Qt::AlignBottom);
    connect(&config_button_, SIGNAL(clicked(bool)), this, SLOT(onConfigButtonClicked()));
}

void WifiAPItem::updateByProfile(WifiProfile & profile)
{
    if (!profile.isPresent())
    {
        ssid_label_.setText(profile.ssid());
        ssid_label_.setVisible(true);
        lock_icon_label_.setVisible(false);
        config_button_.setVisible(true);

        for(int i = 0; i < SIGNAL_ICONS; ++i)
        {
            signal_labels_[i].setVisible(false);
        }
        update();
        return;
    }

    bool visible = !profile.bssid().isEmpty();
    ssid_label_.setVisible(visible);
    lock_icon_label_.setVisible(visible);
    config_button_.setVisible(visible);
    for(int i = 0; i < SIGNAL_ICONS; ++i)
    {
        signal_labels_[i].setVisible(visible);
    }

    if (profile.bssid().isEmpty())
    {
        update();
        return;
    }

    ssid_label_.setText(profile.ssid());
    if (profile.isWep() || profile.isWpa() || profile.isWpa2())
    {
        lock_icon_label_.setPixmap(QPixmap(":/images/lock.png"));
    }
    else
    {
        lock_icon_label_.setVisible(false);
    }

    // Boundary check is necessary in case that the quality is out of range.
    int q = profile.quality() * SIGNAL_ICONS / 100;
    if (q * 100 / SIGNAL_ICONS < profile.quality())
    {
        ++q;
    }
    int count = qMin(q, SIGNAL_ICONS);
    for(int i = 0; i < count; ++i)
    {
        QString path(":/images/signal_fg_%1.png");
        path = path.arg(i + 1);
        signal_labels_[i].setPixmap(path);
    }

    for(int i = count; i < SIGNAL_ICONS; ++i)
    {
        QString path(":/images/signal_bk_%1.png");
        path = path.arg(i + 1);
        signal_labels_[i].setPixmap(path);
    }
    update();
}

bool WifiAPItem::isSelected()
{
    return (selected_bssid == profile_.bssid() && selected_item_ == this);
}

bool WifiAPItem::isAPChecked()
{
    if (data()->contains(TAG_CHECKED))
    {
        return data()->value(TAG_CHECKED).toBool();
    }
    return false;
}

void WifiAPItem::onConfigButtonClicked()
{
    emit config(profile_);
}

}
