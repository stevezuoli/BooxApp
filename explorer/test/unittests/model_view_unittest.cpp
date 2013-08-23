#include "onyx/base/ui.h"
#include "node.h"
#include "model_tree.h"
#include "model_view.h"



using namespace explorer::model;
using namespace explorer::view;

int main(int argc, char * argv[])
{

    QApplication app(argc, argv);
    Q_INIT_RESOURCE(images);

    ModelTree model;

    ModelView view(0);
    view.SetModelTree(&model);
    view.show();

    app.exec();
    return 0;
}

