#include "notes_view.h"
#include "image_global_settings.h"

namespace image
{

bool operator == ( const shared_ptr<NotesPage> & a, const shared_ptr<NotesPage> & b )
{
    return (a->image_ == b->image_) && (a->index_ == b->index_);
}

DisplayPages::DisplayPages()
{
}

DisplayPages::~DisplayPages()
{
}

void DisplayPages::add(shared_ptr<NotesPage> page)
{
    page->image()->lock();
    int idx = pages_.indexOf(page);
    if (idx < 0)
    {
        pages_.push_back(page);
    }
    else
    {
        pages_[idx] = page;
    }
}

void DisplayPages::clear()
{
    for (PagesIter it = pages_.begin(); it != pages_.end(); ++it)
    {
        (*it)->image()->unlock();
    }
    pages_.clear();
}

size_t DisplayPages::size()
{
    return pages_.size();
}

shared_ptr<NotesPage> DisplayPages::getPage(int num)
{
    return pages_[num];
}

NotesView::NotesView(QWidget *parent)
    : BaseView(parent)
    , model_(0)
    , read_mode_(PAGE_LAYOUT)
    , layout_(0)
    , current_page_(0)
    , need_init_view_(false)
#ifdef DISPLAY_SYSTEM_BUSY
    , need_set_rendering_busy_(false)
#endif
    , current_waveform_(onyx::screen::instance().defaultWaveform())
{
    qRegisterMetaType<shared_ptr<ImageItem> >("shared_ptr<ImageItem>");
    qRegisterMetaType<shared_ptr<BaseThumbnail> >("shared_ptr<BaseThumbnail>");
    qRegisterMetaType<ImageStatus>("ImageStatus");

    connect(&slide_timer_, SIGNAL(timeout()), this, SLOT(slideShowNextImage()));
    connect(&status_mgr_, SIGNAL(stylusChanged(const int)), this, SLOT(onStylusChanges(const int)));

    ImageGlobalSettings::instance().needDither(false);
    view_setting_.zoom_setting = ZOOM_TO_WIDTH;
}

NotesView::~NotesView(void)
{
}

void NotesView::updateActions()
{
    // Reading Tools
    std::vector<ReadingToolsType> reading_tools;
    reading_tools_actions_.generateActions(reading_tools,false);
    if (!status_mgr_.isSlideShow())
    {
        reading_tools.clear();
        reading_tools.push_back(NOTES_BACKGROUND_SELECTION);
        reading_tools.push_back(EXPORT_SKETCH_DATA);
        reading_tools_actions_.generateActions(reading_tools, true);

        reading_tools.clear();
        reading_tools.push_back(INSERT_NOTE);
        reading_tools.push_back(REMOVE_NOTE);
        reading_tools.push_back(SCROLL_PAGE);
        reading_tools.push_back(GOTO_PAGE);
        reading_tools_actions_.generateActions(reading_tools, true);
    }

    reading_tools.clear();
    reading_tools.push_back(SLIDE_SHOW);
    reading_tools.push_back(CLOCK_TOOL);
    reading_tools_actions_.generateActions(reading_tools, true);
    reading_tools_actions_.setActionStatus(SLIDE_SHOW, status_mgr_.isSlideShow());

    // Zoom Settings
    std::vector<ZoomFactor> zoom_settings;
    zoom_settings.push_back(ZOOM_TO_PAGE);
    zoom_settings.push_back(ZOOM_TO_WIDTH);
    zoom_settings.push_back(ZOOM_TO_HEIGHT);
    zoom_settings.push_back(ZOOM_SELECTION);
    zoom_settings.push_back(75.0f);
    zoom_settings.push_back(100.0f);
    zoom_settings.push_back(125.0f);
    zoom_settings.push_back(150.0f);
    zoom_settings.push_back(175.0f);
    zoom_settings.push_back(200.0f);
    zoom_settings.push_back(300.0f);
    zoom_settings.push_back(400.0f);
    zoom_setting_actions_.generateActions(zoom_settings);
    zoom_setting_actions_.setCurrentZoomValue(layout_->zoomSetting());

    // View Settings
    PageLayouts page_layouts;
    page_layouts.push_back(PAGE_LAYOUT);
    page_layouts.push_back(CONTINUOUS_LAYOUT);
    page_layouts.push_back(THUMBNAIL_LAYOUT);
    view_actions_.generatePageLayoutActions(page_layouts, read_mode_);

    // set sketch mode
    sketch_actions_.clear();
    SketchModes     sketch_modes;
    SketchColors    sketch_colors;
    SketchShapes    sketch_shapes;

    sketch_modes.push_back(MODE_SKETCHING);
    sketch_modes.push_back(MODE_ERASING);

    sketch_colors.push_back(SKETCH_COLOR_WHITE);
    //sketch_colors.push_back(SKETCH_COLOR_LIGHT_GRAY);
    //sketch_colors.push_back(SKETCH_COLOR_DARK_GRAY);
    sketch_colors.push_back(SKETCH_COLOR_BLACK);

    sketch_shapes.push_back(SKETCH_SHAPE_0);
    sketch_shapes.push_back(SKETCH_SHAPE_1);
    sketch_shapes.push_back(SKETCH_SHAPE_2);
    sketch_shapes.push_back(SKETCH_SHAPE_3);
    sketch_shapes.push_back(SKETCH_SHAPE_4);

    sketch_actions_.generateSketchMode(sketch_modes);
    if (status_mgr_.isSketching())
    {
        sketch_actions_.setSketchMode(MODE_SKETCHING, true);
    }
    else if (status_mgr_.isErasing())
    {
        sketch_actions_.setSketchMode(MODE_ERASING, true);
    }

    if (!sketch_colors.empty())
    {
        sketch_actions_.generateSketchColors(sketch_colors, sketch_proxy_->getColor());
    }
    if (!sketch_shapes.empty())
    {
        sketch_actions_.generateSketchShapes(sketch_shapes, sketch_proxy_->getShape());
    }
    if (!status_mgr_.isSketching())
    {
        sketch_actions_.setSketchColor( INVALID_SKETCH_COLOR );
        sketch_actions_.setSketchShape( INVALID_SKETCH_SHAPE );
    }

    system_actions_.generateActions();
}

void NotesView::initView()
{
    // get view configurations from model
    loadViewOptions();
    if (status_mgr_.isSketching() || status_mgr_.isErasing())
    {
        attachSketchProxy();
    }
    need_init_view_ = false;
}

void NotesView::popupMenu()
{
    if ( onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW )
    {
        // stop fastest update mode to get better image quality.
        if ( current_waveform_ == onyx::screen::ScreenProxy::DW )
        {
            current_waveform_ = onyx::screen::ScreenProxy::GC;
        }
        onyx::screen::instance().setDefaultWaveform(current_waveform_);
    }

    ui::PopupMenu menu(this);
    updateActions();
    if ( !status_mgr_.isSlideShow() )
    {
        menu.addGroup(&zoom_setting_actions_);
        menu.addGroup(&sketch_actions_);
        menu.addGroup(&view_actions_);
    }
    menu.addGroup(&reading_tools_actions_);
    menu.setSystemAction(&system_actions_);
    if (menu.popup() != QDialog::Accepted)
    {
        QApplication::processEvents();
        return;
    }

    // To solve update issue. At first, we disabled the screen update
    // the Qt frame buffer is synchronised by using processEvents.
    // Finally, the screen update is enabled.
    onyx::screen::instance().enableUpdate(false);
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);

