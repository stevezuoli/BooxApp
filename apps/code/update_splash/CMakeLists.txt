
ENABLE_QT()

INCLUDE_DIRECTORIES(inc)

# Header files.
SET(HDRS
)
SET(MOC_HDRS ${HDRS})
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

# src
SET(SRCS
  ${HDRS}
  update_splash.cpp
  ${MOC_SRCS})

ADD_EXECUTABLE(update_splash ${SRCS})

TARGET_LINK_LIBRARIES(update_splash ${QT_LIBRARIES} ${ADD_LIB})
