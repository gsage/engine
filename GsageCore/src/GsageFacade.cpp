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

#include <thread>
#include <chrono>

#include "GsageFacade.h"
#include "UIManager.h"
#include "Logger.h"
#include "GameDataManager.h"
#include "IPlugin.h"
#include "lua/LuaInterface.h"

#include "components/ScriptComponent.h"
#include "systems/LuaScriptSystem.h"
#include "systems/CombatSystem.h"
#include "systems/MovementSystem.h"

#include "EngineEvent.h"

#if GSAGE_PLATFORM == GSAGE_APPLE
#include "macUtils.h"
#endif


INITIALIZE_EASYLOGGINGPP

namespace Gsage {

  const std::string PLUGINS_SECTION = "plugins";
  const Event::Type GsageFacade::LOAD = "load";
  const Event::Type GsageFacade::RESET = "reset";
  const Event::Type GsageFacade::BEFORE_RESET = "beforeReset";

  GsageFacade::GsageFacade() :
      mStarted(false),
      mStopped(false),
      mGameDataManager(0),
      mLuaInterface(0),
      mLuaState(0),
      mStartupScriptRun(false),
      mInputManager(&mEngine),
      mSystemManager(&mEngine),
      mPreviousUpdateTime(std::chrono::high_resolution_clock::now()),
      mExitCode(0),
      mWindowManager(nullptr)
  {
    el::Configurations defaultConf;
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
    defaultConf.setToDefault();
    // To set GLOBAL configurations you may use
    defaultConf.setGlobally(
        el::ConfigurationType::Format, "%datetime %level %msg [%fbase:%line]");
    el::Loggers::reconfigureLogger("default", defaultConf);
    mLuaInterface = new LuaInterface(this);

    // Register core system factories
    registerSystemFactory<CombatSystem>();
    registerSystemFactory<MovementSystem>();
    mSystemManager.registerFactory(LuaScriptSystem::ID, new LuaScriptSystemFactory(mLuaInterface));
  }


  GsageFacade::~GsageFacade()
  {
    if(!mStarted)
      return;

    mStopped = true;
    mEngine.unloadAll();
    mEngine.removeSystems();
    mEngine.removeAllListeners();

    mWindowManager = nullptr;

    for (PluginOrder::reverse_iterator rit = mPluginOrder.rbegin(); rit != mPluginOrder.rend(); ++rit)
    {
      DynLib* l = mLibraries[*rit];

      UNINSTALL_PLUGIN uninstall = reinterpret_cast<UNINSTALL_PLUGIN>(l->getSymbol("dllStopPlugin"));
      uninstall(this);
      l->unload();
      delete l;
    }

    mLibraries.clear();

    if(mGameDataManager)
      delete mGameDataManager;

    if(mLuaInterface)
      delete mLuaInterface;
  }

  bool GsageFacade::initialize(const std::string& configFile,
      const std::string& resourcePath,
      DataProxy* configOverride,
      FileLoader::Encoding configEncoding)
  {
    assert(mStarted == false);
    mStarted = true;

    DataProxy environment;
    environment.put("workdir", resourcePath);
    FileLoader::init(configEncoding, environment);
    DataProxy config;

    if(!FileLoader::getSingletonPtr()->load(configFile, DataProxy(), config))
    {
      LOG(ERROR) << "Failed to load file " << configFile;
      return false;
    }

    LOG(INFO) << "Starting GSAGE, config:\n\t" << configFile;

    if (configOverride != 0)
    {
      mergeInto(config, *configOverride);
    }

    addEventListener(&mEngine, EngineEvent::SHUTDOWN, &GsageFacade::onEngineShutdown);
    addEventListener(&mEngine, EngineEvent::LUA_STATE_CHANGE, &GsageFacade::onLuaStateChange);

    if(!mEngine.initialize(config, environment))
      return false;

    mGameDataManager = new GameDataManager(&mEngine);
    mLuaInterface->setResourcePath(resourcePath);
    if(config.get<bool>("startLuaInterface", true)) {
      mLuaInterface->initialize(mLuaState);
      // execute lua package manager, if it's enabled
      auto pair = config.get<DataProxy>("packager");

      auto scriptsPath = config.get<std::string>("scriptsPath", resourcePath);
      if(pair.second) {
        DataProxy deps = pair.first.get("deps", DataProxy::create(DataWrapper::JSON_OBJECT));
        std::string scriptPath = pair.first.get(
            "script",
            resourcePath + GSAGE_PATH_SEPARATOR + "scripts" + GSAGE_PATH_SEPARATOR + "lib" + GSAGE_PATH_SEPARATOR + "packager.lua"
        );
        if(!mLuaInterface->runPackager(scriptPath, deps)) {
          LOG(ERROR) << "Failed to run lua packager";
          return false;
        }
      }
    }

    if(!configure(config)) {
      return false;
    }

    auto scriptsPath = config.get<std::string>("scriptsPath", resourcePath);
    auto startupScript = config.get<std::string>("startupScript");
    if(startupScript.second) {
      mStartupScript = scriptsPath + GSAGE_PATH_SEPARATOR + startupScript.first;
    }
    return true;
  }

