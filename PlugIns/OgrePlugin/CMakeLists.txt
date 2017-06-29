set(PLUGIN_NAME "OgrePlugin")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${gsage_SOURCE_DIR}/Components/OgreCrowd/include
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_Overlay_INCLUDE_DIR}
  ${OGRE_Terrain_INCLUDE_DIR}
  ${OGRE_SAMPLES_INCLUDEPATH}
)

file(GLOB sources src/*.cpp src/*/*.cpp )

set(LIBS
  OgreCrowd
  GsageCore
  ${OGRE_Terrain_LIBRARY}
  ${OGRE_Overlay_LIBRARY}
)

if(APPLE)
  set(LIBS
    ${LIBS}
    ${COREFOUNDATION_LIBRARY}
    ${CARBON}
    ${IOKIT}
    ${COCOA_LIBRARY}
  )
endif(APPLE)

install(DIRECTORY include/ DESTINATION /usr/local/include/gsage
  FILES_MATCHING PATTERN "*.(h|hpp)"
)

install(TARGETS ${LIB_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(TARGETS ${LIB_NAME} DESTINATION /usr/local/lib)

# Dynamic plugin

gsage_plugin(${PLUGIN_NAME} ${sources})

target_link_libraries(${PLUGIN_NAME} ${LIBS})
set(OGRE_PLUGIN_VERSION_MAJOR 1)
set(OGRE_PLUGIN_VERSION_MINOR 9)
set(OGRE_PLUGIN_VERSION_PATCH 0)
set(OGRE_PLUGIN_VERSION_STRING ${OGRE_PLUGIN_VERSION_MAJOR}.${OGRE_PLUGIN_VERSION_MINOR}.${OGRE_PLUGIN_VERSION_PATCH})

set_target_properties(${PLUGIN_NAME} PROPERTIES VERSION ${OGRE_PLUGIN_VERSION_STRING})
