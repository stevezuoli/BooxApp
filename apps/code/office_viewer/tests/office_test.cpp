
#include "onyx/base/base.h"
#include "onyx/base/qt_support.h"

#include "gtest/gtest.h"
#include "onyx_office_view.h"
#include "onyx_office.h"
#include "main_widget.h"
#include "QApplication"
namespace onyx {
class OfficeTest : public ::testing::Test {
    public:
    void SetUp() {
        static char* name;
        static int FAKE_ARGC = 1;
        static char** FAKE_ARGV = &name;
        app_ = new QApplication(FAKE_ARGC, FAKE_ARGV);
    }
protected:
    QApplication* app_;

};


TEST_F (OfficeTest, Open) {
    QList<QString> filelist;
    for (int i=1; i<2; ++i){
        filelist<<(QString("test%1.doc").arg(QString::number(i)));
    }
    EXPECT_TRUE(OfficeReader::instance().initialize(QSize()));
    foreach(QString file, filelist) {
    EXPECT_TRUE(OfficeReader::instance().open (file));
    EXPECT_TRUE(OfficeReader::instance().close());
    }
}
}
