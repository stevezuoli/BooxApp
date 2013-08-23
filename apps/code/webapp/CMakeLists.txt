ENABLE_QT()

# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

ADD_EXECUTABLE(webapp ${SRCS})
TARGET_LINK_LIBRARIES(webapp
   onyx_cms
   onyx_data
   onyx_screen
   network_service
   dictionary
   tts
   sound
   onyx_wireless
   onyx_sys
   onyx_ui
   private_conf
  ${QT_LIBRARIES}
  ${ADD_LIB}
)
