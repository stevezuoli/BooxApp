#include "download_view.h"
#include "dm_item.h"

using namespace ui;

namespace network_service
{

static const QString CLEAR_BUTTON_STYLE =   "\
QPushButton                             \
{                                       \
    background: white;                  \
    border-width: 1px;                  \
    border-color: black;                \
    border-style: solid;                \
    border-radius: 3;                   \
    color: black;                       \
    padding: 0px;                       \
    min-width: 32px;                    \
    min-height: 30px;                   \
}                                       \
QPushButton:pressed                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    color: white;                       \
    border-color: white;                \
    background-color: black;            \
}                                       \
QPushButton:checked                     \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    color: white;                       \
    border-color: white;                \
    background-color: black;            \
}                                       \
QPushButton:focus                       \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    color: black;                       \
    border-width: 2px;                  \
    border-color: black;                \
    background-color: white;            \
}                                       \
QPushButton:disabled                    \
{                                       \
    padding-left: 0px;                  \
    padding-top: 0px;                   \
    border-color: dark;                 \
    color: dark;                        \
    background-color: white;            \
}";

DownloadView::DownloadView(QWidget *parent, DownloadManager *download_manager)
    : QWidget(parent)
    , hbox_(this)
    , clear_all_(tr(""), 0)
    , widgets_count_(0)
    , latest_file_name_()
    , download_manager_(download_manager)
{
    createLayout();
}

DownloadView::~DownloadView()
{
}

void DownloadView::appendWidget(QWidget *w)
{
    hbox_.insertWidget(0, w);

    connect(w, SIGNAL(loadFinished()), this, SLOT(onLoadFinished()));
    connect(w, SIGNAL(toBeDeleted()), this, SLOT(removeItem()));
    widgets_count_++;
    w->setVisible(true);
}

void DownloadView::removeWidget(QWidget *w)
{
    hbox_.removeWidget(w);
    w->setVisible(false);
    disconnect(w, SIGNAL(loadFinished()), this, SLOT(onLoadFinished()));
    disconnect(w, SIGNAL(toBeDeleted()), this, SLOT(removeItem()));
    widgets_count_--;
    if (widgets_count_ < 0)
    {
        widgets_count_ = 0;
    }
}

void DownloadView::removeItem()
{
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    removeWidget(item);
    if (widgets_count_ == 0)
    {
        setVisible(false);
    }
}

void DownloadView::onClearAllClicked()
{
    if (download_manager_ != 0)
    {
        download_manager_->cleanup();
    }
    noticeLoadFinish();
    latest_file_name_.clear();
}

void DownloadView::createLayout()
{
    setFixedHeight(30);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Dark);

    hbox_.setContentsMargins(2, 2, 2, 0);
    hbox_.setSpacing(2);

    hbox_.addWidget(&clear_all_, 500, Qt::AlignRight);
    QPixmap clear_all_map(":/res/clear_all.png");
    clear_all_.setStyleSheet(CLEAR_BUTTON_STYLE);
    clear_all_.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    clear_all_.setIcon(QIcon(clear_all_map));
    clear_all_.setIconSize(clear_all_map.size());
    clear_all_.setMaximumWidth(clear_all_map.width());

    connect(&clear_all_, SIGNAL(clicked()), this, SLOT(onClearAllClicked()));
}

void DownloadView::noticeLoadFinish()
{
    if (latest_file_name_.isEmpty())
    {
        return;
    }

    MessageDialog dialog(QMessageBox::Information,
                         tr("Download Finished"),
                         tr("Do you want to open downloaded file?"),
                         QMessageBox::Yes|QMessageBox::No);
    if (dialog.exec() != QMessageBox::Yes)
    {
        onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
        return;
    }
    emit loadFinished(latest_file_name_);
}

void DownloadView::onLoadFinished()
{
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    QString file_name = item->fileName();
    if (file_name.endsWith(".pdf") ||
        file_name.endsWith(".epub") ||
        file_name.endsWith(".txt") ||
        file_name.endsWith(".pdb") ||
        file_name.endsWith(".rtf") ||
        file_name.endsWith(".mobi"))
    {
        latest_file_name_ = file_name;
    }

    if (download_manager_ != 0 && download_manager_->stillDownloading())
    {
        return;
    }
    noticeLoadFinish();
}

}
