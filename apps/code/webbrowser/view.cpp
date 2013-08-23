#include <QtGui/QtGui>
#include "view.h"
#include "onyx/data/web_history.h"
#include "network_service/proxy_settings_dialog.h"
#include "network_service/auto_complete.h"
#include "network_service/password_model.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/wireless/wifi_dialog.h"
#include "onyx/sys/sys_status.h"
#include "onyx/data/configuration.h"

#include "web_application.h"

using namespace ui;
using namespace webhistory;
using namespace cms;
using namespace vbf;


namespace webbrowser
{

#define USE_JQUERY

static const int PAGE_REPEAT = 20;
static const int DELTA = 10;

BrowserView::BrowserView(QWidget *parent)
    : QWebView(parent)
    , scrollbar_hidden_(0)
    , progress_(0)
    , update_type_(onyx::screen::ScreenProxy::GU)
    , system_update_type_(onyx::screen::instance().defaultWaveform())
    , enable_text_selection_(false)
    , need_save_url_(true)
    , page_(new WebPage(this))
    , bookmark_model_(0)
    , hand_tool_enabled_(true)
    , connection_state_(WpaConnection::STATE_UNKNOWN)
{
    setPage(page_);

    // In order to receive link clicked event.
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    // Setup connections.
    connect(this, SIGNAL(linkClicked(const QUrl &)), this, SLOT(onLinkClicked(const QUrl &)));
    connect(this, SIGNAL(loadStarted(void)), this, SLOT(onLoadStarted(void)));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(onLoadProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
    connect(page(), SIGNAL(repaintRequested(const QRect &)), this, SLOT(onRepaintRequested(const QRect&)));
    connect(page(), SIGNAL(loadingUrl(const QUrl&)), this, SIGNAL(urlChanged(const QUrl &)));
    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this,
            SLOT(onDownloadRequested(const QNetworkRequest &)));

    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(populateJavaScriptWindowObject()));
    connect(page_, SIGNAL(requestOTA(const QUrl&)), SIGNAL(requestOTA(const QUrl&)));

    connect(WebApplication::accessManager(), SIGNAL(requestSavePassword(const QByteArray &)),
            this, SLOT(onSavePassword(const QByteArray &)));

    WpaConnection & wpa_proxy = SysStatus::instance().wpa_proxy();
    connect(&wpa_proxy, SIGNAL(stateChanged(WifiProfile&, WpaConnection::ConnectionState)),
            this, SLOT(onConnectionChanged(WifiProfile&, WpaConnection::ConnectionState)));

    settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebSettings::JavaEnabled, true);

    page()->setForwardUnsupportedContent(true);

#ifdef USE_JQUERY
    QFile file;
    file.setFileName(":/res/jquery.min.js");
    if (file.open(QIODevice::ReadOnly))
    {
        jquery_ = file.readAll();
        file.close();
    }
#endif
}

BrowserView::~BrowserView()
{
    onyx::screen::instance().setDefaultWaveform(system_update_type_);
}

void BrowserView::attachBookmarkModel(BookmarkModel * model)
{
    bookmark_model_ = model;
}

void BrowserView::hideGif()
{
    QString code = "$('[src*=gif]').hide()";
    page()->mainFrame()->evaluateJavaScript(code);
}

void BrowserView::hideInlineFrames()
{
    QString code = "$('iframe').remove()";
    page()->mainFrame()->evaluateJavaScript(code);
}

void BrowserView::hideObjectElements()
{
    QString code = "$('object').remove()";
    page()->mainFrame()->evaluateJavaScript(code);
}

void BrowserView::hideEmbeddedElements()
{
    QString code = "$('embed').remove()";
    page()->mainFrame()->evaluateJavaScript(code);
}

void BrowserView::hideScrollbar()
{
    QString code = "$('body').css('overflow', 'hidden')";
    page()->mainFrame()->evaluateJavaScript(code);
}

void BrowserView::onLinkClicked(const QUrl &new_url)
{
    storeConf(url());
    qDebug("url clicked %s", qPrintable(new_url.toString()));
    load(new_url);
}

void BrowserView::onLoadStarted(void)
{
    qDebug("Load Start");

    // In order to use javascript to hide scrollbar.
    scrollbar_hidden_ = 0;
    progress_ = 0;

    // Store the screen update type.
    resetUpdateRect();
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
    reportCurrentProcess();
}

void BrowserView::onSavePassword(const QByteArray & data)
{
    QWebFrame * current_frame = page()->currentFrame();
    if (current_frame == 0)
    {
        return;
    }

    QUrl url = current_frame->url();
    AutoComplete::instance()->setFormData(url, data);

    // setFormHtml at this moment?
    if (!AutoComplete::instance()->existInForms(url))
    {
        AutoComplete::instance()->setFormHtml(url, current_frame->toHtml());
    }
    AutoComplete::instance()->evaluate(url);
}

void BrowserView::onLoadProgress(int progress)
{
    // For both vertical and horizontal.
    if (scrollbar_hidden_ <= 2)
    {
        QVariant var = page()->currentFrame()->evaluateJavaScript("document.body.style.overflow = 'hidden';");
        if (!var.isNull())
        {
            ++scrollbar_hidden_;
        }
    }

    // Reduce screen update.
    int prev = progress_ / 10;
    int now = progress / 10;
    if (prev != now)
    {
        progress_ = progress;
        reportCurrentProcess();
    }
}

