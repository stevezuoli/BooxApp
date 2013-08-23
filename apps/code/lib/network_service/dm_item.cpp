#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"
#include "dm_item.h"
#include "access_manager.h"

namespace network_service
{

static const QString PUSH_BUTTON_STYLE =   "\
QPushButton                             \
{                                       \
    background: transparent;            \
    font-size: 12px;                    \
    border-width: 1px;                  \
    border-color: black;                \
    border-style: solid;                \
    border-radius: 3;                   \
    color: black;                       \
    padding: 0px;                       \
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

const QString LABEL_STYLE = "           \
QLabel                                  \
{                                       \
     padding: 0px;                      \
     background: transparent;           \
     font: 12px ;                        \
     color: black;                      \
 }";

/*!
    DownloadItem is a widget that is displayed in the download manager list.
    It moves the data from the QNetworkReply into the QFile as well
    as update the information/progressbar and report errors.
 */
DownloadItem::DownloadItem(QNetworkReply *reply, bool request_file_name, QWidget *parent, QString file_name)
    : QWidget(parent, Qt::FramelessWindowHint)
    , reply_(reply)
    , request_file_name_(request_file_name)
    , bytes_received_(0)
    , bytes_total_(0)
    , input_file_name_(file_name)
    , hbox_(this)
    , download_info_(0)
    , file_name_(0)
    , stop_btn_("", 0)
    , try_again_btn_("", 0)
{
    createLayout();
    init();
}

DownloadItem::~DownloadItem()
{
    emit toBeDeleted();
}

void DownloadItem::init()
{
    if (reply_ == 0)
    {
        return;
    }

    // attach to the reply_
    url_ = reply_->url();
    reply_->setParent(this);
    connect(reply_, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()), Qt::QueuedConnection);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));
    connect(reply_, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(downloadProgress(qint64, qint64)), Qt::QueuedConnection);
    connect(reply_, SIGNAL(metaDataChanged()),
            this, SLOT(metaDataChanged()), Qt::QueuedConnection);
    connect(reply_, SIGNAL(finished()),
            this, SLOT(finished()), Qt::QueuedConnection);

    // reset info
    download_info_.clear();
    getFileName();

    // start timer for the download estimation
    download_time_.start();

    if (reply_->error() != QNetworkReply::NoError)
    {
        error(reply_->error());
        finished();
    }
    bytes_total_ = 0;
}

void DownloadItem::createLayout()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    hbox_.setSpacing(1);
    hbox_.setContentsMargins(1, 2, 1, 0);

    stop_btn_.setStyleSheet(PUSH_BUTTON_STYLE);
    QPixmap stop_btn_pixmap(":/res/stop_downloading.png");
    stop_btn_.setIcon(QIcon(stop_btn_pixmap));
    stop_btn_.setIconSize(stop_btn_pixmap.size());

    try_again_btn_.setStyleSheet(PUSH_BUTTON_STYLE);
    QPixmap try_again_btn_pixmap(":/res/start_downloading.png");
    try_again_btn_.setIcon(QIcon(try_again_btn_pixmap));
    try_again_btn_.setIconSize(try_again_btn_pixmap.size());

    download_info_.setStyleSheet(LABEL_STYLE);
    file_name_.setStyleSheet(LABEL_STYLE);

    // file name label
    hbox_.addWidget(&file_name_);
    hbox_.addWidget(&download_info_);
    hbox_.addWidget(&stop_btn_);
    hbox_.addWidget(&try_again_btn_);

    try_again_btn_.setVisible(false);

    connect(&stop_btn_, SIGNAL(clicked()), this, SLOT(stop()));
    //connect(open_btn_, SIGNAL(clicked()), this, SLOT(open()));
    connect(&try_again_btn_, SIGNAL(clicked()), this, SLOT(tryAgain()));
}

