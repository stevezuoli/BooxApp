ENABLE_QT()

# MET header files.
FILE(GLOB MET_HDRS *.h)
set(MET_HDRS ${MET_HDRS})

# MET srouce files.
FILE(GLOB MET_SRCS *.cpp)
SET(MET_SRCS ${MET_SRCS} ${MET_HDRS})

# Moc files.
SET(MOC_MET_HDRS ${MET_HDRS})
QT4_WRAP_CPP(MOC_MET_SRCS ${MOC_MET_HDRS})

# Executable file.
ADD_EXECUTABLE(met  ${MET_SRCS}  ${MOC_MET_SRCS})
TARGET_LINK_LIBRARIES(met onyx_cms onyx_sys ${QT_LIBRARIES} ${ADD_LIB})

# Install the met to $BUILD/bin
INSTALL(TARGETS met RUNTIME DESTINATION bin)

