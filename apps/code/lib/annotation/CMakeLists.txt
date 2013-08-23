ENABLE_QT()

SET(ANNOTATION_HDRS
  database.h
  annotation.h
  annotation_agent.h
  annotation_document.h
  annotation_page.h
  annotation_io.h
  sketch_proxy.h
  sketch_document.h
  sketch_graphic_context.h
  sketch_io.h
  sketch_page.h
  sketch_point.h
  sketch_stroke.h
  sketch_utils.h
)

SET(ANNOTATION_SRCS
  database.cpp
  annotation.cpp
  annotation_agent.cpp
  annotation_document.cpp
  annotation_page.cpp
  annotation_io.cpp
  sketch_proxy.cpp
  sketch_document.cpp
  sketch_graphic_context.cpp
  sketch_io.cpp
  sketch_page.cpp
  sketch_point.cpp
  sketch_stroke.cpp)

QT4_WRAP_CPP(MOC_SRCS ${ANNOTATION_HDRS})

SET(ANNOTATION_SRCS
  ${ANNOTATION_HDRS}
  ${MOC_SRCS}
  ${ANNOTATION_SRCS}
)

ADD_LIBRARY(annotation ${ANNOTATION_SRCS} )
TARGET_LINK_LIBRARIES(annotation
   onyx_cms
   onyx_screen
   onyx_sys
   onyx_ui
   ${QT_LIBRARIES}
)

ADD_EXECUTABLE(sketch_proxy_unittest unittests/sketch_proxy_unittest.cpp)
TARGET_LINK_LIBRARIES(sketch_proxy_unittest annotation ${QT_LIBRARIES} unittest_main)
SET_TARGET_PROPERTIES(sketch_proxy_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
