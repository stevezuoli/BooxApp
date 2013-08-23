// Author John

#ifndef NODE_HEADER_VIEW_H_
#define NODE_HEADER_VIEW_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "node.h"
#include "model_tree.h"
#include "header_button.h"

using namespace ui;
using namespace explorer::model;

namespace explorer
{

namespace model
{
    class Node;
}

namespace view
{

/// Present a node for end user. Display node in header.
class NodeHeaderView : public QWidget
{
    Q_OBJECT

public:
    NodeHeaderView(QWidget * parent);
    ~NodeHeaderView(void);

public Q_SLOTS:
    void updateNode(BranchNode * node);

Q_SIGNALS:
    void fieldClicked(Field field, SortOrder order);
    void fieldResized(Field field, int x, int width);

private Q_SLOTS:
    void onFieldClicked(Field field, SortOrder order);
    void onFieldResized(Field field, QSize s);

private:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual bool event(QEvent *);

    void createLayout();
    void updateLayout();

    void updateButtons();

private:
    BranchNode* title_node_;
    QHBoxLayout buttons_layout_;

    typedef shared_ptr<HeaderButton> ButtonPtr;
    typedef vector<ButtonPtr> Buttons;
    Buttons buttons_;
};

}  /// end of view

}  /// end of explorer
#endif
