set(APP_NAME "gsage")

include_directories(include)
include_directories(${gsage_SOURCE_DIR}/Core/include)
include_directories(${gsage_SOURCE_DIR}/Vendor/OgreCrowd/include)
include_directories(${gsage_SOURCE_DIR}/Vendor/Sol2/include)
include_directories(${gsage_SOURCE_DIR}/Vendor/jsoncpp/include)
include_directories(${gsage_SOURCE_DIR}/Vendor/easylogging/include)
include_directories(${gsage_SOURCE_DIR}/Vendor/cpp-channel/include)

include_directories(${LUAJIT_INCLUDE_DIR})

FILE(GLOB sources src/*.cpp)

set(LIBS
  GsageCore
  easyloggingpp
  cpp-channel
  ${LUAJIT_LIBRARIES}
)

if(WIN32)
  gsage_executable(${APP_NAME} ${sources} ${gsage_SOURCE_DIR}/resources/editor/gsage.ico ${gsage_SOURCE_DIR}/resources/editor/gsage.rc)
else(WIN32)
  gsage_executable(${APP_NAME} ${sources})
endif(WIN32)

if(APPLE)
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY}
    ${CARBON}
    ${IOKIT}
    ${COCOA_LIBRARY}
  )
endif(APPLE)

target_link_libraries(${APP_NAME} ${LIBS})
