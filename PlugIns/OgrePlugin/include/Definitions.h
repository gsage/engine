#ifndef __DEFINITIONS__
#define __DEFINITIONS__

#include "GsageDefinitions.h"

#if GSAGE_PLATFORM == GSAGE_WIN32
#ifdef OGRE_PLUGIN_EXPORT
#define GSAGE_OGRE_PLUGIN_API __declspec(dllexport)
#else
#define GSAGE_OGRE_PLUGIN_API __declspec(dllimport)
#endif
#else
#define GSAGE_OGRE_PLUGIN_API
#endif

#include "OgrePrerequisites.h"

#if OGRE_VERSION_MAJOR == 2
#define OgreV1 Ogre::v1
// helper method to get vertex/index data
#define GET_IV_DATA(variable) variable[Ogre::VpNormal]
#else
#define OgreV1 Ogre
// helper method to get vertex/index data
#define GET_IV_DATA(variable) variable
#endif

#endif
