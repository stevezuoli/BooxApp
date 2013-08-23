
SET(GPIO_SRCS gpio.h gpio.cpp marvell_gpio.h marvell_gpio.cpp)
SET(MARVELL_GPIO_SRCS marvell_gpio.h marvell_gpio.cpp)

# GPIO library
ADD_LIBRARY(gpio ${GPIO_SRCS})

# Marvell GPIO library
ADD_LIBRARY(onyx_marvell_gpio ${MARVELL_GPIO_SRCS})

# Tools
ADD_EXECUTABLE(value tools/value.cpp)
TARGET_LINK_LIBRARIES(value gpio)
#SET_TARGET_PROPERTIES(value PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

ADD_EXECUTABLE(set_value tools/set_value.cpp)
TARGET_LINK_LIBRARIES(set_value gpio)
SET_TARGET_PROPERTIES(set_value PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

ADD_EXECUTABLE(reset tools/reset.cpp)
TARGET_LINK_LIBRARIES(reset gpio)
SET_TARGET_PROPERTIES(reset PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

# Marvell
ADD_EXECUTABLE(marvell_value tools/marvell_value.cpp)
TARGET_LINK_LIBRARIES(marvell_value onyx_marvell_gpio)
#SET_TARGET_PROPERTIES(marvell_value PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

ADD_EXECUTABLE(marvell_set_value tools/marvell_set_value.cpp)
TARGET_LINK_LIBRARIES(marvell_set_value onyx_marvell_gpio)