    QAction * group = menu.selectedCategory();
    bool disable_update = true;
    if (group == zoom_setting_actions_.category())
    {
        setZoomValue(zoom_setting_actions_.getSelectedZoomValue());

        // clear the background
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
    }
    else if (group == view_actions_.category())
    {
        ViewActionsType type = INVALID_VIEW_TYPE;
        int value = -1;

        type = view_actions_.getSelectedValue(value);
        switch (type)
        {
        case VIEW_ROTATION:
            rotate();
            break;
        case VIEW_PAGE_LAYOUT:
            switchLayout(static_cast<PageLayoutType>(value));
            disable_update = false;
            break;
        default:
            break;
        }
    }
    else if (group == reading_tools_actions_.category())
    {
        int tool = reading_tools_actions_.selectedTool();
        switch (tool)
        {
        case NOTES_BACKGROUND_SELECTION:
            {
                disable_update = openBackgroundThumbnailView();
            }
            break;
        case EXPORT_SKETCH_DATA:
            {
                notes_doc_manager_->exportImages(sketch_proxy_.get());
                updateSketchProxy();
                disable_update = false;
            }
            break;
        case SLIDE_SHOW:
            {
                if (status_mgr_.isSlideShow())
                {
                    stopSlideShow();
                }
                else
                {
                    startSlideShow();
                }
            }
            break;
        case SCROLL_PAGE:
            {
                status_mgr_.setStatus( ID_PAN, FUNC_SELECTED );
                disable_update = false;
            }
            break;
        case INSERT_NOTE:
            {
                insertPage(current_page_);
            }
            break;
        case REMOVE_NOTE:
            {
                removePage(current_page_);
            }
            break;
        case GOTO_PAGE:
            {
                emit popupJumpPageDialog();
            }
            break;
        case CLOCK_TOOL:
            {
                emit clockClicked();
            }
            break;
        default:
            break;
        }
    }
    else if (group == sketch_actions_.category())
    {
        int value = -1;
        bool checked = false;
        SketchActionsType type = sketch_actions_.getSelectedValue(value, checked);
        switch (type)
        {
        case SKETCH_MODE:
            setSketchMode(static_cast<SketchMode>(value), checked);
            break;
        case SKETCH_COLOR:
            setSketchColor(static_cast<SketchColor>(value));
            break;
        case SKETCH_SHAPE:
            setSketchShape(static_cast<SketchShape>(value));
            break;
        default:
            break;
        }
        disable_update = false;
    }
    else if (group == system_actions_.category())
    {
        SystemAction system_action = system_actions_.selected();
        switch (system_action)
        {
        case RETURN_TO_LIBRARY:
            returnToLibrary(true);
            break;
        case SCREEN_UPDATE_TYPE:
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, true);
            onyx::screen::instance().toggleWaveform();
            current_waveform_ = onyx::screen::instance().defaultWaveform();
            disable_update = false;
            break;
        case MUSIC:
            openMusicPlayer();
            break;
        case ROTATE_SCREEN:
            rotate();
            break;
        default:
            break;
        }
    }

    if (!disable_update)
    {
        emit requestUpdateParent(true);
    }
}

void NotesView::returnToLibrary(bool b)
{
    qApp->exit();
}

void NotesView::onNeedNotesThumbnail(const int image_idx, const QRect &rect)
{
    renderNotesThumbnail(image_idx, rect);
}

void NotesView::onClearNotesThumbnails()
{
    // TODO. Implement Me
}

void NotesView::onNotesThumbnailReturn(const int page_number)
{
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view != 0)
    {
        ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);

        // disconnect the signal-slot of notes thumbnail
        disconnect(thumbnail_view, SIGNAL(needThumbnail(const int, const QRect &)),
                   this, SLOT(onNeedNotesThumbnail(const int, const QRect &)));
        disconnect(thumbnail_view, SIGNAL(clearThumbnails()),
                   this, SLOT(onClearNotesThumbnails()));
        disconnect(thumbnail_view, SIGNAL(returnToReading(const int)),
                   this, SLOT(onNotesThumbnailReturn(const int)));
    }

    dynamic_cast<MainWindow*>(parentWidget())->activateView(NOTES_VIEW);
    int last_page = getLastPage();
    if (page_number >= 0 && page_number <= last_page)
    {
        jump(page_number);
    }
    else
    {
        emit currentPageChanged(current_page_, last_page + 1);
    }
    sketch_proxy_->setDrawingArea(this);
}

void NotesView::onNeedBackgroundThumbnail(const int image_idx, const QRect &rect)
{
    render_proxy_.renderThumbnail(image_idx, rect, model_);
}

void NotesView::onClearBackgroundThumbnails()
{
    model_->getThumbsMgr().clear();
}

void NotesView::onBackgroundThumbnailReturn(const int image_index)
{
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view != 0)
    {
        ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);

        // disconnect the signal-slot of background thumbnail
        disconnect(thumbnail_view, SIGNAL(needThumbnail(const int, const QRect &)),
                   this, SLOT(onNeedBackgroundThumbnail(const int, const QRect &)));
        disconnect(thumbnail_view, SIGNAL(clearThumbnails()),
                   this, SLOT(onClearBackgroundThumbnails()));
        disconnect(thumbnail_view, SIGNAL(returnToReading(const int)),
                   this, SLOT(onBackgroundThumbnailReturn(const int)));
    }

    dynamic_cast<MainWindow*>(parentWidget())->activateView(NOTES_VIEW);
    if (image_index >= 0)
    {
        ImageKey background;
        if (model_->getImageNameByIndex(image_index, background))
        {
            setPageBackgroundImage(current_page_, background);

            // update layout of current page
            onNeedPage(current_page_);
        }
    }
    else
    {
        emit currentPageChanged(current_page_, getLastPage() + 1);
    }
    sketch_proxy_->setDrawingArea(this);
}

