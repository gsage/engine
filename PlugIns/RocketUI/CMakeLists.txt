set(LIB_NAME "RocketUIPlugin")

include_directories(
    ${gsage_SOURCE_DIR}/Vendor/gmath/src
    ${gsage_SOURCE_DIR}/Vendor/easylogging/include
    ${gsage_SOURCE_DIR}/Vendor/cpp-channel/include
    ${LIBROCKET_INCLUDE_DIRS}
)

set(LIBROCKET_LIBS ${LIBROCKET_LIBRARIES})

if(UNIX AND NOT APPLE)
  set(LIBROCKET_LIBS -Wl,--whole-archive ${LIBROCKET_LIBRARIES} -Wl,--no-whole-archive)
endif(UNIX AND NOT APPLE)

set(LIBS GsageCore cpp-channel ${LIBROCKET_LIBS})

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

gsage_plugin(${LIB_NAME} ${sources})
target_link_libraries(${LIB_NAME} ${LIBS})
set_target_properties(${LIB_NAME} PROPERTIES COMPILE_FLAGS -DROCKET_STATIC_LIB)
