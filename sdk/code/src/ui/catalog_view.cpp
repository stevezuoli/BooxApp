#include <QKeyEvent>
#include <QPainter>
#include <QVector>
#include "onyx/sys/sys.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/catalog_view.h"
#include "onyx/sys/platform.h"

namespace ui
{

const QString CatalogView::LEFT         = "left";
const QString CatalogView::RIGHT        = "right";
const QString CatalogView::UP           = "up";
const QString CatalogView::DOWN         = "down";
const QString CatalogView::RECYCLE_LEFT = "r-left";
const QString CatalogView::RECYCLE_RIGHT= "r-right";
const QString CatalogView::RECYCLE_UP   = "r-up";
const QString CatalogView::RECYCLE_DOWN = "r-down";

static const int FLAG_STOP = -2;
static const int TITLE_BAR_HEIGHT = 30;

CatalogView::CatalogView(Factory * factory, QWidget *parent)
        : QWidget(parent)
        , layout_(this)
        , factory_(factory)
        , left_margin_(0)
        , top_margin_(0)
        , right_margin_(0)
        , bottom_margin_(0)
        , spacing_(0)
        , checked_(true)
        , policy_(NeighborFirst)
        , show_border_(false)
        , fixed_grid_(false)
        , auto_focus_(false)
        , sub_item_checked_exclusive_(true)
        , size_(200, 150)
        , bk_color_(Qt::white)
        , fixed_size_(false)
#ifdef BUILD_WITH_TFT
        , arrange_policy_(ROW_FIRST)
#endif
{
    createLayout();
}

CatalogView::~CatalogView()
{
    // clearDatas(data());
}

void CatalogView::setSearchPolicy(int policy)
{
    policy_ = policy;
}

int CatalogView::searchPolicy()
{
    return policy_;
}

void CatalogView::createLayout()
{
    layout_.setContentsMargins(left_margin_, top_margin_, right_margin_, bottom_margin_);
    layout_.setSpacing(0);
    layout_.setVerticalSpacing(0);
}

void CatalogView::calculateLayout(int &rows, int &cols)
{
    QSize s = preferItemSize();
    if (s.height() <= 0)
    {
        rows = 1;
    }
    else
    {
        rows = (rect().height() - top_margin_ - bottom_margin_) / s.height();
    }

    if (s.width() <= 0)
    {
        cols = 1;
    }
    else
    {
        cols = (rect().width() - left_margin_ - right_margin_) / s.width();
    }
}

void CatalogView::setMargin(int left, int top, int right, int bottom)
{
    left_margin_ = left;
    top_margin_ = top;
    right_margin_ = right;
    bottom_margin_ = bottom;
    layout_.setContentsMargins(left, top, right, bottom);
}

void CatalogView::margin(int *left, int *top, int *right, int *bottom)
{
    if (left)
    {
        *left = left_margin_;
    }

    if (top)
    {
        *top = top_margin_;
    }

    if (right)
    {
        *right = right_margin_;
    }

    if (bottom)
    {
        *bottom = bottom_margin_;
    }
}

void CatalogView::setTitle(const QString &title)
{
    if (!title.isEmpty())
    {
        setMargin(0, TITLE_BAR_HEIGHT, 0, 0);
        title_ = title;
        update();
    }
}

void CatalogView::setSpacing(int s)
{
    spacing_ = s;
    layout_.setSpacing(spacing_);
    layout_.setVerticalSpacing(spacing_);
}

#ifdef  BUILD_WITH_TFT
void CatalogView::setArrangePolicy(enum ArrangePolicy policy)
{
    if (policy != ROW_FIRST && policy != COLUMN_FIRST)
    {
        qDebug() << "ArrangePolicy not correct " << policy;
        return;
    }
    arrange_policy_ = policy;
}
#endif

/// Arrange all sub widgets according to current widget size.
/// Ensure we fill the grid layout. Widget level, different from associateData,
/// associateData will update all associated data.
void CatalogView::arrangeSubWidgets()
{
    // If rows or columns changed.
    int rows = 0;
    int cols = 0;
    calculateLayout(rows, cols);

    ContentView * p = 0;
    // Always update.
    //if (paginator().rows() != rows || paginator().cols() != cols || isFixedGrid())
    {
        // Remove all widgets from layout.
        for (int i = 0; i < sub_items_.size(); ++i)
        {
            p = sub_items_.at(i);
            layout_.removeWidget(p);
            p->hide();
        }
        sub_items_.clear();
        if (isFixedGrid())
        {
            rows = paginator().rows();
            cols = paginator().cols();
        }
        paginator().setGrid(rows, cols);
        paginator().resize(rows * cols);
    }

#ifdef BUILD_WITH_TFT
    if (arrange_policy_ == COLUMN_FIRST)
    {
        addSubWidgetByColumn(rows, cols);
    }
    else
#endif
    {
        addSubWidgetByRow(rows, cols);
    }
}

void CatalogView::addSubWidgetByRow(int rows, int cols)
{
    ContentView * p = 0;
    int index = 0;

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if (index <  sub_items_.size())
            {
                p = sub_items_.at(index);
                layout_.addWidget(p, i, j);
                p->show();
            }
            else
            {
                p = createSubItem();
                layout_.addWidget(p, i, j);
            }
            ++index;
        }
    }
}

