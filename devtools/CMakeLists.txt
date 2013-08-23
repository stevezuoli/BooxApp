enable_qt()

ADD_EXECUTABLE(fix_source_unittest fix_source_unittest.cpp fix_source.cpp)
TARGET_LINK_LIBRARIES(fix_source_unittest unittest_main)
MAYBE_LINK_TCMALLOC(fix_source_unittest)
SET_TARGET_PROPERTIES(fix_source_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
ADD_TEST(FixSourceUnittest ${TEST_OUTPUT_PATH}/fix_source_unittest)

ADD_EXECUTABLE(fix_source fix_source.cpp fix_source_main.cpp)
TARGET_LINK_LIBRARIES(fix_source boost_filesystem)

add_executable(dither dither.cpp)
target_link_libraries(dither
  floyd_steinberg_grayscale
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY})

add_executable(dither2 dither2.cpp)
target_link_libraries(dither2
  floyd_steinberg_dithering
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY})
