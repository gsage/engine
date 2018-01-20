/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef _I_PLUGIN_
#define _I_PLUGIN_

#include "EngineEvent.h"
#include "lua/LuaInterface.h"
#include "Logger.h"
#include "GsageFacade.h"

#if GSAGE_PLATFORM == GSAGE_WIN32
#ifdef PLUGIN_EXPORT
#define PluginExport __declspec (dllexport)
#else
#define PluginExport
#endif
#else
#define PluginExport
#endif

namespace Gsage {

  class LuaInterface;
  class GsageFacade;

  /**
   * Engine plugin interface
   */
  class IPlugin : public EventSubscriber<IPlugin>
  {

    public:
      IPlugin() : mFacade(0), mLuaInterface(0) {};
      virtual ~IPlugin() {};

      /**
       * Get unique plugin name
       */
      virtual const std::string& getName() const = 0;
      /**
       * Install plugin into the engine
       * @param engine Engine instance
       */
      virtual void initialize(GsageFacade* facade, LuaInterface* luaInterface = 0) { mFacade = facade; mLuaInterface = luaInterface; }

      /**
       * Initialize plugin
       */
      virtual bool install()
      {
        if(!el::Loggers::hasLogger("default")) {
          el::Configurations defaultConf;
          el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
          defaultConf.setToDefault();
          std::stringstream ss;
          ss << "%datetime %level %msg [%fbase:%line]";
          // To set GLOBAL configurations you may use
          defaultConf.setGlobally(
              el::ConfigurationType::Format, ss.str());
          el::Loggers::reconfigureLogger("default", defaultConf);
        }
        if(!installImpl()) {
          return false;
        }

        if(mLuaInterface && mLuaInterface->getState()){
          setupLuaBindings();
        }

        addEventListener(mFacade->getEngine(), EngineEvent::LUA_STATE_CHANGE, &IPlugin::handleLuaStateUpdate);
        return true;
      }

      /**
       * Pure virtual method for install
       */
      virtual bool installImpl() = 0;

      /**
       * Uninstall plugin from the engine
       */
      virtual void uninstall()
      {
        removeEventListener(mFacade->getEngine(), EngineEvent::LUA_STATE_CHANGE, &IPlugin::handleLuaStateUpdate);
        uninstallImpl();
      }

      /**
       * Pure virtual method for uninstall
       */
      virtual void uninstallImpl() = 0;

      /**
       * Set up lua bindings, override if the plugin will have any
       */
      virtual void setupLuaBindings() {};
    protected:
      bool handleLuaStateUpdate(EventDispatcher* sender, const Event& event) {
        setupLuaBindings();
        return true;
      }
      GsageFacade* mFacade;
      LuaInterface* mLuaInterface;
  };
}

#endif
