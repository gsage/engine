set(APP_NAME "game")
set(LIB_NAME "RocketOgre")

include_directories(./include)
include_directories(./resources)
include_directories(${cosmos_SOURCE_DIR}/Core/include)
include_directories(${cosmos_SOURCE_DIR}/Components/OgreCrowd/include)
include_directories(${cosmos_SOURCE_DIR}/Components/Sol2/include)
include_directories(${cosmos_SOURCE_DIR}/GsageFacade/include)

include_directories( ${OIS_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_Terrain_INCLUDE_DIR}
  ${JSONCPP_INCLUDE_DIR}
  ${LUAJIT_INCLUDE_DIR}
  ${OGRE_SAMPLES_INCLUDEPATH}
  ${LIBROCKET_INCLUDE_DIRS}
)

link_directories(${cosmos_BINARY_DIR}/Core)
link_directories(${cosmos_BINARY_DIR}/Components/OgreCrowd)
link_directories(${cosmos_BINARY_DIR}/GsageFacade)


FILE(GLOB headers include/*.h include/*/*.h)

FILE(GLOB sources src/*.cpp s src/*/*.cpp )
ADD_EXECUTABLE(${APP_NAME} ${headers} ${sources})
ADD_LIBRARY(${LIB_NAME} ${sources})
set(LIBS
  GsageFacade
  OgreCrowd
  ${LIBROCKET_LIBRARIES}
  ${OGRE_Overlay_LIBRARIES}
  ${LUAJIT_LIBRARIES}
  ${Boost_LIBRARIES})

if(APPLE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY}
    ${CARBON}
    ${IOKIT}
    ${COCOA_LIBRARY}
    )
  set_property(TARGET ${APP_NAME} PROPERTY MACOSX_BUNDLE TRUE)
  set_property(TARGET ${APP_NAME} PROPERTY MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/resources/Info.plist)
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.gsage.\${PRODUCT_NAME:rfc1034identifier}")
  set ( CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 -image_base 7fff04c4a000" )
  option ( LUA_USE_POSIX "Use POSIX functionality." ON )
  option ( LUA_USE_DLOPEN "Use dynamic linker to load modules." ON )
  set ( LJVM_MODE machasm )

  set(APP_FRAMEWORKS_DIRECTORY "${PROJECT_BINARY_DIR}/bin/${APP_NAME}.app/Contents/Frameworks")
  set(APP_RESOURCES_DIRECTORY "${PROJECT_BINARY_DIR}/bin/${APP_NAME}.app/Contents/Resources/")

  if(NOT EXISTS ${APP_FRAMEWORKS_DIRECTORY})
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
      COMMAND mkdir -p ${APP_FRAMEWORKS_DIRECTORY}
    )
  endif(NOT EXISTS ${APP_FRAMEWORKS_DIRECTORY})

  if(NOT EXISTS ${APP_RESOURCES_DIRECTORY})
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
      COMMAND mkdir -p ${APP_RESOURCES_DIRECTORY}
    )
  endif(NOT EXISTS ${APP_RESOURCES_DIRECTORY})

  add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND cp -rf $ENV{OGRE_HOME}/lib/${CMAKE_BUILD_TYPE}/ ${APP_FRAMEWORKS_DIRECTORY}
  )
  add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND cp -rf ${PROJECT_SOURCE_DIR}/resources/* ${APP_RESOURCES_DIRECTORY}
  )
  add_definitions(-DRESOURCES_FOLDER="../Resources")
endif(APPLE)

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

if(NOT APPLE)
  add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND cp -rf ${PROJECT_SOURCE_DIR}/resources ${PROJECT_BINARY_DIR}/bin
  )

  add_definitions(-DRESOURCES_FOLDER="../../resources")
endif(NOT APPLE)

if(MINGW OR UNIX)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
endif(MINGW OR UNIX)

