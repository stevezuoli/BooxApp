#include "network_service/auto_complete.h"
#include "page.h"
#include "web_application.h"
#include "private_conf/conf.h"

namespace webbrowser
{

WebPage::WebPage(QObject *parent)
    : QWebPage(parent)
    , network_error_(QNetworkReply::NoError)
{
    setNetworkAccessManager(WebApplication::accessManager());
    connect(this, SIGNAL(unsupportedContent(QNetworkReply *)),
            this, SLOT(handleUnsupportedContent(QNetworkReply *)));
}

bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    resetNetworkError();
    if (frame == mainFrame())
    {
        loading_url_ = request.url();
        emit loadingUrl(loading_url_);
    }

    if (type == NavigationTypeFormSubmitted || type == NavigationTypeFormResubmitted)
    {
        QWebFrame * current_frame = currentFrame();
        QWebFrame * dst_frame = (frame != 0) ? frame : current_frame;
        if (dst_frame != 0)
        {
            QSettings settings;
            settings.beginGroup(QLatin1String("websettings"));
            if ( settings.value(QLatin1String("savePasswords"), true).toBool())
            {
                QUrl url = request.url();
                if (!request.rawHeader("Referer").isEmpty())
                {
                    url = QUrl(request.rawHeader("Referer"));
                }
                AutoComplete::instance()->setFormHtml(url, dst_frame->toHtml());
            }
            settings.endGroup();
        }

        if (current_frame != 0)
        {
            QSettings settings;
            settings.beginGroup(QLatin1String("websettings"));
            if ( settings.value(QLatin1String("savePasswords"), true).toBool())
            {
                QUrl url = current_frame->url();
                AutoComplete::instance()->setFormHtml(url, current_frame->toHtml());
            }
            settings.endGroup();
        }
    }
    return QWebPage::acceptNavigationRequest(frame, request, type);
}

void WebPage::displayHtml(const QString & html)
{
    QList<QWebFrame*> frames;
    frames.append(mainFrame());
    while (!frames.isEmpty())
    {
        QWebFrame *frame = frames.takeFirst();
        QList<QWebFrame *> children = frame->childFrames();
        foreach(QWebFrame *frame, children)
        {
            frames.append(frame);
        }
    }
    mainFrame()->setHtml(html);
}

void WebPage::displayConnectingHtml(const QUrl &url)
{
    QString host = url.host();

    // Read file from $HOME/web_browser
    QString path;
#ifdef WIN32
    path = QDir::home().absoluteFilePath("web_browser/connecting.html");
#else
    QDir dir(SHARE_ROOT);
    dir.cd("web_browser");
    path = dir.absoluteFilePath("connecting.html");
#endif

    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        QString title = QApplication::tr("Connecting");
        QString html = QString(QLatin1String(file.readAll())).arg(host).arg(title);
        displayHtml(html);
    }
}

void WebPage::displayFulfillmentHtml(const QString & content, OTAStatus ota_status)
{
    QString info;
    QString status;
    switch (ota_status)
    {
    case OTA_PROCESSING:
        {
            QUrl url(content);
            info = tr("Download content from:%1");
            info = info.arg(url.host());
            status = tr("Please wait...");
        }
        break;
    case OTA_DONE:
        {
            info = tr("Downloaded content has been saved at %1");
            info = info.arg(content);
            status = tr("Succeed!");
        }
        break;
    case OTA_ERROR:
        {
            info = tr("Error:%1");
            info = info.arg(content);
            status = tr("Please check your configurations");
        }
    case OTA_ABORTED:
        {
            info = tr("Fulfillment from %1 aborted");
            info = info.arg(content);
            status = tr("Please return to previous website");
        }
        break;
    default:
        {
            info = tr("Unknown OTA status");
            status = tr("Please return to previous website");
        }
        break;
    }

    // Read file from $HOME/web_browser
    QString path;
#ifdef WIN32
    path = QDir::home().absoluteFilePath("web_browser/ota_status.html");
#else
    QDir dir(SHARE_ROOT);
    dir.cd("web_browser");
    path = dir.absoluteFilePath("ota_status.html");
#endif

    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        QString title = QApplication::tr("Fulfillment");
        QString html = QString(QLatin1String(file.readAll()))
                               .arg(info)
                               .arg(status)
                               .arg(title);
        displayHtml(html);
    }
}

void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    qDebug("Network Reply Error:%d", reply->error());
    network_error_ = reply->error();
    if (reply->error() == QNetworkReply::NoError)
    {
        // Do NOT launch the DRM service by url
        //QUrl url = reply->url();
        //QString path = url.path();
        //if (path.endsWith(".acsm", Qt::CaseInsensitive))
        //{
        //    emit requestOTA(url);
        //    return;
        //}
        WebApplication::downloadManager()->handleUnsupportedContent(reply);
        return;
    }

    // Read file from $HOME/web_browser
    QString path;
#ifdef WIN32
    path = QDir::home().absoluteFilePath("web_browser/load_fail.html");
#else
    QDir dir(SHARE_ROOT);
    dir.cd("web_browser");
    path = dir.absoluteFilePath("load_fail.html");
#endif

    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        QString title = tr("%1").arg(reply->url().toString());
        QString html = QString(QLatin1String(file.readAll()))
                               .arg(title)
                               .arg(reply->errorString())
                               .arg(reply->url().toString());

        QList<QWebFrame*> frames;
        frames.append(mainFrame());
        while (!frames.isEmpty())
        {
            QWebFrame *frame = frames.takeFirst();
            if (frame->url() == reply->url())
            {
                frame->setHtml(html, reply->url());
                return;
            }
            QList<QWebFrame *> children = frame->childFrames();
            foreach(QWebFrame *frame, children)
            {
                frames.append(frame);
            }
        }
        if (loading_url_ == reply->url())
        {
            mainFrame()->setHtml(html, reply->url());
        }
    }
}

QString WebPage::userAgentForUrl ( const QUrl & url ) const
{
    static QString agent;
    if (agent.isEmpty())
    {
        sys::SystemConfig conf;
        sys::PrivateConfig pc;
        agent = QString("Mozilla/5.0+(QtEmbedded;+U;+Linux;+ru-RU;+ID=%1;+V=%2;+SID=%3)+AppleWebKit/527+(KHTML,+like+Gecko,+Safari/419.3)+GALAXY+web_browser");
        agent = agent.arg(conf.serialNumber()).arg(conf.version()).arg(pc.imei());
    }
    return agent;

}

}