void CatalogView::addSubWidgetByColumn(int rows, int cols)
{
    ContentView * p = 0;
    int index = 0;

    for (int j = 0; j < cols; ++j)
    {
        for (int i = 0; i < rows; ++i)
        {
            if (index <  sub_items_.size())
            {
                p = sub_items_.at(index);
                layout_.addWidget(p, i, j);
                p->show();
            }
            else
            {
                p = createSubItem();
                layout_.addWidget(p, i, j);
            }
            ++index;
        }
    }
}

/// Associate data with sub widget to display content.
void CatalogView::associateData(bool force)
{
    // find the last item in the currentInvisibleParent() that will be shown in the last grid
    int count = data().size() - paginator().first_visible();
    count = std::min(count, sub_items_.size());
    for (int i = 0; i < count; ++i)
    {
        sub_items_.at(i)->updateData(data().at(paginator().first_visible() + i), force);
    }
    for (int i = count; i < sub_items_.size(); ++i)
    {
        sub_items_.at(i)->updateData(0, force);
    }
}

void CatalogView::associateEmptyData()
{
    for (int i = 0; i < sub_items_.size(); ++i)
    {
        sub_items_.at(i)->updateData(0);
    }
}

/// Arrange everything, including sub widgets, associate data, focus and position.
/// It does not reset paginator.
void CatalogView::arrangeAll(bool force)
{
    arrangeSubWidgets();
    associateData(force);

    if (isVisible())
    {
        if (sub_items_.size() > 0 && sub_items_.front()->data() && auto_focus_)
        {
            sub_items_.front()->setFocus();
        }
        broadcastPositionSignal();
        onyx::screen::watcher().enqueue(parentWidget(), onyx::screen::ScreenProxy::GC);
    }
}

/// Select the content and broadcast position signal.
bool CatalogView::select(OData *d)
{
    int pos = data().indexOf(ODataPtr(d));
    if (pos < 0)
    {
        return false;
    }

    int page = pos / paginator().items_per_page();
    gotoPage(page + 1);
    int offset = pos - page * paginator().items_per_page();
    setFocusTo(offset / paginator().cols(), offset % paginator().cols());
    onyx::screen::watcher().enqueue(parentWidget(), onyx::screen::ScreenProxy::GC);
    return true;
}

int CatalogView::moveLeft(int current)
{
    if (col(current) == 0)
    {
        if (neighborFirst())
        {
            if (searchNeighbors(LEFT) || searchNeighbors(RECYCLE_RIGHT))
            {
                return FLAG_STOP;
            }
        }
        if (horAutoRecycle())
        {
            return current = paginator().last_visible();
        }
        emit outOfLeft(this, current / paginator().cols(),
                current % paginator().cols());
    }
    return --current;
}

int CatalogView::moveRight(int current)
{
    if (atRightEdge(current))
    {
        if (neighborFirst())
        {
            if (searchNeighbors(RIGHT) ||searchNeighbors(RECYCLE_LEFT))
            {
                return FLAG_STOP;
            }
        }
        if (horAutoRecycle())
        {
            return current = paginator().first_visible();
        }
        emit outOfRight(this, current / paginator().cols(),
                current % paginator().cols());
    }
    return ++current;
}

