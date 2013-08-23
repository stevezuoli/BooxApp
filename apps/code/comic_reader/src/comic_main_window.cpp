
#include <QtGui>
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys.h"
#include "comic_main_window.h"
#include "comic_utils.h"
#include "comic_message_dialog.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QImage>
#include <QLabel>

using namespace compression;

namespace comic_reader
{

#ifndef BUILD_FOR_ARM
    static const QSize my_client_size(600, 800);
#endif

ComicMainWindow::ComicMainWindow(QObject *parent)
#ifndef Q_WS_QWS
    : QMainWindow(0, 0)
#else
    : QMainWindow(0, Qt::FramelessWindowHint)
#endif
    , optCWD("")
    , view_(this)
    , status_bar_(this,
            (SysStatus::instance().hasTouchScreen() ?
            (MENU | PROGRESS | MESSAGE | STYLUS | BATTERY | CLOCK | VOLUME | SCREEN_REFRESH) :
            (MENU | PROGRESS | MESSAGE | STYLUS | BATTERY )))
{

#ifndef Q_WS_QWS
    resize(600, 800);
#else
    resize(qApp->desktop()->screenGeometry().size());
#endif

#ifndef BUILD_FOR_ARM
    setMaximumSize( QSize(700, 900));
#endif

    // Default settings
    rotate = 0;
    isFullScreen = false;
    isScaled = false;

    // Default to blank image
    curImage = new QImage;
    rawImage = new QByteArray;

    image_label_ = new QLabel;
    image_label_->setFrameStyle(QFrame::NoFrame);
    view_.setWidget(image_label_);

    setCentralWidget(&view_);
    setStatusBar(&status_bar_);

    // Supported image formats
    QList<QByteArray> ba = QImageReader::supportedImageFormats();
    while (ba.size())
    {
        imgFormats.append(QString(ba.takeFirst()));
    }

    // Create the image stream
    iStream = new ArchiveFileStream();
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    rx.setPattern(QString("\\.(") + imgFormats.join("|") + ")$");
    iStream->setFilter(rx);

    connect(&status_bar_, SIGNAL(menuClicked()), &view_, SLOT(showContextMenu()));
    connect(&view_, SIGNAL(requestGotoPageDialog()), &status_bar_, SLOT(onMessageAreaClicked()));
    connect(&view_, SIGNAL(requestClockDialog()), &status_bar_, SLOT(onClockClicked()));
    connect(&status_bar_,  SIGNAL(progressClicked(const int, const int)),
            this, SLOT(onPagebarClicked(const int, const int)));
    connect(&view_, SIGNAL(fullScreen(bool)), this, SLOT(onFullScreen(bool)));
    connect(&view_, SIGNAL(comicBookClosed()), this, SLOT(quitApp()));
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
}

ComicMainWindow::~ComicMainWindow()
{
}

void ComicMainWindow::attachModel(ComicModel *model)
{
    model_ = model;
}

ComicView * ComicMainWindow::view()
{
    return &view_;
}

void ComicMainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void ComicMainWindow::quitApp()
{
    qApp->quit();
}

QSize ComicMainWindow::clientSize()
{
#ifndef BUILD_FOR_ARM
    return my_client_size;
#endif
    QSize screen_size = qApp->desktop()->screenGeometry().size();
    if (status_bar_.isVisible())
    {
        return screen_size - QSize(0, status_bar_.frameSize().height()) ;
    }
    else
    {
        return screen_size;
    }
}

void ComicMainWindow::onScreenSizeChanged(int) {
    onyx::screen::instance().enableUpdate(false);
    setFixedSize(qApp->desktop()->screenGeometry().size());
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC);
}

void ComicMainWindow::resizeEvent(QResizeEvent *)
{
    setImage(curImage);
}

void ComicMainWindow::closeFile()
{
    if (rawImage->isEmpty())
    {
        return;
    }
    iStream->close();

    *curImage = QImage();
    if (rawImage) rawImage->clear();
    setImage(NULL);
}

bool ComicMainWindow::open(const QString &path)
{
    if (path.isEmpty())
    {
        return false;
    }

    if (!iStream->open(path))
    {
        QMessageBox::warning(this, COMIC_READER_APP_NAME, tr("Failed to open:\n\n%1").arg(iStream->errstr()));
        return false;
    }
    else
    {
        loadImage();
        // TODO improve the error handling
        return true;
    }

}

void ComicMainWindow::opendir()
{
    QString path;
    if (!iStream->isEmpty())
        path = iStream->cwd();
    else if (!optCWD.isEmpty())
        path = optCWD;
    else
        path = QDir::homePath();

    QString dirname = QFileDialog::getExistingDirectory(this, tr("Display Images in Directory"), path);
    if (dirname.isEmpty()) return;

    if (!iStream->open(dirname))
        QMessageBox::warning(this, tr("KComic"), tr("Unable to open directory: %1").arg(dirname));
    else
        loadImage();
}

bool ComicMainWindow::save()
{
    if (rawImage->isEmpty()) return false;

    QString dir = QFileDialog::getExistingDirectory(this, tr("Save Current Image in Directory"), QDir::homePath());
    if (dir.isEmpty()) return false;

    QString fileName = dir + "/" + QDir::toNativeSeparators(iStream->curFile()).split("/").last();
    return saveFile(fileName, true);
}

bool ComicMainWindow::saveAs()
{
    if (rawImage->isEmpty()) return false;

    QString filter = tr("Images (*.") + imgFormats.join(" *.") + ")";
    QString title  = tr("Save Image as Format");

    QString fileName = QFileDialog::getSaveFileName(this, title, QDir::homePath(), filter);
    if (fileName.isEmpty()) return false;

    return saveFile(fileName, false);
}

