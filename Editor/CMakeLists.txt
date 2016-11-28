set(APP_NAME "GsageEditor")

include_directories(include)
include_directories(${gsage_SOURCE_DIR}/GsageCore/include)
include_directories(${gsage_SOURCE_DIR}/PlugIns/OgrePlugin/include)
include_directories(${gsage_SOURCE_DIR}/Components/imgui)
include_directories(${gsage_SOURCE_DIR}/Components/OgreCrowd/include)
include_directories(${gsage_SOURCE_DIR}/Components/Sol2/include)
include_directories(${gsage_SOURCE_DIR}/GsageFacade/include)
include_directories(${gsage_SOURCE_DIR}/RocketWrapper/include)

include_directories(
  ${LUAJIT_INCLUDE_DIR}
  ${OIS_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}
  ${PYBIND11_INCLUDE_DIRS})

FILE(GLOB sources src/*.cpp)

set(LIBS
  GsageCore
  RocketOgre
  OgrePluginStatic1.9
  imgui
  ${LIBROCKET_LIBRARIES}
  ${OGRE_LIBRARIES}
  ${OIS_LIBRARIES}
  ${LUAJIT_LIBRARIES}
)

#set(PYTHON_MODULE_PREFIX ${PYTHON_MODULE_PREFIX} CACHE INTERNAL "")
#set(PYTHON_MODULE_EXTENSION ${PYTHON_MODULE_EXTENSION} CACHE INTERNAL "so")
add_executable(${APP_NAME} ${sources})

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
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.gsage.${APP_NAME}")
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

#add_library(${LIB_NAME} MODULE ${sources})
#target_link_libraries(${LIB_NAME} PRIVATE ${LIBS})
#target_link_libraries(editorv2 PRIVATE ${PYTHON_LIBRARIES})
#set_target_properties(${LIB_NAME} PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}")

#if(UNIX)
#  target_link_libraries(${LIB_NAME} PRIVATE "-undefined dynamic_lookup")
#endif(UNIX)
