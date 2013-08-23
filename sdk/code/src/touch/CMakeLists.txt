ENABLE_QT()

# Header files.
SET(HDRS
    ${ONYXSDK_DIR}/include/onyx/touch/touch_listener.h
    ${ONYXSDK_DIR}/include/onyx/touch/touch_data.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

ADD_LIBRARY(onyx_touch ${SRCS})
strict_warning(onyx_touch)
INSTALL(TARGETS onyx_touch DESTINATION lib)

