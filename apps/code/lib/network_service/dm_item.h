#ifndef DOWNLOAD_MANAGER_ITEM
#define DOWNLOAD_MANAGER_ITEM

#include "ns_utils.h"

using namespace ui;
namespace network_service
{

class DownloadItem : public QWidget
{
    Q_OBJECT
public:
    DownloadItem(QNetworkReply *reply = 0, bool request_file_name = false, QWidget *parent = 0, QString file_name = QString());
    virtual ~DownloadItem();
    bool downloading() const;
    bool downloadedSuccessfully() const;
    bool downloadCancelled() const;

    inline QString downloadInfo() { return download_info_.text(); }
    inline QString fileName() { return output_.fileName(); }
    inline const QUrl & url() { return url_; }

protected:
    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);

Q_SIGNALS:
    void statusChanged();
    void toBeDeleted();
    void loadFinished();
    void loadError();

private Q_SLOTS:
    void stop();
    void tryAgain();
    void open();

    void downloadReadyRead();
    void error(QNetworkReply::NetworkError code);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void metaDataChanged();
    void finished();

private:
    void createLayout();
    void getFileName();
    void init();
    void updateInfoLabel();
    QString dataString(int size) const;
    QString saveFileName(const QString &directory);

private:
    QUrl           url_;
    QFile          output_;
    QNetworkReply *reply_;
    bool           request_file_name_;
    qint64         bytes_received_;
    qint64         bytes_total_;
    QTime          download_time_;
    QString        input_file_name_;

    QHBoxLayout        hbox_;
    ui::OnyxLabel      download_info_;
    ui::OnyxLabel      file_name_;
    ui::OnyxPushButton stop_btn_;
    ui::OnyxPushButton try_again_btn_;
    //NabooPushButton open_btn_;

    friend class DownloadManager;
};

};

#endif
