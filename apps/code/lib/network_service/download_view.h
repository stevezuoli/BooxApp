#ifndef DOWNLOAD_VIEW_H
#define DOWNLOAD_VIEW_H

#include "onyx/ui/ui.h"
#include "dm_manager.h"

using namespace ui;

namespace network_service
{

class DownloadView : public QWidget
{
    Q_OBJECT
public:
    DownloadView(QWidget *parent = 0, DownloadManager *download_manager = 0);
    virtual ~DownloadView();

    void appendWidget(QWidget *w);
    void removeWidget(QWidget *w);

Q_SIGNALS:
    void loadFinished(const QString &file_name);

private Q_SLOTS:
    void onClearAllClicked();
    void removeItem();
    void onLoadFinished();

private:
    void createLayout();
    void noticeLoadFinish();

private:
    QHBoxLayout     hbox_;
    OnyxPushButton  clear_all_;
    int             widgets_count_;
    QString         latest_file_name_;

    DownloadManager *download_manager_;
};

};
#endif // DOWNLOAD_VIEW_H
