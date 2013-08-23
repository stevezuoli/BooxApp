#ifndef DOWNLOAD_MANAGER_MODEL
#define DOWNLOAD_MANAGER_MODEL

#include "ns_utils.h"

namespace network_service
{

class DownloadManager;
class DownloadModel : public QAbstractListModel
{
    Q_OBJECT
public:
    DownloadModel(DownloadManager *downloadManager, QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

private:
    DownloadManager *download_manager_;
    friend class DownloadManager;
};

};

#endif
