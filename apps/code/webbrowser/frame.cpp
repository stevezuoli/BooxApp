#include <QtGui/QtGui>
#include "frame.h"
#include "onyx/data/web_history.h"
#include "onyx/screen/screen_proxy.h"
#include "web_application.h"
#include "private_conf/conf.h"

using namespace ui;
using namespace network_service;
using namespace webhistory;
namespace webbrowser
{

BrowserFrame::BrowserFrame(QWidget *parent)
#ifdef Q_WS_QWS
    : QWidget(parent, Qt::FramelessWindowHint)
#else
    : QWidget(parent)
#endif
    , layout_(this)
    , view_(0)
    , thumbnail_view_(0)
    , sys_(SysStatus::instance())
    , status_bar_(0, MENU|PROGRESS|MESSAGE|BATTERY|CLOCK|SCREEN_REFRESH|INPUT_URL|INPUT_TEXT)
    , download_view_(0, WebApplication::downloadManager())
    , need_gc_in_loading_(true)
    , keyboard_status_(KEYBOARD_FREE)
{
    // setAttribute(Qt::WA_DeleteOnClose, true);
#ifndef Q_WS_QWS
    resize(600, 800);
#else
    resize(qApp->desktop()->screenGeometry().size());
#endif

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    createLayout();

    connect(&view_, SIGNAL(progressChangedSignal(const int, const int)),
            this, SLOT(onProgressChanged(const int, const int)));
    connect(&view_, SIGNAL(showHome()), this, SLOT(showHomePage()));
    connect(&view_, SIGNAL(connectionChanged(WifiProfile&, WpaConnection::ConnectionState)),
            this, SLOT(onConnectionChanged(WifiProfile&, WpaConnection::ConnectionState)));
    connect(&view_, SIGNAL(viewportRangeChangedSignal(const int, const int, const int)),
            this, SLOT(onRangeChanged(const int, const int, const int)));

    connect(&view_, SIGNAL(inputFormFocused(const QString&, const QString&,
                                            const QString&, const QString&,
                                            const QString&, const QString&)),
            this, SLOT(onInputFormFocused(const QString&, const QString&,
                                          const QString&, const QString&,
                                          const QString&, const QString&)));
    connect(&view_, SIGNAL(inputFormLostFocus()), &keyboard_, SLOT(hide()));
    connect(&view_, SIGNAL(inputFormLostFocus()), &input_widget_, SLOT(hide()));
    connect(&view_, SIGNAL(requestOTA(const QUrl&)), this, SLOT(onRequestOTA(const QUrl&)));
    connect(&view_, SIGNAL(focusOut()), this, SLOT(onWebViewFocusOut()));

    // Keyboard
    connect(&keyboard_, SIGNAL(textFinsihed(const QString&)), this, SLOT(onTextFinished(const QString&)));

    // Status bar
    status_bar_.enableJumpToPage(false);
    connect(&status_bar_, SIGNAL(progressClicked(const int, const int)),
            this, SLOT(onProgressClicked(const int, const int)));
    connect(&status_bar_, SIGNAL(menuClicked()), &view_, SLOT(popupMenu()));
    connect(&status_bar_, SIGNAL(requestInputUrl()), this, SLOT(onInputUrl()));
    connect(&status_bar_, SIGNAL(requestInputText()), this, SLOT(onInputText()));

    connect(&thumbnail_view_, SIGNAL(clicked(QStandardItem *)),
            this, SLOT(onThumbnailClicked(QStandardItem *)));
    connect(&thumbnail_view_, SIGNAL(escape()),
            this, SLOT(onThumbnailEscape()));
    connect(&thumbnail_view_, SIGNAL(positionChanged(const int, const int)),
            this, SLOT(onThumbnailPositionChanged(const int, const int)));

    // Input widget
    input_widget_.attachReceiver(this);

    // connect the signal of system status
    connect(&sys_, SIGNAL(musicPlayerStateChanged(int)),
            this, SLOT(onMusicPlayerStateChanged(int)));
    connect(&sys_, SIGNAL(requestDRMUserInfo(const QString &, const QString &)),
            this, SLOT(onRequestDRMUserInfo(const QString &, const QString &)));
    connect(&sys_, SIGNAL(fulfillmentFinished(const QString &)),
            this, SLOT(onFulfillmentFinished(const QString &)));
    connect(&sys_, SIGNAL(reportWorkflowError(const QString &, const QString &)),
            this, SLOT(onReportWorkflowError(const QString &, const QString &)));

    connect(&download_view_, SIGNAL(loadFinished(const QString&)),
            this, SLOT(onLoadFinished(const QString&)));

#ifdef Q_WS_QWS
    connect(qApp->desktop(), SIGNAL(resized(int)),
            this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif
}

BrowserFrame::~BrowserFrame()
{
}

void BrowserFrame::attachBookmarkModel(BookmarkModel * model)
{
    view_.attachBookmarkModel(model);
}

void BrowserFrame::load(const QString & url_str)
{
    // Depends on the url is local file or not, if the url is empty
    // we need to display the thumbnail view.
    // Check if it's a local file or not at first.
    QUrl url = guessUrlFromString(url_str);
    if (!url.isValid())
    {
        showHomePage();
    }
    else
    {
        showThumbnailView(false);

        QString host = url.host();
        if (!host.isEmpty())
        {
            QString message = tr("Connecting to %1");
            message = message.arg(host);
            status_bar_.showItem(ui::PROGRESS, false);
            status_bar_.setMessage(message);
        }
        view_.load(url);
    }
}

bool BrowserFrame::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip ||
        e->type() == QEvent::HoverMove ||
        e->type() == QEvent::HoverEnter ||
        e->type() == QEvent::HoverLeave)
    {
        e->accept();
        return true;
    }

