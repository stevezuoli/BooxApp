ENABLE_QT()

INCLUDE_DIRECTORIES(.)

SET(KEYBOARD_PLUGIN_HDRS naboo_keyboard_plugin.h)

set(keyboard_moc_hdrs naboo_keyboard_plugin.h)

QT4_WRAP_CPP(MOC_SRCS ${keyboard_moc_hdrs})


SET(KEYBOARD_PLUGIN_SRCS
    ${KEYBOARD_PLUGIN_HDRS}
    ${MOC_SRCS}
    naboo_keyboard_plugin.cpp)



add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)
add_definitions(-DQT_NO_DEBUG)

add_library(nabookeyboard SHARED ${KEYBOARD_PLUGIN_SRCS})
target_link_libraries(nabookeyboard ${QT_LIBRARIES} ${ADD_LIB})

# Install the plugin to QT_PLUGINS_OUTPUT_PATH
SET_TARGET_PROPERTIES(nabookeyboard PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${QT_PLUGINS_OUTPUT_PATH})
