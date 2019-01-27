set(LIB_NAME "GsageCore")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS true)
FILE(GLOB headers include/*h include/*/*h)
FILE(GLOB sources src/*.cpp src/*/*.cpp)

if(APPLE)
  FILE(GLOB mac_sources src/*.mm)
  set(sources ${sources} ${mac_sources})
endif(APPLE)

include_directories(
  include/
  ${gsage_SOURCE_DIR}/Vendor/Sol2/include
  ${gsage_SOURCE_DIR}/Vendor/jsoncpp/include
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${gsage_SOURCE_DIR}/Vendor/gmath/src
  ${gsage_SOURCE_DIR}/Vendor/cpp-channel/include

  ${LUAJIT_INCLUDE_DIR}
  ${INJA_INCLUDE_DIRS}
  ${MSGPACK_INCLUDE_DIRS}
)

add_library(${LIB_NAME} SHARED ${headers} ${sources})

set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX _d COMPILE_FLAGS -DGSAGE_DLL_EXPORT)

set(LIBS
  jsoncpp
  easyloggingpp
  cpp-channel
  ${LUAJIT_LIBRARIES}
  ${INJA_LIBRARIES}
  ${MSGPACK_LIBRARIES})

if(APPLE)
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY})
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

install(DIRECTORY include/ DESTINATION include/gsage
  FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${gsage_SOURCE_DIR}/Vendor/jsoncpp/include/json DESTINATION include/gsage
  FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${gsage_SOURCE_DIR}/Vendor/Sol2/include/ DESTINATION include/gsage
  FILES_MATCHING PATTERN "*.hpp"
)

install(TARGETS ${LIB_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(TARGETS ${LIB_NAME} DESTINATION "${CMAKE_INSTALL_PATH}/lib")