  bool GsageFacade::configure(const DataProxy& configuration, bool restart)
  {
    mPluginsFolders.clear();
    auto pluginsFolders = configuration.get<DataProxy>("pluginsFolders");
    if(pluginsFolders.second) {
      for(auto& pair : pluginsFolders.first) {
        mPluginsFolders.push_back(pair.second.as<std::string>());
      }
    }

    std::string resourcePath = mEngine.env().get("workdir", "");
    auto logConfig = configuration.get<std::string>("logConfig");
    if (logConfig.second) {
      std::string fullLogConfigPath = resourcePath + GSAGE_PATH_SEPARATOR + logConfig.first;
      if(mFilesystem.exists(fullLogConfigPath)) {
        el::Configurations conf(fullLogConfigPath);
        el::Loggers::reconfigureLogger("default", conf);
      }
    }

    std::map<std::string, bool> installedPlugins;
    if(configuration.count(PLUGINS_SECTION) != 0)
    {
      for(auto& pair : configuration.get<DataProxy>(PLUGINS_SECTION).first)
      {
        std::string path = pair.second.as<std::string>();
        if(!loadPlugin(path, true)) {
          return false;
        }
        installedPlugins[path] = true;
      }
    }

    std::vector<std::string> toRemove;
    // unload no longer used plugins
    for(auto& pair : mLibraries) {
      if(!installedPlugins[pair.first]) {
        toRemove.push_back(pair.first);
      }
    }

    for(auto& name : toRemove) {
      LOG(INFO) << "Unloading no longer used plugin " << name;
      unloadPlugin(name);
    }

    // note that old windows will still use old inputHandler
    auto inputHandler = configuration.get<std::string>("inputHandler");
    if(inputHandler.second && inputHandler.first != mConfig.get<std::string>("inputHandler", "")) {
      LOG(INFO) << "Using input handler " << inputHandler.first;
      mInputManager.useFactory(inputHandler.first);
    }

    auto windowManager = configuration.get<DataProxy>("windowManager");
    if(windowManager.second && mConfig.get("windowManager.type", "") != configuration.get("windowManager.type", "")) {
      auto type = windowManager.first.get<std::string>("type");
      if(!type.second) {
        LOG(ERROR) << "Malformed window manager config was provided";
        return false;
      }
      mWindowManager = mWindowManagerFactory.create(type.first);
      if(mWindowManager == nullptr) {
        LOG(ERROR) << "Failed to initialize window manager";
        return false;
      }
      addEventListener(mWindowManager.get(), WindowEvent::CREATE, &GsageFacade::handleWindowManagerEvent);
      addEventListener(mWindowManager.get(), WindowEvent::RESIZE, &GsageFacade::handleWindowManagerEvent);
      addEventListener(mWindowManager.get(), WindowEvent::CLOSE, &GsageFacade::handleWindowManagerEvent);
      if(!mWindowManager->initialize(windowManager.first)) {
        LOG(ERROR) << "Failed to initialize window manager";
        return false;
      }

      LOG(INFO) << "Using " << type.first << " window manager";
    }

    auto systems = configuration.get<DataProxy>("systems");
    std::map<std::string, bool> installedSystems;
    if(systems.second) {
      for(auto pair : systems.first) {
        auto st = pair.second.getValue<std::string>();
        if(!st.second) {
          LOG(WARNING) << "Empty system type";
          continue;
        }

        std::string systemType = st.first;
        auto id = configuration.get<std::string>(std::string("systemTypes.") + systemType);
        if(!id.second) {
          LOG(INFO) << "Skipping system create " << st.first << ", no system type is defined";
          continue;
        }

        EngineSystem* system = mEngine.getSystem(systemType);
        if(!system || system->getKind() != id.first) {
          if(system) {
            LOG(TRACE) << "Recreate " << systemType << " system, installing another implementation: " << system->getKind() << "->" << id.first;
            mEngine.removeSystem(systemType);
          }

          system = mSystemManager.create(id.first, false);
          if(!system) {
            LOG(ERROR) << "Failed to create system of type \"" << id.first << "\"";
            continue;
          }
          system->setFacadeInstance(this);
          LOG(TRACE) << "Installed system " << systemType << " " << id.first;
        }

        if(!mEngine.configureSystem(systemType, configuration.get(systemType, DataProxy()), restart)) {
          LOG(ERROR) << "Failed to configure engine system " << systemType;
          return false;
        }
        installedSystems[systemType] = true;
      }
    }

    toRemove.clear();
    // uninstall not used systems
    for(auto& pair : mEngine.getSystems()) {
      if(!installedSystems[pair.first]) {
        toRemove.push_back(pair.first);
      }
    }

    for(auto& name : toRemove) {
      LOG(TRACE) << "Uninstalling no longer used system " << name;
      mEngine.removeSystem(name);
    }

    for(auto pair : mUIManagers) {
      if(!pair.second->initialized()) {
        pair.second->initialize(this, mLuaInterface->getState());
      }
    }

    mGameDataManager->configure(configuration);
    mConfig = configuration;
    return true;
  }

