set(LIB_NAME "ImGUIPlugin")

include_directories(
  ${gsage_SOURCE_DIR}/Vendor/imgui
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${gsage_SOURCE_DIR}/Vendor/sole
  ${gsage_SOURCE_DIR}/Vendor/gmath/src
)

set(LIBS
  GsageCore
  imgui
)

file(GLOB sources src/*.cpp)

gsage_plugin(${LIB_NAME} SHARED ${sources})
target_link_libraries(${LIB_NAME} ${LIBS})
set_target_properties(${LIB_NAME} PROPERTIES COMPILE_FLAGS -DIMGUI_PLUGIN_EXPORT)