    bool ret = QWidget::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        // Check region.
        QRect rc = view_.updateRect();
        rc = rc.intersected(view_.rect());

        view_.resetUpdateRect();
        int area = rc.width() * rc.height();
        if (area > 0 && onyx::screen::instance().isUpdateEnabled())
        {
            qDebug("update rectange:(%d, %d, %d, %d)", rc.left(), rc.top(), rc.width(), rc.height());
            int max = view_.rect().width() * view_.rect().height();
            /*if (area < (max >> 3))
            {
                qDebug("Ignored the request");
                return true;
            }
            else*/
            if (rc.width() < (view_.width() - (view_.width() >> 2)) ||
                rc.height() < (view_.height() - (view_.height() >> 2)))
            {
                qDebug("using gu waveform");
                onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
                return true;
            }

            if (area > (max - (max >> 2)))
            {
                if (!view_.isLoadingFinished() && need_gc_in_loading_)
                {
                    qDebug("using gc waveform");
                    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
                    need_gc_in_loading_ = false;
                    return true;
                }
            }
        }

        qDebug("using default waveform");
        onyx::screen::instance().updateWidget(this);
        return true;
    }
    return ret;
}

void BrowserFrame::showThumbnailView(bool show)
{
    thumbnail_view_.setVisible(show);
    view_.setVisible(!show);
    tool_bar_.setVisible(!show);

    if (show)
    {
        thumbnail_view_.setFocus();
        thumbnail_view_.updateTitle(QApplication::tr("History"));
        status_bar_.showItem(ui::MENU, false);
        sys::SysStatus::instance().setSystemBusy(false);
    }
    else
    {
        status_bar_.showItem(ui::MENU, true);
        view_.reportCurrentProcess();
        view_.setFocus();
    }
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
}

void BrowserFrame::onThumbnailClicked(QStandardItem *selected_item)
{
    if (selected_item)
    {
        showThumbnailView(false);
        QUrl url = selected_item->data().toUrl();
        qDebug("item title %s", qPrintable(selected_item->text()));
        qDebug("scheme %s", qPrintable(url.scheme()));
        qDebug("path %s", qPrintable(url.path()));
        view_.load(url);
    }
}

void BrowserFrame::onThumbnailEscape()
{
    showThumbnailView(false);
}

