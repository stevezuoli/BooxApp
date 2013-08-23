#include <algorithm>
#include "header_button.h"
#include "onyx/ui/text_layout.h"


static const int SPACING = 5;

QIcon HeaderButton::ascending_icon_;
QIcon HeaderButton::descending_icon_;
QPoint HeaderButton::icon_pos_;

HeaderButton::HeaderButton(QWidget *parent, const Field id)
    : QWidget(parent)
    , field_(id)
    , order_(NO_ORDER)
    , is_dirty_(true)
{
    setAutoFillBackground(false);

    if (ascending_icon_.isNull())
    {
        ascending_icon_.addFile(":/images/up_arrow.png");
    }

    if (descending_icon_.isNull())
    {
        descending_icon_.addFile(":/images/down_arrow.png");
    }
}

HeaderButton::~HeaderButton()
{
}

void HeaderButton::setText(const QString &title)
{
    if (title_layout_.text() != title)
    {
        is_dirty_ = true;
        title_layout_.setText(title);
    }
}

void HeaderButton::setOrder(SortOrder order)
{
    if (order_ != order)
    {
        order_ = order;
        update();
    }
}

/// TODO, change it to better rendering.
void HeaderButton::paintEvent(QPaintEvent *e)
{
    updateLayout();
    QPainter p(this);

    p.fillRect(rect(), QBrush(Qt::black));

    // Draw text.
    QPen pen(Qt::white);
    p.setPen(pen);
    title_layout_.draw(&p, QPoint());

    // Draw icon.
    if (order_ == ASCENDING)
    {
        ascending_icon_.paint(&p, QRect(icon_pos_, iconActualSize()), Qt::AlignCenter);
    }
    else if (order_ == DESCENDING)
    {
        descending_icon_.paint(&p, QRect(icon_pos_, iconActualSize()), Qt::AlignCenter);
    }
}

void HeaderButton::mousePressEvent(QMouseEvent *e)
{
    e->accept();
}

void HeaderButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (order_ == NO_ORDER ||
        order_ == DESCENDING)
    {
        emit clicked(field(), ASCENDING);
    }
    else
    {
        emit clicked(field(), DESCENDING);
    }
    e->accept();
}

void HeaderButton::resizeEvent(QResizeEvent *re)
{
    is_dirty_ = true;
    updateLayout();
    emit sizeChanged(field_, re->size());
}

void HeaderButton::updateLayout()
{
    QFont f(font());
    f.setPixelSize(20);

    QRect rc = rect();
    if (is_dirty_)
    {
        ui::calculateSingleLineLayout(title_layout_, f, title_layout_.text(),
                                      Qt::AlignVCenter|Qt::AlignHCenter, rc, Qt::ElideLeft);
        layout_width_ = static_cast<int>(title_layout_.lineAt(0).naturalTextWidth());
        layout_height_ = static_cast<int>(title_layout_.boundingRect().height());
        int x = ((rc.width() - layout_width_ ) >> 1);
        int y = ((rc.height() - layout_height_) >> 1);
        title_layout_.setPosition(QPoint(x, y));
    }

    // icon.
    if (order_ != NO_ORDER)
    {
        icon_pos_.rx() = (rect().width() - iconActualSize().width() - SPACING);
        icon_pos_.ry() = ((rect().height() - iconActualSize().height()) >> 1);
    }

    is_dirty_ = false;
}

QSize HeaderButton::iconActualSize()
{
    return ascending_icon_.actualSize(QSize(1024, 1024));
}

