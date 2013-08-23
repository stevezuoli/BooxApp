// -*- mode: c++; c-basic-offset: 4; -*-

#include "webapp_frame.h"
#include "webapp_application.h"

namespace webapp
{

static onyx::screen::ScreenProxy::Waveform getWaveformByString(
    const QString & str)
{
    onyx::screen::ScreenProxy::Waveform waveform =
        onyx::screen::ScreenProxy::GC;
    if (str.startsWith("GU"))
    {
        waveform = onyx::screen::ScreenProxy::GU;
    }
    else if (str.startsWith("DW"))
    {
        waveform = onyx::screen::ScreenProxy::DW;
    }
    return waveform;
}

WebFrame::WebFrame(QWidget *parent)
#ifdef Q_WS_QWS
    : QWidget(parent, Qt::FramelessWindowHint)
#else
    : QWidget(parent)
#endif
    , layout_(this)
    , web_view_(0, WebApplication::accessManager(), WebApplication::downloadManager())
    , download_view_(0, WebApplication::downloadManager())
    , web_view_waveform_(onyx::screen::ScreenProxy::GC)
    , sys_(SysStatus::instance())
    , status_bar_(0, MENU|MESSAGE|BATTERY|CLOCK|SCREEN_REFRESH|INPUT_URL|INPUT_TEXT)
    , download_manager_(0, &web_view_)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
#ifndef Q_WS_QWS
    resize(600, 800);
#else
    resize(qApp->desktop()->screenGeometry().size());
#endif

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    createLayout();

    connect(web_view_.page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(populateJavaScriptWindowObject()));
    connect(&download_view_, SIGNAL(loadFinished(const QString&)),
            this, SLOT(onLoadFinished(const QString&)));
    connect(&download_manager_, SIGNAL(requestDownload(const QUrl&, bool)),
            WebApplication::downloadManager(), SLOT(download(const QUrl&, bool)));
    connect(WebApplication::downloadManager(), SIGNAL(itemAdded(DownloadItem *)),
            this, SLOT(onDownloadItemAdded(DownloadItem *)));
}

WebFrame::~WebFrame()
{
}

void WebFrame::onScreenSizeChanged(int)
{
    onyx::screen::instance().enableUpdate(false);
    resize(qApp->desktop()->screenGeometry().size());
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC);
}

void WebFrame::keyPressEvent(QKeyEvent * ke)
{
}

void WebFrame::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_S:
        {
            QVariantList credentials = security_manager_.generateCredentials();
            qDebug("Generated Auth Key:%s", qPrintable(credentials[0].toString()));
            qDebug("Generated OTP:%s", qPrintable(credentials[1].toString()));

            QString client_id = security_manager_.getClientId();
            qDebug("Client ID:%s", qPrintable(client_id));

            QString auth_key = security_manager_.getAuthKey();
            qDebug("Auth Key:%s", qPrintable(auth_key));
        }
        break;
    default:
        QWidget::keyReleaseEvent(ke);
        break;
    }
}

bool WebFrame::event(QEvent *e)
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
        if (!web_view_update_rect_.isEmpty())
        {
            qDebug("Update Web View (%d, %d, %d, %d), Waveform:%d",
                   web_view_update_rect_.left(),
                   web_view_update_rect_.top(),
                   web_view_update_rect_.width(),
                   web_view_update_rect_.height(),
                   web_view_waveform_);
            onyx::screen::instance().updateWidgetRegion(&web_view_,
                                                        web_view_update_rect_,
                                                        web_view_waveform_,
                                                        false);
            web_view_update_rect_ = QRect();
        }
        else
        {
            // Check region.
            QRect rc = web_view_.updateRect();
            rc = rc.intersected(web_view_.rect());

            web_view_.resetUpdateRect();
            int area = rc.width() * rc.height();
            if (area > 0 && onyx::screen::instance().isUpdateEnabled())
            {
                qDebug("update rectange:(%d, %d, %d, %d)", rc.left(), rc.top(),
                       rc.width(), rc.height());
                int max = web_view_.rect().width() * web_view_.rect().height();
                /*if (area < (max >> 3))
                {
                    qDebug("Ignored the request");
                    return true;
                }
                else*/
                if (rc.width() < (web_view_.width() - (web_view_.width() >> 2))
                    || rc.height() <
                    (web_view_.height() - (web_view_.height() >> 2)))
                {
                    qDebug("using gu waveform");
                    onyx::screen::instance().updateWidget(
                            this, onyx::screen::ScreenProxy::GU);
                    return true;
                }

                if (area > (max - (max >> 2)))
                {
                    if (!web_view_.isLoadingFinished())
                    {
                        qDebug("using gc waveform");
                        onyx::screen::instance().flush(
                                this, onyx::screen::ScreenProxy::GC);
                        return true;
                    }
                }
            }

            qDebug("using default waveform");
            onyx::screen::instance().updateWidget(this);
            return true;
        }
    }
    return ret;
}

void WebFrame::closeEvent(QCloseEvent *e)
{
}

void WebFrame::createLayout()
{
    layout_.setContentsMargins(0, 0, 0, 0);
    layout_.setSpacing(1);

    layout_.addWidget(&web_view_);

    layout_.addWidget(&download_view_);
    download_view_.setVisible(false);

    layout_.addWidget(&status_bar_);
}

void WebFrame::populateJavaScriptWindowObject()
{
    qDebug("Populate Java Script Window Object");
    QWebFrame* frame = web_view_.page()->mainFrame();
    frame->addToJavaScriptWindowObject("onyxScreenUpdater", this);
    frame->addToJavaScriptWindowObject("onyxDB", &onyx_db_);
    frame->addToJavaScriptWindowObject("onyxSecurityManager",
                                       &security_manager_);
}

void WebFrame::updateRegion(int x, int y, int width, int height,
                            const QString & update_type)
{
    qDebug("Update Region(%d, %d, %d, %d)", x, y, width, height);
    QRect rect(x, y, width, height);
    QPoint viewport_offset  =
            web_view_.page()->currentFrame()->scrollPosition();
    QPoint pos_in_viewport = QPoint(x, y) - viewport_offset;
    rect.moveTo(pos_in_viewport);
    web_view_update_rect_ = web_view_update_rect_.united(rect);
    web_view_waveform_ = getWaveformByString(update_type);
}

void WebFrame::onLoadFinished(const QString &file_path)
{
    download_manager_.onDownloadFinished(true, file_path);
}

void WebFrame::onDownloadItemAdded(DownloadItem *item)
{
    if (!download_view_.isVisible())
    {
        download_view_.setVisible(true);
    }
    download_view_.appendWidget(item);
}

}
