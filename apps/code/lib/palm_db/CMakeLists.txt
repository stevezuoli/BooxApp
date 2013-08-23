
SET(HDRS
  pdb_stream.h
  ztxt_stream.h
  mobi_stream.h)

SET(SRCS
  pdb_stream.cpp
  ztxt_stream.cpp
  mobi_stream.cpp)

ADD_LIBRARY(palm_db ${SRCS} ${MOC_SRCS})

########### Unit Tests #####################
INCLUDE_DIRECTORIES(.)
ADD_EXECUTABLE(pdb_unittest unittests/pdb_test.cpp)
TARGET_LINK_LIBRARIES(pdb_unittest palm_db ${ZLIB_LIBRARIES})
SET_TARGET_PROPERTIES(pdb_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})

