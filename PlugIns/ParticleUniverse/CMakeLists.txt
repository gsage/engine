set(LIB_NAME "Plugin_ParticleUniverseFactory")

include_directories(./include)
include_directories(${cosmos_SOURCE_DIR}/Core/include)
include_directories(${cosmos_SOURCE_DIR}/Components/HandleManager/include)
include_directories(${cosmos_SOURCE_DIR}/GsageFacade/include)

set(LIBS GsageFacade ${OGRE_LIBRARIES} ${LUABIND_LIBRARIES} ${Boost_LIBRARIES} ${OGRE_Plugin_ParticleUniverse_LIBRARY})
include_directories(
  ${OGRE_INCLUDE_DIRS}
  ${LUABIND_INCLUDE_DIRS}
)

add_definitions(-DPLUGIN_EXPORT)
FILE(GLOB sources src/*.cpp s src/*/*.cpp )
ADD_LIBRARY(${LIB_NAME} SHARED ${sources})

set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

if(MINGW OR UNIX)
  set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/PlugIns)
endif(MINGW OR UNIX)

if(WIN32)
  add_custom_command(TARGET ${LIB_NAME} PRE_BUILD
		COMMAND if not exist .\\bin\\Plugins mkdir ..\\bin\\Plugins)
	add_custom_command(TARGET ${LIB_NAME} POST_BUILD
		COMMAND copy \"$(TargetPath)\" .\\bin\\Plugins)
endif(WIN32)

