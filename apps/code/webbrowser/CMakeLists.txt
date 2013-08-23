ENABLE_QT()

# Header files.
SET(WEBBROWSER_HDRS
  bookmark_model.h
  web_application.h
  frame.h
  view.h
  page.h
  keyboard_dialog.h)
 

# Srouce files.
SET(WEBBROWSER_SRCS
  ${WEBBROWSER_HDRS}
  bookmark_model.cpp
  web_application.cpp
  view.cpp
  frame.cpp
  page.cpp
  keyboard_dialog.cpp
  main.cpp)

# Moc files.
SET(MOC_WEBBROWSER_HDRS
  web_application.h
  view.h
  frame.h
  page.h
  keyboard_dialog.h)
QT4_WRAP_CPP(MOC_WEBBROWSER_SRCS ${MOC_WEBBROWSER_HDRS})

# Resources.
SET(RESOURCE_SRCS res.qrc)
QT4_ADD_RESOURCES(BROWSER_RES ${RESOURCE_SRCS})

# Executable file.
ADD_EXECUTABLE(web_browser ${WEBBROWSER_SRCS} ${MOC_WEBBROWSER_SRCS} ${BROWSER_RES})
TARGET_LINK_LIBRARIES(web_browser
   onyx_cms
   onyx_data
   onyx_screen
   network_service
   tts
   sound
   onyx_wireless
   onyx_sys
   onyx_ui
   private_conf
   ${QT_LIBRARIES}
   ${ADD_LIB}
)

# Install the web_browser to $BUILD/bin
INSTALL(TARGETS web_browser RUNTIME DESTINATION bin)

