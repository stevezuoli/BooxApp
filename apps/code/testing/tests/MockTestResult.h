#ifndef MOCKTESTRESULT_H_
#define MOCKTESTRESULT_H_

#include "testing/TestResult.h"
#include "testing/Failure.h"
#include <sstream>

class MockTestResult : public testing::TestResult
{
public:
    MockTestResult() : testing::TestResult(), msg() {}
    void AddFailure (const testing::Failure & failure)
    {
        TestResult::AddFailure(failure);
        std::stringstream stream;
        stream << failure << std::endl;
        msg = stream.str();
    }

    std::string msg;
};


#endif

