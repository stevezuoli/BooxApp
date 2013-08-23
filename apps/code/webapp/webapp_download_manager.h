#ifndef WEB_DOWNLOAD_MANAGER_H_
#define WEB_DOWNLOAD_MANAGER_H_

#include <QtSql/QtSql>
#include <QtWebKit/QtWebKit>
#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "onyx/ui/ui.h"

using namespace ui;

namespace webapp
{

class OnyxDownloadManager : public QObject
{
    Q_OBJECT
public:
    OnyxDownloadManager(QObject *parent = 0, QWebView *view = 0);
    ~OnyxDownloadManager();

Q_SIGNALS:
    void requestDownload(const QUrl &url, bool requestFileName);

public Q_SLOTS:
    void download(const QString & url_str, const QString & callback_fn);
    void onDownloadFinished(bool succeed, const QString & file_path);

private:
    QString  download_finished_func_;
    QWebView *view_;
};

};   // namespace webapp

#endif
