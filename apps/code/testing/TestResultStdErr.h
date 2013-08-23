#ifndef TESTRESULTSTDERR_H
#define TESTRESULTSTDERR_H

#include "TestResult.h"

namespace testing {

class TestResultStdErr : public TestResult
{
public:
    virtual void AddFailure (const Failure & failure);
    virtual void EndTests ();
};

}

#endif

