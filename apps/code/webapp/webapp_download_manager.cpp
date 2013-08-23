#include "network_service/ns_utils.h"
#include "webapp_download_manager.h"

using namespace network_service;

namespace webapp
{

OnyxDownloadManager::OnyxDownloadManager(QObject *parent, QWebView *view)
: view_(view)
{
}

OnyxDownloadManager::~OnyxDownloadManager()
{
}

void OnyxDownloadManager::download(const QString & url_str, const QString & callback_fn)
{
    download_finished_func_ = callback_fn;

    QUrl url = guessUrlFromString(url_str);
    emit requestDownload(url, false);
}

void OnyxDownloadManager::onDownloadFinished(bool succeed, const QString & file_path)
{
    if (view_ != 0)
    {
        QString code = download_finished_func_ + "(" + (succeed ? "true" : "false") + ",'" + file_path + "');";
        view_->page()->mainFrame()->evaluateJavaScript(code);
    }
}

}