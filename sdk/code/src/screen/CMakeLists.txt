QT4_WRAP_CPP(MOC_SRCS ${ONYXSDK_DIR}/include/onyx/screen/screen_update_watcher.h)
add_library(onyx_screen STATIC screen_proxy.cpp screen_update_watcher.cpp ${MOC_SRCS})
install(TARGETS onyx_screen DESTINATION lib)
