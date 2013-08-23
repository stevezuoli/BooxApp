ENABLE_QT()

SET(MOC_HDRS
    calibration.h)
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

SET(SRCS
    ${MOC_HDRS}
    calibration.cpp
    main.cpp)

ADD_EXECUTABLE(mouse_calibration ${MOC_SRCS} ${SRCS})
TARGET_LINK_LIBRARIES(mouse_calibration onyx_screen onyx_sys onyx_ui
    ${QT_LIBRARIES}
    ${ADD_LIB}
)

# Install the text_viewer to $BUILD/bin
INSTALL(TARGETS mouse_calibration RUNTIME DESTINATION bin)

