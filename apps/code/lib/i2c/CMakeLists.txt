enable_qt()
# sp
FILE(GLOB I2C_HDRS *.h)
FILE(GLOB I2C_SRCS *.cpp)

# I2C lib
SET(I2C_SRCS ${I2C_HDRS} ${I2C_SRCS})
ADD_LIBRARY(onyx_i2c STATIC ${I2C_SRCS})

# I2C man
add_executable(i2c_man tools/i2c_man.cpp)
target_link_libraries(i2c_man onyx_i2c ${QT_LIBRARIES} ${ADD_LIB})
SET_TARGET_PROPERTIES(i2c_man PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