void BrowserFrame::onMusicPlayerStateChanged(int state)
{
    if (state == sys::HIDE_PLAYER || state == sys::STOP_PLAYER)
    {
        onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
    }
}

void BrowserFrame::onThumbnailPositionChanged(const int current,
                                              const int total)
{
    status_bar_.showItem(ui::PROGRESS);
    status_bar_.setProgress(current, total);
}

void BrowserFrame::showHomePage()
{
    loadThumbnails();
    showThumbnailView(true);
}

void BrowserFrame::onConnectionChanged(WifiProfile& profile,
                                       WpaConnection::ConnectionState state)
{
    if (state == WpaConnection::STATE_CONNECTING)
    {
        QString message(tr("Connecting 1%"));
        message = message.arg(profile.ssid());
        status_bar_.setMessage(message);
    }
}

void BrowserFrame::createLayout()
{
    layout_.setContentsMargins(0, 0, 0, 0);
    layout_.setSpacing(1);

    setupToolbar();
    layout_.addWidget(&tool_bar_);
    layout_.addWidget(&view_);
    layout_.addWidget(&keyboard_);
    layout_.addWidget(&input_widget_);
    keyboard_.setVisible(false);
    input_widget_.setVisible(false);

    layout_.addWidget(&thumbnail_view_);
    layout_.addWidget(&download_view_);
    download_view_.setVisible(false);

    layout_.addWidget(&status_bar_);

    connect(WebApplication::downloadManager(), SIGNAL(itemAdded(DownloadItem *)),
            this, SLOT(onDownloadItemAdded(DownloadItem *)));
}

void BrowserFrame::setupToolbar()
{
    // TODO, we will change the icon later.
    QWebPage::WebAction actions[] = { QWebPage::Back, QWebPage::Forward, QWebPage::Reload, QWebPage::Stop };
    const QString icons[] = {
        ":/images/toolbar_backward.png",
        ":/images/toolbar_forward.png",
        ":/images/toolbar_refresh.png",
        ":/images/toolbar_stop.png"};

    for(int i = 0; i < sizeof(actions)/sizeof(actions[0]); ++i)
    {
        QAction * action = view_.pageAction(actions[i]);
        action->setIcon(QIcon(icons[i]));
        tool_bar_.addAction(action);
    }
}

void BrowserFrame::loadThumbnails()
{
    thumbnailModel(model_);
    thumbnail_view_.attachModel(&model_);
}

void BrowserFrame::onDownloadItemAdded(DownloadItem *item)
{
    QString file_name = item->fileName();
    if (file_name.endsWith(".acsm"))
    {
        connect(item, SIGNAL(loadFinished()), this, SLOT(onACSMDownloaded()));
        connect(item, SIGNAL(toBeDeleted()), this, SLOT(onACSMItemDeleted()));
    }
    else
    {
        if (!download_view_.isVisible())
        {
            download_view_.setVisible(true);
        }
        download_view_.appendWidget(item);
    }
}

void BrowserFrame::onACSMItemDeleted()
{
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    disconnect(item, SIGNAL(loadFinished()), this, SLOT(onLoadFinished()));
    disconnect(item, SIGNAL(toBeDeleted()), this, SLOT(removeItem()));
}

void BrowserFrame::onACSMDownloaded()
{
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    QString file_name = item->fileName();
    onRequestOTA(file_name, item->url());
}

void BrowserFrame::onRequestOTA(const QUrl & url)
{
    QString url_str = url.toString();
    QStringList url_strs;
    url_strs << url_str;
    url_strs << "fulfill";
    sys_.startDRMService(url_strs);
    view_.displayFulfillmentProcessing(url_str);
}

void BrowserFrame::onRequestOTA(const QString & acsm, const QUrl & url)
{
    QStringList param_strs;
    param_strs << acsm;
    param_strs << "acsm";
    sys_.startDRMService(param_strs);
    view_.displayFulfillmentProcessing(url.toString());
}

void BrowserFrame::onWebViewFocusOut()
{
    if (input_widget_.isVisible())
    {
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
    }
}

