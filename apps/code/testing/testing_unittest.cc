#include "onyx/base/base.h"
#include "testing/testing.h"

namespace {

TEST(Conditions) {
    EXPECT_TRUE(true);
    EXPECT_TRUE(1+1 == 2);
    int a = 3;
    int b = 2;
    EXPECT_TRUE(a+b == 5);
    EXPECT_EQ(5, a+b);
    EXPECT_FALSE(false);
    EXPECT_FALSE(a+b == 6);
    EXPECT_NE(9, a+b);
}

TEST(Strings) {
    EXPECT_SUBSTR("string", base::string("this is a string"));
}

TEST(Pointers) {
    char* a = new char('a');
    EXPECT_NULL(NULL);
    EXPECT_NULL(0);
    EXPECT_NOTNULL(a);
    delete a;
}

TEST(FloatingPoint) {
    double a = 2.34356;
    double b = 234.24569;
    double c = a+b;
    EXPECT_CLOSE(236.58925, c, 0.00001);
}

}  // namespace
