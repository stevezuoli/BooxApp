
#ifndef COMIC_MAIN_WINDOW_H_
#define COMIC_MAIN_WINDOW_H_

#include "onyx/ui/ui.h"
#include "comic_model.h"
#include "comic_view.h"
#include "archive_file_stream.h"
#include <QMainWindow>
#include <QCloseEvent>

class QDir;
class QAction;
class QMenu;
class QImage;
class QLabel;

using namespace ui;
using namespace compression;

namespace comic_reader
{

class ComicMainWindow: public QMainWindow
{
    Q_OBJECT

public:
    ComicMainWindow(QObject *parent);
    ~ComicMainWindow();

    void attachModel(ComicModel *model);
    ComicView * view();
    bool open(const QString &path);
    // close a single image file for reopen
    void closeFile();

protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);

private Q_SLOTS:

    void opendir();
    bool save();
    bool saveAs();
    void onFullScreen(bool full_screen);
    void rotateRight();
    void rotateLeft();
    void prevFile();
    void nextFile();
    void firstFile();
    void lastFile();
    void gotoFile();
    void quitApp();
    void onScreenSizeChanged(int);
    QSize clientSize();

    void onPagebarClicked(const int percentage, const int value);

private:
    void createMenus();
    void readSettings();
    void writeSettings();
    bool saveFile(const QString &fileName, bool saveRaw);
    void loadImage();
    void setImage(QImage *image = NULL);
    void suggestFile(QString &filepath, char *direction);

    bool event(QEvent * event);

    void positionChanged(const int current, const int total);

private:
    QByteArray *rawImage;
    QImage *curImage;

    QLabel *image_label_;

    ArchiveFileStream *iStream;
    QStringList imgFormats;

    int rotate;
    bool isFullScreen;
    bool isScaled;

    QString optCWD;

    ComicView view_;
    StatusBar status_bar_;
    ComicModel *model_;
};

inline bool smaller(const QSize &a, const QSize &b)	{ return a.width() < b.width() && a.height() < b.height(); }
inline bool larger (const QSize &a, const QSize &b)	{ return a.width() > b.width() || a.height() > b.height(); }

}   // namespace comic_reader

#endif
