#include "../CppUnitLite2.h"


class TestFixture
{
public:
    TestFixture()
    {
        ++s_instanceCount;
        ++s_instancesEverCreated;
    }
    virtual ~TestFixture()
    {
        --s_instanceCount;
    }

    static int s_instanceCount;
    static int s_instancesEverCreated;
};

int TestFixture::s_instanceCount = 0;
int TestFixture::s_instancesEverCreated = 0;


TEST(NoInstancesBeforeTest)
{
    EXPECT_EQ(0, TestFixture::s_instanceCount);
}

TEST_F (TestFixture, OneInstanceDuringFixtureTest)
{
    EXPECT_EQ(1, TestFixture::s_instanceCount);
}

TEST(NoInstancesAfterTest)
{
    EXPECT_EQ(0, TestFixture::s_instanceCount);
    EXPECT_EQ(1, TestFixture::s_instancesEverCreated);
}

TEST_F (TestFixture, OneInstanceDuringSecondFixtureTest)
{
    EXPECT_EQ(1, TestFixture::s_instanceCount);
}

TEST(NoInstancesAfterSecondTest)
{
    EXPECT_EQ(0, TestFixture::s_instanceCount);
    EXPECT_EQ(2, TestFixture::s_instancesEverCreated);
}
