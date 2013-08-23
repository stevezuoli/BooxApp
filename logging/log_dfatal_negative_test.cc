// Author: Hong Jiang <hjiang@18scorpii.com>

#include "logging/log.h"
#include "testing/testing.h"

namespace {

TEST(DFATALTerminatesProgram) {
    LOG(L_ERROR) << "This is ok.";
    LOG(L_DFATAL) << "This should terminate the program.";
}

}  // anonymous namespace
