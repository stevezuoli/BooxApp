#ifndef WEB_PAGE_H_
#define WEB_PAGE_H_

#include <QtGui/QtGui>
#include <QtWebKit/QtWebKit>
#include <QtWebKit/QWebView>
#include "onyx/ui/ui.h"
#include "access_manager.h"
#include "dm_manager.h"

namespace network_service
{

enum OTAStatus
{
    OTA_PROCESSING,
    OTA_DONE,
    OTA_ERROR,
    OTA_ABORTED
};

class WebPage : public QWebPage
{
    Q_OBJECT
public:
    WebPage(QObject *parent = 0,
            NetworkAccessManager * access_manager = 0,
            DownloadManager * download_manager = 0);

    inline QUrl & loadingUrl() { return loading_url_; }
    inline QNetworkReply::NetworkError networkError() { return network_error_; }
    inline void resetNetworkError() { network_error_ = QNetworkReply::NoError; }

    void displayFulfillmentHtml(const QString & content, OTAStatus ota_status);
    void displayConnectingHtml(const QUrl &url);

Q_SIGNALS:
    void loadingUrl(const QUrl &url);
    void requestOTA(const QUrl &url);

protected:
    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);

private Q_SLOTS:
    void handleUnsupportedContent(QNetworkReply *reply);

private:
    void displayHtml(const QString & html);

private:
    DownloadManager             *download_manager_;
    QUrl                        loading_url_;
    QNetworkReply::NetworkError network_error_;
};

};
#endif
