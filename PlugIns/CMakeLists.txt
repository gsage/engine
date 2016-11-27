#if(${OGRE_Plugin_ParticleUniverse_FOUND})
#add_subdirectory(ParticleUniverse)
#endif(${OGRE_Plugin_ParticleUniverse_FOUND})

if(${OGRE_FOUND})
  add_subdirectory(OgrePlugin)
endif(${OGRE_FOUND})
