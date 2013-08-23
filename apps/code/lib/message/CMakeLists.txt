ENABLE_QT()

# Header files.
FILE(GLOB MOC_HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS  ${MOC_HDRS}  ${SRCS}  ${MOC_SRCS})

# Messenger
ADD_LIBRARY(message ${SRCS})
TARGET_LINK_LIBRARIES(message onyx_cms onyx_sys ${QT_LIBRARIES} ${ADD_LIB})