void NotesView::onDefaultBackgroundThumbnailReturn(const int image_index)
{
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view != 0)
    {
        ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);

        // disconnect the signal-slot of background thumbnail
        disconnect(thumbnail_view, SIGNAL(needThumbnail(const int, const QRect &)),
                   this, SLOT(onNeedBackgroundThumbnail(const int, const QRect &)));
        disconnect(thumbnail_view, SIGNAL(clearThumbnails()),
                   this, SLOT(onClearBackgroundThumbnails()));
        disconnect(thumbnail_view, SIGNAL(returnToReading(const int)),
                   this, SLOT(onDefaultBackgroundThumbnailReturn(const int)));
    }

    dynamic_cast<MainWindow*>(parentWidget())->activateView(NOTES_VIEW);
    setSketchMode(MODE_SKETCHING, true);

    ImageKey background;
    if (image_index >= 0 && model_->getImageNameByIndex(image_index, background))
    {
        notes_doc_manager_->setDefaultBackground(background);
    }

    if (need_init_view_)
    {
        initView();
    }

    if (image_index >= 0 && !background.isEmpty())
    {
        setPageBackgroundImage(current_page_, background);
    }
    else
    {
        emit currentPageChanged(current_page_, getLastPage() + 1);
    }
    sketch_proxy_->setDrawingArea(this);
}

void NotesView::onNotesThumbnailReady(shared_ptr<BaseThumbnail> thumb, const int index, const QRect &bounding_rect)
{
    ThumbnailView *thumbnail_view = down_cast<ThumbnailView*>(
        down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW));
    if (thumbnail_view != 0)
    {
        thumbnail_view->setThumbnail(index, thumb, bounding_rect);
    }
}

void NotesView::onBackgroundThumbnailReady(shared_ptr<BaseThumbnail> thumb,
                                           const QRect &bounding_rect)
{
    ThumbnailView *thumbnail_view = down_cast<ThumbnailView*>(
        down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW));
    if (thumbnail_view != 0)
    {
        thumbnail_view->setThumbnail(thumb->key(), thumb, bounding_rect);
    }
}

void NotesView::openNotesThumbnailView()
{
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view == 0)
    {
        return;
    }

    ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);

    // connect the signal-slot of notes thumbnail
    connect(thumbnail_view, SIGNAL(needThumbnail(const int, const QRect &)),
            this, SLOT(onNeedNotesThumbnail(const int, const QRect &)));
    connect(thumbnail_view, SIGNAL(clearThumbnails()),
            this, SLOT(onClearNotesThumbnails()));
    connect(thumbnail_view, SIGNAL(returnToReading(const int)),
            this, SLOT(onNotesThumbnailReturn(const int)));

    thumbnail_view->attachNotesDocManager(notes_doc_manager_.get());
    thumbnail_view->attachSketchProxy(sketch_proxy_.get());
    thumbnail_view->setTotalNumber(getLastPage() + 1);
    down_cast<MainWindow*>(parentWidget())->activateView(THUMBNAIL_VIEW);
    thumbnail_view->setCurrentPage(layout_->getCurrentPage());
}

bool NotesView::openBackgroundThumbnailView()
{
    if (model_->imageCount() <= 0) return false;

    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view == 0)
    {
        return false;
    }

    ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);

    // connect the signal-slot of background thumbnail
    connect(thumbnail_view, SIGNAL(needThumbnail(const int, const QRect &)),
            this, SLOT(onNeedBackgroundThumbnail(const int, const QRect &)));
    connect(thumbnail_view, SIGNAL(clearThumbnails()),
            this, SLOT(onClearBackgroundThumbnails()));
    connect(thumbnail_view, SIGNAL(returnToReading(const int)),
            this, SLOT(onBackgroundThumbnailReturn(const int)));

    thumbnail_view->deattachNotesDocManager();
    thumbnail_view->setTotalNumber(model_->imageCount());
    down_cast<MainWindow*>(parentWidget())->activateView(THUMBNAIL_VIEW);

    int current_page = layout_->getCurrentPage();
    QString background_image = getBackgroundImage(current_page);
    int background_id = model_->getIndexByImageName(background_image);
    if (background_id < 0)
    {
        // the background image might be EMPTY_BACKGROUND
        background_id = 0;
    }
    thumbnail_view->setCurrentPage(background_id);
    return true;
}

bool NotesView::openDefaultBackgroundThumbnailView()
{
    if (model_->imageCount() <= 0) return false;

    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view == 0)
    {
        return false;
    }

    ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);

    // connect the signal-slot of background thumbnail
    connect(thumbnail_view, SIGNAL(needThumbnail(const int, const QRect &)),
            this, SLOT(onNeedBackgroundThumbnail(const int, const QRect &)));
    connect(thumbnail_view, SIGNAL(clearThumbnails()),
            this, SLOT(onClearBackgroundThumbnails()));
    connect(thumbnail_view, SIGNAL(returnToReading(const int)),
            this, SLOT(onDefaultBackgroundThumbnailReturn(const int)));

    thumbnail_view->deattachNotesDocManager();
    thumbnail_view->setTotalNumber(model_->imageCount());
    down_cast<MainWindow*>(parentWidget())->activateView(THUMBNAIL_VIEW);
    thumbnail_view->setCurrentPage(0);
    return true;
}

void NotesView::setSketchMode(const SketchMode mode, bool selected)
{
    FunctionStatus s = selected ? FUNC_SELECTED : FUNC_NORMAL;
    FunctionID id = mode == MODE_SKETCHING ? ID_SKETCHING : ID_ERASING;
    status_mgr_.setStatus(id, s);
    if (sketch_proxy_ != 0)
    {
        sketch_proxy_->setMode(mode);
    }
}

void NotesView::setSketchShape(const SketchShape shape)
{
    sketch_proxy_->setShape(shape);
    status_mgr_.setStatus(ID_SKETCHING, FUNC_SELECTED);
}