  void GsageFacade::addSystem(const std::string& id, EngineSystem* system)
  {
    mEngine.addSystem(id, system);
  }

  void GsageFacade::setLuaState(lua_State* L, bool initialize)
  {
    mLuaState = L;

    if(!initialize)
      return;

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
    if(!mStartupScriptRun)
    {
      if(!mStartupScript.empty())
        mLuaInterface->runScript(mStartupScript);
      mStartupScriptRun = true;
    }

    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> frameTime(now - mPreviousUpdateTime);

    for(auto& listener : mUpdateListeners)
    {
      listener->update(frameTime.count());
    }

    // update engine
    mEngine.update(frameTime.count());
    mInputManager.update(frameTime.count());
    mFilesystem.update(frameTime.count());
    mPreviousUpdateTime = now;
    std::chrono::duration<double> maxTime(1.0/60.0);
    if(maxTime > frameTime) {
      std::this_thread::sleep_for(maxTime - frameTime);
    }

    return !mStopped;
  }

  int GsageFacade::getExitCode() const
  {
    return mExitCode;
  }

  void GsageFacade::shutdown(int exitCode)
  {
    mEngine.fireEvent(EngineEvent(EngineEvent::STOPPING));
    mEngine.shutdown();
    mExitCode = exitCode;
    mStopped = true;
  }

  void GsageFacade::reset()
  {
    mEngine.fireEvent(Event(BEFORE_RESET));
    mEngine.unloadAll();
    mEngine.fireEvent(Event(RESET));
  }

  void GsageFacade::reset(sol::function f)
  {
    mEngine.fireEvent(Event(BEFORE_RESET));
    mEngine.unloadMatching([f](Entity* e) -> bool {
      bool match = f(e);
      return match;
    });
    mEngine.fireEvent(Event(RESET));
  }

