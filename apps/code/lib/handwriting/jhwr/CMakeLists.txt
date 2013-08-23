
ENABLE_QT()

IF(UNIX)
    ADD_DEFINITIONS(-DPLATFORM_POSIX)
ELSE (UNIX)
    ADD_DEFINITIONS(-DPLATFORM_WINDOWS)
ENDIF (UNIX)

SET(JHWR_HDRS jhwr_base.h
              jhwr_chinese.h
              jhwr_europe.h
              jhwr_impl.h
              jhwr_japanese.h
              jhwr_associated_char.h)
QT4_WRAP_CPP(MOC_SRCS ${JHWR_HDRS})
SET(JHWR_SRCS jhwr_base.cpp
              jhwr_chinese.cpp
              jhwr_europe.cpp
              jhwr_impl.cpp
              jhwr_japanese.cpp
              jhwr_associated_char.cpp ${MOC_SRCS})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)
add_definitions(-DQT_NO_DEBUG)

add_library(jhwr_plugin SHARED ${JHWR_SRCS})

link_directories(${CMAKE_SOURCE_DIR}/lib/handwriting/jhwr)
if(BUILD_FOR_ARM)
  TARGET_LINK_LIBRARIES(jhwr_plugin onyx_data jhwr jheuropean jhwrfunc ${QT_LIBRARIES} ${ADD_LIB})
else(BUILD_FOR_ARM)
  TARGET_LINK_LIBRARIES(jhwr_plugin onyx_data ${QT_LIBRARIES} ${ADD_LIB})
endif(BUILD_FOR_ARM)
SET_TARGET_PROPERTIES(jhwr_plugin PROPERTIES  LIBRARY_OUTPUT_DIRECTORY ${HANDWRITING_PLUGINS_OUTPUT_PATH})
