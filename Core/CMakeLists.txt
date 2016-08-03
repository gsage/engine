set(COMMON_LIB "Core")

include_directories(include/)

FILE(GLOB headers include/*h include/*/*h)

FILE(GLOB sources src/*.cpp src/*/*.cpp)

if(APPLE)
  FILE(GLOB mac_sources src/*.mm)
  set(sources ${sources} ${mac_sources})
endif(APPLE)

include_directories( ${OIS_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}
  ${LUABIND_INCLUDE_DIRS}
  ${OGRE_SAMPLES_INCLUDEPATH}
  )

add_library(${COMMON_LIB} STATIC ${headers} ${sources})

set_target_properties(${COMMON_LIB} PROPERTIES DEBUG_POSTFIX _d)

target_link_libraries(${COMMON_LIB} ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${LUABIND_LIBRARIES})
