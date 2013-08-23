
# kermit
FILE(GLOB KERMIT_HDRS *.h)
FILE(GLOB KERMIT_SRCS *.c)

# kermit lib
SET(KERMIT_SRCS ${KERMIT_HDRS} ${KERMIT_SRCS})

ADD_LIBRARY(kermit STATIC ${KERMIT_SRCS})

ADD_EXECUTABLE(kermit_update tools/kermit_update.cpp)
TARGET_LINK_LIBRARIES(kermit_update kermit)

ADD_EXECUTABLE(kermit_show_image tools/kermit_show_image.c)
TARGET_LINK_LIBRARIES(kermit_show_image kermit)

