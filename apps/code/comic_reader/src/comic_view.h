#ifndef COMIC_VIEW_H_
#define COMIC_VIEW_H_

#include "onyx/ui/ui.h"
#include "onyx/ui/system_actions.h"

using namespace ui;

namespace comic_reader
{

class ComicModel;

class ComicView : public QScrollArea
{
    Q_OBJECT

public:
    ComicView(QWidget *parent = NULL);
    ~ComicView(void);

    void attachModel(ComicModel *model);
    void deattachModel();

Q_SIGNALS:
    void rotateScreen();
    void testWakeUp();
    void fullScreen(bool full_screen);
    void comicBookClosed();

    // file navigation signals
    void nextFile();
    void previousFile();
    void requestGotoPageDialog();
    void requestClockDialog();

public Q_SLOTS:
    void showContextMenu();

private Q_SLOTS:
    void gotoPage();
    void processToolActions();
    void processSystemActions();
    void showClock();

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    bool isFullScreenByWidgetSize();
    void updateToolActions();
    bool updateActions();

private:
    ComicModel *model_;
    SystemActions system_actions_;
    ReadingToolsActions reading_tool_actions_;
};

}

#endif
