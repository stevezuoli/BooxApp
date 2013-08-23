#include "../Test.h"
#include "MockTestResult.h"



TEST (TestResultStartsWithZeroTestCount)
{
    MockTestResult result;
    EXPECT_EQ(0, result.TestCount());
}

TEST (TestResultStartsWithZeroFailureCount)
{
    MockTestResult result;
    EXPECT_EQ (0, result.FailureCount());
}


class EmptyTestCase : public testing::Test
{
public:
    EmptyTestCase() : Test("EmptyTestCase", __FILE__, __LINE__) {}
protected:
    virtual void RunTest (testing::TestResult&)
    {
    }
};

TEST (TestCaseIncreasesResultCount)
{
    MockTestResult result;
    EmptyTestCase testCase;
    testCase.Run(result);
    EXPECT_EQ(1, result.TestCount());
}

TEST (EmptyTestCaseIsSuccessful)
{
    MockTestResult result;
    EmptyTestCase testCase;
    testCase.Run(result);
    EXPECT_EQ(0, result.FailureCount());
}


class CheckTestCase : public testing::Test
{
public:
    CheckTestCase(bool condition)
    : Test("CheckTestCase", __FILE__, __LINE__)
    , m_condition(condition)
    {}
protected:
    virtual void RunTest (testing::TestResult& result_)
    {
        EXPECT_TRUE(m_condition);
    }
    bool m_condition;
};


TEST (SuccessfulTestCaseCheckTrue)
{
    MockTestResult result;
    CheckTestCase testCase(true);
    testCase.Run(result);
    EXPECT_EQ(0, result.FailureCount());
}

TEST (FailedTestCaseCheckFalse)
{
    MockTestResult result;
    CheckTestCase testCase(false);
    testCase.Run(result);
    EXPECT_EQ(1, result.FailureCount());
}

TEST (FailedTestCaseHasCorrectTestName)
{
    MockTestResult result;
    CheckTestCase testCase(false);
    testCase.Run(result);
    EXPECT_TRUE(result.msg.find("Failure in CheckTestCase") != std::string::npos);
}

TEST (FailedTestCaseHasCorrectFilename)
{
    MockTestResult result;
    CheckTestCase testCase(false);
    testCase.Run(result);
    EXPECT_TRUE(result.msg.find(__FILE__) != std::string::npos);
}



template <class T>
class CheckEqualTestCase : public testing::Test
{
public:
    CheckEqualTestCase(T a, T b)
    : Test("CheckEqualTestCase", __FILE__, __LINE__)
    , m_a(a)
    , m_b(b)
    {}
protected:
    virtual void RunTest (testing::TestResult& result_)
    {
        EXPECT_EQ (m_a, m_b);
    }
    T m_a;
    T m_b;
};


TEST (SuccessfulTestCaseCheckEqual)
{
    MockTestResult result;
    CheckEqualTestCase<int> testCase(5,5);
    testCase.Run(result);
    EXPECT_EQ(0, result.FailureCount());
}

TEST (FailedTestCaseCheckEqual)
{
    MockTestResult result;
    CheckEqualTestCase<int> testCase(5,4);
    testCase.Run(result);
    EXPECT_EQ(1, result.FailureCount());
}

TEST (SuccessfulTestCaseCheckEqualWithStrings)
{
    MockTestResult result;
    CheckEqualTestCase<std::string> testCase(std::string("Hello"),std::string("Hello"));
    testCase.Run(result);
    EXPECT_EQ(0, result.FailureCount());
}


class CheckCloseTestCase : public testing::Test
{
public:
    CheckCloseTestCase(float a, float b)
    : Test("CheckCloseTestCase", __FILE__, __LINE__)
    , m_a(a)
    , m_b(b)
    {}
protected:
    virtual void RunTest (testing::TestResult& result_)
    {
        EXPECT_CLOSE (m_a, m_b, 0.00001);
    }
    float m_a;
    float m_b;
};


TEST (SuccessfulTestCaseCheckClose)
{
    MockTestResult result;
    CheckCloseTestCase testCase(9.2f,9.2f);
    testCase.Run(result);
    EXPECT_EQ(0, result.FailureCount());
}

TEST (SuccessfulTestCaseCheckCloseWithCloseNumbers)
{
    MockTestResult result;
    CheckCloseTestCase testCase(9.2f,9.200001f);
    testCase.Run(result);
    EXPECT_EQ(0, result.FailureCount());
}


TEST (FailedTestCaseCheckClose)
{
    MockTestResult result;
    CheckCloseTestCase testCase(9.2f,9.201f);
    testCase.Run(result);
    EXPECT_EQ(1, result.FailureCount());
}
