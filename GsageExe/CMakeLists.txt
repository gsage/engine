set(APP_NAME "GsageExe")

include_directories(
  include
  ${gsage_SOURCE_DIR}/GsageCore/include
  ${gsage_SOURCE_DIR}/Components/Sol2/include

  ${JSONCPP_INCLUDE_DIR}
  ${LUAJIT_INCLUDE_DIR}
)

FILE(GLOB headers include/*.h include/*/*.h)

FILE(GLOB sources src/*.cpp s src/*/*.cpp )

gsage_executable(${APP_NAME} ${sources})

set(LIBS
  GsageCore
  ${LUAJIT_LIBRARIES})

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
