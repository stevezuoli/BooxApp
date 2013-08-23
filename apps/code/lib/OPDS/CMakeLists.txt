project(OPDS)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})


# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

ADD_EXECUTABLE(opds ${SRCS})
target_link_libraries(opds ${QT_QTCORE_LIBRARY}
                           ${QT_QTNETWORK_LIBRARY}
                           ${QT_QTXML_LIBRARY})
