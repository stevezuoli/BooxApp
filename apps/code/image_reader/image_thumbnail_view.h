#ifndef THUMBNAIL_VIEW_H
#define THUMBNAIL_VIEW_H

#include "image_utils.h"

using namespace onyx::ui;
using namespace ui;
using namespace sketch;

namespace image
{

class NotesDocumentManager;
class ThumbnailView : public QWidget
{
  Q_OBJECT
  typedef shared_ptr<BaseThumbnail> ThumbPtr;
public:
    explicit ThumbnailView(QWidget *parent = 0);
    ~ThumbnailView(void);

    void attachMainWindow(MainWindow *main_window);
    void deattachMainWindow(MainWindow *main_window);

    void attachSketchProxy(SketchProxy *sketch_proxy);
    void deattachSketchProxy();

    void attachNotesDocManager(NotesDocumentManager *notes_doc_mgr);
    void deattachNotesDocManager();

    void setTotalNumber(const int num);
    void setCurrentPage(int key);
    void updateThumbnails();
    void setThumbnail(int index, ThumbPtr thumb, const QRect &bounding_rect);

Q_SIGNALS:
    void needThumbnail(const int key, const QRect &rect);
    void clearThumbnails();
    void returnToReading(const int page_number = -1);
    void popupJumpPageDialog();

    void positionChanged(const int current, const int total);
    void needFullScreen(bool enable);

private Q_SLOTS:
    void handlePagebarClicked(const int percent, const int value);
    void handlePopupContextMenu();

private:
    // message handlers
    bool event(QEvent * event);
    void paintEvent(QPaintEvent *pe);
    void resizeEvent(QResizeEvent *re);
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void keyReleaseEvent(QKeyEvent *ke);

private:
    void paintPage(QPainter &painter, ThumbPtr page, int index);
    void paintTitle(QPainter &painter, ThumbPtr page, int index);
    void paintSketches(QPainter &painter, ThumbPtr page, int index);
    void paintBoundingRect(QPainter &painter, const ThumbnailPage& thumb);

    void nextScreen();
    void prevScreen();

    void moveCurrentPage(const int next_num);

    void initializePopupMenuActions();
    void popupMenu();

    // get thumbnail layout page by key
    ThumbnailPage& getLayoutPage(const int key);

    // update the first page by current page
    void updatePageNumber(const int key);

    inline int getTotalScreens(const int total_pages);
    inline int getCurrentScreen(const int current);

private:
    typedef std::set<int> RenderSet;
    typedef RenderSet::iterator RenderSetIter;

private:
    ThumbnailLayout             layout_;
    SketchProxy                 *sketch_proxy_;
    NotesDocumentManager        *notes_doc_manager_;
    DisplayPages<BaseThumbnail> display_pages_;
    RenderSet                   left_pages_;
    int                         cur_page_;
    int                         first_page_;
    int                         total_number_;

    QPoint                      mouse_press_pos_;
    SystemActions               system_actions_;
};

inline int ThumbnailView::getTotalScreens(const int total_pages)
{
    if (total_pages % layout_.pages().size() != 0)
    {
        return (total_pages / layout_.pages().size() + 1);
    }
    return total_pages / layout_.pages().size();
}

inline int ThumbnailView::getCurrentScreen(const int current)
{
    return (current / layout_.pages().size());
}

};
#endif
