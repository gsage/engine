#if GSAGE_PLATFORM == GSAGE_WIN32
#ifdef OGRE_PLUGIN_EXPORT
#define GSAGE_OGRE_PLUGIN_API __declspec(dllexport)
#else
#define GSAGE_OGRE_PLUGIN_API __declspec(dllimport)
#endif
#else
#define GSAGE_OGRE_PLUGIN_API
#endif
