set(OGRE_CROWD "OgreCrowd")

include_directories(include/)

FILE(GLOB headers include/*h include/*.hpp include/*/*h include/*/*/*h)

FILE(GLOB sources src/*.cpp src/*/*.cpp src/*/*/*.c)

include_directories(
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_Terrain_INCLUDE_DIR}
)

add_library(${OGRE_CROWD} STATIC ${headers} ${sources})

set_target_properties(${OGRE_CROWD} PROPERTIES DEBUG_POSTFIX _d)

target_link_libraries(${OGRE_CROWD} ${OGRE_LIBRARIES} ${OGRE_Terrain_LIBRARY})