void BrowserView::onLoadFinished(bool ok)
{
    // Restore the screen update type.
    onyx::screen::instance().setDefaultWaveform(update_type_);

    // Ensure we can get a gc update.
    progress_ = 100;
    update_rect_ = rect();

    // Check if we need to store the thumbnail.
    if (ok)
    {
        storeUrl();

        if (WebApplication::accessManager()->autoComplete())
        {
            AutoComplete::instance()->complete(page()->mainFrame());
        }
#ifdef USE_JQUERY
        page()->mainFrame()->evaluateJavaScript(jquery_);
        //hideGif();
        //hideInlineFrames();
        //hideObjectElements();
        //hideEmbeddedElements();
        hideScrollbar();
#endif

        // The keyboard can only be display after load finished
        addFormsFocusEvent();
        addSelectEvents();
    }
    else
    {
        // Failed.
        progress_ = -1;
        reportCurrentProcess();

        // If it's the first time failed, we can popup network dialog
        // if we already have some existing connection, try to connect Network config if necessary.
        static bool tried = false;
        if (needConfigNetwork() && !tried)
        {
            tried = true;

            QTimer::singleShot(400, this, SLOT(configNetwork()));
        }
    }
    updateViewportRange();
}

bool BrowserView::needConfigNetwork()
{
    QNetworkReply::NetworkError error = page_->networkError();
    return (error == QNetworkReply::ConnectionRefusedError ||
            error == QNetworkReply::HostNotFoundError ||
            error == QNetworkReply::TimeoutError ||
            error == QNetworkReply::UnknownNetworkError);
}

void BrowserView::onRepaintRequested(const QRect& rc)
{
    update_rect_ = update_rect_.united(rc);
    if (isTextSelectionEnabled())
    {
        selected_rect_ = selected_rect_.unite(rc);
        selected_rect_ = selected_rect_.intersect(rect());
    }
}

void BrowserView::onConnectionChanged(WifiProfile& profile, WpaConnection::ConnectionState state)
{
    // qDebug("BrowserView::onConnectionChanged");
    /*
    if (state == WpaProxy::STATE_COMPLETE)
    {
        if (conf_dialog_)
        {
            conf_dialog_->accept();
            conf_dialog_.reset(0);
        }
        //reload();
        // launch a timer to reload the web later
        QTimer::singleShot(100, this, SLOT(reload()));
    }
    */
    qDebug("Connection State Changed:%d", state);
    connection_state_ = state;
}

void BrowserView::onDownloadRequested(const QNetworkRequest & request)
{
    WebApplication::downloadManager()->download(request);
}

void BrowserView::reloadCurrentUrl()
{
    if (current_url_.isValid())
    {
        load(current_url_);
    }
}

void BrowserView::configNetwork()
{
    current_url_ = url();
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    {
        HoldAutoSaver hold(this);
        page_->displayConnectingHtml(current_url_);
    }

    // TODO. Check the return value then reload the page.
    QString type = sys::SysStatus::instance().connectionType();
    if (type.contains("wifi", Qt::CaseInsensitive))
    {
        wifiDialog().popup(true);
    }
    else if (type.contains("3g", Qt::CaseInsensitive))
    {
        if(qgetenv("CONNECT_TO_DEFAULT_APN").toInt() > 0 &&
            sys::SysStatus::instance().isPowerSwitchOn())
        {
            sys::SysStatus::instance().connect3g("","","");
            connectingDialog().popup();
        }
        else
        {
            dialUpDialog().popup();
        }
    }

    QTimer::singleShot(400, this, SLOT(reloadCurrentUrl()));
    onyx::screen::instance().setDefaultWaveform(update_type_);
}

void BrowserView::scan()
{
    QString type = sys::SysStatus::instance().connectionType();
    if (type.contains("wifi", Qt::CaseInsensitive))
    {
        wifiDialog().triggerScan();
    }
    else if (type.contains("3g", Qt::CaseInsensitive))
    {
        // dialUpDialog().popup();
    }
}

void BrowserView::connectTo(const QString &ssid, const QString &psk)
{
    QString type = sys::SysStatus::instance().connectionType();
    if (type.contains("wifi", Qt::CaseInsensitive))
    {
        wifiDialog().connect(ssid, psk);
    }
    else if (type.contains("3g", Qt::CaseInsensitive))
    {
        // dialUpDialog().popup();
    }
}

WifiDialog & BrowserView::wifiDialog()
{
    if (!conf_dialog_)
    {
        conf_dialog_.reset(new WifiDialog(0, SysStatus::instance()));
    }
    return *conf_dialog_;
}

DialUpDialog & BrowserView::dialUpDialog()
{
    if (!dial_dialog_)
    {
        dial_dialog_.reset(new DialUpDialog(0, SysStatus::instance()));
    }
    return *dial_dialog_;
}

