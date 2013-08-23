ENABLE_QT()

# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

ADD_EXECUTABLE(scribble_test ${SRCS})
TARGET_LINK_LIBRARIES(scribble_test
  onyxapp
  onyx_data
  onyx_screen
  onyx_ui
  ${QT_LIBRARIES}
  ${ADD_LIB}
)
