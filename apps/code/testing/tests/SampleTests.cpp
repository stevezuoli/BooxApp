#include "onyx/base/base.h"
#include "testing/CppUnitLite2.h"

TEST (MyFirstTest)
{
    int a = 102;
    EXPECT_EQ(102, a);
}


class MyFixture
{
  public:
    MyFixture()
            : someValue(2.0f),
              str("Hello")
    {
    }

    virtual ~MyFixture() {}

    float someValue;
    std::string str;
  private:
    NO_COPY_AND_ASSIGN(MyFixture);
};


TEST_F (MyFixture, TestCase1)
{
    EXPECT_CLOSE (someValue, 2.0f, 0.00001);
    someValue = 13;
}

TEST_F (MyFixture, TestCase2)
{
    EXPECT_CLOSE(someValue, 2.0f, 0.00001);
    EXPECT_EQ(str, "Hello");
}

