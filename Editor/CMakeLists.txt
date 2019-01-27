set(APP_NAME "GsageEditor")

include_directories(include)
include_directories(${gsage_SOURCE_DIR}/GsageCore/include)
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

gsage_executable(${APP_NAME} ${sources})

if(APPLE)
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY}
    ${CARBON}
    ${IOKIT}
    ${COCOA_LIBRARY}
  )
endif(APPLE)

target_link_libraries(${APP_NAME} ${LIBS})