void DownloadItem::getFileName()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));
    //QString default_location = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
    QString default_location = getDownloadFileLocation();
    QString download_directory = settings.value(QLatin1String("download_directory"), default_location).toString();
    if (!download_directory.isEmpty())
    {
        download_directory += QLatin1Char('/');
    }

    QString default_file_name = saveFileName(download_directory);
    QString file_name = default_file_name;
    if (request_file_name_)
    {
        file_name = QFileDialog::getSaveFileName(0, tr("Save File"), default_file_name);
        if (file_name.isEmpty())
        {
            reply_->close();
            return;
        }
    }
    output_.setFileName(file_name);
    if (request_file_name_)
    {
        downloadReadyRead();
    }
}

QString DownloadItem::saveFileName(const QString &directory)
{
    // Move this function into QNetworkReply to also get file name sent from the server
    QString path = input_file_name_;
    if (path.isEmpty())
    {
        path = url_.path();
    }
    QFileInfo info(path);
    QString base_name = info.completeBaseName();
    QString end_name = info.suffix();

    if (base_name.isEmpty())
    {
        base_name = QLatin1String("unnamed_download");
        qDebug() << "DownloadManager:: downloading unknown file:" << url_;
    }

    QString name = directory + base_name + QLatin1Char('.') + end_name;
    if (QFile::exists(name))
    {
        // already exists, don't overwrite
        int i = 1;
        do
        {
            name = directory + base_name + QLatin1Char('-') + QString::number(i++) + QLatin1Char('.') + end_name;
        } while (QFile::exists(name));
    }
    file_name_.setText(base_name);
    return name;
}

void DownloadItem::paintEvent(QPaintEvent *pe)
{
    QPainter p(this);
    QRect rc = rect();
    QPainterPath path;
    path.addRoundedRect(rc, 1, 1);
    QPen pen(Qt::SolidLine);
    pen.setColor(Qt::lightGray);
    pen.setWidth(1);
    p.setPen(pen);
    p.drawPath(path);
}

void DownloadItem::resizeEvent(QResizeEvent *)
{
    QPainterPath p;
    p.addRoundedRect(rect(), 1, 1);
    QRegion mask_region(p.toFillPolygon().toPolygon());
    setMask(mask_region);
}

void DownloadItem::stop()
{
    setUpdatesEnabled(false);
    stop_btn_.setEnabled(false);
    stop_btn_.hide();
    try_again_btn_.setEnabled(true);
    try_again_btn_.show();
    setUpdatesEnabled(true);

    reply_->abort();
}

void DownloadItem::open()
{
    QFileInfo info(output_);
    QUrl url = QUrl::fromLocalFile(info.absolutePath());
    QDesktopServices::openUrl(url);
}

void DownloadItem::tryAgain()
{
    if (!try_again_btn_.isEnabled())
    {
        return;
    }

    try_again_btn_.setEnabled(false);
    try_again_btn_.setVisible(false);
    stop_btn_.setEnabled(true);
    stop_btn_.setVisible(true);

    QNetworkReply *r = getAccessManagerInstance()->get(QNetworkRequest(url_));
    if (reply_ != 0)
    {
        reply_->deleteLater();
    }

    if (output_.exists())
    {
        output_.remove();
    }
    reply_ = r;
    init();
    emit statusChanged();
}

void DownloadItem::downloadReadyRead()
{
    if (request_file_name_ && output_.fileName().isEmpty())
    {
        return;
    }

    if (!output_.isOpen())
    {
        // in case someone else has already put a file there
        if (!request_file_name_)
        {
            getFileName();
        }

        if (!output_.open(QIODevice::WriteOnly))
        {
            download_info_.setText(tr("Error opening save file: %1").arg(output_.errorString()));
            stop();
            emit statusChanged();
            return;
        }
        emit statusChanged();
    }

    if (-1 == output_.write(reply_->readAll()))
    {
        download_info_.setText(tr("Error saving: %1").arg(output_.errorString()));
        stop();
    }
}

void DownloadItem::error(QNetworkReply::NetworkError)
{
    qDebug() << "DownloadItem::error" << reply_->errorString() << url_;
    download_info_.setText(tr("%1").arg(reply_->errorString()));
    try_again_btn_.setEnabled(true);
    try_again_btn_.setVisible(true);
    emit loadError();
}