void BrowserFrame::onRequestDRMUserInfo(const QString & url_str, const QString & param)
{
    QUrl url = network_service::guessUrlFromString(url_str);
    if (!url.isValid())
    {
        qDebug("Invalid DRM URL");
        return;
    }

    QString host = url.host();
    if (host.isEmpty())
    {
        qDebug("Invalid DRM Host");
        return;
    }
    qDebug("Activation Host:%s", qPrintable(host));

    PrivateConfig sys_conf;
    DRMInfo info;
    sys_conf.getDRMInfo(host, info);

    QString title = info.id().isEmpty() ? QApplication::tr("Activate Device") : QApplication::tr("Sign In");
    SignInDialog sign_in_dialog(this, title);
    if (!info.id().isEmpty())
    {
        sign_in_dialog.disableID(true);
    }

    if (sign_in_dialog.popup(info.id(), QString()) != QDialog::Accepted)
    {
        onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GC);
        view_.displayFulfillmentAborted(param);
        return;
    }
    info.set_id(sign_in_dialog.id());
    info.set_password(sign_in_dialog.password());
    info.set_url(host);
    sys_conf.updateDRMInfo(info);

    // restart the DRM service
    QStringList param_strs;
    param_strs << param;
    param_strs << "acsm";
    sys_.startDRMService(param_strs);
    view_.displayFulfillmentProcessing(url.toString());
}

void BrowserFrame::onReportWorkflowError(const QString & workflow, const QString & error_code)
{
    /*QString text = QCoreApplication::tr("Error of Workflow:%1.\n%2.");
    text = text.arg(workflow).arg(error_code);
    ErrorDialog dialog(text, 0);
    dialog.exec();
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GC);*/
    qDebug("Workflow:%s, Error:%s", qPrintable(workflow), qPrintable(error_code));
    view_.displayFulfillmentError(workflow, error_code);
}

void BrowserFrame::thumbnailModel(QStandardItemModel & model)
{
    model.clear();
    WebHistory db;
    QVariantList list;
    db.loadConf(list);
    qSort(list.begin(), list.end(), webhistory::GreaterByAccessTime());

    foreach(QVariant var, list)
    {
        webhistory::ThumbnailItem t = var.toMap();
        QStandardItem * item = new QStandardItem(t.title());
        item->setData(t.url());
        item->setData(t.thumbnail(), ui::THUMB_IMAGE);
        model.appendRow(item);
    }
}

void BrowserFrame::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Escape:
        if (thumbnail_view_.isVisible())
        {
            showThumbnailView(false);
        }
        else
        {
            view_.returnToLibrary();
        }
        break;
    default:
        QWidget::keyReleaseEvent(ke);
        break;
    }
}

/// The keyPressEvent could be sent from virtual keyboard.
void BrowserFrame::keyPressEvent(QKeyEvent * ke)
{
    ke->accept();

    // Disable the parent widget to update screen.
    onyx::screen::instance().enableUpdate(false);

    QKeyEvent * key_event = new QKeyEvent(ke->type(), ke->key(), ke->modifiers(), ke->text());
    QApplication::postEvent(&view_, key_event);

    while (QApplication::hasPendingEvents())
    {
        QApplication::processEvents();
    }
    onyx::screen::instance().enableUpdate(true);

    // Update the line edit.
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
}

void BrowserFrame::closeEvent(QCloseEvent *e)
{
    QWidget::closeEvent(e);
}

void BrowserFrame::configNetwork()
{
    view_.configNetwork();
}

void BrowserFrame::scan()
{
    view_.scan();
}

void BrowserFrame::connectTo(const QString &ssid, const QString &psk)
{
    view_.connectTo(ssid, psk);
}

