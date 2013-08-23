# Make sure you already have cmake 2.6 installed. For cross compiling.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Additional cmake modules
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

# Boox project.
Project(Boox CXX C)
ENABLE_TESTING()

# Global definitions
SET(TTS_LIB "")
SET(ADD_LIB "")
SET(SAMPLE_ROOT "${PROJECT_SOURCE_DIR}/samples")
ADD_DEFINITIONS(-DSAMPLE_ROOT="${SAMPLE_ROOT}")
IF(UNIX OR BUILD_FOR_ARM)
    ADD_DEFINITIONS(-DSHARE_ROOT="/usr/share")
ELSE (UNIX OR BUILD_FOR_ARM)
    ADD_DEFINITIONS(-DSHARE_ROOT="")
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF (UNIX OR BUILD_FOR_ARM)

# Check to use arm toolchain or not
IF(BUILD_FOR_ARM)
    INCLUDE(arm_toolchain)
    USE_ARM_TOOLCHAIN()
ELSE(BUILD_FOR_ARM)
link_directories("/opt/onyx/x86/lib")
ENDIF(BUILD_FOR_ARM)

IF (WIN32)
    ADD_DEFINITIONS(-DCONFIG_NATIVE_WINDOWS)
    ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE_NAMED_PIPE)
ELSE (WIN32)
    ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE_UNIX)
ENDIF (WIN32)
ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE)

# Project include directories.
INCLUDE_DIRECTORIES(BEFORE
    ${CMAKE_SOURCE_DIR}/sdk/code/include
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/apps/code/
    ${CMAKE_SOURCE_DIR}/apps/code/lib/
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_SOURCE_DIR}/third_party/gtest/include
)

# Find thread library for the current platform
include(FindThreads)
include(enable_qt)
include(misc)
include(onyx_test)
include(qt4_wrap_ts)
include(strict_warning)
include(tc)

LINK_DIRECTORIES(${QT_LIBRARY_DIR})

# Output directories.
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/libs)
SET(TEST_OUTPUT_PATH ${PROJECT_BINARY_DIR}/unittests)
SET(QT_PLUGINS_OUTPUT_PATH ${PROJECT_BINARY_DIR}/plugins)
SET(HANDWRITING_PLUGINS_OUTPUT_PATH ${PROJECT_BINARY_DIR}/handwriting/plugins)

# All sub directories.
add_subdirectory(apps/code)
add_subdirectory(boox-opensource/code/src)
add_subdirectory(explorer)
add_subdirectory(hw_test)
add_subdirectory(logging)
add_subdirectory(sdk/code)
add_subdirectory(system_manager)
add_subdirectory(third_party)

# Building qt plugins here.
if(BUILD_FOR_ARM)
    add_subdirectory(driver/screen)
    add_subdirectory(driver/keyboard)
endif(BUILD_FOR_ARM)