void DownloadItem::metaDataChanged()
{
    QVariant location_header = reply_->header(QNetworkRequest::LocationHeader);
    if (location_header.isValid())
    {
        url_ = location_header.toUrl();
        reply_->deleteLater();
        reply_ = getAccessManagerInstance()->get(QNetworkRequest(url_));
        init();
    }
}

void DownloadItem::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal == -1)
    {
        bytes_total_ = 0;
    }
    else
    {
        bytes_total_ = bytesTotal;
    }

    /*if (bytes_received_ == 0)
    {
        sys::SysStatus::instance().reportDownloadState(output_.fileName(), 0);
    }
    if (bytesReceived >= bytesTotal)
    {
        sys::SysStatus::instance().reportDownloadState(output_.fileName(), 100);
    }*/
    bytes_received_ = bytesReceived;
    updateInfoLabel();
}

void DownloadItem::updateInfoLabel()
{
    if (reply_->error() != QNetworkReply::NoError)
    {
        return;
    }

    bool running = !downloadedSuccessfully();

    /*
    // update info label
    double speed = bytes_received_ * 1000.0 / download_time_.elapsed();
    double time_remaining = ((double)(bytes_total_ - bytes_received_)) / speed;
    QString time_remaining_string = tr("seconds");
    if (time_remaining > 60)
    {
        time_remaining = time_remaining / 60;
        time_remaining_string = tr("minutes");
    }
    time_remaining = floor(time_remaining);

    // When downloading the eta should never be 0
    if (time_remaining == 0)
    {
        time_remaining = 1;
    }

    QString info;
    if (running)
    {
        QString remaining;
        if (bytes_total_ != 0)
        {
            remaining = tr("- %4 %5 remaining")
            .arg(time_remaining)
            .arg(time_remaining_string);
        }
        info = QString(tr("%1 of %2 (%3/sec) %4"))
            .arg(dataString(bytes_received_))
            .arg(bytes_total_ == 0 ? tr("?") : dataString(bytes_total_))
            .arg(dataString((int)speed))
            .arg(remaining);
    }
    else
    {
        if (bytes_received_ == bytes_total_)
        {
            info = dataString(output_.size());
        }
        else
        {
            info = tr("%1 of %2 - Stopped")
                .arg(dataString(bytes_received_))
                .arg(dataString(bytes_total_));
        }
    }
    */

    double progress = static_cast<double>(bytes_received_)/static_cast<double>(bytes_total_);
    int download_ratio = progress * 100.0;
    QString info;
    if (running)
    {
        QString remaining;
        info = QString(tr(" %1% ")).arg(download_ratio);
    }
    else
    {
        if (bytes_received_ == bytes_total_)
        {
            info = QString(tr(" 100% "));
        }
        else
        {
            info = tr(" %1% - Stopped ").arg(download_ratio);
        }
    }

    if (download_info_.text() != info)
    {
        onyx::screen::instance().enableUpdate(false);
        download_info_.setText(info);
        QWidget * update_widget = parentWidget() != 0 ? parentWidget() : this;
        onyx::screen::instance().enableUpdate(true);
        onyx::screen::instance().flush(update_widget, onyx::screen::ScreenProxy::GU, false);
    }
}

QString DownloadItem::dataString(int size) const
{
    QString unit;
    if (size < 1024)
    {
        unit = tr("bytes");
    }
    else if (size < 1024*1024)
    {
        size /= 1024;
        unit = tr("kB");
    }
    else
    {
        size /= 1024*1024;
        unit = tr("MB");
    }
    return QString(QLatin1String("%1 %2")).arg(size).arg(unit);
}

bool DownloadItem::downloading() const
{
    return !stop_btn_.isHidden();
}

bool DownloadItem::downloadedSuccessfully() const
{
    return (stop_btn_.isHidden() && try_again_btn_.isHidden());
}

bool DownloadItem::downloadCancelled() const
{
    return !try_again_btn_.isHidden();
}

void DownloadItem::finished()
{
    stop_btn_.setEnabled(false);
    stop_btn_.hide();

    output_.close();
    emit statusChanged();

    if (try_again_btn_.isEnabled() && !try_again_btn_.isHidden())
    {
        // stopped by user
        return;
    }
    emit loadFinished();
}

}
