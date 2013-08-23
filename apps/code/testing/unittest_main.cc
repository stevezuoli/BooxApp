// Copyright 2007 COMPANY_NAME. All Rights Reserved.
// Author: hjiang@dev-gems.com (Hong Jiang)

/// This is a simple main() function which can be linked with unit
/// tests for convenientce.

#include "testing/testing.h"
#include "util/util.h"


int main()
{
    using namespace testing;
    TestResultStdErr result;
    TestRegistry::Instance().Run(result);
    return (result.FailureCount());
}
