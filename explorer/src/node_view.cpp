#include "onyx/ui/text_layout.h"
#include "node.h"
#include "file_node.h"
#include "notes_node.h"
#include "node_view.h"
#include "image_factory.h"
#include "onyx/screen/screen_proxy.h"

using namespace ui;

namespace explorer
{

namespace view
{

static const int INDICATOR_PEN_WIDTH = 3;
static const int PEN_WIDTH = 1;
static const int SPACING = 2;
static const int MAX_RATING = 5;

QImage NodeView::star_black_image_;
QImage NodeView::star_white_image_;

/// Node view implementation.
NodeView::NodeView(QWidget * parent)
    : QWidget(parent)
    , node_(0)
    , view_mode_(LIST_VIEW)
    , selected_(false)
    , layout_dirty_(true)
    , need_paint_(true)
    , has_record_(false)
    , name_layout_()
    , size_layout_()
    , image_()
    , icon_pos_()
    , star_rect_()
{
}

NodeView::~NodeView(void)
{
}

/// Change view mode.
bool NodeView::setViewType(const ViewType mode)
{
    if (mode == view_mode_ || node_ == 0)
    {
        view_mode_ = mode;
        return false;
    }
    layout_dirty_ = true;
    need_paint_ = true;
    view_mode_ = mode;
    return true;
}

/// Always update the widget even with the same node, because the
/// node data may be changed.
void NodeView::setNode(Node *node)
{
    if (node == 0)
    {
        node_ = node;
        need_paint_ = true;
        return;
    }

    // Scan the metadata for file node.
    need_paint_ = true;
    node_ = node;
    if (node_->type() == NODE_TYPE_FILE)
    {
        static_cast<FileNode *>(node)->metadata();
    }
    layout_dirty_ = true;
    has_record_ = false;
}

void NodeView::fieldResized(Field field, int x, int width)
{
    if (node() == 0)
    {
        return;
    }

    // Update the list view layout.
    QFont name_font(font().family(), 20);

    // Icon position. Retrieve icon only when it's dirty.
    if (isLayoutDirty())
    {
        image_ = ImageFactory::instance().image(node_, THUMBNAIL_SMALL);
    }

    QRect rc = rect();
    if (field == NAME)
    {
        QRect icon_rect(2 * SPACING + x, 0, 32, rc.height());;
        QRect name_rect(icon_rect.right() + 2 * SPACING, 0, width - icon_rect.width(), rc.height());

        icon_pos_.setX(icon_rect.left());
        icon_pos_.setY((icon_rect.height() - image_.height()) >> 1);

        // Name layout.
        calculateSingleLineLayout(name_layout_, name_font, node_->display_name(),
                                  Qt::AlignLeft|Qt::AlignVCenter, name_rect);
        return;
    }

    QString string;

    // Type layout.
    if (field == NODE_TYPE)
    {
        QRect type_rect(x, 0, width, rc.height());
        NodeType type = node_->type();

        if (type == NODE_TYPE_FILE)
        {
            string = down_cast<FileNode *>(node_)->suffix();
        }
        else if (type == NODE_TYPE_DIRECTORY)
        {
            string = QApplication::tr("Folder");
        }
        calculateSingleLineLayout(type_layout_, name_font, string,
                                  Qt::AlignHCenter|Qt::AlignVCenter, type_rect);
        return;
    }

    if (field == SIZE)
    {
        QRect size_rect(x, 0, width, rc.height());

        // Size layout if any.
        sizeString(string);
        calculateSingleLineLayout(size_layout_, name_font, string,
                                  Qt::AlignHCenter|Qt::AlignVCenter, size_rect);
        return;
    }

    if (field == LAST_ACCESS_TIME)
    {
        QRect access_rect(x, 0, width, rc.height());
        string = node_->last_read();
        calculateSingleLineLayout(last_access_layout_, name_font, node_->last_read(),
                                  Qt::AlignHCenter|Qt::AlignVCenter, access_rect);
        return;
    }
}

/// TODO: Maybe should emit signals to caller, so that caller can
/// update this node only.
void NodeView::select(bool select)
{
    if (selected_ == select)
    {
        return;
    }
    selected_ = select;
    need_paint_ = true;
}

/// Extract or read metadata from database including thumbnail 
bool NodeView::updateMetadata()
{
    // Don't need to extract data when it's no in thumbnail view or
    // node is empty.
    if ((view_mode_ != THUMBNAIL_VIEW && view_mode_ != DETAILS_VIEW) ||
        node_ == 0)
    {
        return true;
    }

    if (!has_record_)
    {
        if (node_->type() == NODE_TYPE_NOTE)
        {
            image_ = down_cast<NoteNode*>(node_)->info().thumbnail();
            layout_dirty_ = true;
            updateScreen();
            need_paint_ = false;
            return true;
        }
        else if (node_->type() == NODE_TYPE_FILE)
        {
            // Only when it's image or supported content.
            // Otherwise we just return true so caller does not
            // need to extract any data.
            has_record_ = down_cast<FileNode*>(node_)->hasRecord();
            if (has_record_ || checkImageThumbnail())
            {
                layout_dirty_ = true;
                updateScreen();
                need_paint_ = false;
                return true;
            }
            return false;
        }
        return false;
    }
    return true;
}

bool NodeView::checkImageThumbnail()
{
    QImage tmp;
    if (down_cast<FileNode*>(node_)->thumbnail(tmp))
    {
        image_ = tmp;
        return true;
    }
    return false;
}

void NodeView::mousePressEvent(QMouseEvent *me)
{
    // Always report it.
    emit nodePressed(node_, me->globalPos());
}

void NodeView::mouseReleaseEvent(QMouseEvent *me)
{
    // Check the view mode.
    if (view_mode_ == DETAILS_VIEW &&
        node_ &&
        node_->type() == NODE_TYPE_FILE)
    {
        if (star_rect_.left() < me->pos().x())
        {
            int rating = (me->pos().x() - star_rect_.left()) / (star_black_image_.width() + SPACING);
            if (rating >= MAX_RATING)
            {
                updateRating(MAX_RATING);
            }
            else
            {
                updateRating(rating + 1);
            }
            return;
        }
    }
    emit nodeReleased(node_, me->globalPos());
}

void NodeView::keyPressEvent(QKeyEvent *)
{
}

void NodeView::keyReleaseEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Return)
    {
        emit nodeClicked(node_);
    }
}