int CatalogView::moveUp(int current)
{
    if (row(current) == 0)
    {
        if (neighborFirst())
        {
            if (searchNeighbors(UP) || searchNeighbors(RECYCLE_DOWN))
            {
                return FLAG_STOP;
            }
        }
        if (verAutoRecycle())
        {
            return current = paginator().last_visible();
        }
        emit outOfUp(this, current / paginator().cols(),
                current % paginator().cols());
    }
    return current - paginator().cols();
}

int CatalogView::moveDown(int current)
{
    if (atDownEdge(current))
    {
        if (neighborFirst())
        {
            if (searchNeighbors(DOWN) || searchNeighbors(RECYCLE_UP))
            {
                return FLAG_STOP;
            }
        }
        if (verAutoRecycle())
        {
            return current = paginator().first_visible();
        }
        emit outOfDown(this, current / paginator().cols(),
                current % paginator().cols());
        return current + paginator().cols();
    }
    else
    {
        // not at edge, we need to find the last view contains data.
        int last = current + paginator().cols();
        for(int index = last; index >= 0; --index)
        {
            if (sub_items_.at(index)->data())
            {
                return index;
            }
        }
        return last;
    }
}

bool CatalogView::atDownEdge(int current)
{
    current += paginator().cols();
    int r = row(current);
    for(int i = r * cols(); i < (r + 1) * cols(); ++i)
    {
        if (i >= visibleSubItems().size())
        {
            return true;
        }
        if (visibleSubItems().at(i)->data())
        {
            return false;
        }
    }
    return true;
}

bool CatalogView::atRightEdge(int current)
{
    if (col(current) >= cols() - 1)
    {
        return true;
    }

    ++current;
    if (current >= visibleSubItems().size())
    {
        return true;
    }
    if (visibleSubItems().at(current)->data())
    {
        return false;
    }
    return true;
}

int CatalogView::row(int index)
{
    return index / paginator().cols();
}

int CatalogView::col(int index)
{
    return index % paginator().cols();
}

bool CatalogView::neighborFirst()
{
    return policy_ & NeighborFirst;
}

bool CatalogView::horAutoRecycle()
{
    return policy_ & AutoHorRecycle;
}

bool CatalogView::verAutoRecycle()
{
    return policy_ & AutoVerRecycle;
}

Paginator & CatalogView::paginator()
{
    return paginator_;
}

bool CatalogView::gotoPage(const int p)
{
    if (paginator().jump(p-1))
    {
        arrangeAll();
        setFocusTo(0, 0);
        return true;
    }
    return false;
}

/// Set data. It's caller's responsibility to maintain data.
void CatalogView::setData(const ODatas &list, bool force)
{
    datas_ = list;
    resetPaginator(true);
    arrangeAll(force);
}

ODatas & CatalogView::data()
{
    return datas_;
}

/// \index The absolute index in data list.
void CatalogView::setFocusTo(const int row, const int col)
{
    int index = row * paginator().cols() + col;
    if (index >= 0 && index < sub_items_.size())
    {
        setChecked();
        if (sub_items_.at(index)->data())
        {
            sub_items_.at(index)->setFocus();
        }
    }
}

void CatalogView::setFocusToLast()
{
    for(int index = sub_items_.size() - 1; index >= 0; --index)
    {
        if (sub_items_.at(index)->data())
        {
            sub_items_.at(index)->setFocus();
            return;
        }
    }
}

ContentView* CatalogView::focusItem()
{
    QWidget *wnd = focusWidget();
    foreach(ContentView *item, sub_items_)
    {
        if (item == wnd)
        {
            return item;
        }
    }
    return 0;
}

void CatalogView::setCheckedTo(const int row, const int col)
{
    int index = row * paginator().cols() + col;
    if (index >= 0 && index < sub_items_.size())
    {
        setChecked();
        ContentView * item_to_check = sub_items_.at(index);
        if (item_to_check->data())
        {
            item_to_check->setChecked(true);
        }

        // set others to unchecked if exclusive.
        if (sub_item_checked_exclusive_)
        {
            foreach(ContentView *item, sub_items_)
            {
                if (item != item_to_check && item->data())
                {
                    item->setChecked(false);
                }
            }
        }
    }
}

