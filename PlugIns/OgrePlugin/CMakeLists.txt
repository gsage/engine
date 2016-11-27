set(LIB_NAME "OgrePluginStatic1.9")
set(PLUGIN_NAME "OgrePlugin1.9")

include_directories(
  include
  ${gsage_SOURCE_DIR}/GsageCore/include
  ${gsage_SOURCE_DIR}/Components/OgreCrowd/include
  ${gsage_SOURCE_DIR}/Components/Sol2/include
  ${gsage_SOURCE_DIR}/PlugIns/ParticleUniverse/include

  ${OIS_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_Overlay_INCLUDE_DIR}
  ${OGRE_Terrain_INCLUDE_DIR}
  ${LUAJIT_INCLUDE_DIR}
  ${OGRE_SAMPLES_INCLUDEPATH}
  ${JSONCPP_INCLUDE_DIR}
)

file(GLOB sources src/*.cpp src/*/*.cpp )

# static library
add_library(${LIB_NAME} STATIC ${sources})
set(LIBS
  OgreCrowd
  GsageCore
  ${OGRE_Terrain_LIBRARY}
  ${OGRE_Overlay_LIBRARY}
  ${OIS_LIBRARIES}
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

target_link_libraries(${LIB_NAME} ${LIBS})

install(DIRECTORY include/ DESTINATION /usr/local/include/gsage
  FILES_MATCHING PATTERN "*.(h|hpp)"
)

install(TARGETS ${LIB_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(TARGETS ${LIB_NAME} DESTINATION /usr/local/lib)

# Dynamic plugin

add_definitions(-DPLUGIN_EXPORT)
add_library(${PLUGIN_NAME} SHARED ${sources})

target_link_libraries(${PLUGIN_NAME} ${LIBS})

set_target_properties(${PLUGIN_NAME} PROPERTIES PREFIX "")

if(NOT APPLE)
  set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/PlugIns)
endif(NOT APPLE)
