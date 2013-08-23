#include "comic_view.h"
#include "comic_model.h"
#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"

using namespace ui;

namespace comic_reader
{

ComicView::ComicView(QWidget *parent)
    : QScrollArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setBackgroundRole(QPalette::Shadow);
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    connect(this, SIGNAL(nextFile()), parent, SLOT(nextFile()));
    connect(this, SIGNAL(previousFile()), parent, SLOT(prevFile()));
}

ComicView::~ComicView(void)
{
}

void ComicView::attachModel(ComicModel *model)
{
    model_ = model;
}

void ComicView::deattachModel()
{
    if (0 == model_)
    {
        return;
    }

    model_ = 0;
}

void ComicView::processToolActions()
{
    ReadingToolsType tool = reading_tool_actions_.selectedTool();
    switch (tool)
    {
    case ::ui::GOTO_PAGE:
        {
            gotoPage();
        }
        break;
    case ::ui::CLOCK_TOOL:
        {
            showClock();
        }
        break;
    default:
        break;
    }
}

void ComicView::processSystemActions()
{
    SystemAction system_action = system_actions_.selected();
    switch (system_action)
    {
    case RETURN_TO_LIBRARY:
        {
            emit comicBookClosed();
        }
        break;
    case ROTATE_SCREEN:
        {
            sys::SysStatus::instance().rotateScreen();
        }
        break;
    case SCREEN_UPDATE_TYPE:
        {
            onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
            onyx::screen::instance().toggleWaveform();
        }
        break;
    case FULL_SCREEN:
        {
            emit fullScreen(true);
        }
        break;
    case EXIT_FULL_SCREEN:
        {
            emit fullScreen(false);
        }
        break;
    case MUSIC:
        {
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
            sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
        }
        break;
    default:
        break;
    }
}

void ComicView::showClock()
{
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    emit requestClockDialog();
}

void ComicView::updateToolActions()
{
    // Reading tools of go to page and clock.
    std::vector<ReadingToolsType> tools;
    tools.push_back(::ui::GOTO_PAGE);
    tools.push_back(::ui::CLOCK_TOOL);
    reading_tool_actions_.generateActions(tools, false);
}

bool ComicView::updateActions()
{
    updateToolActions();

    std::vector<int> all;
    all.push_back(ROTATE_SCREEN);
    if (isFullScreenByWidgetSize())
    {
        all.push_back(EXIT_FULL_SCREEN);
    } else
    {
        all.push_back(FULL_SCREEN);
    }
    all.push_back(MUSIC);
    all.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(all);
    return true;
}

void ComicView::showContextMenu()
{
    PopupMenu menu(this);
    updateActions();

    menu.addGroup(&reading_tool_actions_);
    menu.setSystemAction(&system_actions_);

    if (menu.popup() != QDialog::Accepted)
    {
        QApplication::processEvents();
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == reading_tool_actions_.category())
    {
        processToolActions();
    }
    else if (group == system_actions_.category())
    {
        processSystemActions();
    }
}

void ComicView::mousePressEvent(QMouseEvent *event)
{
    // TODO
}
void ComicView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // TODO
    if (isFullScreenByWidgetSize())
    {
        emit fullScreen(false);
    }
    else
    {
        emit fullScreen(true);
    }
}

void ComicView::keyPressEvent(QKeyEvent *event)
{
    event->accept();
}

void ComicView::keyReleaseEvent(QKeyEvent *event)
{
//    qDebug() << "ComicView: key release event";
    switch (event->key())
    {
    case Qt::Key_PageDown:
    case Qt::Key_Right:
        emit nextFile();
        break;
    case Qt::Key_PageUp:
    case Qt::Key_Left:
        emit previousFile();
        break;
    case Qt::Key_Return:
        gotoPage();
        break;
    case Qt::Key_Menu:
        showContextMenu();
        break;
    case Qt::Key_Escape:
        comicBookClosed();
        break;
    }
    event->accept();
}

void ComicView::gotoPage()
{
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    emit requestGotoPageDialog();
}

bool ComicView::isFullScreenByWidgetSize()
{
    if (parentWidget()) {
        QSize parentSize = parentWidget()->size();
        if (parentSize == size())
        {
            return true;
        }
    }
    return false;
}

}