void CatalogView::mousePressEvent ( QMouseEvent *event )
{
}

void CatalogView::mouseReleaseEvent ( QMouseEvent *event )
{
}

void CatalogView::setChecked(bool checked)
{
    if (isChecked() != checked)
    {
        checked_ = checked;
        update();
    }
}

bool CatalogView::goNext()
{
    if (paginator().next())
    {
        arrangeAll(true);
        setFocusTo(0, 0);
        return true;
    }
    return false;
}

bool CatalogView::hasNext()
{
    return paginator().isNextEnable();
}

bool CatalogView::hasPrev()
{
    return paginator().isPrevEnable();
}

bool CatalogView::goPrev()
{
    if (paginator().prev())
    {
        arrangeAll(true);
        setFocusToLast();
        return true;
    }
    return false;
}

void CatalogView::keyReleaseEvent ( QKeyEvent *ke )
{
    ke->ignore();
    switch ( ke->key())
    {
    case Qt::Key_PageDown:
        {
            if (!goNext())
            {
                emit keyRelease(this, ke);
                return;
            }
        }
        break;
    case Qt::Key_PageUp:
        {
            if (!goPrev())
            {
                emit keyRelease(this, ke);
                return;
            }
        }
        break;
    default:
        return;
    }
    ke->accept();
}

void CatalogView::keyPressEvent(QKeyEvent*e )
{
    QWidget::keyPressEvent(e);
}

/// This function returns widget that has new focus.
/// If it returns itself, we need to refresh whole screen.
QWidget * CatalogView::moveFocus(int index)
{
    if (index > data().size() || index == FLAG_STOP)
    {
        return 0;
    }

    if (index >= paginator().items_per_page())
    {
        goNext();
        return 0;
    }
    else if ( index < 0)
    {
        goPrev();
        return 0;
    }
    else
    {
        ContentView * p = sub_items_.at(index);
        if (p->data())
        {
            p->setFocus();
            return p;
        }
    }
    return 0;
}

void CatalogView::changeEvent ( QEvent *event )
{
}

void CatalogView::resizeEvent ( QResizeEvent * event )
{
    QWidget::resizeEvent ( event );
    arrangeAll();
}

void CatalogView::paintEvent ( QPaintEvent * event )
{
    QPainter painter(this);
    if (isChecked() && hasBorder())
    {
        int pen_width = 2;
        QPen pen;
        pen.setWidth(pen_width);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        painter.drawRoundedRect(rect().adjusted(pen_width, pen_width, -pen_width , -pen_width), 5, 5);
    }
    if (!title_.isEmpty())
    {
        QPainterPath roundRectPath;
        QRect rc = QRect(0, 0, width(), TITLE_BAR_HEIGHT);
        static const int radius = 20;

        roundRectPath.moveTo(rc.bottomRight());
        roundRectPath.lineTo(rc.right(), rc.top() + radius);
        QRect r1(rc.right() - radius, rc.top(), radius, radius);
        roundRectPath.arcTo(r1, 0, 90);
        roundRectPath.lineTo(rc.left() + radius, rc.top());
        QRect r2(rc.left(), rc.top(), radius, radius);
        roundRectPath.arcTo(r2, 90, 90);
        roundRectPath.lineTo(rc.bottomLeft());
        roundRectPath.lineTo(rc.bottomRight());

        QBrush brush(Qt::white);
        brush.setColor(Qt::black);
        painter.fillPath(roundRectPath, brush);
        painter.drawPath(roundRectPath);
        painter.setPen(Qt::white);
        QFont font(QApplication::font());
        font.setPointSize(ui::defaultFontPointSize() - 4);
        painter.setFont(font);
        painter.drawText(rc.adjusted(10, 0, 0, 0), Qt::AlignLeft|Qt::AlignVCenter, title_);
    }
}

