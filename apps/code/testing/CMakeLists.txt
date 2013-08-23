SET(SRC
  ExceptionHandler.cpp
  Test.cpp
  Failure.cpp
  TestException.cpp
  TestRegistry.cpp
  TestResult.cpp
  TestResultStdErr.cpp
  linux/SignalHandler.cpp)

SET(TESTS_SRC
  tests/main.cpp
  tests/SampleTests.cpp
  tests/TestFixtures.cpp
  tests/TestMacros.cpp
  tests/TestTestCase.cpp
  tests/TestExceptionHandling.cpp)

ADD_LIBRARY(testing ${SRC})

ADD_LIBRARY(unittest_main unittest_main.cc)
TARGET_LINK_LIBRARIES(unittest_main testing)

ADD_EXECUTABLE(testing_self_test ${TESTS_SRC})
TARGET_LINK_LIBRARIES(testing_self_test testing)
MAYBE_LINK_TCMALLOC(testing_self_test)
SET_TARGET_PROPERTIES(testing_self_test PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
ADD_TEST(TestingFrameworkSelfTest ${TEST_OUTPUT_PATH}/testing_self_test)

ADD_EXECUTABLE(testing_unittest testing_unittest.cc)
TARGET_LINK_LIBRARIES(testing_unittest testing unittest_main)
MAYBE_LINK_TCMALLOC(testing_unittest)
SET_TARGET_PROPERTIES(testing_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
ADD_TEST(TestingFrameworkUnitTest ${TEST_OUTPUT_PATH}/testing_unittest)
