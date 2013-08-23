ENABLE_QT()

FILE(GLOB MOC_IMGVIEWER_HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${MOC_IMGVIEWER_HDRS})

FILE(GLOB IMAGE_SRCS *.cpp)
SET(IMAGE_SRCS ${MOC_IMGVIEWER_HDRS} ${IMAGE_SRCS} ${MOC_SRCS})

QT4_ADD_RESOURCES(IMAGES_RES images.qrc)

ADD_LIBRARY(image_lib ${IMAGE_SRCS} ${IMAGES_RES})
TARGET_LINK_LIBRARIES(image_lib
   onyx_base
   onyx_data
   onyx_screen
   onyx_ui
   onyx_sys
   onyx_cms
   floyd_steinberg_grayscale
   floyd_steinberg_dithering
   ${QT_LIBRARIES}
   ${ADD_LIB}
)

add_library(floyd_steinberg_dithering floyd_steinberg_dithering.cpp)
target_link_libraries(floyd_steinberg_dithering ${QT_LIBRARIES})

add_library(floyd_steinberg_grayscale floyd_steinberg_grayscale.cpp)
target_link_libraries(floyd_steinberg_grayscale ${QT_LIBRARIES})

add_executable(image_reader main.cpp ${IMAGES_RES})
target_link_libraries(image_reader image_lib)
maybe_link_tcmalloc(image_reader)

# ----- Tests -----

onyx_test(image_items_manager_test image_items_manager_test.cpp)
target_link_libraries(image_items_manager_test
  image_lib ${QT_LIBRARIES})

# TODO: This test has been failing!
# onyx_test(image_model_test image_model_test.cpp)
# target_link_libraries(image_model_test image_lib  ${QT_LIBRARIES})

onyx_test(image_item_test image_item_test.cpp)
target_link_libraries(image_item_test image_lib ${QT_LIBRARIES})

onyx_test_without_heapcheck(image_application_test image_application_test.cpp)
target_link_libraries(image_application_test image_lib ${QT_LIBRARIES})

onyx_test(floyd_steinberg_grayscale_test floyd_steinberg_grayscale_test.cpp)
target_link_libraries(floyd_steinberg_grayscale_test floyd_steinberg_grayscale
  ${QT_LIBRARIES})
