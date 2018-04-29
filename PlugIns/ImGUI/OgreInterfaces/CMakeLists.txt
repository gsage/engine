set(LIB_NAME "ImGUIOgreRenderer")

include_directories(
  ${gsage_SOURCE_DIR}/PlugIns/ImGUI/Common/include
  ${gsage_SOURCE_DIR}/Vendor/imgui
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${gsage_SOURCE_DIR}/Vendor/sole
  ${gsage_SOURCE_DIR}/Vendor/gmath/src
  ${gsage_SOURCE_DIR}/PlugIns/OgrePlugin/include
  ${OGRE_INCLUDE_DIRS}
)

set(LIBS
  GsageCore
  imgui
  ImGUIPlugin
  OgrePlugin
)

file(GLOB sources src/*.cpp)
if("${OGRE_VERSION_MAJOR}" STREQUAL "2")
  file(GLOB version_dependent src/v2/*.cpp )
else("${OGRE_VERSION_MAJOR}" STREQUAL "2")
  file(GLOB version_dependent  src/v1/*.cpp)
endif("${OGRE_VERSION_MAJOR}" STREQUAL "2")
set(sources ${version_dependent} ${sources})

if(APPLE)
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY})
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/OSXUtils.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set(sources src/OSXUtils.mm ${sources})
endif(APPLE)


if(NOT CONAN_USER_OGRE_STATIC)
  set(LIBS ${LIBS} ${OGRE_LIBRARIES} ${OGRE_Overlay_LIBRARIES})
endif(NOT CONAN_USER_OGRE_STATIC)

gsage_plugin(${LIB_NAME} SHARED ${sources})
target_link_libraries(${LIB_NAME} ${LIBS})
set(OGRE_PLUGIN_VERSION_MAJOR ${OGRE_VERSION_MAJOR})
set(OGRE_PLUGIN_VERSION_MINOR ${OGRE_VERSION_MINOR})
set(OGRE_PLUGIN_VERSION_PATCH ${OGRE_VERSION_PATCH})
set(OGRE_PLUGIN_VERSION_STRING ${OGRE_PLUGIN_VERSION_MAJOR}.${OGRE_PLUGIN_VERSION_MINOR}.${OGRE_PLUGIN_VERSION_PATCH})

set_target_properties(${PLUGIN_NAME} PROPERTIES VERSION ${OGRE_PLUGIN_VERSION_STRING})
