# - Try to find the thrift library
# Once done this will define
#
#  THRIFT_FOUND - system has thrift
#  THRIFT_INCLUDE_DIR - the thrift include directory
#  THRIFT_LIBRARIES - Link these to use thrift
#  THRIFT_DEFINITIONS - Compiler switches required for using thrift
#

IF ( THRIFT_INCLUDE_DIR AND THRIFT_LIBRARIES )
    SET( THRIFT_FIND_QUIETLY TRUE )
ENDIF ( THRIFT_INCLUDE_DIR AND THRIFT_LIBRARIES )

IF ( NOT WIN32 )
   INCLUDE( UsePkgConfig )
   PKGCONFIG( thrift _LibThriftIncDir _LibThriftLinkDir _LibThriftLinkFlags _LibThriftCflags )
ENDIF ( NOT WIN32 )

# Look for thrift include dir and libraries
#MESSAGE(${_LibThriftIncDir})
FIND_PATH(THRIFT_INCLUDE_DIR thrift/Thrift.h ${_LibThriftIncDir} )
MESSAGE(${THRIFT_INCLUDE_DIR})

FIND_LIBRARY( THRIFT_LIBRARIES NAMES thrift PATHS ${_LibThriftLinkDir} )
MESSAGE(${THRIFT_LIBRARIES})

IF ( THRIFT_INCLUDE_DIR AND THRIFT_LIBRARIES )
        SET( THRIFT_FOUND 1 )
        IF ( NOT THRIFT_FIND_QUIETLY )
                MESSAGE ( STATUS "Found Thrift: ${THRIFT_LIBRARIES}" )
                MESSAGE ( STATUS "Found Thrift Include: ${THRIFT_INCLUDE_DIR}" )
        ENDIF ( NOT THRIFT_FIND_QUIETLY )
ELSE ( THRIFT_INCLUDE_DIR AND THRIFT_LIBRARIES )
        IF ( NOT THRIFT_FIND_QUIETLY )
                MESSAGE ( STATUS "Could NOT found thrift." )
        ENDIF ( NOT THRIFT_FIND_QUIETLY )
ENDIF ( THRIFT_INCLUDE_DIR AND THRIFT_LIBRARIES )

# Hide advanced variables from CMake GUIs
MARK_AS_ADVANCED( THRIFT_INCLUDE_DIR THRIFT_LIBRARIES )
