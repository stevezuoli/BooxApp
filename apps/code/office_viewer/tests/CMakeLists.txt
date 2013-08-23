add_definitions(-DTEST_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/data")
add_definitions(-DTEST_TMP_DIR="${CMAKE_SOURCE_DIR}/tests/tmp")

onyx_test(onyx_office_test office_test.cpp )
target_link_libraries(onyx_office_test
  onyx_office ${ONYX_LIBS} ${QT_LIBRARIES})

