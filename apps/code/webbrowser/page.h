#ifndef WEB_BROWSER_PAGE_H_
#define WEB_BROWSER_PAGE_H_

#include <QtGui/QtGui>
#include <QtWebKit/QtWebKit>
#include <QtWebKit/QWebView>
#include "onyx/ui/ui.h"

namespace webbrowser
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
    WebPage(QObject *parent = 0);

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
    QString userAgentForUrl ( const QUrl & url ) const;

private Q_SLOTS:
    void handleUnsupportedContent(QNetworkReply *reply);

private:
    void displayHtml(const QString & html);

private:
    QUrl                        loading_url_;
    QNetworkReply::NetworkError network_error_;
};

};
#endif
