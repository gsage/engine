set(LIB_NAME "SDLPlugin")

include_directories(
  ${gsage_SOURCE_DIR}/Vendor/imgui
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${SDL_INCLUDE_DIRS}
)

set(LIBS
  GsageCore
  easyloggingpp
  ${SDL_LIBRARIES}
)

file(GLOB sources src/*.cpp)

if(APPLE)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/OSXUtils.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set(sources src/OSXUtils.mm ${sources})
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY})
endif(APPLE)

gsage_plugin(${LIB_NAME} SHARED ${sources})
target_link_libraries(${LIB_NAME} ${LIBS})
