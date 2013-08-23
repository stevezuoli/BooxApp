
#include "onyx/base/base.h"
#include "testing/testing.h"
#include "../application.h"

using namespace reader;

namespace
{

TEST(OpenChm)
{
    int argc = 0;
    ReaderApplication app(argc, 0);
    QString path = SAMPLE_ROOT;
    path += "/chm/xue.chm";
    EXPECT_TRUE(app.open(path));
}

TEST(OpenMobi)
{
    int argc = 0;
    ReaderApplication app(argc, 0);
    QString path = SAMPLE_ROOT;
    path += "/prc/HARRY1.prc";
    EXPECT_TRUE(app.open(path));
}


}

