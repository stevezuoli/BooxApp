#include "onyx/ui/status_bar_item_viewport.h"

namespace ui
{

static QRect s_bounding;

StatusBarItemViewport::StatusBarItemViewport(QWidget *parent)
    : StatusBarItem(VIEWPORT, parent)
    , total_(1)
    , current_(1)
{
    setFixedWidth(30);
}

StatusBarItemViewport::~StatusBarItemViewport(void)
{
}

void StatusBarItemViewport::setViewport(const QRect & parent,
                                        const QRect & child,
                                        int current_column,
                                        int total)
{
    total_ = total;
    current_ = current_column;
    int columns_height = 1;
    if (total > 1)
    {
        columns_height = 9;
    }

    parent_ = parent;
    child_ = child;
    s_bounding = parent | child;

    const int w = static_cast<double>(width() - 2);
    const int h = static_cast<double>(height() - columns_height);
    double scale = std::min(w / static_cast<double>(s_bounding.width()),
                            h / static_cast<double>(s_bounding.height()));

    int left = (w - s_bounding.width() * scale) / 2 + 1;
    int top = (h - s_bounding.height() * scale) / 2 + columns_height;

    QRect r = QRect(left, top, s_bounding.width() * scale, s_bounding.height() * scale);
    parent_ = QRect(left + (parent_.left() - s_bounding.left()) * scale,
                    top + (parent_.top() - s_bounding.top()) * scale,
                    parent_.width() * scale,
                    parent_.height() * scale);
    child_ = QRect(left + (child_.left() - s_bounding.left()) * scale,
                    top + (child_.top() - s_bounding.top()) * scale,
                    child_.width() * scale,
                    child_.height() * scale);
    s_bounding = r;
    update();
}

void StatusBarItemViewport::paintEvent(QPaintEvent *pe)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    if (total_ > 1)
    {
        int r = 2;
        int spacing = 2;
        int x = (width() - total_ * (2 * r + spacing)) / 2;
        int y = 1;
        for(int i = 0; i < total_; ++i)
        {
            QPainterPath path;
            path.addEllipse(x + i * (2 * r + 2), y, 2 * r, 2 * r);
            path.closeSubpath();
            painter.fillPath(path, Qt::lightGray);
        }
        for(int i = 0; i < current_; ++i)
        {
            QPainterPath path;
            path.addEllipse(x + i * (2 * r + 2), y, 2 * r, 2 * r);
            path.closeSubpath();
            painter.fillPath(path, Qt::black);
        }
    }

    painter.fillRect(parent_, QBrush(QColor(100, 100, 100, 200)));
    painter.fillRect(child_, QBrush(QColor(255, 255, 255, 200)));
    painter.setPen(Qt::white);
    painter.drawRect(s_bounding.adjusted(-1, -1, 0, -1));
}

}
