set(APP_NAME "game")
set(LIB_NAME "RocketOgre")

include_directories(./include)
include_directories(./resources)
include_directories(${cosmos_SOURCE_DIR}/Core/include)
include_directories(${cosmos_SOURCE_DIR}/Components/OgreCrowd/include)
include_directories(${cosmos_SOURCE_DIR}/GsageFacade/include)

include_directories( ${OIS_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_Terrain_INCLUDE_DIR}
  ${LUABIND_INCLUDE_DIRS}
  ${OGRE_SAMPLES_INCLUDEPATH}
  ${LIBROCKET_INCLUDE_DIRS}
)

link_directories(${cosmos_BINARY_DIR}/GsageCore)
link_directories(${cosmos_BINARY_DIR}/Components/OgreCrowd)
link_directories(${cosmos_BINARY_DIR}/GsageFacade)

add_definitions(-DRESOURCES_FOLDER="../../resources")

FILE(GLOB headers include/*.h include/*/*.h)

FILE(GLOB sources src/*.cpp s src/*/*.cpp )
ADD_EXECUTABLE(${APP_NAME} ${headers} ${sources})
ADD_LIBRARY(${LIB_NAME} ${sources})
set(LIBS GsageFacade ${LIBROCKET_LIBRARIES} ${OGRE_Overlay_LIBRARIES} ${LUABIND_LIBRARIES} ${Boost_LIBRARIES})

if(WIN32)
  set(LINK_FLAGS "/SUBSYSTEM:WINDOWS")
  set_target_properties(${APP_NAME} PROPERTIES DEBUG_POSTFIX _d LINK_FLAGS
    ${LINK_FLAGS})
else(WIN32)
  set_target_properties(${APP_NAME} PROPERTIES DEBUG_POSTFIX _d)
endif(WIN32)

target_link_libraries(${APP_NAME} ${LIBS})
target_link_libraries(${LIB_NAME} ${LIBS})

# post-build copy for win32
if(WIN32 AND NOT MINGW)
  add_custom_command( TARGET ${APP_NAME} PRE_BUILD
    COMMAND if not exist .\\bin mkdir .\\bin )
  add_custom_command( TARGET ${APP_NAME} POST_BUILD
    COMMAND copy \"$(TargetPath)\" .\\bin )
endif(WIN32 AND NOT MINGW)

add_custom_command(TARGET ${APP_NAME} POST_BUILD
  COMMAND cp -rf ${PROJECT_SOURCE_DIR}/resources ${PROJECT_BINARY_DIR}/bin
)

if(MINGW OR UNIX)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
endif(MINGW OR UNIX)

