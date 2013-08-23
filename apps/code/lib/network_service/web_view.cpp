#include <QtGui/QtGui>
#include "web_view.h"
#include "proxy_settings_dialog.h"
#include "auto_complete.h"
#include "password_model.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/wireless/wifi_dialog.h"
#include "onyx/sys/sys_status.h"
#include "onyx/data/configuration.h"

using namespace ui;
using namespace cms;
using namespace vbf;

namespace network_service
{

#define USE_JQUERY

static const int PAGE_REPEAT = 20;
static const int DELTA = 10;

WebView::WebView(QWidget *parent,
                 NetworkAccessManager * access_manager,
                 DownloadManager * download_manager)
    : QWebView(parent)
    , access_manager_(access_manager)
    , download_manager_(download_manager)
    , scrollbar_hidden_(0)
    , progress_(0)
    , update_type_(onyx::screen::ScreenProxy::GU)
    , system_update_type_(onyx::screen::instance().defaultWaveform())
    , enable_text_selection_(false)
    , page_(new WebPage(this, access_manager))
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

    connect(access_manager, SIGNAL(requestSavePassword(const QByteArray &)),
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

WebView::~WebView()
{
    onyx::screen::instance().setDefaultWaveform(system_update_type_);
}

void WebView::hideGif()
{
    QString code = "$('[src*=gif]').hide()";
    page()->mainFrame()->evaluateJavaScript(code);
}

void WebView::hideInlineFrames()
{
    QString code = "$('iframe').remove()";
    page()->mainFrame()->evaluateJavaScript(code);
}

void WebView::hideObjectElements()
{
    QString code = "$('object').remove()";
    page()->mainFrame()->evaluateJavaScript(code);
}

void WebView::hideEmbeddedElements()
{
    QString code = "$('embed').remove()";
    page()->mainFrame()->evaluateJavaScript(code);
}

void WebView::hideScrollbar()
{
    QString code = "$('body').css('overflow', 'hidden')";
    page()->mainFrame()->evaluateJavaScript(code);
}

void WebView::onLinkClicked(const QUrl &new_url)
{
    qDebug("url clicked %s", qPrintable(new_url.toString()));
    load(new_url);
}

void WebView::onLoadStarted(void)
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

void WebView::onSavePassword(const QByteArray & data)
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

void WebView::onLoadProgress(int progress)
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

void WebView::onLoadFinished(bool ok)
{
    // Restore the screen update type.
    onyx::screen::instance().setDefaultWaveform(update_type_);

    // Ensure we can get a gc update.
    progress_ = 100;
    update_rect_ = rect();

    // Check if we need to store the thumbnail.
    if (ok)
    {
        if (access_manager_!= 0 && access_manager_->autoComplete())
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
            networkDialog().connectToDefaultAP();

            // let user configure the network manually, otherwise it would crash
            QTimer::singleShot(400, this, SLOT(configNetwork()));
        }
    }
    updateViewportRange();
}

bool WebView::needConfigNetwork()
{
    QNetworkReply::NetworkError error = page_->networkError();
    return (error == QNetworkReply::ConnectionRefusedError ||
            error == QNetworkReply::HostNotFoundError ||
            error == QNetworkReply::TimeoutError ||
            error == QNetworkReply::UnknownNetworkError);
}

void WebView::onRepaintRequested(const QRect& rc)
{
    update_rect_ = update_rect_.united(rc);
    if (isTextSelectionEnabled())
    {
        selected_rect_ = selected_rect_.unite(rc);
        selected_rect_ = selected_rect_.intersect(rect());
    }
}

void WebView::onConnectionChanged(WifiProfile& profile, WpaConnection::ConnectionState state)
{
    // qDebug("WebView::onConnectionChanged");
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

void WebView::onDownloadRequested(const QNetworkRequest & request)
{
    if (download_manager_ != 0)
    {
        download_manager_->download(request);
    }
}

void WebView::reloadCurrentUrl()
{
    if (current_url_.isValid())
    {
        load(current_url_);
    }
}

void WebView::configNetwork()
{
    current_url_ = url();
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);

    // TODO. Check the return value then reload the page.
    networkDialog().popup(true);
    QTimer::singleShot(400, this, SLOT(reloadCurrentUrl()));
    onyx::screen::instance().setDefaultWaveform(update_type_);
}

void WebView::scan()
{
    networkDialog().triggerScan();
}

void WebView::connectTo(const QString &ssid, const QString &psk)
{
    networkDialog().connect(ssid, psk);
}

WifiDialog & WebView::networkDialog()
{
    if (!conf_dialog_)
    {
        conf_dialog_.reset(new WifiDialog(0, SysStatus::instance()));
    }
    return *conf_dialog_;
}

void WebView::onRangeClicked(const int percentage,
                                 const int value)
{
    int height = page()->mainFrame()->contentsSize().height();
    QPointF pt = currentOffset();
    pt.ry() = height * percentage / 100 - rect().height();
    myScrollTo(pt.toPoint());
    updateViewportRange();
}

void WebView::reportCurrentProcess()
{
    emit progressChangedSignal(progress_, 100);
}

void WebView::openMusicPlayer()
{
    sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
}

void WebView::configureProxy()
{
    network_service::ProxySettingsDialog proxy_setting_dialog(0);
    if (proxy_setting_dialog.popup() == QDialog::Accepted && access_manager_ != 0)
    {
        access_manager_->loadSettings();
    }
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

QWebView *WebView::createWindow(QWebPage::WebWindowType type)
{
    return this;
}

void WebView::mousePressEvent(QMouseEvent*me)
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

void WebView::mouseMoveEvent(QMouseEvent *me)
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

void WebView::mouseReleaseEvent(QMouseEvent*me)
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

void WebView::keyPressEvent(QKeyEvent *e)
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

void WebView::keyReleaseEvent(QKeyEvent *ke)
{
    switch (ke->key())
    {
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

void WebView::contextMenuEvent(QContextMenuEvent * event)
{
    event->accept();
}

/// Ignore the double click event.
void WebView::mouseDoubleClickEvent(QMouseEvent*me)
{
    me->accept();
    // return QWebView::mouseDoubleClickEvent(me);
}

void WebView::startDictLookup()
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

void WebView::stopDictLookup()
{
    hideHelperWidget(dict_widget_.get());
    enableTextSelection(false);
    selected_text_.clear();
}

void WebView::showSearchWidget()
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

bool WebView::updateSearchCriteria()
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

void WebView::onSearch(BaseSearchContext&)
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

void WebView::onSearchClosed()
{
    // Clear all selected text.
    clearSelection();
}

bool WebView::doSearch()
{
    return findText(search_context_.pattern(), search_flags_);
}

void WebView::enableTextSelection(bool enable)
{
    enable_text_selection_ = enable;
}

bool WebView::isTextSelectionEnabled()
{
    return enable_text_selection_;
}

void WebView::fastUpdateWidget(QRect &rect)
{
    onyx::screen::instance().enableUpdate(false);
    repaint();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidgetRegion(this, rect, onyx::screen::ScreenProxy::DW, false);
    rect.setCoords(0, 0, 0, 0);
}

bool WebView::isWidgetVisible(QWidget * wnd)
{
    if (wnd)
    {
        return wnd->isVisible();
    }
    return false;
}

void WebView::hideHelperWidget(QWidget * wnd)
{
    if (wnd)
    {
        wnd->hide();
    }
}

/// Clear highlight text by using javascript.
void WebView::clearSelection()
{
    page()->mainFrame()->evaluateJavaScript("document.selection.empty;");
    page()->mainFrame()->evaluateJavaScript("window.getSelection().removeAllRanges();");
}

void WebView::myScroll(int dx, int dy)
{
    page()->currentFrame()->scroll(dx, dy);
    updateViewportRange();
}

void WebView::myScrollTo(const QPoint &pt)
{
    page()->mainFrame()->setScrollPosition(pt);
    updateViewportRange();
}

QPointF WebView::currentOffset()
{
    return page()->currentFrame()->scrollPosition();
}

void WebView::updateViewportRange()
{
    // Get current location.
    QSizeF s = page()->currentFrame()->contentsSize();
    QPointF pt = currentOffset();
    emit viewportRangeChangedSignal(static_cast<int>(pt.y()),
                                    static_cast<int>(rect().height()),
                                    static_cast<int>(s.height()));
}

void WebView::formFocusedAddValue (const QString& form_id,
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

void WebView::formFocused (const QString& form_id,
                               const QString& form_name,
                               const QString& form_action,
                               const QString& input_type,
                               const QString& input_id,
                               const QString& input_name)
{
    emit inputFormFocused(form_id, form_name, form_action, input_type, input_id, input_name);
}

void WebView::formLostFocus (void)
{
    if (!onyx::screen::instance().isUpdateEnabled())
    {
        onyx::screen::instance().enableUpdate(true);
    }
    emit inputFormLostFocus();
}

void WebView::populateJavaScriptWindowObject (void)
{
    page()->mainFrame()->addToJavaScriptWindowObject("__qWebViewWidget", this);
}

void WebView::addFormsFocusEvent(void)
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

void WebView::addSelectEvents(void)
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

void WebView::selectMouseUp()
{
    QTimer::singleShot(0, this, SLOT(handleSelectPopup()));
}

void WebView::clearFocusWidgets()
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

void WebView::handleSelectPopup()
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

void WebView::selectFocus()
{
    qDebug("Select Focus");
    QTimer::singleShot(0, this, SLOT(handleSelectPopup()));
}

void WebView::selectBlur()
{
    clearFocusWidgets();
}

void WebView::selectChanged()
{
    qDebug("Select Changed");
}

bool WebView::eventFilter(QObject *obj, QEvent *e)
{
    qDebug("Select event:%d", e->type());
    if (e->type() == QEvent::MouseButtonRelease && obj->isWidgetType())
    {
        onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    }
    return QObject::eventFilter(obj, e);
}

void WebView::focusOutEvent(QFocusEvent * e)
{
    qDebug("Focus Out Event");
    emit focusOut();
    QWebView::focusOutEvent(e);
}

bool WebView::deletePassword()
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

void WebView::clearCookies()
{
    access_manager_->clearCookies();
}

}