  bool GsageFacade::loadScene(const std::string& name)
  {
    if(!mGameDataManager->loadScene(name))
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
      mUIManagers[handle]->initialize(this, mLuaInterface->getState());
    }
    return handle;
  }

  UIManager* GsageFacade::getUIManager(const std::string& name) {
    for(auto pair : mUIManagers) {
      if(pair.second->getType() == name) {
        return pair.second;
      }
    }
    return nullptr;
  }

  bool GsageFacade::removeUIManager(int handle)
  {
    if(mUIManagers.count(handle) == 0) {
      return false;
    }

    mUIManagers.erase(handle);
    return true;
  }

  bool GsageFacade::removeWindowManager(const std::string& id)
  {
    if(mWindowManager != nullptr && mWindowManager->getType() == id) {
      mWindowManager = nullptr;
    }
    return mWindowManagerFactory.removeWindowManager(id);
  }

  bool GsageFacade::onEngineShutdown(EventDispatcher* sender, const Event& event)
  {
    shutdown();
    return true;
  }

  bool GsageFacade::onLuaStateChange(EventDispatcher* sender, const Event& event)
  {
    for(auto name : mPluginOrder) {
      mInstalledPlugins[name]->setupLuaBindings();
    }
    return true;
  }

  std::string GsageFacade::getFullPluginPath(const std::string& name) const {
    std::string fullPath;
    for(auto& folder : mPluginsFolders) {
      std::stringstream ss;
#if GSAGE_PLATFORM == GSAGE_APPLE
      char bundlePath[1024];
      mac_getBundlePath(bundlePath, 1024);
      ss << bundlePath << GSAGE_PATH_SEPARATOR;
#endif
      ss << folder << GSAGE_PATH_SEPARATOR << name;
      fullPath = ss.str();
#if GSAGE_PLATFORM == GSAGE_WIN32
      ss << ".dll";
#elif GSAGE_PLATFORM == GSAGE_APPLE
      ss << ".dylib";
#elif GSAGE_PLATFORM == GSAGE_LINUX
      ss << ".so";
#endif
      if(mFilesystem.exists(ss.str())) {
        return fullPath;
      }
    }
    return "";
  }

  bool GsageFacade::loadPlugin(const std::string& path)
  {
    return loadPlugin(path, false);
  }

  bool GsageFacade::loadPlugin(const std::string& path, bool skipLoaded)
  {

    if(mPluginsFolders.size() == 0) {
      LOG(ERROR) << "Failed to install plugin " << path << ": no pluginsFolders defined";
      return false;
    }

    std::string fullPath = getFullPluginPath(path);
    if(fullPath.empty()) {
      LOG(ERROR) << "Failed to find plugin library " << path << " tried paths:\n" << join(mPluginsFolders, '\n');
      return false;
    }

    size_t c = mLibraries.count(path);
    if(skipLoaded && c != 0) {
      return true;
    }

    DynLib* lib = 0;
    if(c == 0){
      lib = new DynLib(fullPath);

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
    mPluginOrder.push_back(path);
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

    mPluginOrder.remove(path);
    mLibraries.erase(path);
    return res;
  }

  bool GsageFacade::installPlugin(IPlugin* plugin)
  {
    plugin->initialize(this, mLuaInterface);
    if(!plugin->install())
    {
      LOG(ERROR) << "Failed to install plugin " << plugin->getName();
      return false;
    } else {
      LOG(INFO) << "Installed plugin " << plugin->getName();
    }

    mInstalledPlugins[plugin->getName()] = plugin;
    return true;
  }

  bool GsageFacade::uninstallPlugin(IPlugin* plugin)
  {
    LOG(INFO) << "Uninstalled plugin " << plugin->getName();
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

  bool GsageFacade::removeUpdateListener(UpdateListener* listener)
  {
    for(size_t i = 0; i < mUpdateListeners.size(); i++) {
      if(mUpdateListeners[i] == listener) {
        mUpdateListeners.erase(mUpdateListeners.begin() + i);
        return true;
      }
    }

    return false;
  }

  void GsageFacade::removeInputFactory(const std::string& id)
  {
    mInputManager.removeFactory(id);
  }

  bool GsageFacade::createSystem(const std::string& systemID)
  {
    EngineSystem* s = mSystemManager.create(systemID, false);
    if(!s) {
      return false;
    }
    s->setFacadeInstance(this);
    return mEngine.configureSystem(s->getName());
  }

  WindowManagerPtr GsageFacade::getWindowManager()
  {
    return mWindowManager;
  }

  bool GsageFacade::handleWindowManagerEvent(EventDispatcher* sender, const Event& event)
  {
    mEngine.fireEvent(event);
    return true;
  }

  const GsageFacade::PluginOrder& GsageFacade::getInstalledPlugins() const
  {
    return mPluginOrder;
  }
}