ConnectingDialog & BrowserView::connectingDialog()
{
    if (!conn_dialog_)
    {
        conn_dialog_.reset(new ConnectingDialog(0, SysStatus::instance()));
    }
    return *conn_dialog_;
}
void BrowserView::updateActions()
{
    //QString font = QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont);
    //font_family_actions_.generateActions(font, true);

    std::vector<ZoomFactor> zoom_settings;
    zoom_settings.push_back(75.0f);
    zoom_settings.push_back(100.0f);
    zoom_settings.push_back(125.0f);
    zoom_settings.push_back(150.0f);
    zoom_settings.push_back(175.0f);
    zoom_settings.push_back(200.0f);
    zoom_settings.push_back(300.0f);
    zoom_settings.push_back(400.0f);
    zoom_setting_actions_.generateActions(zoom_settings);
    zoom_setting_actions_.setCurrentZoomValue(zoomFactor() * ZOOM_ACTUAL);

    std::vector<qreal> size;
    text_size_actions_.generateActions(size, textSizeMultiplier());

    navigation_actions_.generateActions(history());

    std::vector<ui::NetworkType> networks;
    networks.push_back(ui::NETWORK_WIFI);
    networks.push_back(ui::NETWORK_PROXY);
    network_actions_.generateActions(networks);

    // Reading tools
    if (reading_tool_actions_.actions().size() <= 0)
    {
        std::vector<ReadingToolsType> tools;
        tools.push_back(SCROLL_PAGE);
        tools.push_back(SEARCH_TOOL);
        tools.push_back(DICTIONARY_TOOL);
        reading_tool_actions_.generateActions(tools);

        tools.clear();
        tools.push_back(ADD_BOOKMARK);
        tools.push_back(DELETE_BOOKMARK);
        tools.push_back(SHOW_ALL_BOOKMARKS);
        reading_tool_actions_.generateActions(tools, true);

        tools.clear();
        tools.push_back(SAVE_ACCOUNT);
        tools.push_back(DISPLAY_ACCOUNT);
        tools.push_back(DELETE_ACCOUNT);
        tools.push_back(CLEAR_COOKIES);
        reading_tool_actions_.generateActions(tools, true);

        tools.clear();
        tools.push_back(AUTO_LOAD_IMAGE);
        reading_tool_actions_.generateActions(tools, true);
    }
    reading_tool_actions_.setActionStatus(SCROLL_PAGE, hand_tool_enabled_);
    reading_tool_actions_.setActionStatus(SAVE_ACCOUNT, WebApplication::accessManager()->savePassword());
    reading_tool_actions_.setActionStatus(DISPLAY_ACCOUNT, WebApplication::accessManager()->autoComplete());

    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
    reading_tool_actions_.setActionStatus(AUTO_LOAD_IMAGE, settings.value(QLatin1String("autoLoadImages"), true).toBool());
    settings.endGroup();

    // regenerate system actions
    system_actions_.generateActions();
}

void BrowserView::onRangeClicked(const int percentage,
                                 const int value)
{
    int height = page()->mainFrame()->contentsSize().height();
    QPointF pt = currentOffset();
    pt.ry() = height * percentage / 100 - rect().height();
    myScrollTo(pt.toPoint());
    updateViewportRange();
}

void BrowserView::reportCurrentProcess()
{
    emit progressChangedSignal(progress_, 100);
}

void BrowserView::openMusicPlayer()
{
    sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
}

void BrowserView::displayFulfillmentProcessing(const QString & url)
{
    HoldAutoSaver hold(this);
    page_->displayFulfillmentHtml(url, OTA_PROCESSING);
}

void BrowserView::displayFulfillmentAborted(const QString & url)
{
    HoldAutoSaver hold(this);
    page_->displayFulfillmentHtml(url, OTA_ABORTED);
}

void BrowserView::displayFulfillmentDone(const QString & path)
{
    HoldAutoSaver hold(this);
    page_->displayFulfillmentHtml(path, OTA_DONE);
}

void BrowserView::displayFulfillmentError(const QString & workflow, const QString & error_code)
{
    HoldAutoSaver hold(this);
    QString message(tr("Workflow:%1; Code:%2"));
    message = message.arg(workflow).arg(error_code);
    page_->displayFulfillmentHtml(message, OTA_ERROR);
}

/// Popup the menu.
void BrowserView::popupMenu()
{
    ui::PopupMenu menu(this);

    updateActions();

    //menu.addGroup(&font_family_actions_);
    menu.addGroup(&zoom_setting_actions_);
    menu.addGroup(&text_size_actions_);
    menu.addGroup(&navigation_actions_);
    // menu.addGroup(&network_actions_);
    menu.addGroup(&reading_tool_actions_);
    menu.setSystemAction(&system_actions_);
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction * group = menu.selectedCategory();
    /*if (group == font_family_actions_.category())
    {
        changeFontFamily(font_family_actions_.selectedFont());
    }*/
    if (group == zoom_setting_actions_.category())
    {
        changeZoomFactor(zoom_setting_actions_.getSelectedZoomValue());
    }
    else if (group == text_size_actions_.category())
    {
        changeFontSize(text_size_actions_.selectedMultiplier());
    }
    else if (group == navigation_actions_.category())
    {
        if (navigation_actions_.selected() == NAVIGATE_FORWARD)
        {
            forward();
        }
        else if (navigation_actions_.selected() == NAVIGATE_BACKWARD)
        {
            back();
        }
        else if (navigation_actions_.selected() == NAVIGATE_CLEAR_HISTORY)
        {
            clearHistory();
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
        }
        else if (navigation_actions_.selected() == NAVIGATE_HOME)
        {
            sys::SysStatus::instance().setSystemBusy(true, false);
            emit showHome();
        }
    }
    else if (group == network_actions_.category())
    {
        if (network_actions_.selected() == ui::NETWORK_WIFI ||
            network_actions_.selected() == ui::NETWORK_WCDMA)
        {
            configNetwork();
        }
        else if (network_actions_.selected() == ui::NETWORK_PROXY)
        {
            configureProxy();
        }
    }
    else if (group == reading_tool_actions_.category())
    {
        if (reading_tool_actions_.selectedTool() == DICTIONARY_TOOL)
        {
            startDictLookup();
        }
        else if (reading_tool_actions_.selectedTool() == SCROLL_PAGE)
        {
            hand_tool_enabled_ = !hand_tool_enabled_;
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
        }
        else if (reading_tool_actions_.selectedTool() == TEXT_TO_SPEECH)
        {
            // startTTS();
        }
        else if (reading_tool_actions_.selectedTool() == SEARCH_TOOL)
        {
            showSearchWidget();
        }
        else if (reading_tool_actions_.selectedTool() == ADD_BOOKMARK)
        {
            addBookmark();
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
        }
        else if (reading_tool_actions_.selectedTool() == SHOW_ALL_BOOKMARKS)
        {
            displayBookmarks();
        }
        else if (reading_tool_actions_.selectedTool() == DELETE_BOOKMARK)
        {
            deleteBookmark();
        }
        else if (reading_tool_actions_.selectedTool() == SAVE_ACCOUNT)
        {
            QAction * action = reading_tool_actions_.action(SAVE_ACCOUNT);
            WebApplication::accessManager()->enableSavingPassword(action->text().startsWith(QCoreApplication::tr("Save Account")));
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
        }
        else if (reading_tool_actions_.selectedTool() == DISPLAY_ACCOUNT)
        {
            QAction * action = reading_tool_actions_.action(DISPLAY_ACCOUNT);
            WebApplication::accessManager()->enableAutoComplete(action->text().startsWith(QCoreApplication::tr("Display Account")));
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
        }
        else if (reading_tool_actions_.selectedTool() == DELETE_ACCOUNT)
        {
            deletePassword();
        }
        else if (reading_tool_actions_.selectedTool() == CLEAR_COOKIES)
        {
            clearCookies();
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
        }
        else if (reading_tool_actions_.selectedTool() == AUTO_LOAD_IMAGE)
        {
            QAction * action = reading_tool_actions_.action(AUTO_LOAD_IMAGE);
            QSettings settings;
            settings.beginGroup(QLatin1String("websettings"));
            settings.setValue(QLatin1String("autoLoadImages"), action->text().startsWith(QCoreApplication::tr("Auto-load Image")));
            QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, settings.value(QLatin1String("autoLoadImages"), true).toBool());
            settings.endGroup();
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
        }
        return;
    }
    else if (group == system_actions_.category())
    {
        SystemAction system = system_actions_.selected();
        if (system == RETURN_TO_LIBRARY)
        {
            returnToLibrary();
        }
        else if (system == SCREEN_UPDATE_TYPE)
        {
            onyx::screen::instance().toggleWaveform();
            system_update_type_ = update_type_ = onyx::screen::instance().defaultWaveform();
            update();
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        }
        else if (system == MUSIC)
        {
            openMusicPlayer();
        }
        else if (system == ROTATE_SCREEN)
        {
            sys::SysStatus::instance().rotateScreen();
        }
    }
}

