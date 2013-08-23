ENABLE_QT()

include_directories(./adobe)

# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

IF(BUILD_FOR_ARM)
ELSE (BUILD_FOR_ARM)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF(BUILD_FOR_ARM)

IF(BUILD_FOR_ARM)
SET(ADOBE_LIBS mschema hobbes t3 adept dp adobe-expat xml cts hyphen fonts jpeg png z ssl cryptopenssl mschema)
ENDIF(BUILD_FOR_ARM)

IF(WIN32)
SET(ADOBE_LIBS Crypt32 Secur32 shlwapi libeay32 ssleay32 AdeptClient dp expat fonts hobbes t3 crypt_openssl libjpeg png xml cts libhyphen)
ENDIF(WIN32)

ADD_LIBRARY(adobe_view_engine ${SRCS})
TARGET_LINK_LIBRARIES(adobe_view_engine
   onyx_cms
   
   onyx_sys
   onyx_ui
   onyx_base
   onyx_data
   ${QT_LIBRARIES}
   ${ADOBE_LIBS}
)

# DRM handler
ADD_EXECUTABLE(render_tool tools/render_tool.cpp)
TARGET_LINK_LIBRARIES(render_tool
  adobe_view_engine
)
set_target_properties(render_tool PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

