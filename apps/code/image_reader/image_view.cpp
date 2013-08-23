#ifdef BUILD_FOR_ARM
#include <QtGui/qwsdisplay_qws.h>
#include <QtGui/qscreen_qws.h>
#endif

#include "image_application.h"
#include "image_view.h"
#include "image_item.h"
#include "image_global_settings.h"

namespace image
{

static const int OVERLAP_DISTANCE = 80;

RotateDegree getSystemRotateDegree()
{
    int degree = 0;
#ifdef BUILD_FOR_ARM
    degree = QScreen::instance()->transformOrientation();
#endif
    return static_cast<RotateDegree>(degree);
}

DisplayImages::DisplayImages()
{
}

DisplayImages::~DisplayImages()
{
}

void DisplayImages::add(shared_ptr<ImageItem> p)
{
    p->lock();
    int idx = images_.indexOf(p);
    if (idx < 0)
    {
        images_.push_back(p);
    }
    else
    {
        images_[idx] = p;
    }
}

void DisplayImages::clear()
{
    for (ImagesIter it = images_.begin(); it != images_.end(); ++it)
    {
        (*it)->unlock();
    }
    images_.clear();
}

size_t DisplayImages::size()
{
    return images_.size();
}

shared_ptr<ImageItem> DisplayImages::getImage(int num)
{
    return images_[num];
}

ImageView::ImageView(QWidget *parent)
    : BaseView(parent)//, Qt::FramelessWindowHint)
    , model_(0)
    , read_mode_(PAGE_LAYOUT)
    , layout_(0)
    , cur_image_index_(0)
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
    connect(&sketch_proxy_, SIGNAL(requestUpdateScreen()), this, SLOT(onRequestUpdateScreen()));

    // set drawing area to sketch agent
    sketch_proxy_.setDrawingArea(this);
    sketch_proxy_.setWidgetOrient(getSystemRotateDegree());
    initViewSetting();
}

ImageView::~ImageView(void)
{
}

void ImageView::updateActions()
{
    // Reading Tools
    std::vector<ReadingToolsType> reading_tools;
    reading_tools_actions_.generateActions(reading_tools,false);
    if ( !status_mgr_.isSlideShow() )
    {
        if (SysStatus::instance().hasTouchScreen())
        {
            reading_tools.push_back( SCROLL_PAGE );
        }
        if (sys::SystemConfig::isUpdateSplashEnabled())
        {
            reading_tools.push_back(SET_TOBE_BOOTSPLASH);
        }
        reading_tools.push_back(GOTO_PAGE);
        reading_tools_actions_.generateActions(reading_tools);
    }

    reading_tools.clear();
    reading_tools.push_back( SLIDE_SHOW );
    reading_tools.push_back( CLOCK_TOOL );
    reading_tools_actions_.setActionStatus(SLIDE_SHOW,
                                           status_mgr_.isSlideShow());
    reading_tools_actions_.generateActions(reading_tools, true);

    // Zoom Settings
    std::vector<ZoomFactor> zoom_settings;
    zoom_settings.push_back(ZOOM_TO_PAGE);
    zoom_settings.push_back(ZOOM_TO_WIDTH);
    zoom_settings.push_back(ZOOM_TO_HEIGHT);
    if (SysStatus::instance().hasTouchScreen())
    {
        zoom_settings.push_back(ZOOM_SELECTION);
    }
    zoom_settings.push_back(75.0f);
    zoom_settings.push_back(100.0f);
    zoom_settings.push_back(125.0f);
    zoom_settings.push_back(150.0f);
    zoom_settings.push_back(175.0f);
    zoom_settings.push_back(200.0f);
    zoom_settings.push_back(300.0f);
    zoom_settings.push_back(400.0f);
    zoom_setting_actions_.generateActions( zoom_settings );
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
        sketch_actions_.generateSketchColors(sketch_colors, sketch_proxy_.getColor());
    }
    if (!sketch_shapes.empty())
    {
        sketch_actions_.generateSketchShapes(sketch_shapes, sketch_proxy_.getShape());
    }
    if (!status_mgr_.isSketching())
    {
        sketch_actions_.setSketchColor( INVALID_SKETCH_COLOR );
        sketch_actions_.setSketchShape( INVALID_SKETCH_SHAPE );
    }

    std::vector<int> all;
    all.push_back(ROTATE_SCREEN);
    if (isFullScreenCalculatedByWidgetSize())
    {
        all.push_back(EXIT_FULL_SCREEN);
    } else
    {
        all.push_back(FULL_SCREEN);
    }
    all.push_back(MUSIC);
    all.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(all);
}

