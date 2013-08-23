#include "onyx/base/ui.h"
#include "node.h"
#include "model_tree.h"
#include "node_view.h"



using namespace explorer::model;
using namespace explorer::view;

int main(int argc, char * argv[])
{

    QApplication app(argc, argv);
    Q_INIT_RESOURCE(images);

    ModelTree model;

    NodeView view(0);
    view.SetNode(model.GetCurrentNode());
    view.SetViewMode(NodeView::THUMBNAIL_VIEW);
    view.show();

    app.exec();
    return 0;
}

