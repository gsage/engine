cmake_minimum_required(VERSION 2.8.11)

set(APP_NAME "editor")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

include_directories(./application)
include_directories(./include)

FILE(GLOB headers include/*h include/*/*h)
FILE(GLOB sources src/*.cpp src/*/*.cpp src/*.c src/*/*.c src/*/*/*.c application/*.cpp)

set(sources ${sources} "main.cpp")

#qtquick_compiler_add_resources(RESOURCES example.qrc)
add_executable(${APP_NAME} ${sources})
qt5_use_modules(${APP_NAME} Qml Quick Core Network OpenGL)

include_directories(${cosmos_SOURCE_DIR}/Core/include)
include_directories(${cosmos_SOURCE_DIR}/Components/OgreCrowd/include)
include_directories(${cosmos_SOURCE_DIR}/Components/Sol2/include)
include_directories(${cosmos_SOURCE_DIR}/GsageFacade/include)
include_directories(${cosmos_SOURCE_DIR}/RocketWrapper/include)

include_directories(${OIS_INCLUDE_DIRS}
  ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}
  ${LUAJIT_INCLUDE_DIR}
  ${LIBROCKET_INCLUDE_DIRS}
)

if(WIN32)
  set_target_properties(${APP_NAME} PROPERTIES DEBUG_POSTFIX _d)
endif(WIN32)

set(LIBS
  GsageCore
  GsageFacade
  RocketOgre
  ${LIBROCKET_LIBRARIES}
  ${OGRE_LIBRARIES}
  ${OIS_LIBRARIES}
  ${LUAJIT_LIBRARIES}
  )

if(APPLE)
  set_property(TARGET ${APP_NAME} PROPERTY MACOSX_BUNDLE TRUE)
  set_property(TARGET ${APP_NAME} PROPERTY MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/resources/Info.plist)
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.gsage.Editor")
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
  find_package(OpenGL REQUIRED)
  set(LIBS ${LIBS} ${OPENGL_LIBRARIES})
else(APPLE)
  add_definitions(-DRESOURCES_FOLDER="../../resources")
endif(APPLE)

target_link_libraries(${APP_NAME} ${LIBS})

# post-build copy for win32
if(WIN32 AND NOT MINGW)
  add_custom_command( TARGET ${APP_NAME} PRE_BUILD
    COMMAND if not exist .\\bin mkdir .\\bin )
  add_custom_command( TARGET ${APP_NAME} POST_BUILD
    COMMAND copy \"$(TargetPath)\" .\\bin )
endif(WIN32 AND NOT MINGW)

if(MINGW OR UNIX)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
endif(MINGW OR UNIX)

