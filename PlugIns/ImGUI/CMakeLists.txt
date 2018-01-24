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

if(APPLE)
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY})
endif(APPLE)

if(OGRE_FOUND)
  include_directories(${OGRE_INCLUDE_DIRS}
          ${gsage_SOURCE_DIR}/PlugIns/OgrePlugin/include)

  set(LIBS ${LIBS} OgrePlugin)

  if(NOT CONAN_USER_OGRE_STATIC)
    set(LIBS ${LIBS} ${OGRE_LIBRARIES} ${OGRE_Overlay_LIBRARIES})
  endif(NOT CONAN_USER_OGRE_STATIC)

  file(GLOB ogre_related_sources src/ogre/*.cpp)
  set(sources ${ogre_related_sources} ${sources})
  add_definitions(-DOGRE_INTERFACE)
endif(OGRE_FOUND)

gsage_plugin(${LIB_NAME} SHARED ${sources})
target_link_libraries(${LIB_NAME} ${LIBS})
