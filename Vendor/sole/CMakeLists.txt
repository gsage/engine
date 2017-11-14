set(LIB_NAME "sole")

include_directories(./)
FILE(GLOB sources *.cpp *.cxx)
FILE(GLOB headers *.hpp)

add_library(${LIB_NAME} STATIC ${headers} ${sources})

set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX _d)