void ImageView::popupMenu()
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
        if (SysStatus::instance().hasTouchScreen())
        {
            menu.addGroup(&sketch_actions_);
        }
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
        case SET_TOBE_BOOTSPLASH:
            {
                setCurrentImageToBeBootSplash();
                disable_update = false;
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
        case FULL_SCREEN:
            {
                emit needFullScreen(true);
            }
            break;
        case EXIT_FULL_SCREEN:
            {
                emit needFullScreen(false);
            }
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

void ImageView::returnToLibrary(bool b)
{
    qApp->exit();
}

void ImageView::openThumbnailView(bool b)
{
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view == 0)
    {
        return;
    }

    ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);
    attachThumbnailView(thumbnail_view);
    thumbnail_view->attachSketchProxy(&sketch_proxy_);
    down_cast<MainWindow*>(parentWidget())->activateView(THUMBNAIL_VIEW);
    down_cast<ThumbnailView*>(thumbnail_view)->setCurrentPage(layout_->getCurrentPage());
}

void ImageView::setSketchMode(const SketchMode mode, bool selected)
{
    FunctionStatus s = selected ? FUNC_SELECTED : FUNC_NORMAL;
    FunctionID id = mode == MODE_SKETCHING ? ID_SKETCHING : ID_ERASING;
    status_mgr_.setStatus(id, s);
    sketch_proxy_.setMode(mode);
}

void ImageView::setSketchShape(const SketchShape shape)
{
    sketch_proxy_.setShape(shape);
    status_mgr_.setStatus(ID_SKETCHING, FUNC_SELECTED);
}

void ImageView::setSketchColor(const SketchColor color)
{
    sketch_proxy_.setColor(color);
    status_mgr_.setStatus(ID_SKETCHING, FUNC_SELECTED);
}

void ImageView::clearVisibleImages()
{
    // access all of the displaying images
    for (int i = 0; i < static_cast<int>(display_images_.size()); ++i)
    {
        shared_ptr<ImageItem> image_item = display_images_.getImage(i);
        image_item->access(model_);
    }
    display_images_.clear();
}

void ImageView::initLayout()
{
    if (read_mode_ == CONTINUOUS_LAYOUT)
    {
        layout_.reset(new ContinuousPageLayout(view_setting_.rotate_orient,
                                               view_setting_.zoom_setting));
    }
    else
    {
        layout_.reset(new SinglePageLayout(view_setting_.rotate_orient,
                                           view_setting_.zoom_setting));
    }

    connect(layout_.get(), SIGNAL(layoutDoneSignal()), this, SLOT(onLayoutDone()));
    connect(layout_.get(), SIGNAL(needPageSignal(const int)), this, SLOT(onNeedPage(const int)));
    connect(layout_.get(), SIGNAL(needContentAreaSignal(const int)), this, SLOT(onNeedContentArea(const int)));

    layout_->setMargins(cur_margin_);
    layout_->setWidgetArea(QRect(0, 0, size().width(), size().height()));
}

void ImageView::initViewSetting()
{
    RenderSetting render_setting;
    render_setting.setContentArea(QRect(0, 0, width(), height()));
    render_proxy_.updateRenderSetting(render_setting);
}

void ImageView::attachModel(BaseModel * model)
{
    if (model_ == model)
    {
        return;
    }

    // Record the model.
    model_ = static_cast<ImageModel*>(model);

    // connect the signals
    connect(model_, SIGNAL(modelReadySignal(const int)), this, SLOT(onModelReady(const int)));
    connect(model_, SIGNAL(modelClosingSignal()), this, SLOT(onModelClosing()));
    connect(model_, SIGNAL(renderingImageReadySignal(shared_ptr<ImageItem>, ImageStatus, bool)),
            this, SLOT(onImageReady(shared_ptr<ImageItem>, ImageStatus, bool)));
    connect(model_, SIGNAL(renderingThumbnailReadySignal(shared_ptr<BaseThumbnail>, const QRect&)),
            this, SLOT(onThumbnailReady(shared_ptr<BaseThumbnail>, const QRect&)));
    connect(model_, SIGNAL(requestSaveAllOptions()),
            this, SLOT(onSaveViewOptions()));
}

