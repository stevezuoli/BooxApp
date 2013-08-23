#ifndef DOWNLOAD_MANAGER_PROXY
#define DOWNLOAD_MANAGER_PROXY

#include "dm_model.h"
#include "dm_item.h"
#include "auto_saver.h"

namespace network_service
{

class DownloadManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(RemovePolicy removePolicy READ removePolicy WRITE setRemovePolicy)
    Q_ENUMS(RemovePolicy)

public:
    enum RemovePolicy
    {
        Never,
        Exit,
        SuccessFullDownload
    };

public:
    DownloadManager(QWidget *parent = 0);
    ~DownloadManager();
    int activeDownloads() const;

    DownloadModel* model() { return model_; }
    RemovePolicy removePolicy() const;
    bool stillDownloading() const;
    void setRemovePolicy(RemovePolicy policy);

Q_SIGNALS:
    void itemAdded(DownloadItem *item);

public Q_SLOTS:
    void download(const QNetworkRequest &request, bool requestFileName = false);
    inline void download(const QUrl &url, bool requestFileName = false);
    void handleUnsupportedContent(QNetworkReply *reply, bool requestFileName = false);
    void cleanup();

private Q_SLOTS:
    void save() const;
    void updateColumn();

private:
    void addItem(DownloadItem *item);
    void updateItemCount();
    void load();

private:
    AutoSaver*             auto_saver_;
    DownloadModel*         model_;
    QNetworkAccessManager* manager_;
    QList<DownloadItem*>   downloads_;
    RemovePolicy           remove_policy_;

    friend class DownloadModel;
};

inline void DownloadManager::download(const QUrl &url, bool requestFileName)
{
    download(QNetworkRequest(url), requestFileName);
}

DownloadManager * getDownloadManagerInstance();

};

#endif
