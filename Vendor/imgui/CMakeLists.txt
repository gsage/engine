set(LIB_NAME "imgui")

include_directories(./)
include_directories(${gsage_SOURCE_DIR}/GsageCore/include)
FILE(GLOB sources ./*.cpp)

add_library(${LIB_NAME} STATIC ${sources})

set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX _d)