void ImageView::deattachModel()
{
    disconnect(model_, SIGNAL(modelReadySignal(const int)), this, SLOT(onModelReady(const int)));
    disconnect(model_, SIGNAL(modelClosingSignal()), this, SLOT(onModelClosing()));
    disconnect(model_, SIGNAL(renderingImageReadySignal(shared_ptr<ImageItem>, ImageStatus, bool)),
               this, SLOT(onImageReady(shared_ptr<ImageItem>, ImageStatus, bool)));
    disconnect(model_, SIGNAL(renderingThumbnailReadySignal(shared_ptr<BaseThumbnail>, const QRect&)),
               this, SLOT(onThumbnailReady(shared_ptr<BaseThumbnail>, const QRect&)));
    disconnect(model_, SIGNAL(requestSaveAllOptions()),
               this, SLOT(onSaveViewOptions()));
}

void ImageView::attachMainWindow(MainWindow *main_window)
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

    status_mgr_.setStatus(ID_PAN, FUNC_SELECTED);
}

void ImageView::deattachMainWindow(MainWindow *main_window)
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

void ImageView::attachThumbnailView(ThumbnailView *thumb_view)
{
    connect(thumb_view, SIGNAL(needThumbnail(const int, const QRect&)),
            this, SLOT(onNeedThumbnail(const int, const QRect&)));
    connect(thumb_view, SIGNAL(clearThumbnails()),
            this, SLOT(onThumbnailClear()));
    connect(thumb_view, SIGNAL(returnToReading(const int)),
            this, SLOT(onThumbnailReturn(const int)));
}

void ImageView::deattachThumbnailView(ThumbnailView *thumb_view)
{
    disconnect(thumb_view, SIGNAL(needThumbnail(const int, const QRect&)),
               this, SLOT(onNeedThumbnail(const int, const QRect&)));
    disconnect(thumb_view, SIGNAL(clearThumbnails()),
               this, SLOT(onThumbnailClear()));
    disconnect(thumb_view, SIGNAL(returnToReading(const int)),
               this, SLOT(onThumbnailReturn(const int)));
}

void ImageView::gotoPage(const int page_number)
{
    jump(page_number);
}

void ImageView::paintEvent(QPaintEvent *pe)
{
    int count = display_images_.size();
    QPainter painter(this);
    for (int i = 0; i < count; ++i)
    {
        shared_ptr<ImageItem> cur_image = display_images_.getImage(i);
        paintImage(painter, cur_image);
        //PaintComment(painter, cur_image);
    }
}

void ImageView::sendRenderRequest(vbf::PagePtr page)
{
    RenderSetting setting;
    setting.setContentArea(page->displayArea());
    setting.setRotation(view_setting_.rotate_orient);

    // Load the sketch data of this page
    ImageKey image_key;
    if (model_->getImageNameByIndex(page->key(), image_key))
    {
        sketch::PageKey page_key;
        page_key.setNum(page->key());
        sketch_proxy_.loadPage(image_key, page_key, QString());
    }

    render_proxy_.renderImage(page->key(), setting, model_);
}

/// Send request of prerendering
void ImageView::sendPrerenderRequest(const int image_index)
{
    vbf::PagePtr page = layout_->getPage(image_index);
    if (page == 0)
    {
        int width = 0, height = 0;
        if (!model_->getImageActualSize(image_index, width, height))
        {
            return;
        }

        QRect rect(0, 0, width, height);
        layout_->setPageWithoutUpdate(image_index, rect);
        page = layout_->getPage(image_index);
    }

    if (page != 0)
    {
        RenderSetting setting;
        setting.setContentArea(page->displayArea());
        setting.setRotation(view_setting_.rotate_orient);
        render_proxy_.prerenderImage(image_index, setting, model_);
    }
}

