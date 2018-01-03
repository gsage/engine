#if(${OGRE_Plugin_ParticleUniverse_FOUND})
#  add_subdirectory(ParticleUniverse)
#endif(${OGRE_Plugin_ParticleUniverse_FOUND})

if(${OGRE_FOUND})
  add_subdirectory(OgrePlugin)
endif(${OGRE_FOUND})

if(${LIBROCKET_FOUND})
  add_subdirectory(RocketUI)
endif(${LIBROCKET_FOUND})

if(${OIS_FOUND})
  add_subdirectory(OisInput)
endif(${OIS_FOUND})

if(${SDL_FOUND})
  add_subdirectory(SDL)
endif(${SDL_FOUND})

add_subdirectory(ImGUI)