void NodeView::paintEvent(QPaintEvent *pe)
{
    if (node_ == 0)
    {
        return;
    }

    if (isLayoutDirty())
    {
        updateLayouts();
        layout_dirty_ = false;
        need_paint_ = false;
    }

    QPainter painter(this);
    painter.setClipRect(pe->rect());
    switch (view_mode_)
    {
    case LIST_VIEW:
        paintListView(painter);
        break;
    case DETAILS_VIEW:
        paintDetailsView(painter);
        break;
    case THUMBNAIL_VIEW:
        paintThumbnailView(painter);
        break;
    default:
        assert(false);
        break;
    }
}

void NodeView::resizeEvent(QResizeEvent *)
{
    updateLayouts();
}

/// Paint for the list view.
void NodeView::paintListView(QPainter & painter)
{
    QPoint pos;

    if (selected_)
    {
        QRect rc = rect();
        QPainterPath p;
        rc.adjust(PEN_WIDTH, PEN_WIDTH, -PEN_WIDTH, -PEN_WIDTH);
        p.addRoundedRect(rc, 4, 4, Qt::AbsoluteSize);
        QPen pen(QColor(0, 0, 0));
        pen.setWidth(PEN_WIDTH);
        QPen old = painter.pen();
        painter.setPen(pen);
        painter.drawPath(p);
        painter.setPen(old);
    }

    // Draw icon at first.
    painter.drawImage(icon_pos_, image_);

    // Paint name layout.
    name_layout_.draw(&painter, pos);

    // Depends on the type.
    if (size_layout_.lineCount() > 0)
    {
        size_layout_.draw(&painter, pos);
    }

    type_layout_.draw(&painter, pos);
    last_access_layout_.draw(&painter, pos);
}