void ImageView::paintComment(QPainter &painter, shared_ptr<ImageItem> image)
{
    QRect spacing_area;
    if (layout_->getSpacingArea(image->index(), spacing_area))
    {
        // display comments in this area
        QFont f;
        f.setBold(true);
        f.setPixelSize(spacing_area.height() - 2);
        f.setStyleStrategy(QFont::ForceOutline);

        // Should use layout instead of using setText directly.
        // Prepare the layout.
        QTextLayout layout;
        layout.setFont(f);
        layout.setText(image->name());
        QTextOption opt = layout.textOption();
        opt.setAlignment(Qt::AlignHCenter);
        opt.setWrapMode(QTextOption::WrapAnywhere);
        layout.setTextOption(opt);
        layout.beginLayout();
        QTextLine line = layout.createLine();
        while (line.isValid())
        {
            line.setLineWidth(width());
            line = layout.createLine();
        }
        layout.endLayout();

        // Draw layout to the painter.
        int y = spacing_area.top();
        for(int i = 0; i < layout.lineCount(); ++i)
        {
            QTextLine line = layout.lineAt(i);
            line.draw(&painter, QPoint(x(), y));
            y += static_cast<int>(line.height());
        }
    }
}

void ImageView::paintSketches(QPainter & painter, shared_ptr<ImageItem> image)
{
    QPoint page_pos;
    if (!layout_->getContentPos(image->index(), page_pos))
    {
        return;
    }

    // update zoom factor
    vbf::PagePtr page_layout = layout_->getPage(image->index());
    QRect page_area(page_pos, page_layout->displayArea().size());
    sketch_proxy_.setZoom(page_layout->zoomValue() * ZOOM_ACTUAL);
    sketch_proxy_.setContentOrient(layout_->rotateDegree());
    sketch_proxy_.setWidgetOrient(getSystemRotateDegree());

    // draw sketches in this page
    // the page number of any image is 0
    sketch::PageKey page_key;
    page_key.setNum(0);
    sketch_proxy_.updatePageDisplayRegion(image->name(), page_key, page_area);
    sketch_proxy_.paintPage(image->name(), page_key, painter);
}

void ImageView::paintImage(QPainter &painter, shared_ptr<ImageItem> image)
{
    if (image->image() == 0 || image->renderStatus() != IMAGE_STATUS_DONE)
    {
        return;
    }

    vbf::PagePtr page_layout = layout_->getPage(image->index());
    if (page_layout == 0)
    {
        qDebug("The layout is not ready!");
        return;
    }

    QPoint cur_pos;
    if (layout_->getContentPos(image->index(), cur_pos))
    {
        // draw content of page
        painter.drawImage(cur_pos, *(image->image()));
    }
    paintSketches(painter, image);
}

void ImageView::onModelReady(const int init_page_num)
{
    initLayout();
    if (init_page_num > 0)
    {
        // if the initialized page number is not 0
        // set to the page
        cur_image_index_ = init_page_num;
        jump(cur_image_index_);
    }

    resetLayout();

    QWidget* thumbnail_view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (thumbnail_view != 0)
    {
        // attach the model with thumbnail view
        down_cast<ThumbnailView*>(thumbnail_view)->setTotalNumber(model_->imageCount());
    }
}

void ImageView::onModelClosing()
{
    sketch_proxy_.close();
    if (layout_ != 0)
    {
        // clear the cached pages in pages layout
        layout_->clearPages();
    }

    // clear the visible pages
    clearVisibleImages();
}

void ImageView::onSaveViewOptions()
{
    // save & close the sketch document
    sketch_proxy_.save();
}

void ImageView::onPrerendering()
{
    for (int idx = 0; idx < rendering_pages_.size(); ++idx)
    {
        int dst_page = rendering_pages_.at(idx);
        sendPrerenderRequest(dst_page);
    }
}

void ImageView::onNeedThumbnail(const int image_idx, const QRect &rect)
{
    render_proxy_.renderThumbnail(image_idx, rect, model_);
}

void ImageView::onThumbnailReady(shared_ptr<BaseThumbnail> thumb,
                                 const QRect &bounding_rect)
{
    QWidget* thumbnail_view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (thumbnail_view != 0)
    {
        down_cast<ThumbnailView*>(thumbnail_view)->setThumbnail(thumb->key(), thumb, bounding_rect);
    }
}

void ImageView::onThumbnailClear()
{
    model_->getThumbsMgr().clear();
}

