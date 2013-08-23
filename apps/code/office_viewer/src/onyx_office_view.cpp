#include "onyx_office_view.h"
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>

#include "onyx/sys/sys.h"
#include "onyx/ui/ui.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/cms/content_thumbnail.h"
#include "main_widget.h"

using namespace ui;


namespace onyx {

OfficeView::OfficeView(OfficeReader & instance, QWidget *parent)
: QWidget(parent)
, instance_(instance)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);

    // setup connection.
    connect(&instance, SIGNAL(splashScreenDone()), this, SLOT(onSplashScreenDone()));
    connect(&instance, SIGNAL(documentOpened()), this, SLOT(onDocumentOpened()));
    connect(&instance, SIGNAL(insufficientMemory()), this, SLOT(onInsufficientMemory()));
    connect(&instance, SIGNAL(searchStateChanged(int)), this, SLOT(onSearchStateChanged(int)));
    connect(&instance, SIGNAL(jobFinished()), this, SLOT(onJobFinished()), Qt::QueuedConnection );

    // system connection
    connect(&sys::SysStatus::instance(), SIGNAL(sdCardChangedSignal(bool)), this, SLOT(onSdCardChanged(bool)));
    connect(&sys::SysStatus::instance(), SIGNAL(mountTreeSignal(bool, const QString &)),
            this, SLOT(handleMountTreeEvent(bool, const QString &)));
    connect(&sys::SysStatus::instance(), SIGNAL(aboutToShutdown()), this, SLOT(onAboutToShutdown()));
    connect(&sys::SysStatus::instance(), SIGNAL(wakeup()), this, SLOT(onWakeup()));
    connect(&sys::SysStatus::instance(), SIGNAL(musicPlayerStateChanged(int)),
            this, SLOT(onMusicPlayerStateChanged(int)));
}

OfficeView::~OfficeView()
{
}

bool OfficeView::open(const QString &path)
{
    if (showSplash())
    {
        update();
    }

    ContentManager database;
    vbf::openDatabase(path, database);

    // Check thumbnail is ok or not.
    mdb_found_ = vbf::loadDocumentOptions(database, path, conf_);
    vbf::loadBookmarks(database, path, instance_.bookmarks());
    if (!instance_.open(path))
    {
        return false;
    }

    if (mdb_found_ && restoreState())
    {
        return true;
    }
    useDefaultState();
    return true;
}

void OfficeView::updateConf()
{
    using namespace vbf;
    conf_.options[CONFIG_PAGE_NUMBER] = instance_.currentPage();
    conf_.options[CONFIG_VIEW_PORT] = instance_.viewState();
    conf_.info.mutable_title() = instance_.title();
    conf_.options[CONFIG_FLASH_TYPE] = onyx::screen::instance().defaultWaveform();
}

bool OfficeView::restoreState()
{
    using namespace vbf;
    return instance_.restoreViewState(conf_.options[CONFIG_VIEW_PORT].toByteArray());
}

void OfficeView::useDefaultState()
{
    setFont(Medium);
}

bool OfficeView::close()
{
    updateConf();
    using namespace vbf;
    ContentManager database;
    if (openDatabase(instance_.path(), database))
    {
        vbf::saveDocumentOptions(database, instance_.path(), conf_);
        vbf::saveBookmarks(database, instance_.path(), instance_.bookmarks());
    }
    instance_.close();
    emit docClosed();
    return true;
}

void OfficeView::showContextMenu()
{
    PopupMenu menu(this);
    updateActions();

    menu.addGroup(&font_actions_);
    menu.addGroup(&zoom_setting_actions_);
    menu.addGroup(&reading_tool_actions_);
    menu.setSystemAction(&system_actions_);

    if (menu.popup() != QDialog::Accepted)
    {
        QApplication::processEvents();
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == font_actions_.category())
    {
        int index = font_actions_.selectedIndex();
        setFont(index);
    }
    else if (group == reading_tool_actions_.category())
    {
        processToolActions();
    }
    else if (group == zoom_setting_actions_.category())
    {
        processZoomingActions();
    }
    else if (group == system_actions_.category())
    {
        processSystemActions();
    }
}

void OfficeView::setFont(const int index)
{
    instance_.doAction(SET_FONT_SIZE, index);
}

