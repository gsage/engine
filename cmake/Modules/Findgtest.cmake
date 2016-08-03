set(GTEST_DIR $ENV{GTEST_DIR})

FIND_PATH(gtest_INCLUDE_DIR gtest.h
  HINTS
  ${GTEST_DIR}
  PATH_SUFFIXES include/gtest
  PATHS
  /usr/local
  )

FIND_LIBRARY(gtest_LIBRARY
  NAMES gtest gtest_main
  HINTS
  ${GTEST_DIR}
  PATH_SUFFIXES lib lib64 lib
  PATHS
  /usr/local
  )

if(gtest_LIBRARY AND gtest_INCLUDE_DIR)
  set(gtest_FOUND "YES")
else(gtest_LIBRARY AND gtest_INCLUDE_DIR)
  set(gtest_FOUND "NO")
endif(gtest_LIBRARY AND gtest_INCLUDE_DIR)

if(NOT gtest_FOUND)
  if(NOT GTEST_DIR)
    message("WARNING: gtest not found, if you want to run tests, you could specify environment variable GTEST_DIR with gtest libs and includes")
  else()
    message("WARNING: gtest was not found in ${GTEST_DIR}")
  endif()
endif()

MARK_AS_ADVANCED(gtest_INCLUDE_DIR, gtest_LIBRARY, gtest_FOUND)