void ImageView::onThumbnailReturn(const int image_idx)
{
    QWidget* view = down_cast<MainWindow*>(parentWidget())->getView(THUMBNAIL_VIEW);
    if (view == 0)
    {
        return;
    }
    ThumbnailView * thumbnail_view = down_cast<ThumbnailView*>(view);
    deattachThumbnailView(thumbnail_view);

    down_cast<MainWindow*>(parentWidget())->activateView(IMAGE_VIEW);
    if (image_idx >= 0)
    {
        jump(image_idx);
    }
    else
    {
        emit currentPageChanged(cur_image_index_, model_->imageCount());
    }
    sketch_proxy_.setDrawingArea(this);
}

void ImageView::onPagebarClicked(const int percent, const int value)
{
    gotoPage(value);
}

void ImageView::onPopupContextMenu()
{
    popupMenu();
}

void ImageView::attachSketchProxy()
{
    if (status_mgr_.isErasing())
    {
        sketch_proxy_.setMode(MODE_ERASING);
    }
    else if (status_mgr_.isSketching())
    {
        sketch_proxy_.setMode(MODE_SKETCHING);
    }
    sketch_proxy_.attachWidget(this);
    updateSketchProxy();
}

void ImageView::deattachSketchProxy()
{
    sketch_proxy_.deattachWidget(this);
}

void ImageView::updateSketchProxy()
{
    // deactivate all pages
    sketch_proxy_.deactivateAll();

    // activate visible pages
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

            // update zoom factor
            sketch_proxy_.setZoom(page_layout->zoomValue() * ZOOM_ACTUAL);
            sketch_proxy_.setContentOrient(layout_->rotateDegree());
            sketch_proxy_.setWidgetOrient(getSystemRotateDegree());

            ImageKey image_key;
            sketch::PageKey page_key;

            // the page number of any image is 0
            page_key.setNum(0);
            model_->getImageNameByIndex(page_number, image_key);
            sketch_proxy_.activatePage(image_key, page_key);
            sketch_proxy_.updatePageDisplayRegion(image_key, page_key, page_area);
        }
        idx++;
    }
}

void ImageView::onStylusChanges(const int type)
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

void ImageView::resetLayout()
{
    // NOTE: The document should be ready when calling this function
    layout_->clearPages();
    layout_->setFirstPageNumber(0);
    layout_->setLastPageNumber(model_->imageCount() - 1);
    layout_->update();
}

void ImageView::switchLayout(PageLayoutType mode)
{
    if (read_mode_ == mode)
    {
        return;
    }

    if (mode == THUMBNAIL_LAYOUT)
    {
        openThumbnailView(true);
    }
    else
    {
        read_mode_ = mode;
        initLayout();
        layout_->setWidgetArea(QRect(0,
                                     0,
                                     size().width(),
                                     size().height()));
        jump(cur_image_index_);
        resetLayout();
    }
}

