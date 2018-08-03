set(LIB_NAME "cpp-channel")

include_directories(./include)
FILE(GLOB sources src/*.cpp)
FILE(GLOB headers include/*.h include/*/*.h)

add_library(${LIB_NAME} STATIC ${headers} ${sources})

set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX _d)
