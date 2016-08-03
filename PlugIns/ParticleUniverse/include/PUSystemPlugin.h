#ifndef _PUSystemPlugin_H_
#define _PUSystemPlugin_H_

#include "IPlugin.h"

#if GSAGE_PLATFORM == GSAGE_WIN32
#ifdef PLUGIN_EXPORT
#define PluginExport __declspec (dllexport)
#else
#define PluginExport __declspec (dllimport)
#endif
#else
#define PluginExport
#endif

namespace Gsage
{
  class Engine;
}

class LuaInterface;

namespace ParticleUniverseFactory
{
  class PUSystemPlugin : public IPlugin
  {
    public:
      PUSystemPlugin();
      virtual ~PUSystemPlugin();

      const std::string& getName() const;
      /**
       * Registers new factory for type puSystem
       */
      bool install();

      /**
       * Unregisters factory for type puSystem and removes all elements
       */
      void uninstall();
  };
}

#endif
