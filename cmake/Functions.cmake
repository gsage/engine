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

  if(DEFINED ENV{GSAGE_VERSION_PATCH})
    set(GSAGE_VERSION_PATCH $ENV{GSAGE_VERSION_PATCH})
  endif(DEFINED ENV{GSAGE_VERSION_PATCH})

  if(DEFINED ENV{GSAGE_VERSION_BUILD})
    set(GSAGE_VERSION_BUILD $ENV{GSAGE_VERSION_BUILD})
  endif(DEFINED ENV{GSAGE_VERSION_BUILD})

  if(WITH_METAL)
    add_definitions("-DWITH_METAL")
  endif(WITH_METAL)

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

    if(NOT APPLE)
      set(INSTALL_BINARY_DIR "local/bin/gsage")
      set(INSTALL_LIB_DIR "local/lib/")
      set(INSTALL_PLUGINS_DIR "local/lib/gsage")
      set(INSTALL_RESOURCE_DIR "share/gsage/")

      list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_PATH}/${INSTALL_LIB_DIR}")
      list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_PATH}/${INSTALL_PLUGINS_DIR}")
    endif(NOT APPLE)
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

    install(TARGETS ${plugin_name}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib)
    install(TARGETS ${plugin_name} DESTINATION "${CMAKE_INSTALL_PATH}/PlugIns")

  else(WIN32)
    set_target_properties(${plugin_name}
      PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_PATH}
      LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PLUGINS_PATH}
      LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PLUGINS_PATH})
    install(TARGETS ${plugin_name} LIBRARY DESTINATION "${CMAKE_INSTALL_PATH}/${INSTALL_PLUGINS_DIR}")
  endif(WIN32)

  if(APPLE)
    set_target_properties(${plugin_name}
      PROPERTIES BUILD_WITH_INSTALL_RPATH 1
      INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/../;@rpath"
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
      INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/../;@rpath"
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

  install(TARGETS ${executable_name} DESTINATION "${CMAKE_INSTALL_PATH}/${INSTALL_BINARY_DIR}")
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
    set_source_files_properties(${ARGN} PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set(MACOSX_BUNDLE_BUNDLE_NAME "${executable_name}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.gsage.${executable_name}")

    set(CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} ${PROJECT_BINARY_DIR}/bin/Frameworks/)

    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.gsage.engine.${executable_name}")
    set(MACOSX_BUNDLE_BUNDLE_NAME "${executable_name}")
    set(MACOSX_BUNDLE_ICON_FILE "editor/${executable_name}.icns")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${GSAGE_VERSION_MAJOR}.${GSAGE_VERSION_MINOR}.${GSAGE_VERSION_BUILD}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${GSAGE_VERSION_MAJOR}.${GSAGE_VERSION_MINOR}")

    set(INFO_PLIST_FILE ${CMAKE_CURRENT_BINARY_DIR}/${executable_name}.plist)
    configure_file(${gsage_SOURCE_DIR}/Info.plist.in ${INFO_PLIST_FILE})

    set_property(TARGET ${executable_name} PROPERTY MACOSX_BUNDLE TRUE)
    set_property(TARGET ${executable_name} PROPERTY MACOSX_BUNDLE_INFO_PLIST ${INFO_PLIST_FILE})

    set(APP_CONTENTS "${BINARY_OUTPUT_DIR}/${executable_name}.app/Contents")
    set(APP_FRAMEWORKS_DIRECTORY "${APP_CONTENTS}/Frameworks")
    set(APP_RESOURCES_DIRECTORY "${APP_CONTENTS}/Resources")
    set(APP_PLUGINS_DIRECTORY "${APP_CONTENTS}/PlugIns")

    add_custom_command(TARGET ${executable_name} POST_BUILD
      COMMAND [ -L ${APP_FRAMEWORKS_DIRECTORY} ] || ln -s ${PROJECT_BINARY_DIR}/bin/Frameworks/ ${APP_FRAMEWORKS_DIRECTORY}
    )
    add_custom_command(TARGET ${executable_name} POST_BUILD
      COMMAND [ -L ${APP_RESOURCES_DIRECTORY} ] || ln -s ${PROJECT_SOURCE_DIR}/resources/ ${APP_RESOURCES_DIRECTORY}
    )
    add_custom_command(TARGET ${executable_name} POST_BUILD
      COMMAND [ -L ${APP_PLUGINS_DIRECTORY} ] || ln -s ${PLUGINS_PATH} ${APP_PLUGINS_DIRECTORY}
    )
    set_target_properties(${executable_name}
      PROPERTIES BUILD_WITH_INSTALL_RPATH 1
      INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/../;@executable_path/../Plugins;@rpath"
    )
    add_definitions(-DRESOURCES_FOLDER="../Resources")
  else(APPLE)
    add_definitions(-DRESOURCES_FOLDER="../../resources")
  endif(APPLE)

  if(MINGW OR UNIX)
    set(EXECUTABLE_OUTPUT_PATH "${BINARY_OUTPUT_DIR}")
  endif(MINGW OR UNIX)

  install(TARGETS ${executable_name} DESTINATION "${CMAKE_INSTALL_PATH}/${INSTALL_BINARY_DIR}" COMPONENT "${executable_name}")
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

macro(gsage_library library_name type)
  add_library(${library_name} ${type} ${ARGN})

  # build library as a framework
  if("${type}" STREQUAL "SHARED" AND APPLE)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
    set_target_properties(${library_name} PROPERTIES
      FRAMEWORK TRUE
      MACOSX_FRAMEWORK_IDENTIFIER org.gsage.engine.GsageCore
      MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
    )
    install(TARGETS ${library_name} DESTINATION "${PROJECT_BINARY_DIR}/bin/Frameworks/")
  else("${type}" STREQUAL "SHARED" AND APPLE)
    install(TARGETS ${library_name} DESTINATION "${CMAKE_INSTALL_PATH}/${INSTALL_LIB_DIR}")
  endif("${type}" STREQUAL "SHARED" AND APPLE)

endmacro()

macro(install_includes)
  foreach(include IN ITEMS ${ARGN})
    install(DIRECTORY "${include}"
            DESTINATION "${CMAKE_INSTALL_PATH}/include"
            COMPONENT "headers"
            FILES_MATCHING REGEX ".*")
  endforeach(include)
endmacro()

macro(install_resources)
  install(DIRECTORY cmake DESTINATION "${CMAKE_INSTALL_PATH}/${INSTALL_RESOURCE_DIR}")
  if(APPLE)
    set(DYNLIB_PATTERN "*.dylib")
  elseif(WIN32)
    set(DYNLIB_PATTERN "*.dll")
  else(APPLE)
    set(DYNLIB_PATTERN "*.so")
  endif(APPLE)

  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bundle/"
    DESTINATION "${CMAKE_INSTALL_PATH}/${INSTALL_RESOURCE_DIR}"
    FILES_MATCHING
    PATTERN "${DYNLIB_PATTERN}" EXCLUDE
    PATTERN "locales/*.pak"
  )

  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bundle/"
    DESTINATION "${CMAKE_INSTALL_PATH}/${INSTALL_BINARY_DIR}"
    FILES_MATCHING
    PATTERN "cef*.pak"
    PATTERN "devtools_resources.pak"
    PATTERN "*.bin"
    PATTERN "*.dat"
  )

  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bundle/"
    DESTINATION "${CMAKE_INSTALL_PATH}/${INSTALL_LIB_DIR}"
    FILES_MATCHING
    PATTERN "${DYNLIB_PATTERN}"
  )

  install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/"
    DESTINATION "${CMAKE_INSTALL_PATH}/${INSTALL_RESOURCE_DIR}/resources"
    COMPONENT "resources")

  set(DESKTOP_FILE "${gsage_SOURCE_DIR}/resources/editor/gsage.desktop")
  configure_file("${DESKTOP_FILE}.in" "${DESKTOP_FILE}")
  install(FILES "${DESKTOP_FILE}" DESTINATION share/applications)
endmacro()
