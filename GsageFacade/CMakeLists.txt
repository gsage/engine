set(LIB_NAME "GsageFacade")

include_directories(./include)
include_directories(${cosmos_SOURCE_DIR}/Core/include)
include_directories(${cosmos_SOURCE_DIR}/Components/OgreCrowd/include)
include_directories(${cosmos_SOURCE_DIR}/PlugIns/ParticleUniverse/include)

include_directories( ${OIS_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_Overlay_INCLUDE_DIR}
  ${OGRE_Terrain_INCLUDE_DIR}
  ${LUABIND_INCLUDE_DIRS}
  ${OGRE_SAMPLES_INCLUDEPATH}
)

link_directories(${cosmos_BINARY_DIR}/Core)
link_directories(${cosmos_BINARY_DIR}/Components/OgreCrowd)

FILE(GLOB sources src/*.cpp src/*/*.cpp )

ADD_LIBRARY(${LIB_NAME} ${sources})

set(LIBS OgreCrowd Core ${OGRE_Terrain_LIBRARY} ${OGRE_Overlay_LIBRARY})

if(WIN32)
  add_definitions(-DLUABIND_DYNAMIC_LINK)
endif(WIN32)

target_link_libraries(${LIB_NAME} ${LIBS})
