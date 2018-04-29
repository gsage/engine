#if(${OGRE_Plugin_ParticleUniverse_FOUND})
#  add_subdirectory(ParticleUniverse)
#endif(${OGRE_Plugin_ParticleUniverse_FOUND})

if(${LIBROCKET_FOUND} AND "${OGRE_VERSION_MAJOR}" STREQUAL "1")
  add_subdirectory(RocketUI)
endif(${LIBROCKET_FOUND} AND "${OGRE_VERSION_MAJOR}" STREQUAL "1")

if(OIS_FOUND AND "${WITH_OIS}" STREQUAL "True")
  add_subdirectory(OisInput)
endif(OIS_FOUND AND "${WITH_OIS}" STREQUAL "True")

if(${SDL_FOUND})
  add_subdirectory(SDL)
endif(${SDL_FOUND})

if(${OGRE_FOUND})
  add_subdirectory(OgrePlugin)
endif(${OGRE_FOUND})

if(${RECAST_FOUND})
  add_subdirectory(RecastNavigation)
endif(${RECAST_FOUND})

add_subdirectory(ImGUI)
