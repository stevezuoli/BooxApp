#include <cstdio>
#include <cstring>
#include "ExceptionHandler.h"
#include "TestResult.h"
#include "Failure.h"
#include "TestException.h"



namespace ExceptionHandler {


namespace {
    bool g_bHandleExceptions = true;
}


bool IsOn ()
{
    return g_bHandleExceptions;
}

void TurnOn (bool bOn)
{
    g_bHandleExceptions = bOn;
}


void Handle (testing::TestResult& result,
             const testing::TestException& exception,
             const char* testname, const char* filename, int linenumber )
{
    char msg[4096];
    sprintf( msg, "Raised exception %s from:\n  %s(%i)",
             exception.message, exception.file, exception.line );
    result.AddFailure(testing::Failure(msg, testname, filename, linenumber));
}

void Handle (testing::TestResult& result, const char* condition,
             const char* testname, const char* filename, int linenumber)
{
    if (!g_bHandleExceptions)
        throw;

    char msg[1024] = "Unhandled exception ";
    strcat(msg, condition);
    result.AddFailure (testing::Failure(msg, testname, filename, linenumber));
}



}
