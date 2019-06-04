macro(configure)
  if(DEFINED ENV{BINARY_OUTPUT_DIR})
    set(BINARY_OUTPUT_DIR $ENV{BINARY_OUTPUT_DIR})
  else()
    set(BINARY_OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin)
  endif(DEFINED ENV{BINARY_OUTPUT_DIR})

  if(DEFINED ENV{GSAGE_VERSION_MAJOR})
    set(GSAGE_VERSION_MAJOR $ENV{GSAGE_VERSION_MAJOR})
  endif(DEFINED ENV{GSAGE_VERSION_MAJOR})

  if(DEFINED ENV{GSAGE_VERSION_MINOR})
    set(GSAGE_VERSION_MINOR $ENV{GSAGE_VERSION_MINOR})
  endif(DEFINED ENV{GSAGE_VERSION_MINOR})

  if(DEFINED ENV{GSAGE_VERSION_BUILD})
    set(GSAGE_VERSION_BUILD $ENV{GSAGE_VERSION_BUILD})
  endif(DEFINED ENV{GSAGE_VERSION_BUILD})

  if(WITH_METAL)
    add_definitions("-DWITH_METAL")
  endif(WITH_METAL)
  add_definitions(-DGSAGE_VERSION_MAJOR="${GSAGE_VERSION_MAJOR}" -DGSAGE_VERSION_MINOR="${GSAGE_VERSION_MINOR}" -DGSAGE_VERSION_BUILD="${GSAGE_VERSION_BUILD}")

  if(NOT EXISTS BINARY_OUTPUT_DIR)
    file(MAKE_DIRECTORY ${BINARY_OUTPUT_DIR})
  endif(NOT EXISTS BINARY_OUTPUT_DIR)

  if(WIN32)
    add_definitions( -DWIN32 )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  endif(WIN32)

  set(PLUGINS_PATH ${BINARY_OUTPUT_DIR}/PlugIns)

  if(NOT EXISTS PLUGINS_PATH)
    file(MAKE_DIRECTORY ${PLUGINS_PATH})
  endif(NOT EXISTS PLUGINS_PATH)

  if(APPLE AND NOT ANDROID)
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
  endif(APPLE AND NOT ANDROID)

  if(UNIX)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  endif(UNIX)
  if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  endif(MSVC)
  add_definitions(-DSOL_SAFE_USERTYPE=1)
  add_definitions(-DSOL_EXCEPTIONS_SAFE_PROPAGATION=1)
  add_definitions(-DSOL_SAFE_REFERENCES=1)
  add_definitions(-DSOL_SAFE_FUNCTIONS=1)
endmacro()

macro(gsage_plugin plugin_name)
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  gsage_includes()
  add_library(${plugin_name} SHARED ${ARGN})
  set_target_properties(${plugin_name}
    PROPERTIES
    PREFIX "")

  if(WIN32)
    set_target_properties(${plugin_name}
      PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_PATH}
      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PLUGINS_PATH}
      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PLUGINS_PATH}
      COMPILE_FLAGS "-DPLUGIN_EXPORT -D_USRDLL")
  endif(WIN32)

  if(WIN32)
    set(src_path "${PLUGINS_PATH}/${plugin_name}.dll")
    set(dst_path "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")
    STRING(REGEX REPLACE "/" "\\\\" src_path "${src_path}")
    STRING(REGEX REPLACE "/" "\\\\" dst_path "${dst_path}")

    add_custom_command(TARGET ${plugin_name} POST_BUILD
      COMMAND copy ${src_path} ${dst_path}
    )

  else(WIN32)
  set_target_properties(${plugin_name}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_PATH}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PLUGINS_PATH}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PLUGINS_PATH})
  endif(WIN32)

  if(APPLE)
    set_target_properties(${plugin_name}
      PROPERTIES BUILD_WITH_INSTALL_RPATH 1
      INSTALL_RPATH "@rpath;@executable_path/../"
      LINK_FLAGS "-Wl,-F${PROJECT_BINARY_DIR}/bin/Frameworks/"
    )
  endif(APPLE)
endmacro()

macro(gsage_includes)
  include_directories(
    ${LUAJIT_INCLUDE_DIR}
    ${gsage_SOURCE_DIR}/Vendor/Sol2/include
    ${gsage_SOURCE_DIR}/Vendor/jsoncpp/include
    ${gsage_SOURCE_DIR}/Core/include

    ${MSGPACK_INCLUDE_DIR}
  )
endmacro()

