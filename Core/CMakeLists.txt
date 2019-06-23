set(LIB_NAME "GsageCore")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS true)
FILE(GLOB headers include/*h include/*/*h)
FILE(GLOB sources src/*.cpp src/*/*.cpp)

if(APPLE)
  file(GLOB mac_sources src/*.mm src/*/*.mm)
  set_source_files_properties(${mac_sources} PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set(sources ${sources} ${mac_sources})
endif(APPLE)

include_directories(
  PRIVATE include/
  SYSTEM PUBLIC ${gsage_SOURCE_DIR}/Vendor/Sol2/include
  ${gsage_SOURCE_DIR}/Vendor/jsoncpp/include
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${gsage_SOURCE_DIR}/Vendor/gmath/src
  ${gsage_SOURCE_DIR}/Vendor/cpp-channel/include
  ${gsage_SOURCE_DIR}/Vendor/tinyfiledialogs/

  ${LUAJIT_INCLUDE_DIR}
  ${INJA_INCLUDE_DIRS}
  ${MSGPACK_INCLUDE_DIRS}
  ${POCO_INCLUDE_DIRS}
)

gsage_library(${LIB_NAME} SHARED ${sources})

set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX _d COMPILE_FLAGS -DGSAGE_DLL_EXPORT)

set(LIBS
  jsoncpp
  easyloggingpp
  cpp-channel
  tinyfiledialogs
  ${LUAJIT_LIBRARIES}
  ${INJA_LIBRARIES}
  ${MSGPACK_LIBRARIES}
  ${POCO_LIBRARIES}
)

if(APPLE)
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY}
    ${COCOA_LIBRARY}
  )
elseif(UNIX)
  find_package(Threads REQUIRED)
  set(LIBS ${CMAKE_DL_LIBS}
    -Wl,--whole-archive
    Threads::Threads
    -Wl,--no-whole-archive
    ${LIBS}
  )
endif(APPLE)

target_link_libraries(
  ${LIB_NAME}
  ${LIBS}
)

install_includes(
  include/
  ${gsage_SOURCE_DIR}/Vendor/jsoncpp/include/json
  ${gsage_SOURCE_DIR}/Vendor/Sol2/include/
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/Version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/Version.h)

set(APP_NAME "game")
set(PACKAGER_NAME "packager")

gsage_executable(${APP_NAME} cmd/app.cpp)
if(WIN32)
  console_executable(${PACKAGER_NAME} cmd/packager.cpp ${gsage_SOURCE_DIR}/resources/editor/packager.ico ${gsage_SOURCE_DIR}/resources/editor/packager.rc)
else(WIN32)
  console_executable(${PACKAGER_NAME} cmd/packager.cpp)
endif(WIN32)

set(LIBS
  GsageCore
  easyloggingpp
  cpp-channel
)

if(APPLE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/cmd/app.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY}
    ${IOKIT}
    ${COCOA_LIBRARY}
  )
endif(APPLE)

target_link_libraries(${APP_NAME} ${LIBS})
target_link_libraries(${PACKAGER_NAME} ${LIBS})
