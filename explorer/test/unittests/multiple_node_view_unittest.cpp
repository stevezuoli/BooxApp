#include "onyx/base/ui.h"
#include "node.h"
#include "model_tree.h"
#include "node_view.h"

using namespace explorer::model;
using namespace explorer::view;

ModelTree model;

/// Demostrate how to switch between these views.
class MyView : public QWidget
{
public:
    MyView(QWidget *parent);
    ~MyView();

public:
    void CreateViews(const size_t count);
    void ChnageLayout(const int rows, const int columns);

protected:
    virtual void keyReleaseEvent(QKeyEvent *);

private:
    NodeView::ViewMode mode_;
    QGridLayout layout_;
    typedef shared_ptr<NodeView> NodeViewPtr;
    typedef vector<NodeViewPtr> NodeViewPtrs;
    typedef vector<NodeViewPtr>::iterator NodeViewPtrIter;
    NodeViewPtrs views;

};

MyView::MyView(QWidget *parent)
: QWidget(parent)
, mode_(NodeView::DETAILS_VIEW)
, layout_(this)
{
    CreateViews(5);
    ChnageLayout(5, 1);
}

MyView::~MyView()
{
}

void MyView::CreateViews(const size_t count)
{
    if (views.size() > count)
    {
        return;
    }

    for(size_t i = views.size(); i < count; ++i)
    {
        NodeViewPtr view(new NodeView(0));
        if (i >= 10)
        {
            view->SetNode(model.GetCurrentNode());
        }
        else
        {
            view->SetNode(0);
        }
        view->SetViewMode(mode_);
        views.push_back(view);
    }
}

void MyView::ChnageLayout(const int rows, const int columns)
{
    if (layout_.count() > 0)
    {
        for(NodeViewPtrIter it = views.begin(); it != views.end(); ++it)
        {
            layout_.removeWidget(it->get());
            (*it)->hide();
        }
    }

    for(int i = 0; i < rows; ++i)
    {
        for(int j = 0; j < columns; ++j)
        {
            layout_.addWidget(views[i * columns + j].get(), i, j);
            views[i * columns + j]->SetViewMode(mode_);
            views[i * columns + j]->show();
        }
    }
}

/// Test, you can use 1,2,3 to switch between views.
void MyView::keyReleaseEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_1)
    {
        mode_ = NodeView::LIST_VIEW;
        CreateViews(30);
        ChnageLayout(10, 3);
    }
    else if (ke->key() == Qt::Key_2)
    {
        mode_ = NodeView::DETAILS_VIEW;
        CreateViews(5);
        ChnageLayout(5, 1);
    }
    else if (ke->key() == Qt::Key_3)
    {
        mode_ = NodeView::THUMBNAIL_VIEW;
        CreateViews(20);
        ChnageLayout(4, 5);
    }

}

int main(int argc, char * argv[])
{

    QApplication app(argc, argv);
    Q_INIT_RESOURCE(images);

    MyView view(0);
    view.show();

    app.exec();
    return 0;
}