macro(cef_helper_app executable_name)
  add_executable(${executable_name} ${ARGN})

  if(APPLE)
    set_source_files_properties(${ARGN} PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_target_properties(${executable_name}
      PROPERTIES BUILD_WITH_INSTALL_RPATH 1
      INSTALL_RPATH "@rpath;@executable_path/../"
      LINK_FLAGS "-Wl,-F${PROJECT_BINARY_DIR}/bin/Frameworks/"
    )
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.gsage.${executable_name}")
    set(BINARY_OUTPUT_DIR "${BINARY_OUTPUT_DIR}/Frameworks")

    set(CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} ${PROJECT_BINARY_DIR}/bin/Frameworks/)

    set_property(TARGET ${executable_name} PROPERTY MACOSX_BUNDLE TRUE)
    set_property(TARGET ${executable_name} PROPERTY MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/resources/Info.plist)
    set(APP_CONTENTS "${BINARY_OUTPUT_DIR}/${executable_name}.app/Contents")
    set(APP_FRAMEWORKS_DIRECTORY "${APP_CONTENTS}/Frameworks")
    add_custom_command(TARGET ${executable_name} POST_BUILD
      COMMAND [ -L ${APP_FRAMEWORKS_DIRECTORY} ] || ln -s ${PROJECT_BINARY_DIR}/bin/Frameworks/ ${APP_FRAMEWORKS_DIRECTORY}
    )
  endif(APPLE)

  if(MINGW OR UNIX)
    set(EXECUTABLE_OUTPUT_PATH "${BINARY_OUTPUT_DIR}")
  endif(MINGW OR UNIX)
endmacro()

macro(console_executable executable_name)
  gsage_executable_generic(${executable_name} ${ARGN})
  set_target_properties(${executable_name} PROPERTIES DEBUG_POSTFIX _d)
endmacro()

macro(gsage_executable executable_name)
  gsage_executable_generic(${executable_name} ${ARGN})

  if(WIN32)
    set(LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    set_target_properties(${executable_name} PROPERTIES DEBUG_POSTFIX _d LINK_FLAGS
      ${LINK_FLAGS})
  else(WIN32)
    set_target_properties(${executable_name} PROPERTIES DEBUG_POSTFIX _d)
  endif(WIN32)
endmacro()

macro(gsage_executable_generic executable_name)
  add_executable(${executable_name} ${ARGN})

  if(APPLE)
    if(NOT console)
      set_source_files_properties(${ARGN} PROPERTIES COMPILE_FLAGS "-x objective-c++")
      set(MACOSX_BUNDLE_BUNDLE_NAME "${executable_name}")
      set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.gsage.${executable_name}")

      set(CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} ${PROJECT_BINARY_DIR}/bin/Frameworks/)

      set_property(TARGET ${executable_name} PROPERTY MACOSX_BUNDLE TRUE)
      set_property(TARGET ${executable_name} PROPERTY MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/resources/Info.plist)

      set(APP_CONTENTS "${BINARY_OUTPUT_DIR}/${executable_name}.app/Contents")
      set(APP_FRAMEWORKS_DIRECTORY "${APP_CONTENTS}/Frameworks")
      set(APP_RESOURCES_DIRECTORY "${APP_CONTENTS}/Resources")

      add_custom_command(TARGET ${executable_name} POST_BUILD
        COMMAND [ -L ${APP_FRAMEWORKS_DIRECTORY} ] || ln -s ${PROJECT_BINARY_DIR}/bin/Frameworks/ ${APP_FRAMEWORKS_DIRECTORY}
      )
      add_custom_command(TARGET ${executable_name} POST_BUILD
        COMMAND [ -L ${APP_RESOURCES_DIRECTORY} ] || ln -s ${PROJECT_SOURCE_DIR}/resources/ ${APP_RESOURCES_DIRECTORY}
      )
      add_custom_command(TARGET ${executable_name} POST_BUILD
        COMMAND [ -L ${APP_CONTENTS}/PlugIns ] || ln -s ${PLUGINS_PATH} ${APP_CONTENTS}/PlugIns
      )
    endif(NOT console)
    add_definitions(-DRESOURCES_FOLDER="../Resources")
  else(APPLE)
    add_definitions(-DRESOURCES_FOLDER="../../resources")
  endif(APPLE)

  if(MINGW OR UNIX)
    set(EXECUTABLE_OUTPUT_PATH "${BINARY_OUTPUT_DIR}")
  endif(MINGW OR UNIX)
endmacro()

macro(process_templates)
  if(APPLE)
    set(PLUGINS_DIRECTORY "../PlugIns")
    set(OGRE_PLUGINS "")
  else(APPLE)
    set(PLUGINS_DIRECTORY "PlugIns")
    if(OGRE_FOUND)
      get_filename_component(OGRE_PLUGINS "${OGRE_Plugin_OctreeSceneManager_LIBRARY_REL}" DIRECTORY)
    endif(OGRE_FOUND)
  endif(APPLE)
  configure_file(resources/testConfig.json.in ${gsage_SOURCE_DIR}/resources/testConfig.json)
  configure_file(resources/editorConfig.json.in ${gsage_SOURCE_DIR}/resources/editorConfig.json)
endmacro()