void NotesView::setSketchColor(const SketchColor color)
{
    sketch_proxy_->setColor(color);
    status_mgr_.setStatus(ID_SKETCHING, FUNC_SELECTED);
}

/// clear the visible pages
void NotesView::clearVisiblePages()
{
    // NOTE: do NOT access all of the displaying images
    display_pages_.clear();
}

void NotesView::initLayout()
{
    if (read_mode_ == CONTINUOUS_LAYOUT)
    {
        layout_.reset(new ContinuousPageLayout(view_setting_.rotate_orient, view_setting_.zoom_setting));
    }
    else
    {
        layout_.reset(new SinglePageLayout(view_setting_.rotate_orient, view_setting_.zoom_setting));
    }

    connect(layout_.get(), SIGNAL(layoutDoneSignal()), this, SLOT(onLayoutDone()));
    connect(layout_.get(), SIGNAL(needPageSignal(const int)), this, SLOT(onNeedPage(const int)));

    layout_->setMargins(cur_margin_);
    layout_->setWidgetArea(QRect(0, 0, size().width(), size().height()));
}

void NotesView::attachModel(BaseModel * model)
{
    if (model_ == model)
    {
        return;
    }

    // Record the model.
    model_ = down_cast<ImageModel*>(model);
    notes_doc_manager_->attachContentManager(model_->contentManager());

    // connect the signals
    connect(model_, SIGNAL(modelReadySignal(const int)), this, SLOT(onModelReady(const int)));
    connect(model_, SIGNAL(modelClosingSignal()), this, SLOT(onModelClosing()));
    connect(model_, SIGNAL(requestSaveAllOptions()), this, SLOT(saveViewOptions()));
    connect(model_, SIGNAL(renderingThumbnailReadySignal(shared_ptr<BaseThumbnail>, const QRect&)),
            this, SLOT(onBackgroundThumbnailReady(shared_ptr<BaseThumbnail>, const QRect&)));
}

void NotesView::deattachModel()
{
    disconnect(model_, SIGNAL(modelReadySignal(const int)), this, SLOT(onModelReady(const int)));
    disconnect(model_, SIGNAL(modelClosingSignal()), this, SLOT(onModelClosing()));
    disconnect(model_, SIGNAL(requestSaveAllOptions()), this, SLOT(saveViewOptions()));
    disconnect(model_, SIGNAL(renderingThumbnailReadySignal(shared_ptr<BaseThumbnail>, const QRect&)),
               this, SLOT(onBackgroundThumbnailReady(shared_ptr<BaseThumbnail>, const QRect&)));
}

void NotesView::attachMainWindow(MainWindow *main_window)
{
    connect(this, SIGNAL(currentPageChanged(const int, const int)),
            main_window, SLOT(handlePositionChanged(const int, const int)));
    connect(this, SIGNAL(needFullScreen(bool)),
            main_window, SLOT(handleFullScreen(bool)));
    connect(this, SIGNAL(itemStatusChanged(const StatusBarItemType, const int)),
            main_window, SLOT(handleItemStatusChanged(const StatusBarItemType, const int)));
    connect(this, SIGNAL(requestUpdateParent(bool)),
            main_window, SLOT(handleRequestUpdate(bool)));
    connect(this, SIGNAL(popupJumpPageDialog()),
            main_window, SLOT(handlePopupJumpPageDialog()));
    connect(this, SIGNAL(clockClicked()),
            main_window, SLOT(handleClockClicked()));

    connect(main_window, SIGNAL(pagebarClicked(const int, const int)),
            this, SLOT(onPagebarClicked(const int, const int)));
    connect(main_window, SIGNAL(popupContextMenu()),
            this, SLOT(onPopupContextMenu()));
    setSketchMode(MODE_SKETCHING, true);
}

void NotesView::deattachMainWindow(MainWindow *main_window)
{
    disconnect(this, SIGNAL(currentPageChanged(const int, const int)),
               main_window, SLOT(handlePositionChanged(const int, const int)));
    disconnect(this, SIGNAL(needFullScreen(bool)),
               main_window, SLOT(handleFullScreen(bool)));
    disconnect(this, SIGNAL(itemStatusChanged(const StatusBarItemType, const int)),
               main_window, SLOT(handleItemStatusChanged(const StatusBarItemType, const int)));
    disconnect(this, SIGNAL(requestUpdateParent(bool)),
               main_window, SLOT(handleRequestUpdate(bool)));
    disconnect(this, SIGNAL(popupJumpPageDialog()),
               main_window, SLOT(handlePopupJumpPageDialog()));
    disconnect(this, SIGNAL(clockClicked()),
               main_window, SLOT(handleClockClicked()));

    disconnect(main_window, SIGNAL(pagebarClicked(const int, const int)),
               this, SLOT(onPagebarClicked(const int, const int)));
    disconnect(main_window, SIGNAL(popupContextMenu()),
               this, SLOT(onPopupContextMenu()));
}

void NotesView::setSketchProxy(shared_ptr<SketchProxy> proxy)
{
    sketch_proxy_ = proxy;
    connect(sketch_proxy_.get(), SIGNAL(requestUpdateScreen()),
            this, SLOT(onRequestUpdateScreen()));
    sketch_proxy_->setWidgetOrient(getSystemRotateDegree());
    sketch_proxy_->setDrawingArea(this);
    attachSketchProxy();
}

void NotesView::setNotesManager(shared_ptr<NotesDocumentManager> doc_mgr)
{
    notes_doc_manager_ = doc_mgr;
}

void NotesView::gotoPage(const int page_number)
{
    jump(page_number);
}

void NotesView::nextPage()
{
    int current_page = current_page_;
    int next_page = current_page + 1;
    if (next_page >= sketch_proxy_->getPageCount(notes_doc_manager_->notesDocumentPath()))
    {
        // insert a new page
        insertPage(next_page);
    }
    else
    {
        jump(next_page);
    }
}

void NotesView::prevPage()
{
    int current_page = current_page_;
    int prev_page = current_page - 1;
    if (prev_page >= 0)
    {
        jump(prev_page);
    }
}

void NotesView::paintEvent(QPaintEvent *pe)
{
    int count = display_pages_.size();
    QPainter painter(this);
    for (int i = 0; i < count; ++i)
    {
        shared_ptr<NotesPage> cur_page = display_pages_.getPage(i);
        paintPage(painter, *cur_page);
    }
}

