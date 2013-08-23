enable_qt()

set(SRCS
  content_category.cpp
  content_manager.cpp
  content_node.cpp
  content_thumbnail.cpp
  content_options.cpp
  content_bookmarks.cpp
  content_shortcut.cpp
  cms_version.cpp
  notes_manager.cpp
  cms_utils.cpp
  user_db.cpp
  download_db.cpp
  media_db.cpp
  media_info_manager.cpp)

add_library(onyx_cms ${SRCS})
TARGET_LINK_LIBRARIES(onyx_cms
    ${QT_LIBRARIES}
    ${ADD_LIB})

strict_warning(onyx_cms)
