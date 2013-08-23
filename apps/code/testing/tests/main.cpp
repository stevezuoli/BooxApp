#include "../CppUnitLite2.h"
#include "../TestResultStdErr.h"

int main()
{
    using namespace testing;
    TestResultStdErr result;
    TestRegistry::Instance().Run(result);
    return (result.FailureCount());
}

