
# unittest
add_executable(paginator_unittest test.cpp)
target_link_libraries(paginator_unittest onyx_ui ${QT_LIBRARIES} ${ADD_LIB})
SET_TARGET_PROPERTIES(paginator_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
