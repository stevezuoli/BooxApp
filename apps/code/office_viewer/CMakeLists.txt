enable_testing()
enable_qt()

include_directories(BEFORE
src/picsel/alien-include
src/picsel/alien-common
src/picsel/linux-alien-common
src)

if(BUILD_FOR_ARM)
  SET(staticlib picselapp)
else(BUILD_FOR_ARM)

  # Use 600x800 to approximate device screen on PC.
  add_definitions(-DSCREEN_WIDTH=600)
  add_definitions(-DSCREEN_HEIGHT=800)
  SET(staticlib picsellinux)
endif(BUILD_FOR_ARM)

set(ONYX_LIBS onyx_sys onyx_ui onyx_screen)

add_subdirectory(src)
add_subdirectory(tests)
