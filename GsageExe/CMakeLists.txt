set(APP_NAME "GsageExe")

include_directories(
  include
  ${gsage_SOURCE_DIR}/GsageCore/include
  ${gsage_SOURCE_DIR}/Vendor/Sol2/include
  ${gsage_SOURCE_DIR}/Vendor/jsoncpp/include
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${gsage_SOURCE_DIR}/Vendor/cpp-channel/include

  ${LUAJIT_INCLUDE_DIR}
)

FILE(GLOB headers include/*.h include/*/*.h)

FILE(GLOB sources src/*.cpp s src/*/*.cpp )

gsage_executable(${APP_NAME} ${sources})

set(LIBS
  GsageCore
  easyloggingpp
  cpp-channel
)

if(APPLE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY}
    ${CARBON}
    ${IOKIT}
    ${COCOA_LIBRARY}
    )
endif(APPLE)

target_link_libraries(${APP_NAME} ${LIBS})