void NodeView::paintDetailsView(QPainter & painter)
{
    if (selected_)
    {
        QPainterPath p;
        p.addRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8, Qt::AbsoluteSize);
        QPen pen(QColor(0, 0, 0));
        pen.setWidth(PEN_WIDTH);
        QPen old = painter.pen();
        painter.setPen(pen);
        painter.drawPath(p);
        painter.setPen(old);
        painter.drawPath(p);
    }

    painter.drawImage(icon_pos_, image_);

    QPoint pt;
    name_layout_.draw(&painter, pt);

    if (node_->type() != NODE_TYPE_FILE)
    {
        return;
    }

    last_access_layout_.draw(&painter, pt);
    read_time_layout_.draw(&painter, pt);
    read_count_layout_.draw(&painter, pt);
    progress_layout_.draw(&painter, pt);
    size_layout_.draw(&painter, pt);
    paintRatings(painter);
}

void NodeView::paintThumbnailView(QPainter & painter)
{
    if (selected_)
    {
        QRect rc = rect();
        QPainterPath p;
        rc.adjust(PEN_WIDTH, PEN_WIDTH, -PEN_WIDTH, -PEN_WIDTH);
        p.addRoundedRect(rc, 8, 8, Qt::AbsoluteSize);
        QPen pen(QColor(0, 0, 0));
        pen.setWidth(PEN_WIDTH);
        QPen old = painter.pen();
        painter.setPen(pen);
        painter.drawPath(p);
        painter.setPen(old);
    }

    painter.drawImage(icon_pos_, image_);

    name_layout_.draw(&painter, QPoint());
}

void NodeView::paintRatings(QPainter &painter)
{
    int x = star_rect_.left();
    int y = star_rect_.top();
    FileNode *file = down_cast<FileNode *>(node_);
    int i = 0;
    for(; i < file->metadata().rating(); ++i)
    {
        painter.drawImage(x, y, star_black_image_);
        x += star_black_image_.width();
        x += SPACING;
    }

    for(; i < MAX_RATING; ++i)
    {
        painter.drawImage(x, y, star_white_image_);
        x += star_white_image_.width();
        x += SPACING;
    }
}

void NodeView::updateRating(int rating)
{
    FileNode *file = down_cast<FileNode *>(node_);
    ContentNode & node = file->metadata();
    if (node.rating() != rating)
    {
        node.mutable_rating() = rating;
    }
    else
    {
        --node.mutable_rating();
    }
    file->updateMetadata(node);

    // update the screen that is occupied by the widget.
    updateScreen();
}

/// Update layouts according to current view type and widget size.
/// The node view does not ask parent layout to allocate enough room
/// for it. The parent layout should make sure the spacing is enough
/// for node view to arrange its content. Node view leaves the flexibility
/// to its parent layout to arrange the node view items.
/// Get the whole widget size.
/// Arrange the image.
/// Arrange the name layout.
/// Arrange the title layout.
void NodeView::updateLayouts()
{
    if (node_ == 0)
    {
        return;
    }

    if (!isVisible())
    {
        return;
    }

    switch (view_mode_)
    {
    case LIST_VIEW:
        updateListViewLayout(rect());
        break;
    case DETAILS_VIEW:
        updateDetailsViewLayout(rect());
        break;
    case THUMBNAIL_VIEW:
        updateThumbnailViewLayout(rect());
        break;
    default:
        assert(false);
        break;
    }
}

/// Update layout for list view.
void NodeView::updateListViewLayout(QRect rect)
{
    int x = 0;
    int name_width = rect.width() * 229 / 600;
    fieldResized(NAME, x, name_width);

    x += name_width + SPACING;
    int type_width = rect.width() * 99 / 600;
    fieldResized(NODE_TYPE, x, type_width);

    x += type_width + SPACING;
    int size_width = rect.width() * 78 / 600;
    fieldResized(SIZE, x, size_width);

    x += size_width + SPACING;
    int time_width = rect.width() * 188 / 600;
    fieldResized(LAST_ACCESS_TIME, x, time_width);
}

