
ENABLE_QT()

INCLUDE_DIRECTORIES(inc)

# Header files.
SET(HDRS
  inc/baby_sitter.h
  inc/power_manager.h
  inc/touch_screen_manager.h
  inc/sound_manager.h
  inc/udev_watcher.h
  inc/sys_keyboard_filter.h
  inc/mount_entry_watcher.h
  inc/wifi_manager.h
  inc/system_manager.h
  inc/sys_context.h
  inc/screen_manager_interface.h
  inc/bs_screen_manager.h
  inc/cpu_monitor.h
  inc/drm_manager.h
  inc/service_watcher.h
  inc/3G_reporter.h
  inc/3G_manager.h
  inc/messenger_manager.h
)
SET(MOC_HDRS ${HDRS})
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

# Resource
SET(RESOURCE_SRCS images.qrc)
QT4_ADD_RESOURCES(IMAGES_RES ${RESOURCE_SRCS})

# src
SET(SRCS
  ${HDRS}
  src/main.cpp
  src/baby_sitter.cpp
  src/mount_entry_watcher.cpp
  src/power_manager.cpp
  src/sound_manager.cpp
  src/sys_keyboard_filter.cpp
  src/wifi_manager.cpp
  src/system_manager.cpp
  src/touch_screen_manager.cpp
  src/udev_watcher.cpp
  src/bs_screen_manager.cpp
  src/bs_chip.cpp
  src/bs_cmd.cpp
  src/cpu_monitor.cpp
  src/drm_manager.cpp
  src/service_watcher.cpp
  src/3G_manager.cpp
  src/3G_reporter.cpp
  src/messenger_manager.cpp
  ${MOC_SRCS})

ADD_EXECUTABLE(system_manager ${SRCS} ${IMAGES_RES})
TARGET_LINK_LIBRARIES(system_manager onyx_sys gpio onyx_i2c onyx_screen onyx_serial_port ${QT_LIBRARIES} ${ADD_LIB} )

# mount tree watcher
SET(MOC_MOUNT_TREE_WATCHER_HDRS inc/mount_entry_watcher.h tools/mount_tree_watcher.h)
QT4_WRAP_CPP(MOC_MOUNT_TREE_WATCHER_SRCS ${MOC_MOUNT_TREE_WATCHER_HDRS})
SET(MOUNT_TREE_WATCHER_SRCS
    inc/mount_entry_watcher.h
    src/mount_entry_watcher.cpp
    tools/mount_tree_watcher.h
    tools/mount_tree_watcher.cpp)
ADD_EXECUTABLE(mount_tree_watcher ${MOUNT_TREE_WATCHER_SRCS} ${MOC_MOUNT_TREE_WATCHER_SRCS})
TARGET_LINK_LIBRARIES(mount_tree_watcher
    ${QT_LIBRARIES}
    ${ADD_LIB}
)
SET_TARGET_PROPERTIES(mount_tree_watcher PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

# baby sitter
qt4_wrap_cpp(MOC_BABY_SITTER_H inc/baby_sitter.h)
add_library(baby_sitter_lib src/baby_sitter.cpp ${MOC_BABY_SITTER_H})
target_link_libraries(baby_sitter_lib ${QT_LIBRARIES})

add_executable(baby_sitter src/baby_sitter_main.cpp)
target_link_libraries(baby_sitter baby_sitter_lib ${QT_LIBRARIES})
SET_TARGET_PROPERTIES(baby_sitter PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})


# battery test
#SET(MOC_BATTERY_TEST_HDRS test/battery_test.h inc/power_manager.h)
#QT4_WRAP_CPP(MOC_BATTERY_TEST_SRCS ${MOC_BATTERY_TEST_HDRS})
#SET(battery_test_srcs
#    test/battery_test.h
#    test/battery_test.cpp
#    inc/cpu_monitor.h
#    inc/power_manager.h
#    src/power_manager.cpp
#    src/cpu_monitor.cpp)
#ADD_EXECUTABLE(battery_test ${battery_test_srcs} ${MOC_BATTERY_TEST_SRCS})
#TARGET_LINK_LIBRARIES(battery_test screen_manager gpio sys
#    ${QT_LIBRARIES}
#    ${ADD_LIB}
#)
#SET_TARGET_PROPERTIES(battery_test PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