void BrowserView::changeZoomFactor(ZoomFactor zoom)
{
    qreal real_value = zoom / ZOOM_ACTUAL;
    setZoomFactor(real_value);
}

void BrowserView::returnToLibrary()
{
    sync();

    // close the drm service if it is still running
    sys::SysStatus::instance().stopDRMService();

    qDebug("Quit. Connection state:%d", connection_state_);
    if (sys::SysStatus::instance().currentConnection().compare("wifi", Qt::CaseInsensitive) == 0)
    {
        MessageDialog dialog(QMessageBox::Information,
                             tr("Wifi Connection"),
                             tr("Do you want to disconnect wifi?"),
                             QMessageBox::Yes|QMessageBox::No);
        if (dialog.exec() == QMessageBox::Yes)
        {
            sys::SysStatus::instance().stopWpaSupplicant();
        }
    }
    else if (sys::SysStatus::instance().currentConnection().compare("3G", Qt::CaseInsensitive) == 0)
    {
        // close 3g by default.
        /*
        MessageDialog dialog(QMessageBox::Information,
                             tr("3G Connection"),
                             tr("Do you want to disconnect 3G?"),
                             QMessageBox::Yes|QMessageBox::No);
        if (dialog.exec() == QMessageBox::Yes)
        {
            sys::SysStatus::instance().disconnect3g();
        }
        */
        sys::SysStatus::instance().disconnect3g();
    }

    close();
    qApp->exit();

    // We found a strange dead-lock after downloading a book. Use system exit to resolve it temporarily.
    ::exit(0);
}

void BrowserView::changeFontFamily(const QString & family)
{
    // It does not work for documents that specify font already.
    QWebSettings * settings = QWebSettings::globalSettings();
    if (settings->fontFamily(QWebSettings::StandardFont) == family)
    {
        update(rect());
        return;
    }

    for(int i = static_cast<int>(QWebSettings::StandardFont);
        i <= static_cast<int>(QWebSettings::FantasyFont); ++i)
    {
        settings->setFontFamily(static_cast<QWebSettings::FontFamily>(i), family);
    }
    update(rect());
    updateViewportRange();
}