bool OfficeView::updateActions()
{
    updateFontSizeActions();
    updateZoomingActions();
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

void OfficeView::updateFontSizeActions()
{
    vector<int> size;
    for (int i = 0; i <= 4; ++i)
    {
        size.push_back(i);
    }

    font_actions_.generateActions(size, instance_.fontSize());
}

void OfficeView::updateZoomingActions()
{
    if (zoom_setting_actions_.actions().size() <= 0)
    {
        std::vector<ZoomFactor> zoom_settings;
        zoom_settings.clear();
        zoom_settings.push_back(ZOOM_TO_PAGE);
        zoom_settings.push_back(ZOOM_TO_WIDTH);
        zoom_settings.push_back(ZOOM_TO_HEIGHT);
        zoom_settings.push_back(75.0f);
        zoom_settings.push_back(100.0f);
        zoom_settings.push_back(125.0f);
        zoom_settings.push_back(150.0f);
        zoom_settings.push_back(175.0f);
        zoom_settings.push_back(200.0f);
        zoom_settings.push_back(300.0f);
        zoom_settings.push_back(400.0f);
        zoom_setting_actions_.generateActions(zoom_settings);
    }
    zoom_setting_actions_.setCurrentZoomValue(instance_.reading_context().zoom_value_);
}

void OfficeView::updateToolActions()
{
    std::vector<ReadingToolsType> tools;
    tools.push_back(SEARCH_TOOL);
    reading_tool_actions_.generateActions(tools);
    tools.clear();

    if (hasBookmark())
    {
        tools.push_back(::ui::DELETE_BOOKMARK);
    }
    else
    {
        tools.push_back(::ui::ADD_BOOKMARK);
    }
    tools.push_back(SHOW_ALL_BOOKMARKS);
    reading_tool_actions_.generateActions(tools, true);

    // Reading tools of go to page and clock.
    tools.clear();
    tools.push_back(::ui::GOTO_PAGE);
    tools.push_back(::ui::CLOCK_TOOL);
    reading_tool_actions_.generateActions(tools, true);
    reading_tool_actions_.action(::ui::GOTO_PAGE)->setEnabled(instance_.totalPage() > 1);
}

void OfficeView::onSplashScreenDone()
{
}

void OfficeView::keyReleaseEvent(QKeyEvent* ke)
{
    //ke->accept();
    switch (ke->key())
    {
        case Qt::Key_Left:
            instance_.doAction(PAN_LEFT);
            break;
        case Qt::Key_Right:
            instance_.doAction(PAN_RIGHT);
            break;
        case Qt::Key_Plus:
        case Qt::Key_Up:
        case Qt::Key_PageUp:
                instance_.doAction(SCROLL_UP);
            break;
        case Qt::Key_Minus:
        case Qt::Key_Down:
        case Qt::Key_PageDown:
                instance_.doAction(SCROLL_DOWN);
            break;
        case Qt::Key_Return:
            gotoPage();
            break;
        case Qt::Key_Menu:
            showContextMenu();
            break;
        case Qt::Key_Escape:
            close();
            break;
        default:
            break;
    }
    return;
}

void OfficeView::mousePressEvent(QMouseEvent *event)
{
    last_pos_ = event->pos();
}

void OfficeView::mouseReleaseEvent(QMouseEvent * event)
{
    int direction = sys::SystemConfig::direction(last_pos_, event->pos());

    // Click event.
    if (direction == 0)
    {
        instance_.doAction(OPEN_LINK, event->pos());
    }
    else if (instance_.leftLimit() || instance_.rightLimit() || instance_.topLimit() || instance_.bottomLimit())
    {
        int delta_X = event->pos().x() - last_pos_.x();
        int delta_Y = event->pos().y() - last_pos_.y();
        instance_.doAction(PAN, QSize(delta_X, delta_Y));
        return;
    }

    if (direction > 0)
    {
        instance_.doAction(NEXT_PAGE);
    }
    else if (direction < 0)
    {
        instance_.doAction(PREV_PAGE);
    }
}

void OfficeView::mouseMoveEvent(QMouseEvent * event)
{
}

void OfficeView::resizeEvent(QResizeEvent * event)
{
    //  TODO:seems picsel cannot be resized during run time.
     // instance_.doAction(RESIZE, event->size());
}

void OfficeView::paintEvent(QPaintEvent* pe)
{
    qDebug("process paint event.");
    QPainter painter(this);

    if (!instance_.isOpened())
    {
        qDebug("paint picsel splash.");
        paintPicselSplash(painter);
    }
    else
    {
        paintPage(painter);
    }
}

void OfficeView::paintPicselSplash(QPainter &painter)
{
    if (showSplash())
    {
        painter.fillRect(rect(), QBrush(Qt::black));

        QImage image("/usr/share/picsel/bootsplash.png");
        QPoint pt((rect().width() - image.width()) / 2,
                  (rect().height() - image.height()) / 2);
        painter.drawImage(pt, image);

        QRect rc(rect());
        rc.setHeight(pt.y());
        QFont font("", 30);
        painter.setFont(font);
        painter.setPen(QPen(Qt::white));
        painter.drawText(rc, Qt::AlignCenter, QCoreApplication::tr("Loading document..."));
    }
}

void OfficeView::paintPage(QPainter & painter)
{
    painter.drawImage(QPoint(), instance_.image());
    if (hasBookmark())
    {
        static QImage image(":/images/bookmark_flag.png");
        QPoint pt(width() - image.width(), 0);
        painter.drawImage(pt, image);
    }
}

void OfficeView::saveThumbnail()
{
    qDebug("Check thumbnail now.");
    QFileInfo info(instance_.path());
    cms::ContentThumbnail thumbdb(info.absolutePath());

    if (!thumbdb.hasThumbnail(info.fileName(), cms::THUMBNAIL_LARGE))
    {
        qDebug("Save thumbnail now.");
        thumbdb.storeThumbnail(info.fileName(),
                               cms::THUMBNAIL_LARGE,
                               instance_.image().scaled(cms::thumbnailSize(), Qt::KeepAspectRatio));
    }
}

void OfficeView::onJobFinished()
{
    // we receive the first job finished event.
    static bool received = false;
    if (!received)
    {
        received = true;
        ++onyx::screen::instance().userData();
        saveThumbnail();
    }

    emit pageChanged(instance_.currentPage(), instance_.totalPage());
    update();
    qDebug("signal update sent.");
}

void OfficeView::onDocumentOpened()
{
    //to restore previous state

}

void OfficeView::onInsufficientMemory()
{
    ErrorDialog dialog(tr("Insufficient Memory."), this);
    dialog.exec();

    // Could not open, we have to exit directly. Should not use qApp->exit
    // as there is no main loop yet.
    exit(-1);
}

void OfficeView::onSearchStateChanged(int state)
{
    if (search_widget_)
    {
        if (state == NotFound) //  || state == EndOfDocument)
        {
            search_widget_->noMoreMatches();
            instance_.cancelSearch();
        }
    }
}

bool OfficeView::addBookmark()
{
    instance_.doAction(ADD_BOOKMARK);
    update();
    return true;
}

bool OfficeView::deleteBookmark()
{
    instance_.doAction(DELETE_BOOKMARK);
    update();
    return true;
}

bool OfficeView::hasBookmark()
{
    return instance_.hasBookmark(instance_.currentPage());
}

void OfficeView::processToolActions()
{
    ReadingToolsType tool = reading_tool_actions_.selectedTool();
    switch (tool)
    {
    case DICTIONARY_TOOL:
        {
            // startDictLookup();
        }
        break;
    case SEARCH_TOOL:
        {
            showSearchWidget();
        }
        break;
    case ::ui::ADD_BOOKMARK:
        {
            addBookmark();
        }
        break;
    case ::ui::DELETE_BOOKMARK:
        {
            deleteBookmark();
        }
        break;
    case ::ui::SHOW_ALL_BOOKMARKS:
        {
            displayBookmarks();
        }
        break;
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


void OfficeView::getBookmarksModel(QStandardItemModel& bookmarks_model) {
    bookmarks_model.setColumnCount(2);
    vbf::BookmarksIter begin = instance_.bookmarks().begin();
    vbf::BookmarksIter end   = instance_.bookmarks().end();
    int row = 0;
    for (vbf::BookmarksIter iter  = begin; iter != end; ++iter, ++row) {
        int page_number = iter->data().value<int>();
        qDebug() << page_number;
        if (page_number != 0) {
            QStandardItem *title = new QStandardItem(iter->title());
            title->setData(iter->data());
            title->setEditable(false);
            bookmarks_model.setItem(row, 0, title);

            QString str = QString::number(page_number);
            QStandardItem *page = new QStandardItem(str);
            page->setEditable(false);
            page->setTextAlignment(Qt::AlignCenter);
            bookmarks_model.setItem(row, 1, page);
        }
    }
    bookmarks_model.setHeaderData(0, Qt::Horizontal, QVariant::fromValue(tr("Bookmarks")), Qt::DisplayRole);
    bookmarks_model.setHeaderData(1, Qt::Horizontal, QVariant::fromValue(tr("Page")), Qt::DisplayRole);
}

void OfficeView::displayBookmarks() {
    QStandardItemModel bookmarks_model;
    getBookmarksModel(bookmarks_model);
    TreeViewDialog bookmarks_view(this);
    bookmarks_view.setModel(&bookmarks_model);
    bookmarks_view.tree().showHeader(true);
    QVector<int> percentages;
    percentages.push_back(80);
    percentages.push_back(20);
    bookmarks_view.tree().setColumnWidth(percentages);
    int ret = bookmarks_view.popup(tr("Bookmarks"));
    onyx::screen::instance().enableUpdate(false);
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    if (ret != QDialog::Accepted) {
        return;
    }
    QModelIndex index = bookmarks_view.selectedItem();
    if (!index.isValid()) {
        return;
    }
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::INVALID);
    QStandardItem *item = bookmarks_model.itemFromIndex(index);
    int page = item->data().value<int>();

    onyx::screen::instance().enableUpdate(false);
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    instance_.doAction(GOTO_PAGE, page);
}

void OfficeView::showSearchWidget()
{
    if (!search_widget_) {
        search_widget_.reset(new SearchWidget(this, instance_.searchContext()));
        connect(search_widget_.get(), SIGNAL(search(BaseSearchContext &)),
                &instance_, SLOT(search(BaseSearchContext &)));
        connect(search_widget_.get(), SIGNAL(closeClicked()), &instance_, SLOT(cancelSearch()));
    }

    search_widget_->ensureVisible();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::INVALID);
}

void OfficeView::processZoomingActions()
{
    double value = zoom_setting_actions_.getSelectedZoomValue();
    qDebug()<< "zoomvalue" << value;

    if (value == ZOOM_TO_PAGE)
    {
        instance_.doAction(FIT_TO_PAGE);
    }
    else if (value == ZOOM_TO_WIDTH)
    {
        instance_.doAction(FIT_TO_WIDTH);
    }
    else if (value == ZOOM_TO_HEIGHT)
    {
        instance_.doAction(FIT_TO_HEIGHT);
    }
    else
    {
        instance_.doAction(SET_ZOOM_RATIO, value);
    }
}

void OfficeView::processSystemActions()
{
    SystemAction system_action = system_actions_.selected();
    switch (system_action)
    {
    case RETURN_TO_LIBRARY:
        {
            close();
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
            // Start or show music player.
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
            sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
        }
        break;
    default:
        break;
    }
}

void OfficeView::gotoPage()
{
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    emit requestGotoPageDialog();
}

void OfficeView::showClock()
{
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    emit requestClockDialog();
}

// Using restoreViewState does not work well, not sure why.
// so have to use set font size or zoom.
void OfficeView::resizeBackend(const  QSize& s)
{
    // keep zoom and font size. use view state.
    OfficeReader::instance().viewState();
    OfficeReader::instance().doAction(RESIZE, s);
    instance_.restoreViewState();
}

void OfficeView::onSdCardChanged(bool inserted)
{
    if (!inserted && instance_.path().startsWith(SDMMC_ROOT))
    {
        close();
    }
}

void OfficeView::onWakeup()
{
}

/// Handle mount tree event including internal flash and sd card.
void OfficeView::handleMountTreeEvent(bool inserted, const QString &mount_point)
{
    if (!inserted && instance_.path().startsWith(mount_point))
    {
        close();
    }
}

void OfficeView::onAboutToShutdown()
{
    close();
}

void OfficeView::onMusicPlayerStateChanged(int state)
{
    if (state == sys::HIDE_PLAYER || state == sys::STOP_PLAYER)
    {
        onyx::screen::instance().flush();
        update();
    }
}

bool OfficeView::showSplash()
{ 
    return (qgetenv("SHOW_OFFICE_SPLASH").toInt() > 0);
}

bool OfficeView::isFullScreenByWidgetSize()
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

} // namespace onyx

