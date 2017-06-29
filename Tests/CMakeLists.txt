set(APP_NAME "unit-tests")

set(TEST_DEPENDENCIES
  ${TEST_DEPENDENCIES}
  GsageCore
  gtest
  ${JSONCPP_LIBRARIES}
  ${LUAJIT_LIBRARIES}
)

include_directories(
  include
  ${gsage_SOURCE_DIR}/GsageCore/include
  ${gsage_SOURCE_DIR}/Components/Sol2/include
  ${gsage_SOURCE_DIR}/googletest/googletest/include

  ${LUAJIT_INCLUDE_DIR}
  ${JSONCPP_INCLUDE_DIR}
)

add_subdirectory(TestPlugins)

set(TEST_FILES
  Core/TestEngine.cpp
  Core/TestEventDispatcher.cpp
  Core/TestSerializable.cpp
  Core/TestDataProxy.cpp
  Core/TestDictionary.cpp
  Core/TestGsageFacade.cpp
  Core/TestFileLoader.cpp
)

if(${OGRE_FOUND})
  include_directories(
    ${OGRE_INCLUDE_DIRS}
    ${gsage_SOURCE_DIR}/PlugIns/OgrePlugin/include
  )
  set(TEST_DEPENDENCIES
    OgrePlugin
    ${TEST_DEPENDENCIES}
    ${OGRE_LIBRARIES}
  )
  file(GLOB ogreTests Plugins/OgrePlugin/*.cpp)
  set(TEST_FILES ${ogreTests} ${TEST_FILES})
endif(${OGRE_FOUND})

gsage_executable(${APP_NAME} main.cpp ${TEST_FILES})

set_target_properties(${APP_NAME} PROPERTIES DEBUG_POSTFIX _d)

target_link_libraries(${APP_NAME} ${TEST_DEPENDENCIES})

# post-build copy for win32
if(WIN32 AND NOT MINGW)
  add_custom_command( TARGET ${APP_NAME} PRE_BUILD
    COMMAND if not exist .\\bin mkdir .\\bin )
  add_custom_command( TARGET ${APP_NAME} POST_BUILD
    COMMAND copy \"$(TargetPath)\" .\\bin )
  add_custom_command( TARGET ${APP_NAME} POST_BUILD
    COMMAND copy \"${gsage_SOURCE_DIR}\\Tests\\resources\" .\\bin )
endif(WIN32 AND NOT MINGW)

# symlink resources for mac and linux
if(MINGW OR UNIX)
  if(APPLE)
    set(TEST_RESOURCES ${APP_CONTENTS}/testResources)
  else(APPLE)
    set(TEST_RESOURCES ${BINARY_OUTPUT_DIR}/resources)
  endif(APPLE)


  add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND [ -L ${TEST_RESOURCES} ] || ln -s ${gsage_SOURCE_DIR}/Tests/resources ${TEST_RESOURCES}
  )
endif(MINGW OR UNIX)