void BrowserView::configureProxy()
{
    network_service::ProxySettingsDialog proxy_setting_dialog(0);
    if (proxy_setting_dialog.popup() == QDialog::Accepted)
    {
        NetworkAccessManager * access_manager = WebApplication::accessManager();
        access_manager->loadSettings();
    }
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void BrowserView::changeFontSize(qreal font_size)
{
    if (font_size == textSizeMultiplier())
    {
        update(rect());
        return;
    }

    // Record current offset.
    QSize size = page()->mainFrame()->contentsSize();
    QPointF pt = currentOffset();

    // Change size now.
    setTextSizeMultiplier(text_size_actions_.selectedMultiplier());

    // Scroll to there again.
    QSize new_size = page()->mainFrame()->contentsSize();
    pt.rx() = pt.x() * size.width() / new_size.width();
    pt.ry() = pt.y() * size.height() / new_size.height();
    myScrollTo(pt.toPoint());
}

QWebView *BrowserView::createWindow(QWebPage::WebWindowType type)
{
    return this;
}

void BrowserView::mousePressEvent(QMouseEvent*me)
{
    emit inputFormLostFocus();
    position_ = me->pos();

    // Do NOT disable update here because it might cause some screen update issues
    //onyx::screen::instance().enableUpdate(false);
    if (isWidgetVisible(dict_widget_.get()))
    {
        enableTextSelection(true);
        return;
    }

    enableTextSelection(false);
    return QWebView::mousePressEvent(me);
}

void BrowserView::mouseMoveEvent(QMouseEvent *me)
{
    /*
    if (isTextSelectionEnabled())
    {
        QWebView::mouseMoveEvent(me);
        repaint(selected_rect_);
        onyx::screen::instance().fastUpdateWidgetRegion(this, selected_rect_, false);
        selected_rect_.setCoords(0, 0, 0, 0);
    }
    else if(me->buttons() != Qt::NoButton)
    {
        QPoint delta = position_ - me->pos();
        page()->mainFrame()->scroll(delta.x(), delta.y());
        onyx::screen::instance().flush();
        onyx::screen::instance().enableUpdate(true);
        onyx::screen::instance().fastUpdateWidget(this, true);
        position_  = me->pos();
    }
    */

    if (!hand_tool_enabled_)
    {
        QWebView::mouseMoveEvent(me);
    }
    else
    {
        me->accept();
    }
}

void BrowserView::mouseReleaseEvent(QMouseEvent*me)
{
    QPoint delta = position_ - me->pos();
    if (isTextSelectionEnabled())
    {
        QWebView::mouseReleaseEvent(me);
        QContextMenuEvent ce(QContextMenuEvent::Mouse,
                             me->pos(),
                             me->globalPos(),
                             me->modifiers());
        page()->swallowContextMenuEvent(&ce);
        onyx::screen::instance().flush();
        onyx::screen::instance().updateWidgetRegion(this, selected_rect_, onyx::screen::ScreenProxy::DW, true);
        onyx::screen::instance().enableUpdate(true);
        emit selectionChanged();
        return;
    }

    onyx::screen::instance().enableUpdate(true);
    if (abs(delta.x()) < DELTA && abs(delta.y()) < DELTA)
    {
        // Could click a link.
        QWebView::mouseReleaseEvent(me);
        return;
    }

    // Pan.
    resetUpdateRect();

    if (!hand_tool_enabled_)
    {
        QWebView::mouseReleaseEvent(me);
    }
    else
    {
        me->accept();
        myScroll(delta.x(), delta.y());
    }
}

void BrowserView::keyPressEvent(QKeyEvent *e)
{
    // We only handle key release event, so ignore some keys.
    switch (e->key())
    {
    case Qt::Key_Down:
    case Qt::Key_Up:
        break;
    default:
        QWebView::keyPressEvent(e);
        break;
    }
    e->accept();
}

void BrowserView::keyReleaseEvent(QKeyEvent *ke)
{
    switch (ke->key())
    {
    case ui::Device_Menu_Key:
        popupMenu();
        break;
    case Qt::Key_Left:
        myScroll(-rect().width() + PAGE_REPEAT, 0);
        break;
    case Qt::Key_Right:
        myScroll(rect().width() - PAGE_REPEAT, 0);
        break;
    case Qt::Key_PageDown:
    case Qt::Key_Down:
        myScroll(0, rect().height() - PAGE_REPEAT);
        break;
    case Qt::Key_PageUp:
    case Qt::Key_Up:
        myScroll(0, -rect().height() + PAGE_REPEAT);
        break;
    case Qt::Key_C:
        clearCookies();
        break;
    default:
        QWebView::keyReleaseEvent(ke);
        break;
    }
    ke->ignore();
}

void BrowserView::contextMenuEvent(QContextMenuEvent * event)
{
    event->accept();
}

void BrowserView::closeEvent(QCloseEvent * event)
{
    storeConf(url());
    QWebView::closeEvent(event);
}

/// Ignore the double click event.
void BrowserView::mouseDoubleClickEvent(QMouseEvent*me)
{
    me->accept();
    // return QWebView::mouseDoubleClickEvent(me);
}

/// Stores the url if it's not in database yet. It stores thumbnail too.
void BrowserView::storeUrl()
{
    const QString & myurl = url().toString();
    if (!myurl.startsWith("http",Qt::CaseInsensitive) && !myurl.startsWith("ftp",Qt::CaseInsensitive) && !myurl.startsWith("www",Qt::CaseInsensitive))
    {
        return ;
    }

    if (site_list_.size() <= 0)
    {
        WebHistory db;
        db.loadConf(site_list_);
    }

    QString host = url().host();
    bool found = false;
    for(int i = 0; i < site_list_.size(); ++i)
    {
        webhistory::ThumbnailItem site(site_list_.at(i).toMap());
        if ((site.url().host() == host && !host.isEmpty()) ||
            (site.url() == url()))
        {
	    const QImage & image =  thumbnail(webhistory::ThumbnailItem::size());
            site.setUrl(url());
            site.setTitle(title());
            site.setThumbnail(image);
            site.updateAccessTime();
            site_list_[i] = site;
            found = true;
            saveThumbnailForExplorer();
            break;
        }
    }

    if (!found)
    {
        const QImage & image =  thumbnail(webhistory::ThumbnailItem::size());
        webhistory::ThumbnailItem item;
        item.updateAccessTime();
        item.setTitle(title());
        item.setThumbnail(image);
        item.setUrl(url());
        saveThumbnailForExplorer();
        site_list_.append(item);
    }

    sync();
}

void BrowserView::saveThumbnailForExplorer()
{
    const QImage & image =  thumbnail(webhistory::ThumbnailItem::size());
    QString filename;
    QString name = title().toLower();

    if( name.contains("host not found") ) return ;
    if( name.contains("connecting") ) return ;
    name.append(".png");

    filename =  QString("/usr/share/explorer/images/small/").append(name);
    if (! QFileInfo(filename).exists() )
    {
	    QImage small = image.scaled(QSize(32,32), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    	    small.save( filename );
    }
    
    filename =  QString("/usr/share/explorer/images/middle/").append(name);
    if (! QFileInfo(filename).exists() )
    {
	    QImage middle = image.scaled(QSize(84,84), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	    middle. save( filename );
    }

    filename =  QString("/usr/share/explorer/images/big/").append(name);
    if (! QFileInfo(filename).exists() )
    {
	    QImage big = image.scaled(QSize(124,124), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	    big.save( filename );
    }
    
    qDebug("save png %s",qPrintable(filename));
}

void BrowserView::sync()
{
    if (need_save_url_)
    {
        WebHistory db;
        db.saveConf(site_list_);
    }
}

void BrowserView::saveThumbnails()
{
    WebHistory db;
    QVariantList list;
    db.loadConf(list);

    webhistory::ThumbnailItem item;
    item.setTitle(title());
    item.setThumbnail(thumbnail(webhistory::ThumbnailItem::size()));
    item.setUrl(url());
    list.append(item);
    db.saveConf(list);
}

QImage BrowserView::thumbnail(const QSize & size)
{
    QImage image(page()->viewportSize(), QImage::Format_ARGB32);
    QPainter painter(&image);
    page()->mainFrame()->render(&painter);

    return image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void BrowserView::startDictLookup()
{
    if (!dicts_)
    {
        dicts_.reset(new DictionaryManager);
    }

    if (!dict_widget_)
    {
        dict_widget_.reset(new DictWidget(this, *dicts_));
    }

    hideHelperWidget(tts_widget_.get());
    hideHelperWidget(search_widget_.get());

    // When dictionary widget is not visible, it's necessary to update the text view.
    dict_widget_->lookup(selectedText());
    dict_widget_->ensureVisible(selected_rect_, true);
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::INVALID);
}

void BrowserView::stopDictLookup()
{
    hideHelperWidget(dict_widget_.get());
    enableTextSelection(false);
    selected_text_.clear();
}

void BrowserView::showSearchWidget()
{
    if (!search_widget_)
    {
        search_widget_.reset(new SearchWidget(this, search_context_));
        connect(search_widget_.get(), SIGNAL(search(BaseSearchContext &)),
                this, SLOT(onSearch(BaseSearchContext &)));
        connect(search_widget_.get(), SIGNAL(closeClicked()), this, SLOT(onSearchClosed()));
    }

    hideHelperWidget(dict_widget_.get());
    hideHelperWidget(tts_widget_.get());
    search_widget_->ensureVisible();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::INVALID);
}

bool BrowserView::updateSearchCriteria()
{
    search_flags_ = 0;
    search_context_.stop(false);
    if (!search_context_.forward())
    {
        search_flags_ |= QWebPage::FindBackward;
    }
    if (search_context_.case_sensitive())
    {
        search_flags_ |= QWebPage::FindCaseSensitively;
    }
    return true;
}

void BrowserView::onSearch(BaseSearchContext&)
{
    bool ret = false;
    updateSearchCriteria();
    ret = doSearch();

    // No more search result.
    if (!ret)
    {
        // Need to update the search widget to indicate that there is no more
        // matched result.
        search_widget_->noMoreMatches();
    }
}

void BrowserView::onSearchClosed()
{
    // Clear all selected text.
    clearSelection();
}

bool BrowserView::doSearch()
{
    return findText(search_context_.pattern(), search_flags_);
}

void BrowserView::enableTextSelection(bool enable)
{
    enable_text_selection_ = enable;
}

bool BrowserView::isTextSelectionEnabled()
{
    return enable_text_selection_;
}

void BrowserView::fastUpdateWidget(QRect &rect)
{
    onyx::screen::instance().enableUpdate(false);
    repaint();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidgetRegion(this, rect, onyx::screen::ScreenProxy::DW, false);
    rect.setCoords(0, 0, 0, 0);
}

bool BrowserView::isWidgetVisible(QWidget * wnd)
{
    if (wnd)
    {
        return wnd->isVisible();
    }
    return false;
}

void BrowserView::hideHelperWidget(QWidget * wnd)
{
    if (wnd)
    {
        wnd->hide();
    }
}

/// Clear highlight text by using javascript.
void BrowserView::clearSelection()
{
    page()->mainFrame()->evaluateJavaScript("document.selection.empty;");
    page()->mainFrame()->evaluateJavaScript("window.getSelection().removeAllRanges();");
}

void BrowserView::myScroll(int dx, int dy)
{
    page()->currentFrame()->scroll(dx, dy);
    updateViewportRange();
}

void BrowserView::myScrollTo(const QPoint &pt)
{
    page()->mainFrame()->setScrollPosition(pt);
    updateViewportRange();
}

QPointF BrowserView::currentOffset()
{
    return page()->currentFrame()->scrollPosition();
}

void BrowserView::updateViewportRange()
{
    // Get current location.
    QSizeF s = page()->currentFrame()->contentsSize();
    QPointF pt = currentOffset();
    emit viewportRangeChangedSignal(static_cast<int>(pt.y()),
                                    static_cast<int>(rect().height()),
                                    static_cast<int>(s.height()));
}

void BrowserView::formFocusedAddValue (const QString& form_id,
                                       const QString& form_name,
                                       const QString& form_action,
                                       const QString& input_type,
                                       const QString& input_id,
                                       const QString& input_name,
                                       const QString& value)
{
    QString scriptSource;

    if (!input_id.isEmpty())
    {
        scriptSource = "document.getElementById('" + input_id + "').value = '" + value + "';";
    }
    else if (!form_id.isEmpty())
    {
        scriptSource = 
            "var inputList = document.getElementById('" + form_id + "').getElementsByTagName('" + input_type + "');"
            "for (var i = 0; i < inputList.length; ++i) {"
                "if (inputList[i].name == '" + input_name + "') {"
                    "inputList[i].value = '" + value + "';"
                    "break;"
                "}"
            "}";
    }
    else if (!form_action.isEmpty())
    {
        scriptSource = 
            "var formList = document.getElementsByTagName('form');"
            "for (var i = 0; i < formList.length; ++i) {"
                "if (formList[i].action == '" + form_action + "') {"
                    "var inputList = formList[i].getElementsByTagName('" + input_type + "');"
                    "for (var j = 0; j < inputList.length; ++j) {"
                        "if (inputList[j].name == '" + input_name + "') {"
                            "inputList[j].value = '" + value + "';"
                            "break;"
                        "}"
                    "}"
                    "break;"
                "}"
            "}";
    }
    else if (!form_name.isEmpty())
    {
        scriptSource = 
            "var formList = document.getElementsByTagName('form');"
            "for (var i = 0; i < formList.length; ++i) {"
                "if (formList[i].name == '" + form_name + "') {"
                    "var inputList = formList[i].getElementsByTagName('" + input_type + "');"
                    "for (var j = 0; j < inputList.length; ++j) {"
                        "if (inputList[j].name == '" + input_name + "') {"
                            "inputList[j].value = '" + value + "';"
                            "break;"
                        "}"
                    "}"
                    "break;"
                "}"
            "}";
    }
    else
    {
        scriptSource = 
            "var inputList = document.getElementsByTagName('" + input_type + "');"
            "for (var i = 0; i < inputList.length; ++i) {"
                "if (inputList[i].name == '" + input_name + "') {"
                    "inputList[i].value = '" + value + "';"
                    "break;"
                "}"
            "}";
    }

    page()->mainFrame()->evaluateJavaScript(scriptSource);
}

void BrowserView::formFocused (const QString& form_id,
                               const QString& form_name,
                               const QString& form_action,
                               const QString& input_type,
                               const QString& input_id,
                               const QString& input_name)
{
    emit inputFormFocused(form_id, form_name, form_action, input_type, input_id, input_name);
}

void BrowserView::formLostFocus (void)
{
    if (!onyx::screen::instance().isUpdateEnabled())
    {
        onyx::screen::instance().enableUpdate(true);
    }
    emit inputFormLostFocus();
}

void BrowserView::populateJavaScriptWindowObject (void)
{
    page()->mainFrame()->addToJavaScriptWindowObject("__qWebViewWidget", this);
}

void BrowserView::addFormsFocusEvent(void)
{
    QString scriptSource =
        "function addFocusHandler(tagName) {"
            "var inputList = document.getElementsByTagName(tagName);"
            "for (var i = 0; i < inputList.length; ++i) {"
                "var formAction = inputList[i].form.action;"
                "var formName   = inputList[i].form.name;"
                "var formId     = inputList[i].form.id;"
                "var inputName  = inputList[i].name;"
                "var inputId    = inputList[i].id;"

                "if (tagName == 'input') {"
                    "var inputType = inputList[i].type;"
                    "if (inputType != 'password' && inputType != 'text')"
                        "continue;"
                "}"

                "inputList[i].setAttribute('onclick', \"__qWebViewWidget.formFocused('\" + formId + \"', '\" + formName + \"', '\" + formAction + \"','\" + tagName + \"','\" + inputId + \"','\" + inputName + \"')\");"
                "inputList[i].setAttribute('onblur', '__qWebViewWidget.formLostFocus()');"
            "}"
        "}"
        "addFocusHandler('input');"
        "addFocusHandler('textarea');";

    page()->mainFrame()->evaluateJavaScript(scriptSource);
}

void BrowserView::addSelectEvents(void)
{
    QString scriptSource =
        "function addSelectHandler(tagName) {"
            "var selectList = document.getElementsByTagName(tagName);"
            "for (var i = 0; i < selectList.length; ++i) {"
                "selectList[i].setAttribute('onmouseup', '__qWebViewWidget.selectMouseUp()');"
                "selectList[i].setAttribute('onfocus', '__qWebViewWidget.selectFocus()');"
                "selectList[i].setAttribute('onblur', '__qWebViewWidget.selectBlur()');"
                "selectList[i].setAttribute('onchange', '__qWebViewWidget.selectChanged()');"
            "}"
        "}"
        "addSelectHandler('select');";

    page()->mainFrame()->evaluateJavaScript(scriptSource);
}

static void collectPopupMenus(const QObject *parent,
                              QWidgets &widgets)
{
    const QObjectList & all = parent->children();
    foreach(const QObject *object, all)
    {
         
        if (object->isWidgetType())
        {
            QComboBox * wnd = const_cast<QComboBox *>(static_cast<const QComboBox *>(object));
            if (wnd && wnd->isVisible())
            {
                widgets.push_back(wnd);
                collectPopupMenus(wnd, widgets);
            }
        }
    }
}

void BrowserView::selectMouseUp()
{
    QTimer::singleShot(0, this, SLOT(handleSelectPopup()));
}

void BrowserView::clearFocusWidgets()
{
    if (!focus_widgets_.isEmpty())
    {
        foreach(QWidget *widget, focus_widgets_)
        {
            widget->removeEventFilter(this);
        }
        focus_widgets_.clear();
    }
}

void BrowserView::handleSelectPopup()
{
    qDebug("handle select popup.");
    QWidget * focus_widget = QApplication::focusWidget();
    if (focus_widget != 0 && focus_widget != this)
    {
        // clear previous focused widgets
        clearFocusWidgets();

        // install event filters for new focused widgets
        collectPopupMenus(focus_widget, focus_widgets_);
        foreach(QWidget *widget, focus_widgets_)
        {
            widget->installEventFilter(this);
        }
    }
}

void BrowserView::selectFocus()
{
    qDebug("Select Focus");
    QTimer::singleShot(0, this, SLOT(handleSelectPopup()));
}

void BrowserView::selectBlur()
{
    clearFocusWidgets();
}

void BrowserView::selectChanged()
{
    qDebug("Select Changed");
}

bool BrowserView::eventFilter(QObject *obj, QEvent *e)
{
    qDebug("Select event:%d", e->type());
    if (e->type() == QEvent::MouseButtonRelease && obj->isWidgetType())
    {
        onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    }
    return QObject::eventFilter(obj, e);
}

void BrowserView::focusOutEvent(QFocusEvent * e)
{
    qDebug("Focus Out Event");
    emit focusOut();
    QWebView::focusOutEvent(e);
}

void BrowserView::storeConf(const QUrl & url)
{
    if (url.isEmpty())
    {
        return;
    }

    QString path = url.path();
    QFileInfo info(path);
    if (!info.exists())
    {
        return;
    }

    ContentManager database;
    Configuration conf;
    if (!openDatabase(path, database))
    {
        return;
    }

    loadDocumentOptions(database, path, conf);

    QString progress("%1%");
    int height = page()->mainFrame()->contentsSize().height();
    QPointF pt = currentOffset();
    int value = static_cast<int>(pt.y() * 100) / height;
    conf.info.mutable_progress() = progress.arg(value);
    vbf::saveDocumentOptions(database, path, conf);
}

void BrowserView::clearHistory()
{
    WebHistory db;
    db.clear();
}

void BrowserView::displayBookmarks()
{
    if (bookmark_model_ == 0) return;

    QStandardItemModel bookmarks_model;
    bookmark_model_->getBookmarksModel(bookmarks_model);

    TreeViewDialog bookmarks_view(this);
    bookmarks_view.setModel(&bookmarks_model);
    bookmarks_view.tree().showHeader(true);

    QVector<int> percentages;
    percentages.push_back(50);
    percentages.push_back(50);
    bookmarks_view.tree().setColumnWidth(percentages);
    int ret = bookmarks_view.popup(tr("Bookmarks"));

    // Returned from the TOC view
    onyx::screen::instance().enableUpdate( false );
    while (QApplication::hasPendingEvents())
    {
        QApplication::processEvents();
    }
    onyx::screen::instance().enableUpdate( true );

    if (ret != QDialog::Accepted)
    {
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        return;
    }

    QModelIndex index = bookmarks_view.selectedItem();
    if ( !index.isValid() )
    {
        return;
    }

    QStandardItem *item = bookmarks_model.itemFromIndex( index );
    QString url = item->data().toString();
    load(QUrl(url));
}

bool BrowserView::addBookmark()
{
    if (bookmark_model_ == 0) return false;

    if (bookmark_model_->addBookmark(title(), url().toString()))
    {
        bookmark_model_->saveBookmarks();
        update();
        return true;
    }
    return false;
}

bool BrowserView::deleteBookmark()
{
    if (bookmark_model_ == 0) return false;

    QStandardItemModel bookmarks_model;
    bookmark_model_->getBookmarksModel(bookmarks_model);

    TreeViewDialog bookmarks_view(this);
    bookmarks_view.setModel(&bookmarks_model);
    bookmarks_view.tree().showHeader(true);

    QVector<int> percentages;
    percentages.push_back(50);
    percentages.push_back(50);
    bookmarks_view.tree().setColumnWidth(percentages);
    int ret = bookmarks_view.popup(tr("Delete Bookmarks"));

    // Returned from the TOC view
    onyx::screen::instance().enableUpdate( false );
    while (QApplication::hasPendingEvents())
    {
        QApplication::processEvents();
    }
    onyx::screen::instance().enableUpdate( true );
    if (ret != QDialog::Accepted)
    {
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        return false;
    }

    QModelIndex index = bookmarks_view.selectedItem();
    if ( !index.isValid() )
    {
        return false;
    }

    QStandardItem *item = bookmarks_model.itemFromIndex( index );
    QString url = item->data().toString();
    if (bookmark_model_->deleteBookmark(title(), url))
    {
        bookmark_model_->saveBookmarks();
        update();
        return true;
    }
    return false;
}

bool BrowserView::deletePassword()
{
    network_service::PasswordModel passwords;
    QStandardItemModel passwords_model;
    passwords.getModel(passwords_model);

    TreeViewDialog passwords_view(this);
    passwords_view.setModel(&passwords_model);
    passwords_view.tree().showHeader(true);

    QVector<int> percentages;
    percentages.push_back(50);
    percentages.push_back(50);
    passwords_view.tree().setColumnWidth(percentages);
    int ret = passwords_view.popup(tr("Delete Passwords"));

    // Returned from the TOC view
    onyx::screen::instance().enableUpdate( false );
    while (QApplication::hasPendingEvents())
    {
        QApplication::processEvents();
    }
    onyx::screen::instance().enableUpdate( true );
    if (ret != QDialog::Accepted)
    {
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        return false;
    }

    QModelIndex index = passwords_view.selectedItem();
    if ( !index.isValid() )
    {
        return false;
    }

    QStandardItem *item = passwords_model.itemFromIndex( index );
    QString url = item->data().toString();
    if (passwords.removePassword(url))
    {
        update();
        return true;
    }
    return false;
}

void BrowserView::clearCookies()
{
    WebApplication::accessManager()->clearCookies();
}



}