void ImageView::onImageReady(shared_ptr<ImageItem> image, ImageStatus status, bool update_screen)
{
    bool need_prerender = false;
    if ( update_screen )
    {
#ifdef DISPLAY_SYSTEM_BUSY
        need_set_rendering_busy_ = false;
        if ( sys::SysStatus::instance().isSystemBusy() )
        {
            // if it is the first time rendering, set busy to be false
            sys::SysStatus::instance().setSystemBusy( false );
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
            if (image->index() == (*idx)->key() &&
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
        if (display_images_.size() > 0)
        {
            onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
        }
        else
        {
            onyx::screen::instance().setDefaultWaveform(current_waveform_);
        }

        // add to displaying images
        display_images_.add(image);

        // retrieve the next one and send render request
        if (!layout_pages_.isEmpty())
        {
            // retrieve the list of prerender images
            vbf::PagePtr next_page = layout_pages_.front();
            int prev_page_number = image->index();
            int next_page_number = next_page->key();
            model_->renderPolicy()->getRenderRequests(next_page_number,
                                                      prev_page_number,
                                                      model_->imageCount(),
                                                      rendering_pages_);
            sendRenderRequest(next_page);
            QTimer::singleShot(200, this, SLOT(onPrerendering()));
        }
        else
        {
            int cur_page = layout_->getCurrentPage();
            model_->setCurrentImage(cur_page);
            emit currentPageChanged(cur_page, model_->imageCount());
            need_prerender = true;
        }

        // update to paint the page
        update();
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::INVALID);

        // rollback to current default mode after update
        if (layout_pages_.isEmpty())
        {
            onyx::screen::instance().setDefaultWaveform(current_waveform_);
        }
    }

    int idx = rendering_pages_.indexOf(image->index());
    if (idx >= 0 && idx < rendering_pages_.size())
    {
        rendering_pages_.remove(idx);
    }

    if (need_prerender)
    {
        QTimer::singleShot(500, this, SLOT(onPrerendering()));
    }
}

void ImageView::onLayoutDone()
{
    // clear the previous visible pages
    clearVisibleImages();
    layout_->getVisiblePages(layout_pages_);
    if (layout_pages_.empty())
    {
        return;
    }

    if (status_mgr_.isErasing() || status_mgr_.isSketching())
    {
        updateSketchProxy();
    }

    // send the render request for first page
    vbf::PagePtr page = layout_pages_.front();

    // update previous page
    int previous_page = cur_image_index_;

    // update current page
    cur_image_index_ = page->key();

    // save the sketch if current page changes
    if (cur_image_index_ != previous_page)
    {
        sketch_proxy_.save();
    }

    // retrieve the list of prerender images
    model_->renderPolicy()->getRenderRequests(cur_image_index_,
                                              previous_page,
                                              model_->imageCount(),
                                              rendering_pages_);
    sendRenderRequest(page);

#ifdef DISPLAY_SYSTEM_BUSY
    // the waiting time should be adjust in device
    need_set_rendering_busy_ = true;
    QTimer::singleShot(100, this, SLOT(onRenderBusy()));
#endif
}

void ImageView::onNeedPage(const int page_number)
{
    int width = 0, height = 0;
    if (!static_cast<ImageModel*>(model_)->getImageActualSize(page_number, width, height))
    {
        return;
    }

    QRect rect;
    rect.setWidth(width);
    rect.setHeight(height);
    layout_->setPage(page_number, rect);
}

void ImageView::onNeedContentArea(const int page_number)
{
    // TODO. Implement me or remove me
}

#ifdef DISPLAY_SYSTEM_BUSY
void ImageView::onRenderBusy()
{
    if (need_set_rendering_busy_ && display_images_.size() == 0)
    {
        sys::SysStatus::instance().setSystemBusy(true);
    }
}
#endif

void ImageView::mousePressEvent(QMouseEvent *me)
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
            rubber_band_->setGeometry(QRect(stroke_area_.getOriginPosition(),
                                            QSize()));
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

void ImageView::mouseReleaseEvent(QMouseEvent *me)
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

void ImageView::mouseMoveEvent(QMouseEvent *me)
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

void ImageView::keyReleaseEvent(QKeyEvent *ke)
{
    int offset = 0;
    switch(ke->key())
    {
    case Qt::Key_PageDown:
    case Qt::Key_Down:
        {
            offset = (height() - OVERLAP_DISTANCE);
            if (isLandscape())
            {
                offset = (width() - OVERLAP_DISTANCE);
            }
            scroll(0, offset);
        }
        break;
    case Qt::Key_Right:
        {
            offset = (width() - OVERLAP_DISTANCE);
            if (isLandscape())
            {
                offset = (height() - OVERLAP_DISTANCE);
            }
            scroll(offset, 0);
        }
        break;
    case Qt::Key_PageUp:
    case Qt::Key_Up:
        {
            offset = -(height() - OVERLAP_DISTANCE);
            if (isLandscape())
            {
                offset = - (width() - OVERLAP_DISTANCE);
            }
            scroll(0, offset);
        }
        break;
    case Qt::Key_Left:
        {
            offset = -(width() - OVERLAP_DISTANCE);
            if (isLandscape())
            {
                offset = - (height() - OVERLAP_DISTANCE);
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

void ImageView::resizeEvent(QResizeEvent *re)
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
void ImageView::wheelEvent(QWheelEvent *we)
{
    int offset = we->delta();
    scroll(0, -offset);
}
#endif

void ImageView::openMusicPlayer()
{
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
    sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
}

bool ImageView::hitTest(const QPoint &point)
{
    // Hit test whether clicking the content area of image
    return false;
}

void ImageView::scroll(int offset_x, int offset_y)
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

void ImageView::zoomIn(const QRect &zoom_rect)
{
    layout_->zoomIn(zoom_rect);
    view_setting_.zoom_setting = layout_->zoomSetting();
}

void ImageView::zoomToBestFit()
{
    layout_->zoomToBestFit();
    view_setting_.zoom_setting = ZOOM_TO_PAGE;
}

void ImageView::zoomToWidth()
{
    layout_->zoomToWidth();
    view_setting_.zoom_setting = ZOOM_TO_WIDTH;
}

void ImageView::zoomToHeight()
{
    layout_->zoomToHeight();
    view_setting_.zoom_setting = ZOOM_TO_HEIGHT;
}

void ImageView::selectionZoom()
{
    status_mgr_.setStatus(ID_ZOOM_IN, FUNC_SELECTED);
}

void ImageView::setPan()
{
    status_mgr_.setStatus(ID_PAN, FUNC_SELECTED);
}

void ImageView::setZoomValue(float value)
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

void ImageView::pan(int offset_x, int offset_y)
{
    layout_->pan(offset_x, offset_y);
}

void ImageView::jump(int page_number)
{
    layout_->jump(page_number);
}

void ImageView::rotate()
{
    emit rotateScreen();

    RotateDegree degree = getSystemRotateDegree();
    sketch_proxy_.setWidgetOrient( degree );
}

void ImageView::startSlideShow()
{
    status_mgr_.setStatus(ID_SLIDE_SHOW, FUNC_SELECTED);
    sys::SysStatus::instance().enableIdle(false);

    // reset the reading layout and zoom
    zoomToBestFit();
    switchLayout(PAGE_LAYOUT);
    slide_timer_.start(SLIDE_TIME_INTERVAL);

    // enter full screen mode
    emit needFullScreen(true);
}

void ImageView::stopSlideShow()
{
    status_mgr_.setStatus(ID_SLIDE_SHOW, FUNC_NORMAL);
    sys::SysStatus::instance().resetIdle();

    // stop the slide timer
    slide_timer_.stop();

    // exit full screen mode
    emit needFullScreen(false);
}

void ImageView::slideShowNextImage()
{
    int current = model_->getCurrentImageIndex();
    int total   = model_->imageCount();

    if ((++current) >= total)
    {
        current = 0;
    }

    jump(current);
}

void ImageView::onRenderSettingChanged(ImageRenderSetting setting, bool checked)
{
    switch (setting)
    {
    case IMAGE_NEED_DITHER:
        {
            ImageGlobalSettings::instance().needDither(checked);
        }
        break;
    case IMAGE_NEED_CONVERT:
        {
            ImageGlobalSettings::instance().needConvert(checked);
        }
        break;
    case IMAGE_NEED_SMOOTH:
        {
            ImageGlobalSettings::instance().needSmooth(checked);
        }
        break;
    default:
        break;
    }

    // TODO. rerender the pages
    int count = display_images_.size();
    for (int i = 0; i < count; ++i)
    {
        shared_ptr<ImageItem> cur_image = display_images_.getImage(i);
        render_proxy_.renderImage(cur_image->index(),
                                  cur_image->renderSetting(),
                                  model_);
    }
}

void ImageView::onRequestUpdateScreen()
{
    onyx::screen::instance().enableUpdate(false);
    repaint();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget( this, onyx::screen::ScreenProxy::GU );
}

void ImageView::onWakeUp()
{
    // update the screen to make sure the sketches are drawable.
    update();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);
}

bool ImageView::setCurrentImageToBeBootSplash()
{
    // image path
    int current_idx = layout_->getCurrentPage();
    ImageKey path;
    if (!model_->getImageNameByIndex(current_idx, path))
    {
        return false;
    }

    QStringList args;
    args << path;

    QStringList envs = QProcess::systemEnvironment();
    QProcess process;
    process.setEnvironment(envs);

    // the application.
    process.start("update_splash", args);
    process.waitForFinished();
    return true;
}

bool ImageView::isFullScreenCalculatedByWidgetSize()
{
    if (parentWidget())
    {
        QSize parentSize = parentWidget()->size();
        // TODO find a better way to do this
        if (parentSize.height() == size().height())
        {
            return true;
        }
    }
    return false;
}

}
