# - Try to find the dbus library
# Once done this will define
#
#  TSLIB_FOUND - system has tslib
#  TSLIB_INCLUDE_DIR - the tslib include directory
#  TSLIB_LIBRARIES - Link these to use tslib
#  TSLIB_DEFINITIONS - Compiler switches required for using tslib
#

IF ( TSLIB_INCLUDE_DIR AND TSLIB_LIBRARIES )
    SET( TSLIB_FIND_QUIETLY TRUE )
ENDIF ( TSLIB_INCLUDE_DIR AND TSLIB_LIBRARIES )

IF ( NOT WIN32 )
   INCLUDE( UsePkgConfig )
   PKGCONFIG( tslib-0.0 _LibTSIncDir _LibTSLIBLinkDir _LibTSLIBLinkFlags _LibTSLIBCflags )
ENDIF ( NOT WIN32 )

# Look for ts include dir and libraries
MESSAGE( ${_LibTSIncDir})
FIND_PATH(TSLIB_INCLUDE_DIR tslib.h ${_LibTSIncDir} )
MESSAGE(${TSLIB_INCLUDE_DIR})

FIND_LIBRARY( TSLIB_LIBRARIES NAMES ts PATHS ${_LibTSLIBLinkDir} )
MESSAGE(${TSLIB_LIBRARIES})

IF ( TSLIB_INCLUDE_DIR AND TSLIB_LIBRARIES )
        SET( TSLIB_FOUND 1 )
        IF ( NOT TSLIB_FIND_QUIETLY )
                MESSAGE ( STATUS "Found TS Lib: ${TSLIB_LIBRARIES}" )
        ENDIF ( NOT TSLIB_FIND_QUIETLY )
ELSE ( TSLIB_INCLUDE_DIR AND TSLIB_LIBRARIES )
        IF ( NOT TSLIB_FIND_QUIETLY )
                MESSAGE ( STATUS "Could NOT found ts lib." )
        ENDIF ( NOT TSLIB_FIND_QUIETLY )
ENDIF ( TSLIB_INCLUDE_DIR AND TSLIB_LIBRARIES )

# Hide advanced variables from CMake GUIs
MARK_AS_ADVANCED( TSLIB_INCLUDE_DIR TSLIB_LIBRARIES )
