ENABLE_QT()
link_directories(${CMAKE_SOURCE_DIR}/lib/tts/aisound)

INCLUDE_DIRECTORIES(chm/libchm)
INCLUDE_DIRECTORIES(chm/libchm_qt_wrapper)

# Html Plugin files.
# FILE(GLOB HTML_PLUGIN_HDRS html/*.h)
# FILE(GLOB HTML_PLUGIN_SRCS html/*.cpp)

# Chm Plugin files.
FILE(GLOB_RECURSE CHM_PLUGIN_HDRS chm/*.h)
FILE(GLOB_RECURSE CHM_PLUGIN_SRCS chm/*.cpp)
FILE(GLOB_RECURSE CHM_PLUGIN_C_SRCS chm/*.c)

# Mobipocket Plugin files.
# FILE(GLOB MOBI_PLUGIN_HDRS mobi/*.h)
# FILE(GLOB MOBI_PLUGIN_SRCS mobi/*.cpp)

# Reader header files.
FILE(GLOB READER_HDRS *.h)
set(READER_HDRS
    ${READER_HDRS}
    ${CHM_PLUGIN_HDRS})

# Reader srouce files.
FILE(GLOB READER_SRCS *.cpp)
SET(READER_SRCS
    ${READER_SRCS}
    ${READER_HDRS}
    ${CHM_PLUGIN_SRCS}
    ${CHM_PLUGIN_C_SRCS})

# Moc files.
SET(MOC_READER_HDRS ${READER_HDRS})
QT4_WRAP_CPP(MOC_READER_SRCS ${MOC_READER_HDRS})

# Resources.
SET(READER_RES_SRC res.qrc)
QT4_ADD_RESOURCES(READER_RES ${READER_RES_SRC})

# Executable file.
ADD_EXECUTABLE(html_reader
  ${READER_SRCS}
  ${MOC_READER_SRCS}
  ${READER_RES}
  main.cxx)
TARGET_LINK_LIBRARIES(html_reader
    onyx_cms dictionary
   onyx_sys tts onyx_screen onyx_ui sound
   onyx_data
   ${QT_LIBRARIES}
   ${ADD_LIB}
)

# Install the reader to $BUILD/bin
INSTALL(TARGETS html_reader RUNTIME DESTINATION bin)

## Unittest
SET(UNITTEST_SRCS
  ${READER_SRCS}
  ${MOC_READER_SRCS}
  ${READER_RES}
  unittest/unittest.cpp)

ADD_EXECUTABLE(reader_unittest ${UNITTEST_SRCS})

TARGET_LINK_LIBRARIES(reader_unittest
    onyx_cms dictionary
   onyx_sys tts onyx_screen onyx_ui sound unittest_main onyx_data
   ${QT_LIBRARIES}
   ${ADD_LIB}
)
SET_TARGET_PROPERTIES(reader_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
# ADD_TEST(ReaderUnittest ${TEST_OUTPUT_PATH}/reader_unittest)