# suspend test
SET(MOC_SUSPEND_TEST_HDRS inc/power_manager.h)
QT4_WRAP_CPP(MOC_SUSPEND_TEST_SRCS ${MOC_SUSPEND_TEST_HDRS})
SET(suspend_test_srcs
    inc/power_manager.h
    src/power_manager.cpp
    src/cpu_monitor.cpp
    test/suspend.cpp)
ADD_EXECUTABLE(suspend_test ${suspend_test_srcs} ${MOC_SUSPEND_TEST_SRCS})
TARGET_LINK_LIBRARIES(suspend_test onyx_sys gpio onyx_i2c
    ${QT_LIBRARIES}
    ${ADD_LIB}
)
SET_TARGET_PROPERTIES(suspend_test PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

# pppd test
SET(MOC_PPPD_TEST_HDRS inc/3G_manager.h inc/3G_reporter.h)
QT4_WRAP_CPP(MOC_PPPD_TEST_SRCS ${MOC_PPPD_TEST_HDRS})
SET(pppd_test_srcs
    inc/3G_reporter.h
    inc/3G_manager.h
    src/3G_reporter.cpp
    src/3G_manager.cpp
    test/pppd_test.cpp)
ADD_EXECUTABLE(pppd_test ${pppd_test_srcs} ${MOC_PPPD_TEST_SRCS})
TARGET_LINK_LIBRARIES(pppd_test gpio onyx_serial_port onyx_sys ${QT_LIBRARIES} ${ADD_LIB})
SET_TARGET_PROPERTIES(pppd_test PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

# screen test
SET(MOC_SCREEN_TEST_HDRS inc/screen_manager_interface.h inc/bs_screen_manager.h test/my_widget.h)
QT4_WRAP_CPP(MOC_SCREEN_TEST_SRCS ${MOC_SCREEN_TEST_HDRS})
SET(screen_test_srcs
    inc/bs_chip.h
    inc/bs_cmd.h
    inc/bs_screen_manager.h
    src/bs_chip.cpp
    src/bs_cmd.cpp
    src/bs_screen_manager.cpp
    test/my_widget.h
    test/my_widget.cpp
    test/screen.cpp)
ADD_EXECUTABLE(screen_test.oar ${screen_test_srcs} ${MOC_SCREEN_TEST_SRCS})
TARGET_LINK_LIBRARIES(screen_test.oar onyx_sys onyx_screen
    ${QT_LIBRARIES}
    ${ADD_LIB}
)
#SET_TARGET_PROPERTIES(screen_test PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

# 3G reporter
#SET(MOC_3GREPORT_HDRS tools/3G_reporter.h)
#QT4_WRAP_CPP(MOC_3GREPORT_SRCS ${MOC_3GREPORT_HDRS})
#SET(3GREPORT_SRCS tools/3G_reporter.cpp)
#ADD_EXECUTABLE(3g_reporter ${3GREPORT_SRCS} ${MOC_3GREPORT_SRCS})
#TARGET_LINK_LIBRARIES(3g_reporter onyx_serial_port
#    ${QT_LIBRARIES}
#    ${ADD_LIB}
#)
qt4_wrap_cpp(MOC_AT_CMD_SRCS inc/3G_reporter.h)
ADD_EXECUTABLE(at_command tools/at_command.cpp src/3G_reporter.cpp ${MOC_AT_CMD_SRCS})
TARGET_LINK_LIBRARIES(at_command onyx_serial_port ${QT_LIBRARIES}  ${ADD_LIB})

# sketch test
#SET(MOC_SKETCH_TEST_HDRS inc/screen_manager.h)
#QT4_WRAP_CPP(MOC_SKETCH_TEST_SRCS ${MOC_SKETCH_TEST_HDRS})
#SET(sketch_test_srcs
#    inc/bs_chip.h
#    inc/bs_cmd.h
#    inc/screen_manager.h
#    src/bs_chip.cpp
#    src/bs_cmd.cpp
#    src/screen_manager.cpp
#    test/sketch.cpp)
#ADD_EXECUTABLE(sketch_test ${sketch_test_srcs} ${MOC_SKETCH_TEST_SRCS})
#TARGET_LINK_LIBRARIES(sketch_test sys onyx_screen
#    ${QT_LIBRARIES}
#    ${ADD_LIB}
#)
#SET_TARGET_PROPERTIES(sketch_test PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

