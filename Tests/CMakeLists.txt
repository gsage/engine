set(APP_NAME "unit-tests")

set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Core
  GsageFacade ${OGRE_LIBRARIES} ${gtest_LIBRARY} ${LIBRARY_NAME})

include_directories(${cosmos_SOURCE_DIR}/Core/include)
include_directories(${cosmos_SOURCE_DIR}/GsageFacade/include)

include_directories(
  ${OGRE_INCLUDE_DIRS}
  ${gtest_INCLUDE_DIR}
)

set(TEST_FILES
  Core/TestEngine.cpp
  Core/TestEventDispatcher.cpp
  Core/TestSerializable.cpp
  Core/TestPtreeExtensions.cpp)

ADD_EXECUTABLE(${APP_NAME} main.cpp ${TEST_FILES})

set_target_properties(${APP_NAME} PROPERTIES DEBUG_POSTFIX _d)

target_link_libraries(${APP_NAME} ${TEST_DEPENDENCIES})

# post-build copy for win32
if(WIN32 AND NOT MINGW)
  add_custom_command( TARGET ${APP_NAME} PRE_BUILD
    COMMAND if not exist .\\bin mkdir .\\bin )
  add_custom_command( TARGET ${APP_NAME} POST_BUILD
    COMMAND copy \"$(TargetPath)\" .\\bin )
endif(WIN32 AND NOT MINGW)

if(MINGW OR UNIX)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

  # TODO: fix it later, directory path is hardcoded D:
  # plus we have a shell script there, what is not a cross platform solution

  add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND cp -rf ${PROJECT_SOURCE_DIR}/Tests/resources ${PROJECT_BINARY_DIR}
  )
endif(MINGW OR UNIX)
