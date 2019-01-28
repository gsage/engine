set(LIB_NAME "tinyfiledialogs")

include_directories(./)
FILE(GLOB sources tinyfiledialogs.c)

set(LIB_TYPE STATIC)

add_library(${LIB_NAME} ${LIB_TYPE} ${sources})
