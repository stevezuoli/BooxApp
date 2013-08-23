#include "onyx/data/sketch_proxy.h"
#include "testing/testing.h"

using namespace sketch;

namespace
{

TEST(LoadFromDatabase)
{
    QDir home = QDir::home();
    QString db = home.filePath("temp.sketch");

    SketchProxy sketch_proxy;
    EXPECT_TRUE(sketch_proxy.loadFromDatabase(db));
    sketch_proxy.close();
}

}   // end of namespace
