ADD_EXECUTABLE(log_dfatal_negative_test log_dfatal_negative_test.cc)
TARGET_LINK_LIBRARIES(log_dfatal_negative_test unittest_main)
MAYBE_LINK_TCMALLOC(log_dfatal_negative_test)
SET_TARGET_PROPERTIES(log_dfatal_negative_test PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
ADD_TEST(LogDFATALNegativeTest ${TEST_OUTPUT_PATH}/log_dfatal_negative_test)

IF(NOT DEFINED NDEBUG)
SET_TESTS_PROPERTIES(
  LogDFATALNegativeTest
  PROPERTIES
  WILL_FAIL TRUE)
ENDIF(NOT DEFINED NDEBUG)