void NotesView::paintPage(QPainter &painter, NotesPage & page)
{
    shared_ptr<ImageItem> image = page.image();
    if (image->image() == 0 || image->renderStatus() != IMAGE_STATUS_DONE)
    {
        return;
    }

    vbf::PagePtr page_layout = layout_->getPage(page.index());
    if (page_layout == 0)
    {
        qDebug("The layout is not ready!");
        return;
    }

    QPoint cur_pos;
    if (layout_->getContentPos(page.index(), cur_pos))
    {
        // draw content of page
        painter.drawImage(cur_pos, *(image->image()));
    }
    paintSketches(painter, page);
}

void NotesView::paintSketches(QPainter & painter, NotesPage & page)
{
    // page position
    QPoint page_pos;
    if (!layout_->getContentPos(page.index(), page_pos))
    {
        return;
    }

    // update zoom factor
    vbf::PagePtr page_layout = layout_->getPage(page.index());
    QRect page_area(page_pos, page_layout->displayArea().size());

    sketch_proxy_->setZoom(page_layout->zoomValue() * ZOOM_ACTUAL);
    sketch_proxy_->setContentOrient(layout_->rotateDegree());
    sketch_proxy_->setWidgetOrient(getSystemRotateDegree());

    // draw sketches in this page
    sketch::PageKey page_key;
    page_key.setNum(page.index());
    sketch_proxy_->updatePageDisplayRegion(notes_doc_manager_->notesDocumentPath(), page_key, page_area);
    sketch_proxy_->paintPage(notes_doc_manager_->notesDocumentPath(), page_key, painter);
}

/// sender render request for one page
void NotesView::render(vbf::PagePtr page)
{
    RenderSetting setting;
    setting.setContentArea(page->displayArea());
    setting.setRotation(view_setting_.rotate_orient);

    // load image key from sketch proxy
    sketch::PageKey page_key;
    page_key.setNum(page->key());
    ImageKey image_key = getBackgroundImage(page->key());

    // load the sketch data of this page
    sketch_proxy_->loadPage(notes_doc_manager_->notesDocumentPath(), page_key, image_key);
    shared_ptr<ImageItem> image = model_->getImage(image_key);
    if (image == 0)
    {
        image = notes_doc_manager_->emptyBackgroundImage();
    }
    ImageStatus status = image->render(setting, model_);
    onImageReady(image, page->key(), status, true);
}

void NotesView::renderNotesThumbnail(const int image_idx, const QRect &rect)
{
    assert(model_ != 0);

    // get the background id of image
    sketch::PageKey page_key;
    page_key.setNum(image_idx);
    QString background = getBackgroundImage(image_idx);

    // get the image by name
    shared_ptr<ImageItem> image(model_->getImage(background));
    if (image == 0)
    {
        image = notes_doc_manager_->emptyBackgroundImage();
    }

    // calculate the display area of the thumbnail image
    QRect thumb_rect;
    getThumbnailRectangle(rect, image->actualSize(), &thumb_rect);

    // retrieve the thumbnail image, if it has been rendered, just return it
    shared_ptr<ImageThumbnail> thumb(new ImageThumbnail(image->name()));
    thumb->setKey(image->index());
    ImageStatus status = image->renderThumbnail(rect, thumb_rect, thumb, model_);
    if (status == IMAGE_STATUS_DONE)
    {
        thumb->updateDisplayArea(thumb_rect);
        onNotesThumbnailReady(thumb, image_idx, rect);
    }
}

void NotesView::onImageReady(shared_ptr<ImageItem> image,
                             const int index,
                             ImageStatus status,
                             bool update_screen)
{
    if ( update_screen )
    {
#ifdef DISPLAY_SYSTEM_BUSY
        need_set_rendering_busy_ = false;
        if ( sys::SysStatus::instance().isSystemBusy() )
        {
            // if it is the first time rendering, set busy to be false
            sys::SysStatus::instance().setSystemBusy(false);
        }
#endif

        if (status != IMAGE_STATUS_DONE)
        {
            qDebug("Failed on rendering image!");
            return;
        }

        // remove the mapping page in layout pages
        bool found = false;
        VisiblePagesIter begin = layout_pages_.begin();
        VisiblePagesIter end = layout_pages_.end();
        VisiblePagesIter idx = begin;
        for (; idx != end; ++idx)
        {
            if (index == (*idx)->key() &&
                image->renderSetting().contentArea().size() == (*idx)->displayArea().size())
            {
                layout_pages_.erase(idx);
                found = true;
                break;
            }
        }
        if (!found)
        {
            qDebug("Image is out of date");
            return;
        }

        // set the waveform by current paging mode
        if (display_pages_.size() > 0)
        {
            onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
        }
        else
        {
            onyx::screen::instance().setDefaultWaveform(current_waveform_);
        }

        // add to displaying pages
        display_pages_.add(shared_ptr<NotesPage>(new NotesPage(image, index)));

        // retrieve the next one and send render request
        if (!layout_pages_.isEmpty())
        {
            // retrieve the list of prerender images
            vbf::PagePtr next_page = layout_pages_.front();
            int prev_page_number = index;
            int next_page_number = next_page->key();
            render(next_page);
        }
        else
        {
            int cur_page = layout_->getCurrentPage();
            emit currentPageChanged(cur_page, getLastPage() + 1);
        }

        // update to paint the page
        update();

        // rollback to current default mode after screen update
        if (layout_pages_.isEmpty())
        {
            onyx::screen::instance().setDefaultWaveform(current_waveform_);
        }
    }
}

/// get path of default background image
QString NotesView::getDefaultBackgroundImage(int dst_page)
{
    // TODO. get path of default background image
    // if the neighbor page has background image, use it
    // otherwise retrieve the image from notes document manager.
    int next_page = dst_page + 1;
    int prev_page = dst_page - 1;

    // load image key from sketch proxy
    ImageKey image_key;
    if (prev_page >= 0)
    {
        sketch::PageKey prev_page_key;
        prev_page_key.setNum(prev_page);
        image_key = sketch_proxy_->getBackgroundImage(notes_doc_manager_->notesDocumentPath(), prev_page_key);
    }

    if (image_key.isEmpty())
    {
        sketch::PageKey next_page_key;
        next_page_key.setNum(next_page);
        image_key = sketch_proxy_->getBackgroundImage(notes_doc_manager_->notesDocumentPath(), next_page_key);
    }

    if (image_key.isEmpty())
    {
        image_key = notes_doc_manager_->defaultBackground();
    }
    return image_key;
}

