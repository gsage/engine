set(PLUGIN_NAME "OgrePlugin")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${gsage_SOURCE_DIR}/Vendor/OgreCrowd/include
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${gsage_SOURCE_DIR}/Vendor/gmath/src
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_Overlay_INCLUDE_DIR}
  ${OGRE_Terrain_INCLUDE_DIR}
  ${OGRE_SAMPLES_INCLUDEPATH}
)

file(GLOB sources src/*.cpp src/*/*.cpp )

set(LIBS
  OgreCrowd
  GsageCore
  easyloggingpp
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
    ${OpenGL_LIBRARY}
    ${OPENGL_glu_LIBRARY}
  )
endif(APPLE)

if(WIN32)
  find_package(OpenGL)
  set(LIBS
    ${OPENGL_LIBRARIES}
    ${CONAN_LIBS_OGRE}
    ${LIBS}
  )
endif(WIN32)

# Linux specific
if(UNIX AND NOT APPLE)
  find_package(OpenGL)
  find_package(X11)
  find_package(Threads REQUIRED)
  find_library(Xaw Xaw7 PATHS ${X11_LIB_SEARCH_PATH})

  set(THREADS_PREFER_PTHREAD_FLAG ON)

  set(LIBS
    ${OPENGL_LIBRARIES}
    ${X11_LIBRARIES}
    -Wl,--whole-archive
    ${CMAKE_DL_LIBS}
    ${X11_Xrandr_LIB}
    ${X11_Xext_LIB}
    ${Xaw}
    Threads::Threads
    ${CONAN_LIBS_OGRE}
    -Wl,--no-whole-archive
    ${LIBS})
endif(UNIX AND NOT APPLE)

install(DIRECTORY include/ DESTINATION include/gsage
  FILES_MATCHING PATTERN "*.(h|hpp)"
)

install(TARGETS ${LIB_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

# Dynamic plugin

gsage_plugin(${PLUGIN_NAME} ${sources})

target_link_libraries(${PLUGIN_NAME} ${LIBS})
set(OGRE_PLUGIN_VERSION_MAJOR 1)
set(OGRE_PLUGIN_VERSION_MINOR 9)
set(OGRE_PLUGIN_VERSION_PATCH 0)
set(OGRE_PLUGIN_VERSION_STRING ${OGRE_PLUGIN_VERSION_MAJOR}.${OGRE_PLUGIN_VERSION_MINOR}.${OGRE_PLUGIN_VERSION_PATCH})

set_target_properties(${PLUGIN_NAME} PROPERTIES VERSION ${OGRE_PLUGIN_VERSION_STRING} COMPILE_FLAGS -DOGRE_PLUGIN_EXPORT)
