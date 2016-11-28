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
#include "UIManager.h"
#include "Logger.h"
#include "GameDataManager.h"
#include "IPlugin.h"
#include "lua/LuaInterface.h"
#include "EngineEvent.h"


INITIALIZE_EASYLOGGINGPP

namespace Gsage {

  const std::string PLUGINS_SECTION = "plugins";
  const std::string GsageFacade::LOAD = "load";
  const std::string GsageFacade::RESET = "reset";

  GsageFacade::GsageFacade() :
      mStarted(false),
      mStopped(false),
      mGameDataManager(0),
      mLuaInterface(0),
      mLuaState(0),
      mStartupScriptRun(false),
      mInputManager(&mEngine),
      mPreviousUpdateTime(std::chrono::high_resolution_clock::now())
  {
    el::Configurations defaultConf;
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
    defaultConf.setToDefault();
    // Values are always std::string
    defaultConf.set(el::Level::Info,
        el::ConfigurationType::Format, "%datetime %level %msg");
    // default logger uses default configurations
    el::Loggers::reconfigureLogger("default", defaultConf);
    LOG(INFO) << "Log using default file";
    // To set GLOBAL configurations you may use
    defaultConf.setGlobally(
        el::ConfigurationType::Format, "%datetime %level %msg [%fbase:%line]");
    el::Loggers::reconfigureLogger("default", defaultConf);
    mLuaInterface = new LuaInterface(this);
  }

  GsageFacade::~GsageFacade()
  {
    if(!mStarted)
      return;

    mStopped = true;
    for(auto& pair : mLibraries)
    {
      UNINSTALL_PLUGIN uninstall = reinterpret_cast<UNINSTALL_PLUGIN>(pair.second->getSymbol("dllStopPlugin"));
      uninstall(this);
      pair.second->unload();
      delete pair.second;
    }

    mEngine.removeSystems();
    mLibraries.clear();

    if(mGameDataManager)
      delete mGameDataManager;

    if(mLuaInterface)
      delete mLuaInterface;
  }

  bool GsageFacade::initialize(const std::string& rsageConfigPath,
      const std::string& resourcePath,
      Dictionary* configOverride,
      FileLoader::Encoding configEncoding)
  {
    assert(mStarted == false);
    mStarted = true;

    std::string configPath = resourcePath + "/" + rsageConfigPath;

    Dictionary environment;
    environment.put("workdir", resourcePath);
    FileLoader::init(configEncoding, Dictionary());

    LOG(INFO) << "Starting game, config:\n\t" << configPath;
    if(!FileLoader::getSingletonPtr()->load(configPath, Dictionary(), mConfig))
    {
      return false;
    }

    if (configOverride != 0)
    {
      unionDict(mConfig, *configOverride);
    }

    addEventListener(&mEngine, EngineEvent::HALT, &GsageFacade::onEngineHalt);

    auto inputHandler = mConfig.get<std::string>("inputHandler");
    if(inputHandler.second) {
      mInputManager.useFactory(inputHandler.first);
    }

    if(!mEngine.initialize(mConfig, environment))
      return false;

    mGameDataManager = new GameDataManager(&mEngine, mConfig);
    mLuaInterface->setResourcePath(resourcePath);
    mLuaInterface->initialize();

    for(auto pair : mUIManagers) {
      pair.second->initialize(&mEngine, mLuaInterface->getState());
    }

    if(mConfig.count(PLUGINS_SECTION) != 0)
    {
      for(auto& pair : mConfig.get<Dictionary>(PLUGINS_SECTION).first)
      {
        loadPlugin(pair.second.as<std::string>());
      }
    }

    auto startupScript = mConfig.get<std::string>("startupScript");
    if(startupScript.second) {
      mStartupScript = resourcePath + GSAGE_PATH_SEPARATOR + startupScript.first;
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
      mLuaInterface->initialize(L);

    for(auto pair : mUIManagers) {
      pair.second->setLuaState(L);
    }
  }

  lua_State* GsageFacade::getLuaState()
  {
    return mLuaInterface->getState();
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

    auto now = std::chrono::high_resolution_clock::now();

    double frameTime = std::chrono::duration_cast<std::chrono::duration<double>>(now - mPreviousUpdateTime).count();

    for(auto& listener : mUpdateListeners)
    {
      listener->update(frameTime);
    }
    // update engine
    mEngine.update(frameTime);
    mInputManager.update(frameTime);
    mPreviousUpdateTime = now;
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

  int GsageFacade::addUIManager(UIManager* value)
  {
    int handle = mUIManagers.size();
    mUIManagers[handle] = value;
    if(mLuaInterface != 0 && mLuaInterface->getState()) {
      mUIManagers[handle]->initialize(&mEngine, mLuaInterface->getState());
    }
    return handle;
  }

  bool GsageFacade::removeUIManager(int handle)
  {
    if(mUIManagers.count(handle) == 0) {
      return false;
    }

    mUIManagers.erase(handle);
    return true;
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
    return res;
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
    std::string name = plugin->getName();
    if(mInstalledPlugins.count(name) == 0)
      return false;

    plugin->uninstall();
    mInstalledPlugins.erase(name);
    return true;
  }

  void GsageFacade::addUpdateListener(UpdateListener* listener)
  {
    mUpdateListeners.push_back(listener);
  }
}
