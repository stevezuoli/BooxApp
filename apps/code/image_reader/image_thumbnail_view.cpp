#include "image_thumbnail_view.h"
#include "notes_doc_manager.h"

using namespace ui;

namespace image
{

ThumbnailView::ThumbnailView(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , layout_()
    , sketch_proxy_(0)
    , notes_doc_manager_(0)
    , display_pages_()
    , left_pages_()
    , cur_page_(-1)
    , first_page_(-1)
    , total_number_(0)
{
    initializePopupMenuActions();
}

ThumbnailView::~ThumbnailView(void)
{
}

void ThumbnailView::attachMainWindow(MainWindow *main_window)
{
    connect(this, SIGNAL(positionChanged(const int, const int)),
            main_window, SLOT(handlePositionChanged(const int, const int)));
    connect(this, SIGNAL(needFullScreen(bool)),
            main_window, SLOT(handleFullScreen(bool)));
    connect(this, SIGNAL(popupJumpPageDialog()),
            main_window, SLOT(handlePopupJumpPageDialog()));

    connect(main_window, SIGNAL(pagebarClicked(const int, const int)),
            this, SLOT(handlePagebarClicked(const int, const int)));
    connect(main_window, SIGNAL(popupContextMenu()),
            this, SLOT(handlePopupContextMenu()));

    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
}

void ThumbnailView::deattachMainWindow(MainWindow *main_window)
{
    disconnect(this, SIGNAL(positionChanged(const int, const int)),
               main_window, SLOT(handlePositionChanged(const int, const int)));
    disconnect(this, SIGNAL(needFullScreen(bool)),
               main_window, SLOT(handleFullScreen(bool)));
    disconnect(this, SIGNAL(popupJumpPageDialog()),
               main_window, SLOT(handlePopupJumpPageDialog()));

    disconnect(main_window, SIGNAL(pagebarClicked(const int, const int)),
               this, SLOT(handlePagebarClicked(const int, const int)));
    disconnect(main_window, SIGNAL(popupContextMenu()),
               this, SLOT(handlePopupContextMenu()));
}

void ThumbnailView::attachSketchProxy(SketchProxy *sketch_proxy)
{
    sketch_proxy_ = sketch_proxy;
    sketch_proxy_->setDrawingArea(this);
}

void ThumbnailView::deattachSketchProxy()
{
    sketch_proxy_ = 0;
}

void ThumbnailView::attachNotesDocManager(NotesDocumentManager *notes_doc_mgr)
{
    notes_doc_manager_ = notes_doc_mgr;
}

void ThumbnailView::deattachNotesDocManager()
{
    notes_doc_manager_ = 0;
}

void ThumbnailView::setTotalNumber(const int num)
{
    total_number_ = num;
}

void ThumbnailView::updatePageNumber(const int key)
{
    int thumb_num = layout_.pages().size();

    assert(thumb_num > 0);
    first_page_ = (key / thumb_num) * thumb_num;
    cur_page_ = key;
}

void ThumbnailView::moveCurrentPage(const int next_num)
{
    int num_layout = layout_.pages().size();
    if (next_num < first_page_ || next_num >= (first_page_ + num_layout))
    {
        setCurrentPage(next_num);
    }
    else
    {
        cur_page_ = next_num;
        update();
    }
}

void ThumbnailView::setCurrentPage(int key)
{
    if (key < 0 || key >= total_number_)
    {
        return;
    }

    left_pages_.clear();
    display_pages_.clear();

    updatePageNumber(key);

    // update page bar
    emit positionChanged(getCurrentScreen(cur_page_), getTotalScreens(total_number_));

    int num = layout_.pages().size();
    int end = first_page_ + num;
    if (end >= total_number_)
    {
        end = total_number_;
    }

    for (int i = first_page_; i < end; ++i)
    {
        left_pages_.insert(i);
    }

    updateThumbnails();

    // update the whole screen
    update();
}

void ThumbnailView::updateThumbnails()
{
    RenderSetIter begin = left_pages_.begin();
    if (begin == left_pages_.end())
    {
        return;
    }

    ThumbnailPages & pages = layout_.pages();
    int idx = *begin - first_page_;
    emit needThumbnail(*begin, pages[idx].image_area);
}

void ThumbnailView::setThumbnail(int index, ThumbPtr thumb, const QRect &bounding_rect)
{
    if (index < first_page_ || index >= (first_page_ + layout_.pages().size()))
    {
        printf("Old image, abandon it\n");
        return;
    }

    if (getLayoutPage(index - first_page_).image_area != bounding_rect)
    {
        // if bounding rectangle is not equal to the layout image rectangle
        // it means the thumbnail is out of date. just igore it
        printf("Redundant thumbnail image, abandon it\n");
        return;
    }

    // load sketches if it has
    if (sketch_proxy_ != 0)
    {
        QString page_key;
        if (notes_doc_manager_ != 0)
        {
            page_key.setNum(index);
            sketch_proxy_->loadPage(notes_doc_manager_->notesDocumentPath(), page_key, thumb->path());
        }
        else
        {
            page_key.setNum(0);
            sketch_proxy_->loadPage(thumb->path(), page_key, QString());
        }
    }

    RenderSetIter idx = left_pages_.find(index);
    if (idx != left_pages_.end())
    {
        display_pages_.push_back(thumb);
        left_pages_.erase(idx);
        updateThumbnails();
    }
    else
    {
        printf("Updated thumbnail image, refresh it\n");
    }

    // QRect update_rect = bounding_rect.unite(getLayoutPage(key).text_area);
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
    update();
}

void ThumbnailView::paintPage(QPainter &painter, ThumbPtr page, int index)
{
    ThumbnailPages & layout_pages = layout_.pages();
    ThumbnailPage & page_layout = layout_pages[index];
    painter.drawImage(page->displayArea().topLeft(), *(page->image()));
}

void ThumbnailView::paintSketches(QPainter &painter, ThumbPtr page, int index)
{
    if (sketch_proxy_ == 0)
    {
        qDebug("Cannot find sketch proxy");
        return;
    }

    // update zoom factor
    sketch_proxy_->setZoom(page->zoom() * ZOOM_ACTUAL);
    sketch_proxy_->setContentOrient(ROTATE_0_DEGREE);
    sketch_proxy_->setWidgetOrient(getSystemRotateDegree());

    // draw sketches in this page
    sketch::PageKey page_key;
    if (notes_doc_manager_ != 0)
    {
        page_key.setNum(index + first_page_);
    }
    else
    {
        page_key.setNum(0);
    }

    if (notes_doc_manager_ != 0)
    {
        sketch_proxy_->updatePageDisplayRegion(notes_doc_manager_->notesDocumentPath(), page_key, page->displayArea());
        sketch_proxy_->paintPage(notes_doc_manager_->notesDocumentPath(), page_key, painter);
    }
    else
    {
        sketch_proxy_->updatePageDisplayRegion(page->path(), page_key, page->displayArea());
        sketch_proxy_->paintPage(page->path(), page_key, painter);
    }
}

void ThumbnailView::paintTitle(QPainter &painter, ThumbPtr page, int index)
{
    // get the layout page by thumbnail
    ThumbnailPage& layout_page = getLayoutPage(index);

    // display comments in this area
    QFont f;
    f.setPixelSize(layout_page.text_area.height() >> 1);
    f.setStyleStrategy(QFont::ForceOutline);

    // Should use layout instead of using setText directly.
    // Prepare the layout.
    QTextLayout layout;
    layout.setFont(f);
    layout.setText(page->name());

    QTextOption opt = layout.textOption();
    opt.setAlignment(Qt::AlignHCenter);
    opt.setWrapMode(QTextOption::WrapAnywhere);
    layout.setTextOption(opt);
    layout.beginLayout();
    QTextLine line = layout.createLine();
    while (line.isValid())
    {
        line.setLineWidth(layout_page.text_area.width());
        line = layout.createLine();
    }
    layout.endLayout();

    // Draw layout to the painter.
    int y = layout_page.text_area.top();
    for(int i = 0; i < layout.lineCount(); ++i)
    {
        QTextLine line = layout.lineAt(i);
        line.draw(&painter, QPoint(layout_page.text_area.left(), y));
        y += static_cast<int>(line.height());
    }
}

void ThumbnailView::paintBoundingRect(QPainter &painter, const ThumbnailPage& thumb)
{
    painter.drawRect(thumb.image_area);
}

void ThumbnailView::paintEvent(QPaintEvent *pe)
{
    QPainter painter(this);

    // draw the rectangle of current page
    painter.fillRect(rect(), QBrush(Qt::white));

    if (cur_page_ >= 0)
    {
        paintBoundingRect(painter, getLayoutPage(cur_page_ - first_page_));
    }

    // draw the thumbnail images
    int count = display_pages_.size();
    for (int i = 0; i < count; ++i)
    {
        ThumbPtr cur_thumb = display_pages_.get_page(i);
        paintPage(painter, cur_thumb, i);
        paintTitle(painter, cur_thumb, i);
        paintSketches(painter, cur_thumb, i);
    }
}

void ThumbnailView::resizeEvent(QResizeEvent *re)
{
    layout_.setWidgetSize(re->size());
    emit clearThumbnails();
    setCurrentPage(cur_page_);
}

void ThumbnailView::mousePressEvent(QMouseEvent *me)
{
    switch (me->button())
    {
    case Qt::LeftButton:
        {
            mouse_press_pos_ = me->pos();
        }
        break;
    default:
        break;
    }
    me->accept();
}

void ThumbnailView::mouseReleaseEvent(QMouseEvent *me)
{
    switch (me->button())
    {
    case Qt::LeftButton:
        {
            if (!mouse_press_pos_.isNull())
            {
                int sys_offset = sys::SystemConfig::direction(mouse_press_pos_, me->pos());
                if (sys_offset != 0)
                {
                    sys_offset > 0 ? nextScreen() : prevScreen();
                    me->accept();
                    return;
                }
            }

            int thumb_idx = 0;
            if (layout_.hitTest(me->pos(), thumb_idx) &&
                thumb_idx < static_cast<int>(display_pages_.size()))
            {
                thumb_idx += first_page_;
                emit returnToReading(thumb_idx);
            }
        }
        break;
    case Qt::RightButton:
        break;
    default:
        break;
    }
    me->accept();
}

void ThumbnailView::keyReleaseEvent(QKeyEvent *ke)
{
    switch(ke->key())
    {
    case Qt::Key_PageDown:
        {
            nextScreen();
        }
        break;
    case Qt::Key_PageUp:
        {
            prevScreen();
        }
        break;
    case Qt::Key_Left:
        {
            int prev = cur_page_ - 1;
            moveCurrentPage(prev);
        }
        break;
    case Qt::Key_Right:
        {
            int next = cur_page_ + 1;
            moveCurrentPage(next);
        }
        break;
    case Qt::Key_Up:
        {
            int up = cur_page_ - layout_.context().columns;
            moveCurrentPage(up);
        }
        break;
    case Qt::Key_Down:
        {
            int down = cur_page_ + layout_.context().columns;
            moveCurrentPage(down);
        }
        break;
    case Qt::Key_F10:
    case ui::Device_Menu_Key:
        {
            handlePopupContextMenu();
        }
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        {
            emit returnToReading(cur_page_);
        }
        break;
    case Qt::Key_Escape:
        {
            emit returnToReading(-1);
        }
        break;
    default:
        break;
    }
}

void ThumbnailView::nextScreen()
{
    int num = layout_.pages().size();
    int end = first_page_ + num;
    if (end >= total_number_)
    {
        return;
    }

    setCurrentPage(end);
}

void ThumbnailView::prevScreen()
{
    if (first_page_ == 0)
    {
        return;
    }

    int num = layout_.pages().size();
    int start = first_page_ - num;
    if (start < 0)
    {
        start = 0;
    }

    setCurrentPage(start);
}

ThumbnailPage& ThumbnailView::getLayoutPage(const int key)
{
    ThumbnailPages & pages = layout_.pages();
    return pages[key];
}

bool ThumbnailView::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::UpdateRequest)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
    }
    return ret;
}