/// Update layout for detailed view.
void NodeView::updateDetailsViewLayout(QRect rect)
{
    // Margin
    static const int SPACING = 6;
    QFont name_font(font().family(), 20);

    // Icon.
    if (isLayoutDirty())
    {
        image_ = ImageFactory::instance().image(node_, THUMBNAIL_MIDDLE);
    }

    // Load the rating image if necessary.
    if (star_black_image_.isNull())
    {
        star_black_image_.load(":/images/star_black.png");
    }

    if (star_white_image_.isNull())
    {
        star_white_image_.load(":/images/star_white.png");
    }

    int top = ((rect.height() - image_.height()) >> 1);
    icon_pos_.setX(SPACING);
    icon_pos_.setY(top);

    int left = image_.width() + 2 * SPACING;
    int width = rect.width() - (star_black_image_.width() + SPACING) * MAX_RATING - left;
    int height = 0;

    // Name layout.
    QRect name_rect(left, top, width, name_font.pixelSize());
    height += ui::calculateSingleLineLayout(name_layout_, name_font, node_->display_name(),
                                            Qt::AlignLeft|Qt::AlignTop, name_rect);

    if (node_->type() != NODE_TYPE_FILE)
    {
        return;
    }

    QFont desc_font(font().family(), 13, QFont::DemiBold);

    // Rating position.
    star_rect_.setRect(name_rect.right() + SPACING, top,
                       (star_black_image_.width() + SPACING) * 5,
                       star_black_image_.height());

    // Last access.
    FileNode *file = down_cast<FileNode *>(node_);

    QRect access_rect(left, top + height, width, desc_font.pixelSize());
    if (file->metadata().title().isEmpty())
    {
        QString access(QApplication::tr("Last Access: %1"));
        access = access.arg(node_->last_read());
        height += ui::calculateSingleLineLayout(last_access_layout_, desc_font, access,
            Qt::AlignLeft|Qt::AlignTop, access_rect);
    }
    else
    {
        QString title(QApplication::tr("Title : %1"));
        title = title.arg(file->metadata().title());
        height += ui::calculateSingleLineLayout(last_access_layout_, desc_font, title,
            Qt::AlignLeft|Qt::AlignTop, access_rect);
    }

    // Size
    QRect size_rect(name_rect.right() + SPACING,
                    access_rect.bottom() + desc_font.pixelSize(),
                    star_rect_.width(),
                    desc_font.pixelSize());
    QString size_string;
    sizeString(size_string);
    ui::calculateSingleLineLayout(size_layout_, desc_font, size_string,
                                  Qt::AlignLeft|Qt::AlignTop, size_rect);


    // Read time is disabled now. There is no enough space to display.
    // Or we can also change the height of each item.
    /*
    QRect time_rect(left, top + height, width, desc_font.pixelSize());
    QString time(tr("Read time: %1"));
    time = time.arg(file->metadata().read_time());
    height += ui::calculateSingleLineLayout(read_time_layout_, desc_font, time,
                                            Qt::AlignLeft|Qt::AlignTop, time_rect);
    */

    // Read count
    QRect count_rect(left, top + height, width, desc_font.pixelSize());
    if (file->metadata().authors().isEmpty())
    {
        QString count(QApplication::tr("Read count: %1"));
        count = count.arg(file->metadata().read_count());
        height += ui::calculateSingleLineLayout(read_count_layout_, desc_font, count,
            Qt::AlignLeft|Qt::AlignTop, count_rect);
    }
    else
    {
        QString author(QApplication::tr("Author : %1"));
        author = author.arg(file->metadata().authors());
        height += ui::calculateSingleLineLayout(read_count_layout_, desc_font, author,
            Qt::AlignLeft|Qt::AlignTop, count_rect);
    }

    // Progress
    QRect progress_rect(left, top + height, width, desc_font.pixelSize());
    QString string(QApplication::tr("Progress: %1"));
    if (file->metadata().progress().size() <= 0)
    {
        string = string.arg(QApplication::tr("Not Read"));
    }
    else
    {
        string = string.arg(file->metadata().progress());
    }

    // Append progress if it contains expired date.
    checkExpirationInfo(file, string);

    height += ui::calculateSingleLineLayout(progress_layout_, desc_font, string,
                                            Qt::AlignLeft|Qt::AlignTop, progress_rect);
}

