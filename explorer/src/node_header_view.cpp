
#include "onyx/ui/text_layout.h"
#include "node.h"
#include "node_header_view.h"
#include "image_factory.h"
#include "file_node.h"
#include "dir_node.h"

namespace explorer
{

namespace view
{

static const int SPACING = 2;

/// Node view implementation.
NodeHeaderView::NodeHeaderView(QWidget * parent)
    : QWidget(parent)
    , title_node_(0)
    , buttons_layout_(this)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Light);
    createLayout();
}

NodeHeaderView::~NodeHeaderView(void)
{
}

void NodeHeaderView::updateNode(BranchNode * node)
{
    if (node == 0)
    {
        return;
    }

    // Always make it as dirty.
    title_node_ = node;

    // update layout.
    updateButtons();

    update();
}

void NodeHeaderView::onFieldClicked(Field by, SortOrder order)
{
    emit fieldClicked(by, order);
}

void NodeHeaderView::onFieldResized(Field field, QSize s)
{
    // Conver to position.
    int x = 0;
    for(int i = 0; i < static_cast<int>(buttons_.size()); ++i)
    {
        if (buttons_[i]->field() != field)
        {
            x += buttons_[i]->size().width();
            x += SPACING;
        }
        else
        {
            break;
        }
    }
    emit fieldResized(field, x, s.width());
}

void NodeHeaderView::mousePressEvent(QMouseEvent *)
{
}

void NodeHeaderView::mouseReleaseEvent(QMouseEvent *)
{
}

void NodeHeaderView::keyPressEvent(QKeyEvent *)
{
}

void NodeHeaderView::keyReleaseEvent(QKeyEvent *)
{
}

bool NodeHeaderView::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::LanguageChange && buttons_.size() > 0)
    {
        buttons_[0].get()->setText(QApplication::tr("Name"));
        buttons_[1].get()->setText(QApplication::tr("Type"));
        buttons_[2].get()->setText(QApplication::tr("Size"));
        buttons_[3].get()->setText(QApplication::tr("Last Access"));
    }
    return ret;
}

void NodeHeaderView::createLayout()
{
    buttons_layout_.setSpacing(SPACING);
    buttons_layout_.setContentsMargins(0, 0, 0, 0);

    setFixedHeight(30);

    updateLayout();
}

void NodeHeaderView::updateLayout()
{
    if (buttons_.size() <= 0)
    {
        ButtonPtr name(new HeaderButton(this, NAME));
        name->setText(QApplication::tr("Name"));
        buttons_.push_back(name);
        buttons_layout_.addWidget(name.get(), 229);
        connect(name.get(), SIGNAL(clicked(Field, SortOrder)),
                this, SLOT(onFieldClicked(Field, SortOrder)));
        connect(name.get(), SIGNAL(sizeChanged(Field, QSize)),
                this, SLOT(onFieldResized(Field, QSize)));

        ButtonPtr type(new HeaderButton(this, NODE_TYPE));
        type->setText(QApplication::tr("Type"));
        buttons_.push_back(type);
        buttons_layout_.addWidget(type.get(), 99);
        connect(type.get(), SIGNAL(clicked(Field, SortOrder)),
                this, SLOT(onFieldClicked(Field, SortOrder)));
        connect(type.get(), SIGNAL(sizeChanged(Field, QSize)),
                this, SLOT(onFieldResized(Field, QSize)));


        ButtonPtr size(new HeaderButton(this, SIZE));
        size->setText(QApplication::tr("Size"));
        buttons_.push_back(size);
        buttons_layout_.addWidget(size.get(), 78);
        connect(size.get(), SIGNAL(clicked(Field, SortOrder)),
                this, SLOT(onFieldClicked(Field, SortOrder)));
        connect(size.get(), SIGNAL(sizeChanged(Field, QSize)),
                this, SLOT(onFieldResized(Field, QSize)));


        ButtonPtr last(new HeaderButton(this, LAST_ACCESS_TIME));
        last->setText(QApplication::tr("Last Access"));
        buttons_.push_back(last);
        buttons_layout_.addWidget(last.get(), 188);
        connect(last.get(), SIGNAL(clicked(Field, SortOrder)),
                this, SLOT(onFieldClicked(Field, SortOrder)));
        connect(last.get(), SIGNAL(sizeChanged(Field, QSize)),
                this, SLOT(onFieldResized(Field, QSize)));

    }
}

void NodeHeaderView::updateButtons()
{
    if (title_node_ == 0)
    {
        return;
    }

    HeaderButton * ptr = 0;
    switch (title_node_->sort_field())
    {
    case NAME:
        ptr = buttons_[0].get();
        break;
    case NODE_TYPE:
        ptr = buttons_[1].get();
        break;
    case SIZE:
        ptr = buttons_[2].get();
        break;
    case LAST_ACCESS_TIME:
        ptr = buttons_[3].get();
        break;
    default:
        break;
    }

    for(int i = 0; i < static_cast<int>(buttons_.size()); ++i)
    {
        if (buttons_[i].get() == ptr)
        {
            ptr->setOrder(title_node_->sort_order());
        }
        else
        {
            buttons_[i]->setOrder(NO_ORDER);
        }
    }
}

}   // namespace view

}   // namespace explorer