void ThumbnailView::handlePagebarClicked(const int percent, const int value)
{
    int cur_screen = getCurrentScreen(cur_page_);
    if (cur_screen == value)
    {
        return;
    }
    int dst_page = value * layout_.pages().size();
    setCurrentPage(dst_page);
}

/// Initialize the actions of popup menu
void ThumbnailView::initializePopupMenuActions()
{
    std::vector<int> sys_actions;
    sys_actions.push_back(ROTATE_SCREEN);
    sys_actions.push_back(SCREEN_UPDATE_TYPE);
    sys_actions.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(sys_actions);
}

/// Popup menu
void ThumbnailView::popupMenu()
{
    ui::PopupMenu menu(this);
    menu.setSystemAction(&system_actions_);

    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == system_actions_.category())
    {
        SystemAction system_action = system_actions_.selected();
        switch (system_action)
        {
        case RETURN_TO_LIBRARY:
            emit returnToReading();
            break;
        case SCREEN_UPDATE_TYPE:
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, true);
            onyx::screen::instance().toggleWaveform();
            update();
            break;
        case ROTATE_SCREEN:
            SysStatus::instance().rotateScreen();
            break;
        default:
            break;
        }
    }
}

void ThumbnailView::handlePopupContextMenu()
{
    popupMenu();
}

}