/// Update layout for thumbnail view.
void NodeView::updateThumbnailViewLayout(QRect rect)
{
    // Margin
    // rect.adjust(GetHorMargin(view_mode_), 0, 0, 0);

    // icon position the center of the widget.
    QSize size = cms::thumbnailSize(THUMBNAIL_LARGE);
    if (isLayoutDirty())
    {
        if (node_->type() != NODE_TYPE_FILE &&
            node_->type() != NODE_TYPE_NOTE)
        {
            image_ = ImageFactory::instance().image(node_, THUMBNAIL_LARGE);
        }
        else
        {
            bool has_thumbnail = false;
            if (node_->type() == NODE_TYPE_FILE)
            {
                has_record_ = down_cast<FileNode*>(node_)->hasRecord();
                if (has_record_)
                {
                    has_thumbnail = down_cast<FileNode*>(node_)->thumbnail(image_);
                }

                // Check again.
                if (!has_thumbnail)
                {
                    has_thumbnail = checkImageThumbnail();
                }
            }
            else if (node_->type() == NODE_TYPE_NOTE)
            {
                image_ = down_cast<NoteNode*>(node_)->info().thumbnail();
                if (image_.isNull())
                {
                    image_ = ImageFactory::instance().image(node_, THUMBNAIL_LARGE);
                }
                else
                {
                    has_thumbnail = true;
                    has_record_ = true;
                }
            }
            if (!has_thumbnail)
            {
                image_ = ImageFactory::instance().image(node_, THUMBNAIL_LARGE);
            }
        }
    }

    static const int MARGIN = 5;
    icon_pos_.setX((rect.width() - image_.width()) >> 1);
    icon_pos_.setY(MARGIN + ((size.height() - image_.height()) >> 1));
    rect.adjust(MARGIN, size.height() + MARGIN, - MARGIN, 0);

    // Check if title is available or not.
    QFont name_font(font().family(), 19);
    ui::calculateMultiLinesLayout(name_layout_, name_font, node_->display_name(),
                                  Qt::AlignHCenter|Qt::AlignTop, rect, 2);
}

void NodeView::sizeString(QString & string)
{
    if (node_->type() != NODE_TYPE_FILE)
    {
        string.clear();
        return;
    }

    unsigned int file_size = 0;
    file_size = down_cast<FileNode *>(node_)->fileSize();
    if (file_size > 1024 * 1024 * 1024)
    {
        QString tmp("%1.%2%3");
        int gb = file_size >> 30;
        int left = ((file_size - (gb << 30)) * 10) >> 30;
        string = tmp.arg(gb).arg(left).arg(QApplication::tr("GB"));
    }
    else if (file_size > 1024 * 1024)
    {
        QString tmp("%1.%2%3");
        int mb = file_size >> 20;
        int left = ((file_size - (mb << 20)) * 10) >> 20;
        string = tmp.arg(mb).arg(left).arg(QApplication::tr("MB"));
    }
    else if (file_size > 1024)
    {
        QString tmp("%1%2");
        string = tmp.arg(file_size >> 10).arg(QApplication::tr("KB"));
    }
    else
    {
        QString tmp("%1%2");
        string = tmp.arg(file_size).arg(QApplication::tr("Bytes"));
    }
}

void NodeView::checkExpirationInfo(FileNode * file,
                                   QString & string)
{
    unsigned int seconds = file->expriedDate();
    if (seconds <= 0)
    {
        return;
    }

    bool returned = file->hasReturned();
    string.append(". ");
    QDateTime now = QDateTime::currentDateTime();
    QDateTime expired = QDateTime::fromTime_t(seconds);
    if (!returned)
    {
        if (expired > now)
        {
            int days = now.daysTo(expired);
            if (days > 1)
            {
                QString s(QApplication::tr("%1 days to expire."));
                s = s.arg(days);
                string.append(s);
            }
            else
            {
                const int SECONDS = 3600;
                int secs = now.secsTo(expired);
                if (secs > SECONDS)
                {
                    QString s(QApplication::tr("%1 hours %2 minutes to expire."));
                    s = s.arg(secs / SECONDS).arg((secs % SECONDS) / 60);
                    string.append(s);
                }
                else
                {
                    QString s(QApplication::tr("%1 minutes to expire."));
                    s = s.arg(secs / 60);
                    string.append(s);
                }
            }
        }
        else
        {
            string.append(QApplication::tr("Expired."));
        }
    }
    else
    {
        string.append(QApplication::tr("Returned."));
    }
}

/// Update the screen region that is occupied by the widget.
void NodeView::updateScreen()
{
    // Disable the screen update now.
    onyx::screen::instance().enableUpdate(false);

    // Update widget.
    update();

    // Make sure all content is copied to Qt framebuffer.
    QApplication::flush();

    // Now, enable screen update.
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);

}

}

}
