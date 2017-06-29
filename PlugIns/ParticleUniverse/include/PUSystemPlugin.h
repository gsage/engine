#ifndef _PUSystemPlugin_H_
#define _PUSystemPlugin_H_

#include "IPlugin.h"

namespace Gsage
{
  class Engine;
  class LuaInterface;
  class PUSystemPlugin : public IPlugin
  {
    public:
      PUSystemPlugin();
      virtual ~PUSystemPlugin();

      const std::string& getName() const;
      /**
       * Registers new factory for type puSystem
       */
      bool installImpl();

      /**
       * Unregisters factory for type puSystem and removes all elements
       */
      void uninstallImpl();

      /**
       * Set up lua bindings for a new particle emitter
       */
      void setupLuaBindings();
    private:
      bool handleSystemAdded(const Event& event);
  };
}

#endif
