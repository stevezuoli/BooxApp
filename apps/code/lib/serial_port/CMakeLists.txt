enable_qt()
# sp
FILE(GLOB SP_HDRS *.h)
FILE(GLOB SP_SRCS *.cpp)

# sp lib
SET(SP_SRCS ${SP_HDRS} ${SP_SRCS})

ADD_LIBRARY(onyx_serial_port STATIC ${SP_SRCS})


