set(LIB_NAME "GsageCore")

FILE(GLOB headers include/*h include/*/*h)
FILE(GLOB sources src/*.cpp src/*/*.cpp)

if(APPLE)
  FILE(GLOB mac_sources src/*.mm)
  set(sources ${sources} ${mac_sources})
endif(APPLE)

include_directories(
  include/
  ${gsage_SOURCE_DIR}/Components/Sol2/include

  ${LUAJIT_INCLUDE_DIR}
  ${JSONCPP_INCLUDE_DIR}
  ${MSGPACK_INCLUDE_DIRS}
)

add_library(${LIB_NAME} SHARED ${headers} ${sources})

set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX _d)

set(LIBS
  ${LUAJIT_LIBRARIES}
  ${JSONCPP_LIBRARIES}
  ${MSGPACK_LIBRARIES})

if(APPLE)
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY})
endif(APPLE)

target_link_libraries(
  ${LIB_NAME}
  ${LIBS}
)

install(DIRECTORY include/ DESTINATION /usr/local/include/gsage
  FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${gsage_SOURCE_DIR}/Components/Sol2/include/ DESTINATION /usr/local/include/gsage
  FILES_MATCHING PATTERN "*.hpp"
)

install(TARGETS ${LIB_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(TARGETS ${LIB_NAME} DESTINATION /usr/local/lib)
