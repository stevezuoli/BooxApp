#include "../CppUnitLite2.h"
#include "../TestException.h"
#include "MockTestResult.h"


class ExceptionTestCase : public testing::Test
{
  public:
    ExceptionTestCase() : Test("ExceptionTestCase", __FILE__, __LINE__)
    {}
  protected:
    virtual void RunTest (testing::TestResult& )
    {
        throw 10;
    }
};


TEST (ExceptionCausesFailedTest)
{
    MockTestResult result;
    ExceptionTestCase testCase;
    testCase.Run(result);
    EXPECT_EQ(1, result.FailureCount());
}

TEST (ExceptionIncreasesTestCount)
{
    MockTestResult result;
    ExceptionTestCase testCase;
    testCase.Run(result);
    EXPECT_EQ(1, result.TestCount());
}

TEST (ExceptionTestCaseHasCorrectFilename)
{
    MockTestResult result;
    ExceptionTestCase testCase;
    testCase.Run(result);
    EXPECT_TRUE(result.msg.find(__FILE__) != std::string::npos);
}

TEST (ExceptionTestCaseThrowsExceptionIfNotHandled)
{
    ExceptionHandler::TurnOn(false);

    bool bExceptionFound = false;
    MockTestResult result;
    ExceptionTestCase testCase;
    try {
        testCase.Run(result);
    }
    catch(...) {
        bExceptionFound = true;
    }
    ExceptionHandler::TurnOn(true);
    EXPECT_TRUE(bExceptionFound);
}



class ExceptionInsideCheckTestCase : public testing::Test
{
  public:
    ExceptionInsideCheckTestCase() : Test("ExceptionTestCaseInsideCheck", __FILE__, __LINE__)
    {}
  protected:
    int MyFunction()
    {
        if( true )
            throw 10;
        return 1;
    }
    virtual void RunTest (testing::TestResult& result_)
    {
        EXPECT_EQ (1, MyFunction());
    }
};


TEST (ExceptionInsideCheckCausesFailedTest)
{
    MockTestResult result;
    ExceptionInsideCheckTestCase testCase;
    testCase.Run(result);
    EXPECT_EQ(1, result.FailureCount());
}


TEST (ExceptionInsideCheckTestCaseHasCorrectFilename)
{
    MockTestResult result;
    ExceptionInsideCheckTestCase testCase;
    testCase.Run(result);
    EXPECT_TRUE(result.msg.find(__FILE__) != std::string::npos);
}

TEST (ExceptionInsideCheckTestCaseThrowsExceptionIfNotHandled)
{
    ExceptionHandler::TurnOn(false);

    bool bExceptionFound = false;
    MockTestResult result;
    ExceptionInsideCheckTestCase testCase;
    try {
        testCase.Run(result);
    }
    catch(...) {
        bExceptionFound = true;
    }
    ExceptionHandler::TurnOn(true);
    EXPECT_TRUE(bExceptionFound);
}

// class TestCaseChecksException : public testing::Test
// {
// public:
//     TestCaseChecksException() : Test("TestCaseChecksException", __FILE__, __LINE__)
//     {}
// protected:
//     int MyFunction()
//     {
//         if( true )
//             throw testing::TestException(__FILE__,__LINE__,"dummy");
//         return 1;
//     }
//     virtual void RunTest (testing::TestResult& result_)
//     {
//         CHECK_ASSERT( MyFunction() );
//     }
// };

// TEST (CheckForRaisedException)
// {
//     MockTestResult result;
//     TestCaseChecksException testCase;
//     testCase.Run(result);
//     EXPECT_EQ(0, result.FailureCount());
// }

class TestExceptionOutput : public testing::TestResult
{
  public:
    TestExceptionOutput() : testing::TestResult(), lastCondition() {}
    virtual void AddFailure (const testing::Failure & failure)
    {
        lastCondition = failure.Condition();
    }

    std::string lastCondition;
};

class TestCaseUnhandledException : public testing::Test
{
  public:
    struct MyException {};

    TestCaseUnhandledException() : Test( "TestCaseUnhandledException", __FILE__, __LINE__ ) {}

    virtual void RunTest (testing::TestResult& )
    {
        throw testing::TestException( "Foo", 10, "(TestException)" );
    }
};

TEST (CheckForExceptionMessage)
{
    TestExceptionOutput result;
    TestCaseUnhandledException testCase;
    testCase.Run(result);
    EXPECT_EQ (std::string( "Raised exception (TestException) from:\n  Foo(10)" ), result.lastCondition);
}

// NOTE(hjiang): This won't pass on some OSes because of memory
// protection error.

// class TestCaseSystemException : public testing::Test
// {
// public:
//     TestCaseSystemException() : Test("TestCaseSystemException", __FILE__, __LINE__)
//     {}
// protected:
//     virtual void RunTest (testing::TestResult& )
//     {
//         int * p = 0;
//         *p = 2;
//     }
// };

// TEST (SystemExceptionShouldFailTest)
// {
//     MockTestResult result;
//     TestCaseSystemException testCase;
//     testCase.Run(result);
//     EXPECT_EQ(1, result.FailureCount());
// }

// TEST (SystemExceptionIncreasesTestCount)
// {
//     MockTestResult result;
//     TestCaseSystemException testCase;
//     testCase.Run(result);
//     EXPECT_EQ(1, result.TestCount());
// }

// TEST (SystemExceptionPrintsCorrectMessage)
// {
//     testing::TestExceptionOutput result;
//     TestCaseSystemException testCase;
//     testCase.Run(result);
//     EXPECT_EQ (std::string( "Unhandled exception (unknown, GPF?) " ), result.lastCondition);
// }

bool ThrowingFunction()
{
    throw testing::TestException( "Test.cpp", 10, "Hello" );
}

class TestCaseCheckFunctionException : public testing::Test
{
  public:
    TestCaseCheckFunctionException() : Test("TestCaseCheckFunctionException", __FILE__, __LINE__)
    {}
  protected:
    virtual void RunTest (testing::TestResult& result_)
    {
        EXPECT_TRUE(ThrowingFunction());
    }
};

TEST (CheckMacroReportsExceptionThrown)
{
    TestExceptionOutput result;
    TestCaseCheckFunctionException testCase;
    testCase.Run(result);

    EXPECT_EQ(std::string( "Raised exception Hello from:\n  Test.cpp(10)" ), result.lastCondition);
}