QString NotesView::getBackgroundImage(int dst_page)
{
    QString page_str;
    page_str.setNum(dst_page);
    QString background_image = sketch_proxy_->getBackgroundImage(notes_doc_manager_->notesDocumentPath(), page_str);
    if (!notes_doc_manager_->isBackgroundExisting(background_image))
    {
        background_image = notes_doc_manager_->defaultBackground();

        // set the background image
        sketch_proxy_->updateBackgroundImage(notes_doc_manager_->notesDocumentPath(), page_str, background_image);
    }
    return background_image;
}

void NotesView::onLayoutDone()
{
    // clear the previous visible pages
    clearVisiblePages();
    layout_->getVisiblePages(layout_pages_);
    if (layout_pages_.empty())
    {
        return;
    }

    if (status_mgr_.isErasing() || status_mgr_.isSketching())
    {
        updateSketchProxy();
    }

    // update previous page
    int previous_page = current_page_;

    // update current page
    current_page_ = layout_->getCurrentPage();

    // save the sketch if current page changes
    if (current_page_ != previous_page)
    {
        sketch_proxy_->save();
    }

    // send the render request for first page
    vbf::PagePtr page = layout_pages_.front();
    render(page);

#ifdef DISPLAY_SYSTEM_BUSY
    // the waiting time should be adjust in device
    need_set_rendering_busy_ = true;
    QTimer::singleShot(100, this, SLOT(onRenderBusy()));
#endif
}

void NotesView::onNeedPage(const int page_number)
{
    int width = 0, height = 0;
    QString background_image = getBackgroundImage(page_number);
    if (background_image == notes_doc_manager_->emptyBackground())
    {
        width  = EMPTY_BACKGROUND_WIDTH;
        height = EMPTY_BACKGROUND_HEIGHT;
    }
    else
    {
        int image_id = model_->getIndexByImageName(background_image);
        if (!model_->getImageActualSize(image_id, width, height))
        {
            return;
        }
    }

    QRect rect;
    rect.setWidth(width);
    rect.setHeight(height);
    layout_->setPage(page_number, rect);
}

#ifdef DISPLAY_SYSTEM_BUSY
void NotesView::onRenderBusy()
{
    if (need_set_rendering_busy_ && display_pages_.size() == 0)
    {
        sys::SysStatus::instance().setSystemBusy(true);
    }
}
#endif

void NotesView::mousePressEvent(QMouseEvent *me)
{
    switch (me->button())
    {
    case Qt::LeftButton:
        if (status_mgr_.isZoomIn())
        {
            current_waveform_ = onyx::screen::instance().defaultWaveform();
            onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
            stroke_area_.initArea(me->pos());
            if (rubber_band_ == 0)
            {
                rubber_band_.reset(new QRubberBand(QRubberBand::Rectangle, this));
            }
            rubber_band_->setGeometry(QRect(stroke_area_.getOriginPosition(), QSize()));
            rubber_band_->show();
        }
        else if(status_mgr_.isPan())
        {
            pan_area_.setStartPoint(me->pos());
        }
        else if (status_mgr_.isSketching())
        {
            // the mouse events has been eaten by sketch proxy
        }
        else if (status_mgr_.isErasing())
        {
            // the mouse events has been eaten by sketch proxy
        }
        break;
    case Qt::RightButton:
        {
            popupMenu();
        }
        break;
    default:
        break;
    }
    me->accept();
}

void NotesView::mouseReleaseEvent(QMouseEvent *me)
{
#define MOVE_ERROR 5
    switch (me->button())
    {
    case Qt::LeftButton:
        if (status_mgr_.isZoomIn())
        {
            stroke_area_.expandArea(me->pos());
            rubber_band_->hide();

            // clear the background
            onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
            onyx::screen::instance().setDefaultWaveform(current_waveform_);
            zoomIn(stroke_area_.getRect());
            status_mgr_.setStatus(ID_ZOOM_IN, FUNC_NORMAL);
        }
        else if (status_mgr_.isPan())
        {
            pan_area_.setEndPoint(me->pos());
            int offset_x = 0, offset_y = 0;
            pan_area_.getOffset(offset_x, offset_y);
            // MUST judge whether clicking hyperlink
            if (abs(offset_x) < MOVE_ERROR && abs(offset_y) < MOVE_ERROR)
            {
                hitTest(me->pos());
            }
            else
            {
                pan(offset_x, offset_y);
            }
        }
        else if (status_mgr_.isSketching())
        {
            // the mouse events has been eaten by sketch proxy
        }
        else if (status_mgr_.isErasing())
        {
            // the mouse events has been eaten by sketch proxy
        }
        else if (status_mgr_.isSlideShow())
        {
            stopSlideShow();
        }

        break;
    case Qt::RightButton:
        {
        }
        break;
    default:
        break;
    }
    me->accept();
}

void NotesView::mouseMoveEvent(QMouseEvent *me)
{
    if (status_mgr_.isZoomIn())
    {
        stroke_area_.expandArea(me->pos());
        rubber_band_->setGeometry(QRect(stroke_area_.getOriginPosition(),
                                        me->pos()).normalized());
    }
    else if (status_mgr_.isSketching())
    {
        // the mouse events has been eaten by sketch proxy
    }
    else if (status_mgr_.isErasing())
    {
        // the mouse events has been eaten by sketch proxy
    }
    me->accept();
}

