set(LIB_NAME "RecastNavigationPlugin")

include_directories(
  ${gsage_SOURCE_DIR}/Vendor/imgui
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${gsage_SOURCE_DIR}/Vendor/gmath/src
  ${SDL_INCLUDE_DIRS}
  ${RECAST_INCLUDE_DIRS}
)

set(LIBS
  GsageCore
  easyloggingpp
  ${RECAST_LIBRARIES}
)

file(GLOB sources src/*.cpp src/*/*.cpp)

gsage_plugin(${LIB_NAME} SHARED ${sources})
target_link_libraries(${LIB_NAME} ${LIBS})
