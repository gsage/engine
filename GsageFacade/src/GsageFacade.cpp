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

#include "GsageFacade.h"

#include <boost/property_tree/json_parser.hpp>

#include "UIManager.h"
#include "Logger.h"
#include "GameDataManager.h"
#include "IPlugin.h"

#include "components/MovementComponent.h"
#include "components/RenderComponent.h"

#include "systems/OgreRenderSystem.h"

#include "lua/LuaInterface.h"

#include "EngineEvent.h"
#include "OgreDynLibManager.h"

_INITIALIZE_EASYLOGGINGPP

namespace Gsage {

  const std::string PLUGINS_SECTION = "plugins";
  const std::string GsageFacade::LOAD = "load";
  const std::string GsageFacade::RESET = "reset";

  GsageFacade::GsageFacade() :
      mStarted(false),
      mStopped(false),
      mGameDataManager(0),
      mLuaInterface(0),
      mUIManager(0),
      mLuaState(0),
      mStartupScriptRun(false)
    {
      easyloggingpp::Configurations c;
      c.setToDefault();
      c.parseFromText("*TRACE:\n FORMAT = %level %msg");
    }

    GsageFacade::~GsageFacade()
    {
      if(!mStarted)
        return;

      for(auto& pair : mLibraries)
      {
        pair.second->unload();
      }

      if(mGameDataManager)
        delete mGameDataManager;

      if(mLuaInterface)
        delete mLuaInterface;
    }

    bool GsageFacade::initialize(const std::string& rsageConfigPath, const std::string& resourcePath, DataNode* configOverride)
    {
      assert(mStarted == false);
      mStarted = true;

      std::string configPath = resourcePath + "/" + rsageConfigPath;

      LOG(INFO) << "Starting game, config:\n\t" << configPath;
      std::ifstream stream(configPath);
      try
      {
        boost::property_tree::read_json(stream, mConfig);
      }
      catch(boost::property_tree::json_parser_error& ex)
      {
        LOG(ERROR) << "Failed to read game config file: \"" << configPath << "\" reason: " << ex.message();
        return false;
      }

      mConfig.put("workdir", resourcePath);
      if (configOverride != 0) 
      {
        mergeNodes(mConfig, *configOverride);
      }

      // create rsage core engine
      addEventListener(&mEngine, EngineEvent::HALT, &GsageFacade::onEngineHalt);

      if(!mEngine.initialize(mConfig))
        return false;

      mLuaInterface = new LuaInterface(this, resourcePath);
      mGameDataManager = new GameDataManager(&mEngine, mConfig);
      if(mUIManager)
      {
        mUIManager->initialize(&mEngine);
        if(!mLuaState)
          setLuaState(mUIManager->getLuaState());
      }
      else if(mLuaState)
      {
        mLuaInterface->attachToState(mLuaState);
      }

      if(mConfig.count(PLUGINS_SECTION) != 0)
      {
        for(auto& pair : mConfig.get_child(PLUGINS_SECTION))
        {
          loadPlugin(pair.second.get_value<std::string>());
        }
      }

      auto startupScript = mConfig.get_optional<std::string>("startupScript");
      if(startupScript) {
        mStartupScript = startupScript.get();
        LOG(INFO) << mStartupScript;
      }
      return true;
  }

  void GsageFacade::addSystem(const std::string& id, EngineSystem* system)
  {
    mEngine.addSystem(id, system);
  }

  void GsageFacade::setLuaState(lua_State* L)
  {
    mLuaState = L;
    if(mLuaInterface && L)
      mLuaInterface->attachToState(L);
  }

  bool GsageFacade::update()
  {
    if(mStopped)
      return false;

    if(!mStartupScriptRun)
    {
      if(!mStartupScript.empty())
        mLuaInterface->runScript(mStartupScript);
      mStartupScriptRun = true;
    }

    mTimeSinceLastUpdate = mTimer.getMicroseconds();
    double frameTime = double(mTimeSinceLastUpdate) / 1000000;
    mTimer.reset();

    for(auto& listener : mUpdateListeners)
    {
      listener->update(frameTime);
    }
    // update engine
    mEngine.update(frameTime);
    return !mStopped;
  }

  void GsageFacade::halt()
  {
    mStopped = true;
  }

  void GsageFacade::reset()
  {
    mEngine.unloadAll();
    mEngine.fireEvent(Event(RESET));
  }

  bool GsageFacade::loadArea(const std::string& name)
  {
    if(!mGameDataManager->loadArea(name))
    {
      return false;
    }

    mEngine.fireEvent(Event(LOAD));
    return true;
  }

  bool GsageFacade::loadSave(const std::string& name)
  {
    if(!mGameDataManager->loadSave(name))
      return false;
    mEngine.fireEvent(Event(LOAD));
    return true;
  }

  bool GsageFacade::dumpSave(const std::string& name)
  {
    return mGameDataManager->dumpSave(name);
  }

  void GsageFacade::setUIManager(UIManager* value)
  {
    mUIManager = value;
  }

  bool GsageFacade::onEngineHalt(EventDispatcher* sender, const Event& event)
  {
    halt();
    return true;
  }

  bool GsageFacade::loadPlugin(const std::string& path)
  {
    DynLib* lib = 0;
    if(mLibraries.count(path) == 0)
    {
      lib = new DynLib(path);

      if(!lib->load())
      {
        delete lib;
        return false;
      }

      mLibraries[path] = lib;
    }
    else
    {
      lib = mLibraries[path];
    }
    INSTALL_PLUGIN install = reinterpret_cast<INSTALL_PLUGIN>(lib->getSymbol("dllStartPlugin"));
    return install(this);
  }

  bool GsageFacade::unloadPlugin(const std::string& path)
  {
    if(mLibraries.count(path) == 0)
    {
      LOG(ERROR) << "Failed to unload plugin \"" << path << "\": no such plugin installed";
      return false;
    }
    UNINSTALL_PLUGIN uninstall = reinterpret_cast<UNINSTALL_PLUGIN>(mLibraries[path]->getSymbol("dllStopPlugin"));
    bool res = uninstall(this);
    mLibraries[path]->unload();
    mLibraries.erase(path);
    return true;
  }

  bool GsageFacade::installPlugin(IPlugin* plugin)
  {
    plugin->initialize(&mEngine, mLuaInterface);
    if(!plugin->install())
    {
      LOG(ERROR) << "Failed to install plugin " << plugin->getName();
      return false;
    }

    mInstalledPlugins[plugin->getName()] = plugin;
    return true;
  }

  bool GsageFacade::uninstallPlugin(IPlugin* plugin)
  {
    plugin->uninstall();
    mInstalledPlugins.erase(plugin->getName());
    return true;
  }

  void GsageFacade::addUpdateListener(UpdateListener* listener)
  {
    mUpdateListeners.push_back(listener);
  }
}