void ComicMainWindow::loadImage()
{
    *rawImage = iStream->rawdata();
    if (!curImage->loadFromData((*rawImage)))
    {
        QMessageBox::warning(this, COMIC_READER_APP_NAME, tr("Error loading image data: %1 [%2]")
                .arg(iStream->curFile())
                .arg(rawImage->size())
            );
        return;
    }

    setImage(curImage);
    optCWD = iStream->cwd();
    // Set the progress bar after loading image
    positionChanged(iStream->fileIndex(), iStream->getPageList().size());
}

bool ComicMainWindow::saveFile(const QString &fileName, bool saveRaw)
{
    if (!saveRaw)
    {
        bool ret = curImage->save(fileName);
        return ret;
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this, COMIC_READER_APP_NAME,
                             tr("Cannot write to file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QDataStream stream(&file);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    int ret = stream.writeRawData(rawImage->constData(), rawImage->length());
    QApplication::restoreOverrideCursor();

    if (ret != rawImage->length())
    {
        QMessageBox::warning(this, COMIC_READER_APP_NAME,
                             tr("Error writing file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
    return true;
}

void ComicMainWindow::setImage(QImage *image)
{
    image_label_->clear();
    if (image && !image->isNull())
    {
        QImage newImage, tmpimg;

        if (rotate)
        {
            tmpimg = image->transformed(QMatrix().rotate(rotate),
                    Qt::SmoothTransformation);
        }
        else
        {
            tmpimg = *image;
        }

        if ( smaller(tmpimg.size(), view_.maximumViewportSize()) ||
             larger (tmpimg.size(), view_.maximumViewportSize()) )
        {
            isScaled = true;
            newImage = tmpimg.scaled(view_.maximumViewportSize(),
                    Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        else
        {
            isScaled = false;
            newImage = tmpimg;
        }
        tmpimg = QImage();

        image_label_->setPixmap(QPixmap::fromImage(newImage));
        image_label_->resize(newImage.size());
    }
    else
    {
        isScaled = false;
    }
}

void ComicMainWindow::onFullScreen(bool full_screen)
{
    if (full_screen)
    {
        statusBar()->hide();
        this->setWindowState(windowState() | Qt::WindowFullScreen);
    }
    else
    {
        statusBar()->show();
        this->setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
    isFullScreen = full_screen;
    setImage(curImage);
    update();
}

void ComicMainWindow::rotateRight()
{
    rotate += 90;
    if (rotate >= 360) rotate = 0;
    setImage(curImage);
}

void ComicMainWindow::rotateLeft()
{
    rotate -= 90;
    if (rotate < 0) rotate = 270;
    setImage(curImage);
}

void ComicMainWindow::suggestFile(QString &filepath, char *direction)
{
    if (filepath.isEmpty()) return;

    QString ftype;
    if (iStream->isArchive())
        ftype = " archive?";
    else
        ftype = " directory?";

    QFileInfo path_info(filepath);
    QString file_name(path_info.fileName());
    ComicMessageDialog * dialog = new ComicMessageDialog(this,
            QString("Do you want to load the ") + direction + ftype + "\n\n" + file_name);
    int ret = dialog->exec();

    if (ret == QMessageBox::Yes)
    {
        if (!iStream->open(filepath))
        {
            ComicMessageDialog warning(this, tr("Failed to open:\n\n%1").arg(iStream->errstr()),
                    QMessageBox::Warning, QMessageBox::Ok);
            warning.exec();
        }
        else
        {
            loadImage();
        }
    }
}

void ComicMainWindow::prevFile()
{
    if (iStream->isEmpty()) return;

    if (iStream->prev())
        loadImage();
    else
    {
        QString newf = iStream->suggestPrev();
        if (newf.isEmpty()) return;
        suggestFile(newf, QString("previous").toLatin1().data());
    }
}

void ComicMainWindow::nextFile()
{
    if (iStream->isEmpty()) return;

    if (iStream->next())
        loadImage();
    else
    {
        QString newf = iStream->suggestNext();
        if (newf.isEmpty()) return;
        suggestFile(newf, QString("next").toLatin1().data());
    }
}

void ComicMainWindow::firstFile()
{
    if (!iStream->isEmpty() && iStream->first())
        loadImage();
}

void ComicMainWindow::lastFile()
{
    if (!iStream->isEmpty() && iStream->last())
        loadImage();
}

void ComicMainWindow::gotoFile()
{
    if (iStream->isEmpty()) return;
    
    QStringList list = iStream->getPageList();
    if (list.size() < 2) return;

    bool ok;
    QString item = QInputDialog::getItem(this, tr("Select File"),
        tr("File:"), list, list.indexOf(iStream->curFile()), false, &ok);

    if (ok && iStream->page(list.indexOf(item)))
        loadImage();
}

bool ComicMainWindow::event(QEvent * event)
{
    bool ret = QMainWindow::event(event);
    if (event->type() == QEvent::UpdateRequest
            && onyx::screen::instance().isUpdateEnabled())
    {
        if (0 == onyx::screen::instance().userData())
        {
            ++onyx::screen::instance().userData();
            onyx::screen::instance().updateWidget(this,
                    onyx::screen::ScreenProxy::GC);
        }
        else
        {
            onyx::screen::instance().updateWidget(this);
        }
    }
    return ret;
}

void ComicMainWindow::positionChanged(const int current, const int total)
{
    status_bar_.setProgress(current + 1, total);
}

void ComicMainWindow::onPagebarClicked(const int percentage, const int value)
{
    if (iStream->isEmpty())
    {
        return;
    }
    if (iStream->page(value-1))
    {
        loadImage();
    }
}

}   // namespace comic_reader