ContentView* CatalogView::createSubItem()
{
    ContentView * instance = 0;
    if (!factory_)
    {
        static Factory s_factory;
        instance = s_factory.createView(this, sub_item_type_);
    }
    else
    {
        instance = factory_->createView(this, sub_item_type_);
    }
    instance->setBkColor(bk_color_);
    connect(instance, SIGNAL(activated(ContentView*,int)), this, SLOT(onItemActivated(ContentView *,int)));
    connect(instance, SIGNAL(keyRelease(ContentView*, QKeyEvent*)), this, SLOT(onItemKeyRelease(ContentView *, QKeyEvent*)));
    connect(instance, SIGNAL(mouse(QPoint, QPoint)), this, SLOT(onMouseMoved(QPoint, QPoint)));
    connect(instance, SIGNAL(focusChanged(ContentView*)), this, SLOT(onItemFocusChanged(ContentView*)));
    sub_items_.push_back(instance);

    QSize s = preferItemSize();
    if (s.height() <= 0)
    {
        if (fixed_size_)
        {
            // instance->setFixedWidth(s.width());
        }
    }
    else if (s.width() <= 0)
    {
        if (fixed_size_)
        {
            // instance->setFixedHeight(s.height());
        }
    }
    else
    {
        if (fixed_size_)
        {
            int w = (rect().width() - left_margin_ - right_margin_) /  cols();
            int h = (rect().height() - top_margin_ - bottom_margin_) / rows();
            instance->setFixedSize(QSize(w, h));
            //instance->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        }
    }
    return instance;
}

void CatalogView::setFixedGrid(int rows, int cols)
{
    fixed_grid_ = true;
    paginator().setGrid(rows, cols);
}

bool CatalogView::isFixedGrid()
{
    return fixed_grid_;
}

QSize CatalogView::preferItemSize()
{
    if (isFixedGrid())
    {
        return QSize(width() / paginator().cols(), height() / paginator().rows());
    }
    return size_;
}

void CatalogView::setPreferItemSize(const QSize &size, bool fixed)
{
    fixed_size_ = fixed;
    // Always update.
    // if (size_ != size)
    {
        size_ = size;
        arrangeAll();
    }
}

int CatalogView::rows()
{
    return paginator().rows();
}

int CatalogView::cols()
{
    return paginator().cols();
}

void CatalogView::setSubItemType(const QString &type)
{
    sub_item_type_ = type;
}

void CatalogView::setSubItemBkColor(Qt::GlobalColor color)
{
    bk_color_ = color;
    foreach(ContentView * view, visibleSubItems())
    {
        view->setBkColor(color);
    }
}

void CatalogView::broadcastPositionSignal()
{
    emit positionChanged(paginator().currentPage(), paginator().pages());
}

void CatalogView::resetPaginator(bool sync_layout)
{
    if (sync_layout && !isFixedGrid())
    {
        int rows = 0;
        int cols = 0;
        calculateLayout(rows, cols);
        paginator().reset(0, rows * cols, data().size());
        paginator().setGrid(rows, cols);
    }
    else
    {
        paginator().reset(0, paginator().rows() * paginator().cols(), data().size());
    }
}

void CatalogView::onItemActivated(ContentView *item, int user_data)
{
    if (item == 0 || item->data() == 0)
    {
        return;
    }
    setChecked();
    emit itemActivated(this, item, user_data);
}

void CatalogView::onItemFocusChanged(ContentView *item)
{
    onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
}

void CatalogView::onItemKeyRelease(ContentView *item, QKeyEvent *key)
{
    QWidget * p = 0;
    int index = visibleSubItems().indexOf(item);

    // check index at first.

    switch (key->key())
    {
    case Qt::Key_Left:
        {
            index = moveLeft(index);
        }
        break;
    case Qt::Key_Right:
        {
            index = moveRight(index);
        }
        break;
    case Qt::Key_Up:
        {
            index = moveUp(index);
        }
        break;
    case Qt::Key_Down:
        {
            index = moveDown(index);
        }
        break;
    default:
        break;
    }
    p = moveFocus(index);
    if (p)
    {
//        onyx::screen::watcher().enqueue(item, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
//        onyx::screen::watcher().enqueue(p, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
        onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
    }
}

void CatalogView::onMouseMoved(QPoint last, QPoint current)
{
    int direction = sys::SystemConfig::direction(last, current);

    if (direction > 0)
    {
        goNext();
    }
    else if (direction < 0)
    {
        goPrev();
    }
}

QString CatalogView::invert(const QString &type)
{
    if (type == LEFT)
    {
        return RIGHT;
    }
    else if (type == RIGHT)
    {
        return LEFT;
    }
    else if (type == UP)
    {
        return DOWN;
    }
    else if (type == DOWN)
    {
        return UP;
    }
    else if (type == RECYCLE_RIGHT)
    {
        return RECYCLE_LEFT;
    }
    else if (type == RECYCLE_LEFT)
    {
        return RECYCLE_RIGHT;
    }
    else if (type == RECYCLE_UP)
    {
        return RECYCLE_DOWN;
    }
    else if (type == RECYCLE_DOWN)
    {
        return RECYCLE_UP;
    }
    return QString();
}

void CatalogView::setNeighbor(CatalogView *neighbor, const QString &type)
{
    // caller should check.
    addNeighbor(type, neighbor);
    neighbor->addNeighbor(invert(type), this);
}

bool CatalogView::removeNeighbor(CatalogView *neighbor, const QString& type)
{
    CatalogViews & views = neighbors(type);
    int index = views.indexOf(neighbor);
    if (index < 0)
    {
        return false;
    }
    views.erase(views.begin() + index);
    return neighbor->removeNeighbor(this, invert(type));
}

ContentView* CatalogView::findShortestItem(CatalogView *view,
                                           QWidget *target,
                                           int & dist,
                                           const QPoint &offset)
{
    // visit all and check the shortest item.
    int tmp = INT_MAX;
    ContentView *ret = 0;
    if (view->visibleSubItems().size() > 0)
    {
        ret = view->visibleSubItems().first();
    }
    foreach(ContentView * item, view->visibleSubItems())
    {
        int d = 0;
        QPoint target_center = ui::globalCenter(target);
        QPoint item_center = ui::globalCenter(item);
        target_center += offset;
        d = ui::distance(target_center, item_center);

        if (d < tmp && item->data())
        {
            tmp = d;
            ret = item;
        }
    }
    dist = tmp;
    return ret;
}

CatalogView::CatalogViews & CatalogView::neighbors(const QString &type)
{
    return neighbors_[type];
}

bool CatalogView::addNeighbor(const QString &type, CatalogView *neighbor)
{
    CatalogViews & views = neighbors(type);
    if (views.indexOf(neighbor) >= 0)
    {
        return false;
    }
    views.push_back(neighbor);
    return true;
}

bool CatalogView::searchNeighbors(const QString &type)
{
    // set focus to shortest item.
    CatalogViews & views = neighbors(type);
    if (views.isEmpty())
    {
        return false;
    }
    QPoint offset;
    int dist = INT_MAX;
    ContentView * ret = 0;

    QWidget * wnd = focusItem();
    if (wnd == 0)
    {
        return false;
    }

    foreach(CatalogView *view, views)
    {
        if (!view->isVisible())
        {
            continue;
        }

        // Adjust offset if necessary.
        if (type == RECYCLE_LEFT)
        {
            offset.setX(view->mapToGlobal(QPoint()).x() - wnd->mapToGlobal(QPoint()).x());
        }
        else if (type == RECYCLE_RIGHT)
        {
            offset.setX(view->mapToGlobal(QPoint()).x() + view->width());
        }
        else if (type == RECYCLE_UP)
        {
            offset.setY(-view->height() - height());
        }
        else if (type == RECYCLE_DOWN)
        {
            offset.setY(view->mapToGlobal(QPoint()).y() + view->height());
        }

        int tmp = 0;
        ContentView * item = findShortestItem(view, wnd, tmp, offset);
        if (dist > tmp)
        {
            dist = tmp;
            ret = item;
        }
    }
    if (ret)
    {
        ret->setFocus();
    }
    return true;
}

void CatalogView::setColumnStretch(int col, int stretch)
{
    layout_.setColumnStretch(col, stretch);
}

void CatalogView::setRowStretch(int row, int stretch)
{
    layout_.setRowStretch(row, stretch);
}

void CatalogView::enableAutoFocus(bool enable)
{
    auto_focus_ = enable;
}

void CatalogView::setSubItemCheckedExclusive(bool value)
{
    sub_item_checked_exclusive_ = value;
}

}
