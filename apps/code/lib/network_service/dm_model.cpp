#include "dm_model.h"
#include "dm_manager.h"

namespace network_service
{

DownloadModel::DownloadModel(DownloadManager *downloadManager, QObject *parent)
    : QAbstractListModel(parent)
    , download_manager_(downloadManager)
{
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.column() >= columnCount(index.parent()))
    {
        return QVariant();
    }

    if (role == Qt::ToolTipRole)
    {
        if (!download_manager_->downloads_.at(index.column())->downloadedSuccessfully())
        {
            return download_manager_->downloads_.at(index.column())->downloadInfo();
        }
    }
    return QVariant();
}

int DownloadModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : download_manager_->downloads_.count();
}

int DownloadModel::rowCount(const QModelIndex &parent) const
{
    return 1;
}

bool DownloadModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (parent.isValid())
    {
        return false;
    }

    int lastColumn = column + count - 1;
    for (int i = lastColumn; i >= column; --i)
    {
        DownloadItem *item = download_manager_->downloads_.at(i);
        if (item->downloadedSuccessfully() || item->downloadCancelled())
        {
            beginRemoveColumns(parent, i, i);
            download_manager_->downloads_.takeAt(i)->deleteLater();
            endRemoveColumns();
        }
    }
    download_manager_->auto_saver_->changeOccurred();
    return true;
}

}
