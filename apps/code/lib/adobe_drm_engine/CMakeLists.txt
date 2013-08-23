ENABLE_QT()

include_directories(./adobe)
include_directories(./libcurl)

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
SET(ADOBE_LIBS mschema hobbes t3 adept dp xml cts hyphen fonts jpeg png z ssl  cryptopenssl adobe-expat mschema)
ENDIF(BUILD_FOR_ARM)


IF(WIN32)
SET(ADOBE_LIBS Crypt32 Secur32 shlwapi libeay32 ssleay32 AdeptClient dp expat fonts hobbes t3 crypt_openssl libjpeg png xml cts libhyphen)
ENDIF(WIN32)

ADD_LIBRARY(adobe_drm_engine ${SRCS})
TARGET_LINK_LIBRARIES(adobe_drm_engine
   
   network_service
   onyx_data
   onyx_ui
   onyx_screen
   onyx_base
   onyx_sys
   onyx_cms
   curl
   ${QT_LIBRARIES}
   ${ADOBE_LIBS}
)

# DRM handler
#ADD_EXECUTABLE(drm_service tools/drm_service.cpp)
#TARGET_LINK_LIBRARIES(drm_service
#  adobe_drm_engine
#)