void NotesView::keyReleaseEvent(QKeyEvent *ke)
{
    int offset = 0;
    switch(ke->key())
    {
    case Qt::Key_PageDown:
        {
            nextPage();
        }
        break;
    case Qt::Key_Down:
        {
            offset = height();
            if (isLandscape())
            {
                offset = width();
            }
            scroll(0, offset);
        }
        break;
    case Qt::Key_Right:
        {
            offset = width();
            if (isLandscape())
            {
                offset = height();
            }
            scroll(offset, 0);
        }
        break;
    case Qt::Key_PageUp:
        {
            prevPage();
        }
        break;
    case Qt::Key_Up:
        {
            offset = -height();
            if (isLandscape())
            {
                offset = -width();
            }
            scroll(0, offset);
        }
        break;
    case Qt::Key_Left:
        {
            offset = -width();
            if (isLandscape())
            {
                offset = -height();
            }
            scroll(offset, 0);
        }
        break;
    case Qt::Key_Z:
        {
            selectionZoom();
        }
        break;
    case Qt::Key_B:
        {
            zoomToBestFit();
        }
        break;
    case Qt::Key_P:
        {
            setPan();
        }
        break;
    case Qt::Key_W:
        {
            zoomToWidth();
        }
        break;
    case Qt::Key_H:
        {
            zoomToHeight();
        }
        break;
    case Qt::Key_S:
        {
            if (!slide_timer_.isActive())
            {
                startSlideShow();
            }
            else
            {
                stopSlideShow();
            }
        }
        break;
    case Qt::Key_E:
        {
            notes_doc_manager_->exportImages(sketch_proxy_.get());
            updateSketchProxy();
        }
        break;
    case Qt::Key_F3:
        {
        }
        break;
    case Qt::Key_F4:
        {
        }
        break;
    case Qt::Key_Escape:
        {
            if ( status_mgr_.isSlideShow() )
            {
                stopSlideShow();
            }
            else
            {
                returnToLibrary(true);
            }
        }
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        {
            emit popupJumpPageDialog();
        }
        break;
    case Qt::Key_Delete:
        {
            removePage(current_page_);
        }
        break;
    case Qt::Key_I:
        {
            insertPage(current_page_);
        }
        break;
    case ui::Device_Menu_Key:
        {
            popupMenu();
        }
        break;
    default:
        break;
    }
    ke->accept();
}

void NotesView::resizeEvent(QResizeEvent *re)
{
    if (layout_ != 0 &&
        layout_->setWidgetArea(QRect(0,
                                     0,
                                     re->size().width(),
                                     re->size().height())))
    {
        layout_->update();
    }
}

#ifndef QT_NO_WHEELEVENT
void NotesView::wheelEvent(QWheelEvent *we)
{
    int offset = we->delta();
    scroll(0, -offset);
}
#endif

void NotesView::openMusicPlayer()
{
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
    sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
}

bool NotesView::hitTest(const QPoint &point)
{
    // Hit test whether clicking the content area of image
    return false;
}

void NotesView::scroll(int offset_x, int offset_y)
{
    if ( status_mgr_.isSlideShow() )
    {
        return;
    }

    int x = offset_x;
    int y = offset_y;
    if (isLandscape())
    {
        x = offset_y;
        y = offset_x;
    }
    layout_->scroll(x, y);
}

void NotesView::zoomIn(const QRect &zoom_rect)
{
    layout_->zoomIn(zoom_rect);
    view_setting_.zoom_setting = layout_->zoomSetting();
}

void NotesView::zoomToBestFit()
{
    layout_->zoomToBestFit();
    view_setting_.zoom_setting = ZOOM_TO_PAGE;
}

void NotesView::zoomToWidth()
{
    layout_->zoomToWidth();
    view_setting_.zoom_setting = ZOOM_TO_WIDTH;
}

void NotesView::zoomToHeight()
{
    layout_->zoomToHeight();
    view_setting_.zoom_setting = ZOOM_TO_HEIGHT;
}

void NotesView::selectionZoom()
{
    status_mgr_.setStatus(ID_ZOOM_IN, FUNC_SELECTED);
}

void NotesView::setPan()
{
    status_mgr_.setStatus(ID_PAN, FUNC_SELECTED);
}

void NotesView::setZoomValue(float value)
{
    if (value == ZOOM_TO_HEIGHT)
    {
        zoomToHeight();
    }
    else if (value == ZOOM_TO_WIDTH)
    {
        zoomToWidth();
    }
    else if (value == ZOOM_TO_PAGE)
    {
        zoomToBestFit();
    }
    else if (value == ZOOM_SELECTION)
    {
        selectionZoom();
    }
    else
    {
        layout_->setZoom(value);
        view_setting_.zoom_setting = value;
    }
}

void NotesView::pan(int offset_x, int offset_y)
{
    layout_->pan(offset_x, offset_y);
}

void NotesView::jump(int page_number)
{
    layout_->jump(page_number);
}

void NotesView::rotate()
{
    emit rotateScreen();
    RotateDegree degree = getSystemRotateDegree();
    sketch_proxy_->setWidgetOrient(degree);
}

void NotesView::startSlideShow()
{
    status_mgr_.setStatus(ID_SLIDE_SHOW, FUNC_SELECTED);

    // reset the reading layout and zoom
    zoomToBestFit();
    switchLayout(PAGE_LAYOUT);
    slide_timer_.setSingleShot(true);
    slide_timer_.start(SLIDE_TIME_INTERVAL);

    // enter full screen mode
    emit needFullScreen(true);
}

void NotesView::stopSlideShow()
{
    status_mgr_.setStatus(ID_SLIDE_SHOW, FUNC_NORMAL);

    // stop the slide timer
    slide_timer_.stop();

    // exit full screen mode
    emit needFullScreen(false);
}

void NotesView::slideShowNextImage()
{
    int current = current_page_;
    int total   = sketch_proxy_->getPageCount(notes_doc_manager_->notesDocumentPath());
    if ((++current) >= total)
    {
        current = 0;
    }
    jump(current);
}

void NotesView::onRequestUpdateScreen()
{
    onyx::screen::instance().enableUpdate(false);
    repaint();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
}


void NotesView::onPagebarClicked(const int percent, const int value)
{
    gotoPage(value);
}

void NotesView::onPopupContextMenu()
{
    popupMenu();
}

void NotesView::onModelReady(const int init_page_num)
{
    // if it is the first time rendering, set busy to be false
    sys::SysStatus::instance().setSystemBusy(false);
    need_init_view_ = true;
    if (!notes_doc_manager_->needSelectBackground() || !openDefaultBackgroundThumbnailView())
    {
        initView();
    }
}

void NotesView::saveViewOptions()
{
    vbf::Configuration & conf = notes_doc_manager_->conf();
    conf.options[CONFIG_PAGE_LAYOUT] = read_mode_;
    conf.options[CONFIG_SKETCH_COLOR] = sketch_proxy_->getColor();
    conf.options[CONFIG_SKETCH_SHAPE] = sketch_proxy_->getShape();

    layout_->saveConfiguration(conf);
    notes_doc_manager_->saveConf();
    notes_doc_manager_->saveNoteIndex(sketch_proxy_.get());
    sketch_proxy_->save();
    updateSketchProxy();
}

void NotesView::onWakeUp()
{
    // update the screen to make sure the sketches are drawable.
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
}

