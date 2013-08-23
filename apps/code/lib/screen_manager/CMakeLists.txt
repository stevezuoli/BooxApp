
ENABLE_QT()

SET(BS_SRCS
  bs_chip.cpp
  bs_cmd.cpp)

SET(BS_HDRS
  bs_chip.h
  bs_cmd.h)

INCLUDE_DIRECTORIES(.)

# Screen manager library
# ADD_LIBRARY(screen_manager  STATIC ${BS_SRCS} ${BS_HDRS} screen_manager.h screen_manager.cpp)

if (BUILD_FOR_ARM)
# Tool
# Initialize the controller for 6 inch screen.
ADD_EXECUTABLE(bs60_init ${BS_SRCS} ${BS_HDRS} bs60_init/bs60_init.cpp bs60_init/bs60_init.h)

# Initialize the controller for 8 inch screen.
# ADD_EXECUTABLE(bs80_init ${BS_SRCS} ${BS_HDRS} bs80_init/bs80_init.cpp bs80_init/bs80_init.h)

# Initialize the controller for 9.7 inch screen.
# ADD_EXECUTABLE(bs97_init ${BS_SRCS} ${BS_HDRS} bs97_init/bs97_init.cpp bs97_init/bs97_init.h)

# Update screen by using full waveform.
ADD_EXECUTABLE(bs_disp_full ${BS_SRCS} ${BS_HDRS} tools/bs_disp_full.cpp)
SET_TARGET_PROPERTIES(bs_disp_full PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

# Read/Write waveform and command data.
ADD_EXECUTABLE(bs_sfmrw bs_chip.h bs_chip.cpp bs_sfmrw/bs_sfm.cpp bs_sfmrw/bs_sfm.h bs_sfmrw/bs_sfmrw.cpp)

# Read temperature.
#ADD_EXECUTABLE(bs_report_temp tools/report_temp.cpp ${BS_SRCS} ${BS_HDRS})
#TARGET_LINK_LIBRARIES(bs_report_temp ${ADD_LIB} ${QT_LIBRARIES})
#SET_TARGET_PROPERTIES(bs_report_temp PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})


# Performance test.
ADD_EXECUTABLE(bs_performance ${BS_SRCS} ${BS_HDRS} test/bs_performance.cpp)
TARGET_LINK_LIBRARIES(bs_performance ${ADD_LIB} ${QT_LIBRARIES})
SET_TARGET_PROPERTIES(bs_performance PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

# Loop test used to test battery.
ADD_EXECUTABLE(bs_loop ${BS_SRCS} ${BS_HDRS} test/bs_loop.cpp)
SET_TARGET_PROPERTIES(bs_loop PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

# Load image
ADD_EXECUTABLE(bs_load_img ${BS_SRCS} ${BS_HDRS} ptm.cpp pgm.cpp tools/bs_load_img.cpp)
TARGET_LINK_LIBRARIES(bs_load_img)
SET_TARGET_PROPERTIES(bs_load_img PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})


#
ADD_EXECUTABLE(bs_state_switch ${BS_SRCS} ${BS_HDRS} test/bs_state_switch.cpp)
SET_TARGET_PROPERTIES(bs_state_switch PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})



# TODO, combine them into together and place in the system manager.
#ADD_EXECUTABLE(open_wacom  tools/open_wacom.cpp)
#TARGET_LINK_LIBRARIES(open_wacom ${ADD_LIB})
#ADD_EXECUTABLE(wacom_init  wacom_init.cpp)
#TARGET_LINK_LIBRARIES(wacom_init ${ADD_LIB})

# TODO The reg tool is used to test the register, we can also use
# bs_rd_reg or bs_wr_reg instead. Now the GPIO pins are proved correct
# so it's not necessary to use it anymore.
#ADD_EXECUTABLE(reg_tool bs_chip.h bs_chip.cpp tools/reg_tool.cpp)
#TARGET_LINK_LIBRARIES(reg_tool ${ADD_LIB})

endif (BUILD_FOR_ARM)
