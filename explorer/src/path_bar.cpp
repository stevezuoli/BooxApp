
#include <QtGui/QtGui>
#include "path_bar.h"
#include "onyx/ui/text_layout.h"

static const int SPACING = 3;
static const int MARGIN  = 8;
static const int MAX_WIDTH = 200;
static const int PATHBAR_HEIGHT = 50;

PathBarButton::PathBarButton(QWidget *parent)
: QWidget(parent)
, checked_(false)
, layout_dirty_(true)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QFont f(font().family(), 18, QFont::Bold);
    setFont(f);
}

PathBarButton::PathBarButton(QWidget *parent, const QString &path)
: QWidget(parent)
, checked_(false)
, image_(new QImage(path))
{
    setFixedWidth(image_->width() + MARGIN * 2);
}

PathBarButton::~PathBarButton()
{
}

QSize PathBarButton::sizeHint() const
{
    QFontMetrics fm(font());
    int w = fm.width(text()) + MARGIN * 2;
    int h = static_cast<int>(fm.height());
    return QSize(w, h);
}

QSize PathBarButton::minimumSizeHint() const
{
    return sizeHint();
}

void PathBarButton::setText(const QString &string)
{
    if (text_ != string)
    {
        text_ = string;
        layout_.setText("");
        updateGeometry();
    }
}

void PathBarButton::setChecked(bool check)
{
    checked_ = check;
}

void PathBarButton::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void PathBarButton::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if (!isChecked())
    {
        emit clicked(this);
    }
}

bool PathBarButton::event(QEvent * event)
{
    return QWidget::event(event);
}

void PathBarButton::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);

    if ((layout_.text() != text_ || layout_dirty_) && !image_)
    {
        ui::calculateSingleLineLayout(layout_, font(), text_,
            Qt::AlignHCenter|Qt::AlignVCenter, rect().adjusted(MARGIN, 0, -MARGIN, 0), Qt::ElideLeft);
    }

    QPainterPath roundRectPath;
    QRect rc = rect().adjusted(1, 1, -1, -1);
    static const int radius = 8;

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
    QPen pen(QColor(0, 0, 0));
    pen.setWidth(2);
    if (isChecked())
    {
        brush.setColor(Qt::black);
        painter.fillPath(roundRectPath, brush);
        painter.drawPath(roundRectPath);
        pen.setColor(Qt::white);
    }
    else
    {
        painter.fillPath(roundRectPath, brush);
        painter.drawPath(roundRectPath);
    }

    if (image_)
    {
        int x = ((rect().width() - image_->width()) >> 1);
        int y = ((rect().height() - image_->height()) >> 1);
        painter.drawImage(x, y, *image_);
    }
    else
    {
        painter.setPen(pen);
        layout_.draw(&painter, QPoint());
    }
}

void PathBarButton::resizeEvent(QResizeEvent * event)
{
    if (!image_)
    {
        layout_dirty_ = true;
    }
}


PathBar::PathBar(QWidget *parent, ModelTree & model)
: QWidget(parent)
, layout_(this)
, home_(0, ":/images/desktop.png")
, branch_(0)
, close_button_("", this)
, tree_(model)
, pathbar_mode_(true)
{
    setFocusPolicy(Qt::NoFocus);
    setFixedHeight(PATHBAR_HEIGHT);
    createLayout();
}

PathBar::~PathBar()
{
}

bool PathBar::updateAll()
{
    items_.clear();

    FolderNode *folder = tree_.folderNode(tree_.currentNode());
    if (folder && folder->canGoUp())
    {
        QDir dir(folder->absolute_path());
        int root_size = folder->root().size();
        items_.push_front(dir.dirName());

        while (dir.cdUp())
        {
            QString abs = dir.absolutePath();
            if (!dir.dirName().isEmpty() && abs.size() > root_size)
            {
                items_.push_front(dir.dirName());
            }
            else
            {
                break;
            }
        }
    }

    // Check the filters.
    bool has_filter = !tree_.currentNode()->name_filters().isEmpty();
    close_button_.setVisible(has_filter);
    setPathbarMode(!has_filter);

    if (tree_.currentNode()->type() == NODE_TYPE_ROOT)
    {
        branch_.setVisible(false);
    }
    else
    {
        branch_.setVisible(true);
        branch_.setText(tree_.currentNode()->display_name());
        branch_.setName(tree_.currentNode()->name());
    }
    updateLayout();
    update();
    return true;
}

void PathBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    int x1 = rect().left();
    int x2 = rect().right();
    int y = rect().bottom();
    p.drawLine(x1, y, x2, y);

    y -= 2;
    p.drawLine(x1, y, x2, y);
}

void PathBar::onButtonClicked(PathBarButton *button)
{
    // find the index.
    if (button == &home_)
    {
        emit desktopClicked();
        return;
    }
    else if (button == &branch_)
    {
        emit branchClicked(branch_.name());
        return;
    }

    FolderNode *folder = tree_.folderNode(tree_.currentNode());
    if (folder)
    {
        QDir dir(folder->root());
        for(Buttons::iterator it = buttons_.begin(); it != buttons_.end(); ++it)
        {
            dir.cd((*it)->text());
            if ((*it).get() == button)
            {
                break;
            }
        }
        emit pathClicked(dir.absolutePath());
    }
}

void PathBar::onCloseClicked()
{
    emit closeClicked();
}

void PathBar::createLayout()
{
    layout_.setSpacing(0);
    layout_.setContentsMargins(0, 0, 5, 3);

    // home button.
    connect(&home_, SIGNAL(clicked(PathBarButton *)), this, SLOT(onButtonClicked(PathBarButton *)));
    layout_.addWidget(&home_, 0, Qt::AlignLeft);

    connect(&branch_, SIGNAL(clicked(PathBarButton *)), this, SLOT(onButtonClicked(PathBarButton *)));
    layout_.addWidget(&branch_, 0, Qt::AlignLeft);

    layout_.addStretch(0);
    layout_.addWidget(&close_button_);

    close_button_.setIcon(QIcon(":/images/close.png"));
    close_button_.useDefaultHeight();
    close_button_.setFocusPolicy(Qt::NoFocus);
    connect(&close_button_, SIGNAL(clicked()), this, SLOT(onCloseClicked()));
    close_button_.hide();
}

void PathBar::updateLayout()
{
    int max = maxItems();
    int i = 0, j = 0;
    for(j = buttons_.size(), i = buttons_.size();
        i < max, j < items_.count();
        ++i, ++j)
    {
        ButtonPtr button(new PathBarButton(0));
        connect(button.get(), SIGNAL(clicked(PathBarButton *)), this,
            SLOT(onButtonClicked(PathBarButton *)));

        button->setChecked(false);
        // button->setText(items_[j]);

        layout_.insertWidget(i + 2, button.get(), 0, Qt::AlignLeft);
        buttons_.push_back(button);
    }

    if (items_.count() <= 0)
    {
        if (branch_.isVisible())
        {
            branch_.setChecked(true);
            home_.setChecked(false);
        }
        else
        {
            branch_.setChecked(false);
            home_.setChecked(true);
        }
    }
    else
    {
        home_.setChecked(false);
        branch_.setChecked(false);
    }


    i = 0;
    for(int j = 0; i < max && j < items_.count(); ++i, ++j)
    {
        buttons_[i]->setText(items_[j]);
        buttons_[i]->setChecked(false);
        buttons_[i]->show();
    }

    if (i > 0)
    {
        buttons_[i - 1]->setChecked(true);
    }

    // Should release them.
    buttons_.erase(buttons_.begin() + i, buttons_.end());
}

/// Calculate how many items that can be displayed with current
/// widget width.
int  PathBar::maxItems()
{
    return items_.count();
}
