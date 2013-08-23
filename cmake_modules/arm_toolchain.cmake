# Define arm tool chain
MACRO (USE_ARM_TOOLCHAIN)
    MESSAGE("Use arm toolchain.")

    # Define the dependency libraries root path.
    if (DUOKAN_SDK_ROOT)
        message("DUOKAN SDK Root path ${DUOKAN_SDK_ROOT}")
        SET(CMAKE_FIND_ROOT_PATH ${DUOKAN_SDK_ROOT})
        link_directories(${DUOKAN_SDK_ROOT}/lib)
    else (DUOKAN_SDK_ROOT)
        message("Use default path: /opt/duokan/release/")
        SET(CMAKE_FIND_ROOT_PATH "/opt/duokan/release/")
        link_directories("/opt/duokan/release/lib")
    endif (DUOKAN_SDK_ROOT)
    SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

    if (${CMAKE_FIND_ROOT_PATH})
        SET(CMAKE_INSTALL_PREFIX  ${CMAKE_FIND_ROOT_PATH})
    else (${CMAKE_FIND_ROOT_PATH})
        SET(CMAKE_INSTALL_PREFIX  "/opt/duokan/release/")
    endif (${CMAKE_FIND_ROOT_PATH})

    # For libraries and headers in the target directories
    SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    # Set up development build mode
    SET(CMAKE_CXX_FLAGS "-s ")

    if (USE_CORTEX_A8)
        set(CMAKE_CXX_FLAGS "-s -fPIC -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp ")
    endif (USE_CORTEX_A8)

    SET(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Devel."
        FORCE)

    # if (LINK_ZLIB_DEFAULT)
    #    SET(ADD_LIB m rt pthread dl z)
    #else (LINK_ZLIB_DEFAULT)
    #    set(ADD_LIB m rt pthread dl)
    #endif (LINK_ZLIB_DEFAULT)

    ADD_DEFINITIONS(-DBUILD_FOR_ARM)

ENDMACRO(USE_ARM_TOOLCHAIN)