void NotesView::loadViewOptions()
{
    // load the configuration from model
    Configuration & conf = notes_doc_manager_->conf();
    bool ok = false;
    read_mode_ = static_cast<PageLayoutType>(conf.options[CONFIG_PAGE_LAYOUT].toInt(&ok));
    if (!ok)
    {
        read_mode_ = PAGE_LAYOUT;
    }

    SketchColor color = static_cast<SketchColor>(conf.options[CONFIG_SKETCH_COLOR].toInt(&ok));
    if (ok)
    {
        setSketchColor(color);
    }

    SketchShape shape = static_cast<SketchShape>(conf.options[CONFIG_SKETCH_SHAPE].toInt(&ok));
    if (ok)
    {
        setSketchShape(shape);
    }

    // create empty pages layout by current reading mode
    initLayout();

    // initialize the pages layout by configurations.
    // If the configurations are invalid, the layout is initialized by default.
    layout_->loadConfiguration(conf);
    int page_key = conf.options[CONFIG_PAGE_NUMBER].toInt(&ok);
    if (!ok)
    {
        page_key = 0;
    }
    layout_->setLastPageNumber(page_key);

    resetLayout();
}

void NotesView::onModelClosing()
{
    if (layout_ != 0)
    {
        // clear the cached pages in pages layout
        layout_->clearPages();
    }

    // clear the visible pages
    clearVisiblePages();
}

void NotesView::resetLayout()
{
    // NOTE: The document should be ready when calling this function
    layout_->clearPages();
    layout_->setFirstPageNumber(0);

    int last_page_num = getLastPage();
    if (last_page_num > 0)
    {
        layout_->setLastPageNumber(last_page_num);
    }
    else
    {
        layout_->setLastPageNumber(0);
    }
    layout_->update();
}

void NotesView::switchLayout(PageLayoutType mode)
{
    if (read_mode_ == mode)
    {
        return;
    }
    
    if (mode == THUMBNAIL_LAYOUT)
    {
        openNotesThumbnailView();
    }
    else
    {
        read_mode_ = mode;
        initLayout();
        layout_->setWidgetArea(QRect(0,
                                     0,
                                     size().width(),
                                     size().height()));
        jump(current_page_);
        resetLayout();
    }
}

void NotesView::updateSketchProxy()
{
    // deactivate all pages
    sketch_proxy_->deactivateAll();

    // activate visible pages
    if (layout_ != 0)
    {
        vbf::VisiblePages visible_pages;
        layout_->getVisiblePages(visible_pages);
        VisiblePagesIter idx = visible_pages.begin();
        while (idx != visible_pages.end())
        {
            vbf::PagePtr page_layout = *idx;
            int page_number = page_layout->key();
            QPoint page_pos;
            if (layout_->getContentPos(page_number, page_pos))
            {
                QRect page_area;
                page_area.setTopLeft(page_pos);
                page_area.setSize(page_layout->displayArea().size());

                sketch_proxy_->setZoom(page_layout->zoomValue() * ZOOM_ACTUAL);
                sketch_proxy_->setContentOrient(layout_->rotateDegree());
                sketch_proxy_->setWidgetOrient(getSystemRotateDegree());

                ImageKey image_key;
                sketch::PageKey page_key;
                page_key.setNum(page_number);
                sketch_proxy_->activatePage(notes_doc_manager_->notesDocumentPath(), page_key);
                sketch_proxy_->updatePageDisplayRegion(notes_doc_manager_->notesDocumentPath(), page_key, page_area);
            }
            idx++;
        }
    }
}

void NotesView::onStylusChanges(const int type)
{
    switch (type)
    {
    case ID_SKETCHING:
    case ID_ERASING:
        attachSketchProxy();
        break;
    default:
        deattachSketchProxy();
        break;
    }
    emit itemStatusChanged(STYLUS, type);
}

void NotesView::attachSketchProxy()
{
    if (sketch_proxy_ == 0) return;

    if (status_mgr_.isErasing())
    {
        sketch_proxy_->setMode(MODE_ERASING);
    }
    else if (status_mgr_.isSketching())
    {
        sketch_proxy_->setMode(MODE_SKETCHING);
    }
    sketch_proxy_->attachWidget(this);
    updateSketchProxy();
}

void NotesView::deattachSketchProxy()
{
    if (sketch_proxy_ == 0) return;
    sketch_proxy_->deattachWidget(this);
}

int NotesView::getLastPage()
{
    sketch::PageKey last_page = sketch_proxy_->getLastPage(notes_doc_manager_->notesDocumentPath());
    int last_page_num = layout_->getLastPageNumber();
    if (!last_page.isEmpty())
    {
        int last_sketch_page = last_page.toInt();
        if (last_sketch_page > last_page_num)
        {
            last_page_num = last_sketch_page;
        }
    }
    return last_page_num;
}

void NotesView::insertPage(int dst_page)
{
    // insert a new page in sketch proxy
    QString background = getDefaultBackgroundImage(dst_page);
    sketch::PageKey dst_page_key;
    dst_page_key.setNum(dst_page);
    if (sketch_proxy_->insertPage(notes_doc_manager_->notesDocumentPath(), dst_page_key, background))
    {
        // update the page count of layout
        int last_page_num = getLastPage();
        if (last_page_num > 0)
        {
            layout_->setLastPageNumber(last_page_num);
        }
        else
        {
            layout_->setLastPageNumber(1);
        }
        jump(dst_page);
    }
}

void NotesView::removePage(int dst_page)
{
    sketch::PageKey dst_page_key;
    dst_page_key.setNum(dst_page);
    if (sketch_proxy_->removePage(notes_doc_manager_->notesDocumentPath(), dst_page_key))
    {
        // update the page count of layout
        int last_page_num = getLastPage();
        if (last_page_num > 1)
        {
            layout_->setLastPageNumber(last_page_num - 1);
        }
        else
        {
            layout_->setLastPageNumber(1);
        }

        if (last_page_num > 0 && dst_page > 0)
        {
            jump(dst_page - 1);
        }
        else
        {
            jump(0);
        }
        sketch_proxy_->save();
    }
}

void NotesView::setPageBackgroundImage(const int page_number, const QString & background_image)
{
    sketch::PageKey page_key;
    page_key.setNum(page_number);
    sketch_proxy_->updateBackgroundImage(notes_doc_manager_->notesDocumentPath(), page_key, background_image);
}

}
