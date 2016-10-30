set(COMMON_LIB "GsageCore")

include_directories(include/)

FILE(GLOB headers include/*h include/*/*h)

FILE(GLOB sources src/*.cpp src/*/*.cpp)

if(APPLE)
  FILE(GLOB mac_sources src/*.mm)
  set(sources ${sources} ${mac_sources})
endif(APPLE)

include_directories( ${OIS_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}
  ${LUAJIT_INCLUDE_DIR}
  ${OGRE_SAMPLES_INCLUDEPATH}
  ${JSONCPP_INCLUDE_DIR}
  ${MSGPACK_INCLUDE_DIRS}
  )

add_library(${COMMON_LIB} ${headers} ${sources})

set_target_properties(${COMMON_LIB} PROPERTIES DEBUG_POSTFIX _d)

target_link_libraries(${COMMON_LIB}
  ${OGRE_LIBRARIES}
  ${OIS_LIBRARIES}
  ${LUAJIT_LIBRARIES}
  ${JSONCPP_LIBRARIES}
  ${MSGPACK_LIBRARIES}
)

install(DIRECTORY include/ DESTINATION /usr/local/include/gsage
  FILES_MATCHING PATTERN "*.h"
)

install(TARGETS ${COMMON_LIB}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(TARGETS ${COMMON_LIB} DESTINATION /usr/local/lib)
