ENABLE_QT()

# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

if(BUILD_FOR_ARM OR NOT UNIX)
ADD_EXECUTABLE(naboo_reader ${SRCS})
TARGET_LINK_LIBRARIES(naboo_reader
  adobe_view_engine
  adobe_drm_engine
  onyxapp
  dictionary
  tts
  sound
  onyx_data
  onyx_screen
  onyx_ui
  private_conf
  ${QT_LIBRARIES}
  ${ADD_LIB}
)
endif(BUILD_FOR_ARM OR NOT UNIX)