void BrowserFrame::onProgressChanged(const int current,
                                     const int total)
{
    if (!view_.isVisible())
    {
        return;
    }

    if (current == 0 || view_.isLoadingFinished())
    {
        need_gc_in_loading_ = true;
    }

    if (current >= 0)
    {
        QString message(tr("Loading... %1%"));
        message = message.arg(current);
        status_bar_.showItem(ui::PROGRESS, false);
        status_bar_.setMessage(message);
    }
    else
    {
        QString message(tr("Failed to load page."));
        status_bar_.showItem(ui::PROGRESS, false);
        status_bar_.setMessage(message);
    }
}

void BrowserFrame::onRangeChanged(const int current,
                                  const int step,
                                  const int total)
{
    if (!view_.isVisible() || total <= 0)
    {
        return;
    }

    status_bar_.showItem(ui::PROGRESS);
    status_bar_.setProgress((current + step) * 100 / total, 100, false);
    QString message(tr("%1%"));
    message = message.arg((current + step) * 100 / total);
    status_bar_.setMessage(message);
}

void BrowserFrame::onProgressClicked(const int percent,
                                     const int value)
{
    if (view_.isVisible())
    {
        view_.onRangeClicked(percent, value);
    }
    else if (thumbnail_view_.isVisible())
    {
        thumbnail_view_.jumpToScreen(value - 1);
    }
}

void BrowserFrame::onScreenSizeChanged(int)
{
    onyx::screen::instance().enableUpdate(false);
    resize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC);
}

void BrowserFrame::onInputFormFocused(const QString& form_id,
                                      const QString& form_name,
                                      const QString& form_action,
                                      const QString& input_type,
                                      const QString& input_id,
                                      const QString& input_name)
{
    // fill keyboard private data
    keyboard_priv_.form_action = form_action;
    keyboard_priv_.form_id     = form_id;
    keyboard_priv_.form_name   = form_name;
    keyboard_priv_.input_type  = input_type;
    keyboard_priv_.input_id    = input_id;
    keyboard_priv_.input_name  = input_name;

    // update keyboard status
    keyboard_status_ = FORM_FOCUSED;

    input_widget_.setVisible(false);
    keyboard_.setVisible(true);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void BrowserFrame::onTextFinished(const QString& text)
{
    // reset keyboard status
    if (keyboard_status_ == FORM_FOCUSED)
    {
        view_.formFocusedAddValue(keyboard_priv_.form_id,
                                  keyboard_priv_.form_name,
                                  keyboard_priv_.form_action,
                                  keyboard_priv_.input_type,
                                  keyboard_priv_.input_id,
                                  keyboard_priv_.input_name,
                                  text);
    }

    // refresh screen
    keyboard_.hide();
    keyboard_.clearText();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);

    if (keyboard_status_ == URL_INPUTTING)
    {
        load(text);
    }
    keyboard_status_ = KEYBOARD_FREE;
}

void BrowserFrame::onInputUrl()
{
    if (keyboard_status_ != URL_INPUTTING || keyboard_.isHidden())
    {
        // update keyboard status
        keyboard_status_ = URL_INPUTTING;
        input_widget_.setVisible(false);
        keyboard_.setVisible(true);
    }
    else if (keyboard_.isVisible())
    {
        keyboard_.setVisible(false);
        keyboard_status_ = KEYBOARD_FREE;
    }
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void BrowserFrame::onInputText()
{
    if (input_widget_.isHidden())
    {
        input_widget_.setVisible(true);
        keyboard_.setVisible(false);
    }
    else
    {
        input_widget_.setVisible(false);
    }
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void BrowserFrame::onLoadFinished(const QString &file_name)
{
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
    sys::SysStatus::instance().reportDownloadState(file_name, 100, true);
    view_.returnToLibrary();
}

void BrowserFrame::onFulfillmentFinished(const QString & file_path)
{
    if (file_path.isEmpty())
    {
        return;
    }

    view_.displayFulfillmentDone(file_path);
    MessageDialog dialog(QMessageBox::Information,
                         tr("Fulfillment Finished"),
                         tr("Do you want to open the file?"),
                         QMessageBox::Yes|QMessageBox::No);
    if (dialog.exec() != QMessageBox::Yes)
    {
        onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GC);
        return;
    }
    onLoadFinished(file_path);
}

}
