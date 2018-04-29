set(APP_NAME "unit-tests")

set(TEST_DEPENDENCIES
  ${TEST_DEPENDENCIES}
  GsageCore
  jsoncpp
  ImGUIPlugin
  easyloggingpp
  ${LUAJIT_LIBRARIES}
  ${GTEST_LIBRARIES}
)

include_directories(
  include
  ${gsage_SOURCE_DIR}/GsageCore/include
  ${gsage_SOURCE_DIR}/Vendor/Sol2/include
  ${gsage_SOURCE_DIR}/Vendor/jsoncpp/include
  ${gsage_SOURCE_DIR}/Vendor/imgui
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${gsage_SOURCE_DIR}/Vendor/gmath/src
  ${gsage_SOURCE_DIR}/PlugIns/ImGUI/Common/include

  ${LUAJIT_INCLUDE_DIR}
  ${GTEST_INCLUDE_DIRS}
)

add_subdirectory(TestPlugins)

set(TEST_FILES
  Core/TestEngine.cpp
  Core/TestEventDispatcher.cpp
  Core/TestSerializable.cpp
  Core/TestDataProxy.cpp
  Core/TestGsageFacade.cpp
  Core/TestFileLoader.cpp
  Core/TestPath.cpp
  Plugins/ImGUI/TestDockspace.cpp
)

if(APPLE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/lua.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES}
    ${COREFOUNDATION_LIBRARY}
    ${CARBON}
    ${IOKIT}
    ${COCOA_LIBRARY}
    )
endif(APPLE)

if(${OGRE_FOUND})
  include_directories(
    ${OGRE_INCLUDE_DIRS}
    ${gsage_SOURCE_DIR}/PlugIns/OgrePlugin/include
  )

  file(GLOB ogreTests Plugins/OgrePlugin/*.cpp)
  set(TEST_FILES ${ogreTests} ${TEST_FILES})
  if(UNIX AND NOT APPLE)
    find_package(OpenGL)
    find_package(X11)
    set(TEST_DEPENDENCIES ${OPENGL_LIBRARIES} ${X11_LIBRARIES} ${TEST_DEPENDENCIES})
  endif(UNIX AND NOT APPLE)

  set(TEST_DEPENDENCIES
    OgrePlugin
    ${TEST_DEPENDENCIES}
  )
endif(${OGRE_FOUND})

macro(test_runner app_name main test_files)
  gsage_executable(${app_name} ${main} ${test_files})

  set_target_properties(${app_name} PROPERTIES DEBUG_POSTFIX _d)
  set(LIBS ${TEST_DEPENDENCIES})

  target_link_libraries(${app_name} ${LIBS})

  # symlink resources for mac and linux
  if(MINGW OR UNIX)
    if(APPLE)
      set(TEST_RESOURCES ${APP_CONTENTS}/testResources)
    else(APPLE)
      set(TEST_RESOURCES ${BINARY_OUTPUT_DIR}/resources)
    endif(APPLE)

    add_custom_command(TARGET ${app_name} POST_BUILD
      COMMAND [ -L ${TEST_RESOURCES} ] || ln -s ${gsage_SOURCE_DIR}/Tests/resources ${TEST_RESOURCES}
    )
  endif(MINGW OR UNIX)

  # post-build copy for win32
  if(WIN32)
    set(resource_path \"${gsage_SOURCE_DIR}\\Tests\\resources\")
    set(runtime_path "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\\resources")

    STRING(REGEX REPLACE "/" "\\\\" resource_path "${resource_path}")
    STRING(REGEX REPLACE "/" "\\\\" runtime_path "${runtime_path}")

    add_custom_command( TARGET ${app_name} POST_BUILD
      COMMAND xcopy "${resource_path}" "${runtime_path}" /s /e /h /i /y)
  endif(WIN32)
endmacro()

test_runner("unit-tests" "unit.cpp" "${TEST_FILES}")
test_runner("functional-tests" "functional.cpp" "")
