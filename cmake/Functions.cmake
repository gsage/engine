macro(configure)
  if(DEFINED ENV{BINARY_OUTPUT_DIR})
    set(BINARY_OUTPUT_DIR $ENV{BINARY_OUTPUT_DIR})
  else()
    set(BINARY_OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin)
  endif(DEFINED ENV{BINARY_OUTPUT_DIR})

  if(NOT EXISTS BINARY_OUTPUT_DIR)
    file(MAKE_DIRECTORY ${BINARY_OUTPUT_DIR})
  endif(NOT EXISTS BINARY_OUTPUT_DIR)

  set(PLUGINS_PATH ${BINARY_OUTPUT_DIR}/PlugIns)

  if(NOT EXISTS PLUGINS_PATH)
    file(MAKE_DIRECTORY ${PLUGINS_PATH})
  endif(NOT EXISTS PLUGINS_PATH)

  if(APPLE)
    # This has to be before most other options so CMake properly handles the
    # compiler variables, it MUST bebefore the project() definition
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
      set ( CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 -image_base 7fff04c4a000" )
      option ( LUA_USE_POSIX "Use POSIX functionality." ON )
      option ( LUA_USE_DLOPEN "Use dynamic linker to load modules." ON )
      set ( LJVM_MODE machasm )
      message("-- Using fix for luajit")
    endif()

    option(BUILD_UNIVERSAL_BINARIES "Build universal binaries for all architectures supported" ON)
    if (NOT CMAKE_OSX_ARCHITECTURES AND BUILD_UNIVERSAL_BINARIES)
      if(IOS)
        # set the architecture for iOS
        if (${IOS_PLATFORM} STREQUAL "OS")
          set (IOS_ARCH armv6 armv7 armv7s arm64)
          set (CMAKE_OSX_ARCHITECTURES ${IOS_ARCH} CACHE string  "Build architecture for iOS")
        else (${IOS_PLATFORM} STREQUAL "OS")
          set (IOS_ARCH i386 x86_64)
          set (CMAKE_OSX_ARCHITECTURES ${IOS_ARCH} CACHE string  "Build architecture for iOS Simulator")
        endif (${IOS_PLATFORM} STREQUAL "OS")

      else(IOS)
        # set the architectures for OS X
        set (OSXI_ARCH i386 x86_64)
        set (CMAKE_OSX_ARCHITECTURES ${OSXI_ARCH} CACHE string  "Build architecture for OS X universal binaries")
      endif(IOS)
    endif (NOT CMAKE_OSX_ARCHITECTURES AND BUILD_UNIVERSAL_BINARIES)
  endif(APPLE)

  if(WIN32)
    add_definitions( -DWIN32 )
  endif(WIN32)

  if(UNIX)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_FLAGS "-fPIC -O0 ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
  endif(UNIX)
endmacro()

macro(gsage_plugin plugin_name)
  add_library(${plugin_name} SHARED ${ARGN})
  add_definitions(-DPLUGIN_EXPORT -D_USRDLL)

  set_target_properties(${plugin_name}
    PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_PATH})
endmacro()

macro(gsage_executable executable_name)
  add_executable(${executable_name} ${ARGN})

  if(APPLE)
    set_source_files_properties(${ARGN} PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.gsage.${executable_name}")

    set_property(TARGET ${executable_name} PROPERTY MACOSX_BUNDLE TRUE)
    set_property(TARGET ${executable_name} PROPERTY MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/resources/Info.plist)

    set(APP_CONTENTS "${BINARY_OUTPUT_DIR}/${executable_name}.app/Contents")
    set(APP_FRAMEWORKS_DIRECTORY "${APP_CONTENTS}/Frameworks")
    set(APP_RESOURCES_DIRECTORY "${APP_CONTENTS}/Resources")

    add_custom_command(TARGET ${executable_name} POST_BUILD
      COMMAND [ -L ${APP_FRAMEWORKS_DIRECTORY} ] || ln -s $ENV{OGRE_HOME}/lib/${CMAKE_BUILD_TYPE}/ ${APP_FRAMEWORKS_DIRECTORY}
    )
    add_custom_command(TARGET ${executable_name} POST_BUILD
      COMMAND [ -L ${APP_RESOURCES_DIRECTORY} ] || ln -s ${PROJECT_SOURCE_DIR}/resources/ ${APP_RESOURCES_DIRECTORY}
    )
    add_custom_command(TARGET ${executable_name} POST_BUILD
      COMMAND [ -L ${APP_CONTENTS}/PlugIns ] || ln -s ${PLUGINS_PATH} ${APP_CONTENTS}/PlugIns
    )
    add_definitions(-DRESOURCES_FOLDER="../Resources")
  else(APPLE)
    add_definitions(-DRESOURCES_FOLDER="../../resources")
  endif(APPLE)

  if(WIN32)
    set(LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    set_target_properties(${executable_name} PROPERTIES DEBUG_POSTFIX _d LINK_FLAGS
      ${LINK_FLAGS})
  else(WIN32)
    set_target_properties(${executable_name} PROPERTIES DEBUG_POSTFIX _d)
  endif(WIN32)

  # post-build copy for win32
  if(WIN32 AND NOT MINGW)
    add_custom_command( TARGET ${executable_name} PRE_BUILD
      COMMAND if not exist ${BINARY_OUTPUT_DIR} mkdir ${BINARY_OUTPUT_DIR})
    add_custom_command( TARGET ${executable_name} POST_BUILD
      COMMAND copy \"$(TargetPath)\" ${BINARY_OUTPUT_DIR} )
  endif(WIN32 AND NOT MINGW)

  if(MINGW OR UNIX)
    set(EXECUTABLE_OUTPUT_PATH ${BINARY_OUTPUT_DIR})
  endif(MINGW OR UNIX)
endmacro()

macro(process_templates)
  if(APPLE)
    set(PLUGINS_DIRECTORY "../PlugIns")
    set(OGRE_PLUGINS "")
  else(APPLE)
    set(PLUGINS_DIRECTORY "PlugIns")
    if(OGRE_FOUND)
      get_filename_component(OGRE_PLUGINS ${OGRE_Plugin_OctreeSceneManager_LIBRARY_REL} PATH)
    endif(OGRE_FOUND)
  endif(APPLE)
  configure_file(resources/testConfig.json.in ${gsage_SOURCE_DIR}/resources/testConfig.json)
  configure_file(resources/gameConfig.json.in ${gsage_SOURCE_DIR}/resources/gameConfig.json)
  configure_file(resources/plugins.cfg.in ${gsage_SOURCE_DIR}/resources/plugins.cfg)
endmacro()
