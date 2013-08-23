ENABLE_QT()

# Header files.
set(hrds
    ${ONYXSDK_DIR}/include/onyx/sound/async_player.h
    ${ONYXSDK_DIR}/include/onyx/sound/sound.h
    ${ONYXSDK_DIR}/include/onyx/sound/wave.h)

IF (BUILD_WITH_TFT)
    set(hrds
        ${ONYXSDK_DIR}/include/onyx/sound/async_player.h
        ${ONYXSDK_DIR}/include/onyx/sound/sound.h
        ${ONYXSDK_DIR}/include/onyx/sound/wave.h
        ${ONYXSDK_DIR}/include/onyx/sound/alsa_sound.h)
endif (BUILD_WITH_TFT)

# Source files.
QT4_WRAP_CPP(MOC_SRCS ${hrds})


set(srcs async_player.cpp sound.cpp wave.cpp)
IF (BUILD_WITH_TFT)
    set(srcs async_player.cpp sound.cpp wave.cpp alsa_sound.cpp)
endif (BUILD_WITH_TFT)

SET(srcs ${srcs} ${hrds} ${MOC_SRCS})

IF (BUILD_WITH_TFT)
    set(ALSALIB asound)
ELSE (BUILD_WITH_TFT)
    set(ALSALIB )
ENDIF (BUILD_WITH_TFT)

# Sound library.
ADD_LIBRARY(sound ${srcs})
TARGET_LINK_LIBRARIES(sound
   ${ALSALIB}
   ${QT_LIBRARIES}
)

# Unittest
add_definitions(-DSAMPLE_ROOT="${CMAKE_SOURCE_DIR}/unittest/data")
FILE(GLOB WAV_UNITTEST_SRCS unittest/wave_unittest.cpp)
ADD_EXECUTABLE(wav_unittest ${WAV_UNITTEST_SRCS})
TARGET_LINK_LIBRARIES(wav_unittest sound onyx_sys
   gtest_main
   ${QT_LIBRARIES}
   ${ADD_LIB}
)
SET_TARGET_PROPERTIES(wav_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
ADD_TEST(WavUnittest ${TEST_OUTPUT_PATH}/wav_unittest)


# Async player
FILE(GLOB ASYNC_PLAYER_UNITTEST_SRCS unittest/async_player_unittest.cpp)

ADD_EXECUTABLE(async_player_unittest ${ASYNC_PLAYER_UNITTEST_SRCS})
TARGET_LINK_LIBRARIES(async_player_unittest sound onyx_sys
   gtest_main
   ${QT_LIBRARIES}
   ${ADD_LIB}
)

SET_TARGET_PROPERTIES(async_player_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
ADD_TEST(AsyncPlayerUnittest ${TEST_OUTPUT_PATH}/async_player_unittest)


# Sound unittest
FILE(GLOB SOUND_UNITTEST_SRCS unittest/sound_unittest.cpp)

ADD_EXECUTABLE(sound_unittest ${SOUND_UNITTEST_SRCS})
TARGET_LINK_LIBRARIES(sound_unittest sound onyx_sys
   gtest_main
   ${QT_LIBRARIES}
   ${ADD_LIB}
)

SET_TARGET_PROPERTIES(sound_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
 ADD_TEST(SoundUnittest ${TEST_OUTPUT_PATH}/sound_unittest)

# For make install
#INSTALL(FILES ${hrds} DESTINATION include/onyx/sound)
INSTALL(TARGETS sound DESTINATION lib)

