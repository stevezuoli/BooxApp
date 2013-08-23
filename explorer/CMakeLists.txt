ENABLE_QT()

INCLUDE_DIRECTORIES(include)

# Header files.
FILE(GLOB MOC_HDRS include/*.h)
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

# Source files.
FILE(GLOB SRCS src/*.cpp)
SET(SRCS
  ${MOC_HDRS}
  ${SRCS}
  ${MOC_SRCS})

# Resources.
SET(RESOURCE_SRCS images.qrc)
QT4_ADD_RESOURCES(IMAGES_RES ${RESOURCE_SRCS})

# Explorer, TODO, remove onyx_wifi to network later.
ADD_EXECUTABLE(explorer ${SRCS} ${IMAGES_RES})
TARGET_LINK_LIBRARIES(explorer
  onyx_cms onyx_ui onyx_wireless onyx_sys onyx_screen private_conf
  ${QT_LIBRARIES}
  ${ADD_LIB}
)

# Install the explorer to $BUILD/bin
INSTALL(TARGETS explorer RUNTIME DESTINATION bin)

# Unittests
QT4_WRAP_CPP(MOC_CONTROLLER_SRCS include/system_controller.h)
ADD_EXECUTABLE(model_tree_unittest
  include/system_controller.h
  src/dir_node.cpp
  src/file_node.cpp
  src/branch_node.cpp
  src/library_node.cpp
  src/desktop_node.cpp
  src/shortcuts_node.cpp
  src/notes_node.cpp
  src/writepad_node.cpp
  src/folder_node.cpp
  src/node.cpp
  src/mdb.cpp
  src/model_tree.cpp
  src/explorer_conf.cpp
  src/system_controller.cpp
  src/sites_database.cpp
  src/websites_node.cpp
  src/games_node.cpp
  ${MOC_CONTROLLER_SRCS}
  test/model_tree_unittest.cpp)
TARGET_LINK_LIBRARIES(model_tree_unittest onyx_cms onyx_ui ${QT_LIBRARIES} unittest_main)
SET_TARGET_PROPERTIES(model_tree_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
# ADD_TEST(ModelTreeUnittest ${TEST_OUTPUT_PATH}/model_tree_unittest)

