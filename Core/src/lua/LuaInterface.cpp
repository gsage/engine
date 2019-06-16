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

#include "lua/LuaInterface.h"
#include "sol.hpp"

#include "GsageFacade.h"
#include "Version.h"
#include "GameDataManager.h"
#include "EngineEvent.h"
#include "KeyboardEvent.h"
#include "MouseEvent.h"
#include "ResourceMonitor.h"
#include "Path.h"

#include "components/StatsComponent.h"
#include "components/ScriptComponent.h"
#include "components/RenderComponent.h"
#include "components/MovementComponent.h"

#include "systems/CombatSystem.h"
#include "systems/MovementSystem.h"
#include "systems/RenderSystem.h"
#include "systems/LuaScriptSystem.h"

#include "lua/LuaEventProxy.h"
#include "lua/LuaEventConnection.h"

#if GSAGE_PLATFORM == GSAGE_LINUX || GSAGE_PLATFORM == GSAGE_APPLE
#include <limits.h>
#include <stdlib.h>
#elif GSAGE_PLATFORM == GSAGE_WIN32
#include <windows.h>
#include <tchar.h>
#include "WIN32/CommandLine.h"
#endif

namespace Gsage {

  LogProxy::LogProxy()
  {
  }

  LogProxy::~LogProxy()
  {
  }

  void LogProxy::subscribe(const std::string& name, sol::protected_function function)
  {
    el::Helpers::installLogDispatchCallback<LogSubscriber>(name);
    mSubscribers[name] = el::Helpers::logDispatchCallback<LogSubscriber>(name);
    mSubscribers[name]->setWrappedFunction(function);
  }

  void LogProxy::unsubscribe(const std::string& name)
  {
    el::Helpers::uninstallLogDispatchCallback<LogSubscriber>(name);
  }

  void LogProxy::flushMessages()
  {
    for(auto& pair : mSubscribers) {
      pair.second->flushMessages();
    }
  }

  void LogProxy::LogSubscriber::handle(const el::LogDispatchData* data)
  {
    if(!mReady || !mWrapped.valid()) {
      return;
    }
    const el::LogMessage* msg = data->logMessage();

    mMessages << LogMessage{
      msg->level(),
      msg->line(),
      msg->file(),
      msg->func(),
      msg->verboseLevel(),
      msg->message()
    };
  }

  void LogProxy::LogSubscriber::flushMessages()
  {
    LogMessage msg;
    while(mMessages.get(msg) > 0) {
      mWrapped(msg);
    }
  }

  void LogProxy::LogSubscriber::setWrappedFunction(sol::protected_function wrapped)
  {
    if(wrapped.valid()) {
      mWrapped = wrapped;
      mReady = true;
    }
  }
  PropertyDescription::PropertyDescription(const std::string& name, const std::string& docstring)
    : mName(name)
    , mDocstring(docstring)
  {
  }

  const std::string& PropertyDescription::getName() const
  {
    return mName;
  }

  const std::string& PropertyDescription::getDocstring() const
  {
    return mDocstring;
  }

  LuaInterface::LuaInterface(GsageFacade* instance, const std::string& resourcePath)
    : mInstance(instance)
    , mEventProxy(new LuaEventProxy())
    , mState(0)
    , mResourcePath(resourcePath)
    , mStateView(0)
    , mStateCreated(false)
  {
  }

  LuaInterface::~LuaInterface()
  {
    if(mEventProxy)
      delete mEventProxy;

    if(mStateView)
      delete mStateView;

    closeLuaState();
  }

  void LuaInterface::closeLuaState()
  {
    if(mStateCreated) {
      lua_close(mState);
    }
  }

  bool LuaInterface::initialize(lua_State* L)
  {
    if(L != 0 && L == mState) {
      return true;
    }

    if(mStateView)
      delete mStateView;

    if(mState) {
      closeLuaState();
    }

    if(L == 0) {
      mState = lua_open();
      mStateCreated = true;
    } else {
      mState = L;
      mStateCreated = false;
    }

    luaL_openlibs(mState);
    mStateView = new sol::state_view(mState);
    sol::state_view lua = *mStateView;

    lua.new_usertype<EventDispatcher>("EventDispatcher",
        "new", sol::no_constructor,
        "id", [](EventDispatcher* instance) { return (unsigned long long)(instance); }
    );

    lua.new_usertype<PropertyDescription>("PropertyDescription",
        "new", sol::constructors<sol::types<const std::string&, const std::string&>>(),
        "name", sol::property(&PropertyDescription::getName),
        "docstring", sol::property(&PropertyDescription::getDocstring)
    );

    lua.new_usertype<UpdateListener>("UpdateListener");

    lua.new_usertype<ResourceMonitor>("ResourceMonitor",
        sol::base_classes, sol::bases<UpdateListener>(),
        "new", sol::constructors<sol::types<float>>(),
        "stats", sol::property(&ResourceMonitor::getStats)
    );

    lua.new_usertype<ResourceMonitor::Stats>("ResourceMonitorStats",
        "physicalMem", &ResourceMonitor::Stats::physicalMem,
        "virtualMem", &ResourceMonitor::Stats::virtualMem,
        "lastCPU", &ResourceMonitor::Stats::lastCPU,
        "lastSysCPU", &ResourceMonitor::Stats::lastSysCPU,
        "lastUserCPU", &ResourceMonitor::Stats::lastUserCPU
    );
    lua.new_usertype<GsageFacade>("Facade",
        "new", sol::no_constructor,
        "configure", &GsageFacade::configure,
        "filesystem", sol::property(&GsageFacade::filesystem),
        "shutdown", &GsageFacade::shutdown,
        "reset", sol::overload(
          (void(GsageFacade::*)()) &GsageFacade::reset,
          (void(GsageFacade::*)(sol::function))&GsageFacade::reset
        ),
        "loadSave", &GsageFacade::loadSave,
        "dumpSave", &GsageFacade::dumpSave,
        "loadScene", &GsageFacade::loadScene,
        "loadPlugin", sol::overload(
          (bool(GsageFacade::*)(const std::string&))&GsageFacade::loadPlugin,
          (bool(GsageFacade::*)(const std::string&, bool))&GsageFacade::loadPlugin
        ),
        "getInstalledPlugins", &GsageFacade::getInstalledPlugins,
        "getFullPluginPath", &GsageFacade::getFullPluginPath,
        "unloadPlugin", &GsageFacade::unloadPlugin,
        "createSystem", &GsageFacade::createSystem,
        "getWindowManager", &GsageFacade::getWindowManager,
        "addUpdateListener", &GsageFacade::addUpdateListener,
        "BEFORE_RESET", sol::var(GsageFacade::BEFORE_RESET),
        "RESET", sol::var(GsageFacade::RESET),
        "LOAD", sol::var(GsageFacade::LOAD)
    );

    lua.new_usertype<WindowManager>("WindowManager",
        "new", sol::no_constructor,
        "getWindow", (WindowPtr(WindowManager::*)(const std::string&))&WindowManager::getWindow,
        "openDialog", [] (WindowManager* w, Window::DialogMode mode, const std::string& title, const std::string& defaultFilePath, sol::table filters) {
            std::vector<std::string> actualFilters;
            for(auto iter = filters.begin();
                      iter != filters.end();
                      ++iter) {
              actualFilters.push_back((*iter).second.as<std::string>());
            }

            return w->openDialog(mode, title, defaultFilePath, actualFilters);
        }
    );

    lua.new_usertype<Filesystem>("Filesystem",
        sol::base_classes, sol::bases<EventDispatcher>(),
        "new", sol::no_constructor,
        "createFile", &Filesystem::createFile,
        "unzip", &Filesystem::unzip,
        "copytreeAsync", &Filesystem::copytreeAsync,
        "isDirectory", &Filesystem::isDirectory,
        "copy", &Filesystem::copy,
        "ls", &Filesystem::ls,
        "exists", &Filesystem::exists,
        "rmdir", &Filesystem::rmdir,
        "directory", &Filesystem::directory,
        "extension", &Filesystem::extension,
        "basename", &Filesystem::basename,
        "filename", &Filesystem::filename,
        "mkdir", &Filesystem::mkdir,
        "join", [](Filesystem* fs, DataProxy data) {
          std::vector<std::string> v;
          for(auto& pair : data) {
            v.push_back(pair.second.as<std::string>());
          }
          return fs->join(v);
        }
    );

    lua.new_usertype<CopyWorker>("CopyWorker",
        "new", sol::no_constructor,
        "getID", &CopyWorker::getID,
        "src", &CopyWorker::getSrc,
        "dst", &CopyWorker::getDst,
        sol::meta_function::to_string, &CopyWorker::str
    );

    lua.new_usertype<Window>("Window",
        "new", sol::no_constructor,
        "getWindowHandle", &Window::getWindowHandle,
        "getPosition", &Window::getPosition,
        "setPosition", &Window::setPosition,
        "getSize", &Window::getSize,
        "setSize", &Window::setSize,
        "getDisplayBounds", &Window::getDisplayBounds,
        "getScaleFactor", &Window::getScaleFactor,

        "FILE_DIALOG_OPEN_FOLDER", sol::var(Window::FILE_DIALOG_OPEN_FOLDER),
        "FILE_DIALOG_OPEN", sol::var(Window::FILE_DIALOG_OPEN),
        "FILE_DIALOG_OPEN_MULTIPLE", sol::var(Window::FILE_DIALOG_OPEN_MULTIPLE),
        "FILE_DIALOG_SAVE", sol::var(Window::FILE_DIALOG_SAVE),

        "FILE_DIALOG_OKAY", sol::var(Window::OKAY),
        "FILE_DIALOG_CANCEL", sol::var(Window::CANCEL),
        "FILE_DIALOG_FAILURE", sol::var(Window::FAILURE)
    );

    lua.new_usertype<Reflection>("Reflection",
        "props", sol::property(&Reflection::getProps, &Reflection::setProps)
    );

    auto vector3 = sol::usertype<Vector3>(
        sol::constructors<sol::types<double, double, double>>(),
        "x", &Vector3::X,
        "y", &Vector3::Y,
        "z", &Vector3::Z,
        "squaredDistance", [](Vector3 rhs, const Vector3 lhs) {
          double distance = Vector3::Distance(rhs, lhs);
          return distance * distance;
        },
        "crossProduct", [](Vector3 rhs, const Vector3 lhs) { return Vector3::Cross(rhs, lhs); },
        "clamp", [](Vector3 rhs, double maxLength) { return Vector3::ClampMagnitude(rhs, maxLength); },
        "ZERO", sol::property([] () { return Vector3(0, 0, 0); }),
        "UNIT_X", sol::property([] () { return Vector3(1, 0, 0); }),
        "UNIT_Y", sol::property([] () { return Vector3(0, 1, 0); }),
        "UNIT_Z", sol::property([] () { return Vector3(0, 0, 1); }),
        "NEGATIVE_UNIT_X", sol::property([] () { return Vector3(-1, 0, 0); }),
        "NEGATIVE_UNIT_Y", sol::property([] () { return Vector3(0, -1, 0); }),
        "NEGATIVE_UNIT_Z", sol::property([] () { return Vector3(0, 0, -1); }),
        "UNIT_SCALE", sol::property([] () { return Vector3(1, 1, 1); }),
        sol::meta_function::multiplication, sol::overload(
          [](Vector3 rhs, const Vector3 lhs) { return rhs *= lhs; },
          [](Vector3 rhs, const double lhs) { return rhs * lhs; }
        ),
        sol::meta_function::addition, [](Vector3 rhs, const Vector3 lhs) { return rhs + lhs; },
        sol::meta_function::equal_to, [](const Vector3& rhs, const Vector3& lhs) { return rhs == lhs; },
        sol::meta_function::division, sol::overload(
            [](Vector3 rhs, const Vector3& lhs) { return rhs /= lhs; },
            [](Vector3 rhs, double lhs) { return rhs / lhs; }
        )
    );

    auto radian = sol::usertype<Radian>(
        "degrees", sol::property(&Radian::valueDegrees),
        "radians", sol::property(&Radian::valueRadians)
    );

    auto quaternion = sol::usertype<Quaternion>(
        "new", sol::factories(
          [](double w, double x, double y, double z) { return std::make_shared<Quaternion>(x, y, z, w); },
          [](double scalar, Vector3 vector) {return std::make_shared<Quaternion>(vector, scalar); }
        ),
        "w", &Quaternion::W,
        "x", &Quaternion::X,
        "y", &Quaternion::Y,
        "z", &Quaternion::Z,
        sol::meta_function::equal_to, [](const Quaternion& rhs, const Quaternion& lhs) { return rhs == lhs; },
        sol::meta_function::multiplication, sol::overload(
          [](Quaternion rhs, const Quaternion lhs) { return rhs *= lhs; },
          [](Quaternion rhs, const Vector3 lhs) { return rhs * lhs; }
        ),
        "getPitch", &Gsage::Quaternion::getPitch,
        "getYaw", &Gsage::Quaternion::getYaw,
        "getRoll", &Gsage::Quaternion::getRoll
    );

    lua.new_usertype<Path3D>("Path3D",
        "new", sol::constructors<sol::types<Path3D::Vector>>(),
        "dump", [&](Path3D* path, sol::table t) {
          DataProxy res = DataProxy::wrap(t);
          path->dump(res);
          return res;
        },
        "points", [](Path3D* self, sol::this_state s) -> sol::object {
          sol::state_view lua(s);
          sol::table res = lua.create_table();
          DataProxy t = DataProxy::wrap(res);
          self->dump(t);
          return res;
        }
    );

    lua.new_usertype<BoundingBox>("BoundingBox",
        "new", sol::constructors<sol::types<Gsage::Vector3, Gsage::Vector3>, sol::types<BoundingBox::Extent>>(),
        "min", &BoundingBox::min,
        "max", &BoundingBox::max,
        "contains", &BoundingBox::contains,
        "EXTENT_NULL", sol::var(BoundingBox::Extent::EXTENT_NULL),
        "EXTENT_FINITE", sol::var(BoundingBox::Extent::EXTENT_FINITE),
        "EXTENT_INFINITE", sol::var(BoundingBox::Extent::EXTENT_INFINITE)
    );

    auto geometry = lua.create_table();

    // shortcuts (can be overwritten in 3D library)
    lua["Quaternion"] = lua.create_table_with("new", [](double w, double x, double y, double z) { return std::make_shared<Quaternion>(x, y, z, w); });
    lua["Vector3"] = lua.create_table_with("new", [](double x, double y, double z){ return std::make_shared<Gsage::Vector3>(x, y, z); });
    lua["Radian"] = lua.create_table_with("new", [](double value){ return std::make_shared<Gsage::Radian>(value); });

    geometry["X_AXIS"] = Geometry::X_AXIS;
    geometry["Y_AXIS"] = Geometry::Y_AXIS;
    geometry["Z_AXIS"] = Geometry::Z_AXIS;
    geometry["TS_WORLD"] = Geometry::TransformSpace::TS_WORLD;
    geometry["TS_LOCAL"] = Geometry::TransformSpace::TS_LOCAL;

    lua["geometry"] = geometry;

    geometry.set_usertype("Vector3", vector3);
    geometry.set_usertype("Quaternion", quaternion);
    geometry.set_usertype("Radian", radian);

    // raw geometry data
    lua.new_usertype<Geom>("Geom",
        "verts", [](Geom* geom){
          float* verts;
          int count;
          std::vector<Gsage::Vector3> res;
          std::tie(verts, count) = geom->getVerts();
          for(int i = 0; i < count; i += 3) {
            res.emplace_back(verts[i], verts[i + 1], verts[i + 2]);
          }
          return res;
        },
        "tris", [](Geom* geom){
          int* tris;
          int count;
          std::vector<int> res;
          std::tie(tris, count) = geom->getTris();
          for(int i = 0; i < count; i++) {
            res.emplace_back(tris[i]);
          }
          return res;
        },
        "bmin", [](Geom* geom) {
            float* v = geom->getMeshBoundsMin();
            return Gsage::Vector3(v[0], v[1], v[2]);
         },
        "bmax", [](Geom* geom) {
            float* v = geom->getMeshBoundsMax();
            return Gsage::Vector3(v[0], v[1], v[2]);
         }
    );

    lua.new_usertype<Texture>("Texture",
        "valid", sol::property(&Texture::isValid),
        "setSize", &Texture::setSize,
        "hasData", &Texture::hasData,
        "name", sol::property(&Texture::getName),
        "handle", [](Texture* texture) -> size_t { return (size_t)texture; },

        "RESIZE", sol::var(Texture::RESIZE),
        "RECREATE", sol::var(Texture::RECREATE)
    );

    // --------------------------------------------------------------------------------
    // Systems

    lua.new_usertype<EngineSystem>("EngineSystem",
        "enabled", sol::property(&EngineSystem::isEnabled, &EngineSystem::setEnabled),
        "info", sol::property(&EngineSystem::getSystemInfo),
        "config", sol::property(&EngineSystem::getConfig),
        "restart", &EngineSystem::restart
    );

    lua.new_usertype<LuaScriptSystem>("ScriptSystem",
        "addUpdateListener", &LuaScriptSystem::addUpdateListener,
        "removeUpdateListener", &LuaScriptSystem::removeUpdateListener
    );


    lua.new_usertype<MovementSystem>("MovementSystem",
        sol::base_classes, sol::bases<EngineSystem>()
    );

    lua.new_usertype<RenderSystem>("RenderSystem",
        "new", sol::no_constructor,
        "createTexture", &RenderSystem::createTexture,
        "getTexture", &RenderSystem::getTexture,
        "deleteTexture", &RenderSystem::deleteTexture
    );

    // --------------------------------------------------------------------------------
    // Components

    lua.new_usertype<StatsComponent>("StatsComponent",
        sol::base_classes, sol::bases<EventDispatcher, Reflection>(),
        "hasStat", &StatsComponent::hasStat,
        "getBool", sol::overload(
          (const bool(StatsComponent::*)(const std::string&))&StatsComponent::getStat<bool>,
          (const bool(StatsComponent::*)(const std::string&, const bool&))&StatsComponent::getStat<bool>
        ),
        "getString", sol::overload(
          (const std::string(StatsComponent::*)(const std::string&))&StatsComponent::getStat<std::string>,
          (const std::string(StatsComponent::*)(const std::string&, const std::string&))&StatsComponent::getStat<std::string>
        ),
        "getNumber", sol::overload(
          (const double(StatsComponent::*)(const std::string&)) &StatsComponent::getStat<double>,
          (const double(StatsComponent::*)(const std::string&, const double&)) &StatsComponent::getStat<double>
        ),
        "data", sol::property(&StatsComponent::data),
        "set", sol::overload(
          &StatsComponent::setStat<bool>,
          &StatsComponent::setStat<std::string>,
          &StatsComponent::setStat<double>
        ),
        "increase", &StatsComponent::increase
    );

    lua.new_usertype<ScriptComponent>("ScriptComponent",
        sol::base_classes, sol::bases<Reflection>(),
        "state", sol::property(&ScriptComponent::getData)
    );

    lua.new_usertype<RenderComponent>("RenderComponent",
        "new", sol::no_constructor,
        "setPosition", (void(RenderComponent::*)(float, float, float))&RenderComponent::setPosition,
        "getPosition", [](RenderComponent* self) {
          Gsage::Vector3 position = self->getPosition();
          return std::make_tuple(position.X, position.Y, position.Z);
        },

        "position", sol::property((void(RenderComponent::*)(const Gsage::Vector3&))&RenderComponent::setPosition, &RenderComponent::getPosition),
        "direction", sol::property(&RenderComponent::getDirection),
        "orientation", sol::property(
          &RenderComponent::setOrientation,
          &RenderComponent::getOrientation
        ),
        "facingOrientation", sol::property(&RenderComponent::getFaceOrientation),
        "lookAt", sol::overload(
          (void(RenderComponent::*)(const Gsage::Vector3&, const Geometry::RotationAxis, Geometry::TransformSpace))&RenderComponent::lookAt,
          (void(RenderComponent::*)(const Gsage::Vector3&))&RenderComponent::lookAt
        ),
        "rotate", &RenderComponent::rotate,
        "playAnimation", &RenderComponent::playAnimation,
        "resetAnimation", &RenderComponent::resetAnimationState,
        "setAnimationState", &RenderComponent::setAnimationState,
        "adjustAnimationSpeed", &RenderComponent::adjustAnimationStateSpeed,
        "STATIC", sol::var(RenderComponent::STATIC),
        "DYNAMIC", sol::var(RenderComponent::DYNAMIC)
    );

    lua.new_usertype<MovementComponent>("MovementComponent",
        sol::base_classes, sol::bases<Reflection>(),
        "props", sol::property(&MovementComponent::getProps, &MovementComponent::setProps),
        "go", sol::overload(
          (void(MovementComponent::*)(const Gsage::Vector3&))&MovementComponent::go,
          (void(MovementComponent::*)(float, float, float))&MovementComponent::go
        ),
        "move", sol::overload(
          (void(MovementComponent::*)(Path3DPtr, const DataProxy&))&MovementComponent::move,
          (void(MovementComponent::*)(Path3DPtr))&MovementComponent::move
        ),
        "stop", &MovementComponent::resetPath,
        "speed", sol::property(&MovementComponent::getSpeed, &MovementComponent::setSpeed),
        "currentTarget", sol::property(&MovementComponent::currentTarget),
        "target", sol::property(&MovementComponent::getFinalTarget),
        "hasTarget", sol::property(&MovementComponent::hasTarget)
    );

    lua.new_usertype<Entity>("Entity",
        "id", sol::property(&Entity::getId),
        "class", sol::property(&Entity::getClass),
        "props", sol::property(&Entity::getProps),
        "vars", sol::property(&Entity::getVars, &Entity::setVars),
        "hasComponent", &Entity::hasComponent,
        "hasComponents", &Entity::hasComponents,
        "componentNames", sol::property(&Entity::getComponentNames)
    );

    lua["Entity"]["getVars"] = &Entity::getVars;

    // register component getters
    lua["Entity"]["script"] = &Entity::getComponent<ScriptComponent>;
    lua["Entity"]["stats"] = &Entity::getComponent<StatsComponent>;
    lua["Entity"]["movement"] = &Entity::getComponent<MovementComponent>;

    lua.new_usertype<Engine>("Engine",
        sol::base_classes, sol::bases<EventDispatcher>(),
        "settings", sol::property(&Engine::settings),
        "env", sol::property(&Engine::env),
        "setEnv", sol::overload(
          &Engine::setEnv<int>,
          &Engine::setEnv<const std::string&>,
          &Engine::setEnv<const DataProxy&>,
          &Engine::setEnv<double>,
          &Engine::setEnv<float>,
          &Engine::setEnv<bool>
        ),
        "configureSystem", (bool(Engine::*)(const std::string&, const DataProxy&, bool))&Engine::configureSystem
    );

    lua["Engine"]["removeEntity"] = (bool(Engine::*)(const std::string& id))&Engine::removeEntity;
    lua["Engine"]["removeSystem"] = &Engine::removeSystem;
    lua["Engine"]["getEntity"] = &Engine::getEntity;
    lua["Engine"]["getSystem"] = (EngineSystem*(Engine::*)(const std::string& name))&Engine::getSystem;
    lua["Engine"]["getSystems"] = &Engine::getSystems;
    lua["Engine"]["hasSystem"] = (bool(Engine::*)(const std::string& name))&Engine::hasSystem;
    lua["Engine"]["getSystemNames"] = [](Engine* e) -> std::vector<std::string>{
      std::vector<std::string> res;
      for(auto& pair : e->getSystems()) {
        res.push_back(pair.first);
      }
      return res;
    };
    lua["Engine"]["getEntities"] = &Engine::getEntities;

    // register system getters
    lua["Engine"]["script"] = &Engine::getSystem<LuaScriptSystem>;
    lua["Engine"]["movement"] = &Engine::getSystem<MovementSystem>;

    lua.new_usertype<GameDataManager>("DataManager",
        "getEntityData", &GameDataManager::getEntityData,
        "removeEntity", &GameDataManager::removeEntity,
        "scenesFolder", sol::property(&GameDataManager::getScenesFolder),
        "charactersFolder", sol::property(&GameDataManager::getCharactersFolder),
        "savesFolder", sol::property(&GameDataManager::getSavesFolder),
        "fileExtension", sol::property(&GameDataManager::getFileExtension),
        "saveScene", &GameDataManager::saveScene,
        "getSceneData", &GameDataManager::getSceneData,
        "setSceneData", &GameDataManager::setSceneData,
        "createEntity", sol::overload(
          (Entity*(GameDataManager::*)(const std::string&))&GameDataManager::createEntity,
          (Entity*(GameDataManager::*)(const std::string&, const DataProxy&))&GameDataManager::createEntity,
          [] (GameDataManager* self, const std::string& name, DataProxy params)
          {
            return self->createEntity(name, params);
          },
          (Entity*(GameDataManager::*)(DataProxy))&GameDataManager::createEntity
        ),
        "read", [] (GameDataManager* self, const std::string& path) {
          std::string res;
          if(!FileLoader::getSingletonPtr()->load(path, res)) {
            return std::make_tuple(res, false);
          }
          return std::make_tuple(res, true);
        },
        "readJSON", [&] (GameDataManager* self, const std::string& path) {
          DataProxy res;
          if(!FileLoader::getSingletonPtr()->load(path, DataProxy(), res)) {
            return std::make_tuple(res, false);
          }
          return std::make_tuple(res, true);
        },
        "write", [] (GameDataManager* self, const std::string& path, const std::string& data, const std::string& rootDir) {
          return FileLoader::getSingletonPtr()->dump(path, data, rootDir);
        },
        "addSearchFolder", [](GameDataManager* self, int index, const std::string& path) { FileLoader::getSingletonPtr()->addSearchFolder(index, path); },
        "removeSearchFolder", [](GameDataManager* self, const std::string& path) { return FileLoader::getSingletonPtr()->removeSearchFolder(path); },
        "loadTemplate", [] (GameDataManager* self, const std::string& path, const DataProxy& context) {
          std::string res;
          bool success = FileLoader::getSingletonPtr()->loadTemplate(path, res, context);
          return std::make_tuple(res, success);
        },
        "addTemplateCallback", [] (GameDataManager* self, const std::string& name, int argsNumber, sol::function function) {
          return FileLoader::getSingletonPtr()->addTemplateCallback(name, argsNumber, function);
        }
    );

    lua.new_usertype<LuaEventProxy>("LuaEventProxy",
        "new", sol::constructors<void()>()
    );
    lua["LuaEventProxy"]["bind"] = (bool(LuaEventProxy::*)(EventDispatcher*, Event::ConstType, const sol::object&))&LuaEventProxy::addEventListener;
    lua["LuaEventProxy"]["unbind"] = &LuaEventProxy::removeEventListener;

    lua.new_usertype<LuaEventConnection>("LuaEventConnection",
        "new", sol::constructors<sol::types<sol::protected_function>>()
    );
    lua["LuaEventConnection"]["bind"] = (long(LuaEventConnection::*)(EventDispatcher*, Event::ConstType))&LuaEventConnection::bind;
    lua["LuaEventConnection"]["unbind"] = &LuaEventConnection::unbind;

    // events

    lua.new_usertype<Event>("Event",
        "new", sol::constructors<sol::types<Event::ConstType>>(),
        "type", sol::property(&Event::getType)
    );

    registerEvent<SystemChangeEvent>("SystemChangeEvent",
        "onSystemChange",
        sol::base_classes, sol::bases<Event>(),
        "systemID", &SystemChangeEvent::mSystemId,
        "system", &SystemChangeEvent::mSystem,
        "SYSTEM_ADDED", sol::var(SystemChangeEvent::SYSTEM_ADDED),
        "SYSTEM_REMOVED", sol::var(SystemChangeEvent::SYSTEM_REMOVED),
        "SYSTEM_STARTED", sol::var(SystemChangeEvent::SYSTEM_STARTED),
        "SYSTEM_STOPPING", sol::var(SystemChangeEvent::SYSTEM_STOPPING)
    );

    registerEvent<FileEvent>("FileEvent",
        "onFile",
        sol::base_classes, sol::bases<Event>(),
        "id", &FileEvent::id,
        "COPY_COMPLETE", sol::var(FileEvent::COPY_COMPLETE),
        "COPY_FAILED", sol::var(FileEvent::COPY_FAILED)
    );

    registerEvent<SelectEvent>("SelectEvent",
        "onSelect",
        sol::base_classes, sol::bases<Event>(),
        "hasFlags", &SelectEvent::hasFlags,
        "entity", sol::property(&SelectEvent::getEntityId),
        "OBJECT_SELECTED", sol::var(SelectEvent::OBJECT_SELECTED),
        "ROLL_OVER", sol::var(SelectEvent::ROLL_OVER),
        "ROLL_OUT", sol::var(SelectEvent::ROLL_OUT)
    );

    registerEvent<SettingsEvent>("SettingsEvent",
        "onSettings",
        sol::base_classes, sol::bases<Event>(),
        "UPDATE", sol::var(SettingsEvent::UPDATE),
        "settings", sol::readonly(&SettingsEvent::settings)
    );

    registerEvent<StatEvent>("StatEvent",
        "onStat",
        sol::base_classes, sol::bases<Event>(),
        "id", sol::property(&StatEvent::getId),
        "STAT_CHANGE", sol::var(StatEvent::STAT_CHANGE)
    );

    registerEvent<KeyboardEvent>("KeyboardEvent",
        "onKeyboard",
        sol::base_classes, sol::bases<Event>(),
        "text", sol::readonly(&KeyboardEvent::text),
        "key", sol::readonly(&KeyboardEvent::key),
        "modifiers", sol::property(&KeyboardEvent::getModifiersState),
        "isModifierDown", &KeyboardEvent::isModifierDown,
        "KEY_DOWN", sol::var(KeyboardEvent::KEY_DOWN),
        "KEY_UP", sol::var(KeyboardEvent::KEY_UP)
    );

    registerEvent<TextInputEvent>("TextInputEvent",
        "onInput",
        sol::base_classes, sol::bases<Event>(),
        "INPUT", sol::var(TextInputEvent::INPUT)
    );

    registerEvent<EntityEvent>("EntityEvent",
        "onEntity",
        sol::base_classes, sol::bases<Event>(),
        "id", sol::readonly(&EntityEvent::mEntityId),
        "CREATE", sol::var(EntityEvent::CREATE),
        "REMOVE", sol::var(EntityEvent::REMOVE)
    );

    registerEvent<EngineEvent>("EngineEvent",
        "onEngine",
        sol::base_classes, sol::bases<Event>(),
        "STOPPING", sol::var(EngineEvent::STOPPING),
        "SHUTDOWN", sol::var(EngineEvent::SHUTDOWN)
    );

    registerEvent<DropFileEvent>("DropFileEvent",
        "onFileDrop",
        sol::base_classes, sol::bases<Event>(),
        "DROP_FILE", sol::var(DropFileEvent::DROP_FILE),
        "DROP_BEGIN", sol::var(DropFileEvent::DROP_BEGIN),
        "file", &DropFileEvent::file
    );

    registerEvent<WindowEvent>("WindowEvent",
        "onWindow",
        sol::base_classes, sol::bases<Event>(),
        "CLOSE", sol::var(WindowEvent::CLOSE),
        "RESIZE", sol::var(WindowEvent::RESIZE),
        "MOVE", sol::var(WindowEvent::MOVE),
        "CREATE", sol::var(WindowEvent::CREATE),
        "x", &WindowEvent::x,
        "y", &WindowEvent::y,
        "width", &WindowEvent::width,
        "height", &WindowEvent::height,
        "handle", &WindowEvent::handle
    );

    lua.create_table("Keys");

    lua["Keys"]["Ctrl"] = KeyboardEvent::Ctrl;
    lua["Keys"]["Shift"] = KeyboardEvent::Shift;
    lua["Keys"]["Alt"] = KeyboardEvent::Alt;
    lua["Keys"]["KC_UNASSIGNED"] = KeyboardEvent::KC_UNASSIGNED;
    lua["Keys"]["KC_ESCAPE"] = KeyboardEvent::KC_ESCAPE;
    lua["Keys"]["KC_1"] = KeyboardEvent::KC_1;
    lua["Keys"]["KC_2"] = KeyboardEvent::KC_2;
    lua["Keys"]["KC_3"] = KeyboardEvent::KC_3;
    lua["Keys"]["KC_4"] = KeyboardEvent::KC_4;
    lua["Keys"]["KC_5"] = KeyboardEvent::KC_5;
    lua["Keys"]["KC_6"] = KeyboardEvent::KC_6;
    lua["Keys"]["KC_7"] = KeyboardEvent::KC_7;
    lua["Keys"]["KC_8"] = KeyboardEvent::KC_8;
    lua["Keys"]["KC_9"] = KeyboardEvent::KC_9;
    lua["Keys"]["KC_0"] = KeyboardEvent::KC_0;
    lua["Keys"]["KC_MINUS"] = KeyboardEvent::KC_MINUS;
    lua["Keys"]["KC_EQUALS"] = KeyboardEvent::KC_EQUALS;
    lua["Keys"]["KC_BACK"] = KeyboardEvent::KC_BACK;
    lua["Keys"]["KC_TAB"] = KeyboardEvent::KC_TAB;
    lua["Keys"]["KC_Q"] = KeyboardEvent::KC_Q;
    lua["Keys"]["KC_W"] = KeyboardEvent::KC_W;
    lua["Keys"]["KC_E"] = KeyboardEvent::KC_E;
    lua["Keys"]["KC_R"] = KeyboardEvent::KC_R;
    lua["Keys"]["KC_T"] = KeyboardEvent::KC_T;
    lua["Keys"]["KC_Y"] = KeyboardEvent::KC_Y;
    lua["Keys"]["KC_U"] = KeyboardEvent::KC_U;
    lua["Keys"]["KC_I"] = KeyboardEvent::KC_I;
    lua["Keys"]["KC_O"] = KeyboardEvent::KC_O;
    lua["Keys"]["KC_P"] = KeyboardEvent::KC_P;
    lua["Keys"]["KC_LBRACKET"] = KeyboardEvent::KC_LBRACKET;
    lua["Keys"]["KC_RBRACKET"] = KeyboardEvent::KC_RBRACKET;
    lua["Keys"]["KC_RETURN"] = KeyboardEvent::KC_RETURN;
    lua["Keys"]["KC_LCONTROL"] = KeyboardEvent::KC_LCONTROL;
    lua["Keys"]["KC_A"] = KeyboardEvent::KC_A;
    lua["Keys"]["KC_S"] = KeyboardEvent::KC_S;
    lua["Keys"]["KC_D"] = KeyboardEvent::KC_D;
    lua["Keys"]["KC_F"] = KeyboardEvent::KC_F;
    lua["Keys"]["KC_G"] = KeyboardEvent::KC_G;
    lua["Keys"]["KC_H"] = KeyboardEvent::KC_H;
    lua["Keys"]["KC_J"] = KeyboardEvent::KC_J;
    lua["Keys"]["KC_K"] = KeyboardEvent::KC_K;
    lua["Keys"]["KC_L"] = KeyboardEvent::KC_L;
    lua["Keys"]["KC_SEMICOLON"] = KeyboardEvent::KC_SEMICOLON;
    lua["Keys"]["KC_APOSTROPHE"] = KeyboardEvent::KC_APOSTROPHE;
    lua["Keys"]["KC_GRAVE"] = KeyboardEvent::KC_GRAVE;
    lua["Keys"]["KC_LSHIFT"] = KeyboardEvent::KC_LSHIFT;
    lua["Keys"]["KC_BACKSLASH"] = KeyboardEvent::KC_BACKSLASH;
    lua["Keys"]["KC_Z"] = KeyboardEvent::KC_Z;
    lua["Keys"]["KC_X"] = KeyboardEvent::KC_X;
    lua["Keys"]["KC_C"] = KeyboardEvent::KC_C;
    lua["Keys"]["KC_V"] = KeyboardEvent::KC_V;
    lua["Keys"]["KC_B"] = KeyboardEvent::KC_B;
    lua["Keys"]["KC_N"] = KeyboardEvent::KC_N;
    lua["Keys"]["KC_M"] = KeyboardEvent::KC_M;
    lua["Keys"]["KC_COMMA"] = KeyboardEvent::KC_COMMA;
    lua["Keys"]["KC_PERIOD"] = KeyboardEvent::KC_PERIOD;
    lua["Keys"]["KC_SLASH"] = KeyboardEvent::KC_SLASH;
    lua["Keys"]["KC_RSHIFT"] = KeyboardEvent::KC_RSHIFT;
    lua["Keys"]["KC_MULTIPLY"] = KeyboardEvent::KC_MULTIPLY;
    lua["Keys"]["KC_LMENU"] = KeyboardEvent::KC_LMENU;
    lua["Keys"]["KC_SPACE"] = KeyboardEvent::KC_SPACE;
    lua["Keys"]["KC_CAPITAL"] = KeyboardEvent::KC_CAPITAL;
    lua["Keys"]["KC_F1"] = KeyboardEvent::KC_F1;
    lua["Keys"]["KC_F2"] = KeyboardEvent::KC_F2;
    lua["Keys"]["KC_F3"] = KeyboardEvent::KC_F3;
    lua["Keys"]["KC_F4"] = KeyboardEvent::KC_F4;
    lua["Keys"]["KC_F5"] = KeyboardEvent::KC_F5;
    lua["Keys"]["KC_F6"] = KeyboardEvent::KC_F6;
    lua["Keys"]["KC_F7"] = KeyboardEvent::KC_F7;
    lua["Keys"]["KC_F8"] = KeyboardEvent::KC_F8;
    lua["Keys"]["KC_F9"] = KeyboardEvent::KC_F9;
    lua["Keys"]["KC_F10"] = KeyboardEvent::KC_F10;
    lua["Keys"]["KC_NUMLOCK"] = KeyboardEvent::KC_NUMLOCK;
    lua["Keys"]["KC_SCROLL"] = KeyboardEvent::KC_SCROLL;
    lua["Keys"]["KC_NUMPAD7"] = KeyboardEvent::KC_NUMPAD7;
    lua["Keys"]["KC_NUMPAD8"] = KeyboardEvent::KC_NUMPAD8;
    lua["Keys"]["KC_NUMPAD9"] = KeyboardEvent::KC_NUMPAD9;
    lua["Keys"]["KC_SUBTRACT"] = KeyboardEvent::KC_SUBTRACT;
    lua["Keys"]["KC_NUMPAD4"] = KeyboardEvent::KC_NUMPAD4;
    lua["Keys"]["KC_NUMPAD5"] = KeyboardEvent::KC_NUMPAD5;
    lua["Keys"]["KC_NUMPAD6"] = KeyboardEvent::KC_NUMPAD6;
    lua["Keys"]["KC_ADD"] = KeyboardEvent::KC_ADD;
    lua["Keys"]["KC_NUMPAD1"] = KeyboardEvent::KC_NUMPAD1;
    lua["Keys"]["KC_NUMPAD2"] = KeyboardEvent::KC_NUMPAD2;
    lua["Keys"]["KC_NUMPAD3"] = KeyboardEvent::KC_NUMPAD3;
    lua["Keys"]["KC_NUMPAD0"] = KeyboardEvent::KC_NUMPAD0;
    lua["Keys"]["KC_DECIMAL"] = KeyboardEvent::KC_DECIMAL;
    lua["Keys"]["KC_OEM_102"] = KeyboardEvent::KC_OEM_102;
    lua["Keys"]["KC_F11"] = KeyboardEvent::KC_F11;
    lua["Keys"]["KC_F12"] = KeyboardEvent::KC_F12;
    lua["Keys"]["KC_F13"] = KeyboardEvent::KC_F13;
    lua["Keys"]["KC_F14"] = KeyboardEvent::KC_F14;
    lua["Keys"]["KC_F15"] = KeyboardEvent::KC_F15;
    lua["Keys"]["KC_KANA"] = KeyboardEvent::KC_KANA;
    lua["Keys"]["KC_ABNT_C1"] = KeyboardEvent::KC_ABNT_C1;
    lua["Keys"]["KC_CONVERT"] = KeyboardEvent::KC_CONVERT;
    lua["Keys"]["KC_NOCONVERT"] = KeyboardEvent::KC_NOCONVERT;
    lua["Keys"]["KC_YEN"] = KeyboardEvent::KC_YEN;
    lua["Keys"]["KC_ABNT_C2"] = KeyboardEvent::KC_ABNT_C2;
    lua["Keys"]["KC_NUMPADEQUALS"] = KeyboardEvent::KC_NUMPADEQUALS;
    lua["Keys"]["KC_PREVTRACK"] = KeyboardEvent::KC_PREVTRACK;
    lua["Keys"]["KC_AT"] = KeyboardEvent::KC_AT;
    lua["Keys"]["KC_COLON"] = KeyboardEvent::KC_COLON;
    lua["Keys"]["KC_UNDERLINE"] = KeyboardEvent::KC_UNDERLINE;
    lua["Keys"]["KC_KANJI"] = KeyboardEvent::KC_KANJI;
    lua["Keys"]["KC_STOP"] = KeyboardEvent::KC_STOP;
    lua["Keys"]["KC_AX"] = KeyboardEvent::KC_AX;
    lua["Keys"]["KC_UNLABELED"] = KeyboardEvent::KC_UNLABELED;
    lua["Keys"]["KC_NEXTTRACK"] = KeyboardEvent::KC_NEXTTRACK;
    lua["Keys"]["KC_NUMPADENTER"] = KeyboardEvent::KC_NUMPADENTER;
    lua["Keys"]["KC_RCONTROL"] = KeyboardEvent::KC_RCONTROL;
    lua["Keys"]["KC_MUTE"] = KeyboardEvent::KC_MUTE;
    lua["Keys"]["KC_CALCULATOR"] = KeyboardEvent::KC_CALCULATOR;
    lua["Keys"]["KC_PLAYPAUSE"] = KeyboardEvent::KC_PLAYPAUSE;
    lua["Keys"]["KC_MEDIASTOP"] = KeyboardEvent::KC_MEDIASTOP;
    lua["Keys"]["KC_VOLUMEDOWN"] = KeyboardEvent::KC_VOLUMEDOWN;
    lua["Keys"]["KC_VOLUMEUP"] = KeyboardEvent::KC_VOLUMEUP;
    lua["Keys"]["KC_WEBHOME"] = KeyboardEvent::KC_WEBHOME;
    lua["Keys"]["KC_NUMPADCOMMA"] = KeyboardEvent::KC_NUMPADCOMMA;
    lua["Keys"]["KC_DIVIDE"] = KeyboardEvent::KC_DIVIDE;
    lua["Keys"]["KC_SYSRQ"] = KeyboardEvent::KC_SYSRQ;
    lua["Keys"]["KC_RMENU"] = KeyboardEvent::KC_RMENU;
    lua["Keys"]["KC_PAUSE"] = KeyboardEvent::KC_PAUSE;
    lua["Keys"]["KC_HOME"] = KeyboardEvent::KC_HOME;
    lua["Keys"]["KC_UP"] = KeyboardEvent::KC_UP;
    lua["Keys"]["KC_PGUP"] = KeyboardEvent::KC_PGUP;
    lua["Keys"]["KC_LEFT"] = KeyboardEvent::KC_LEFT;
    lua["Keys"]["KC_RIGHT"] = KeyboardEvent::KC_RIGHT;
    lua["Keys"]["KC_END"] = KeyboardEvent::KC_END;
    lua["Keys"]["KC_DOWN"] = KeyboardEvent::KC_DOWN;
    lua["Keys"]["KC_PGDOWN"] = KeyboardEvent::KC_PGDOWN;
    lua["Keys"]["KC_INSERT"] = KeyboardEvent::KC_INSERT;
    lua["Keys"]["KC_DELETE"] = KeyboardEvent::KC_DELETE;
    lua["Keys"]["KC_LWIN"] = KeyboardEvent::KC_LWIN;
    lua["Keys"]["KC_RWIN"] = KeyboardEvent::KC_RWIN;
    lua["Keys"]["KC_APPS"] = KeyboardEvent::KC_APPS;
    lua["Keys"]["KC_POWER"] = KeyboardEvent::KC_POWER;
    lua["Keys"]["KC_SLEEP"] = KeyboardEvent::KC_SLEEP;
    lua["Keys"]["KC_WAKE"] = KeyboardEvent::KC_WAKE;
    lua["Keys"]["KC_WEBSEARCH"] = KeyboardEvent::KC_WEBSEARCH;
    lua["Keys"]["KC_WEBFAVORITES"] = KeyboardEvent::KC_WEBFAVORITES;
    lua["Keys"]["KC_WEBREFRESH"] = KeyboardEvent::KC_WEBREFRESH;
    lua["Keys"]["KC_WEBSTOP"] = KeyboardEvent::KC_WEBSTOP;
    lua["Keys"]["KC_WEBFORWARD"] = KeyboardEvent::KC_WEBFORWARD;
    lua["Keys"]["KC_WEBBACK"] = KeyboardEvent::KC_WEBBACK;
    lua["Keys"]["KC_MYCOMPUTER"] = KeyboardEvent::KC_MYCOMPUTER;
    lua["Keys"]["KC_MAIL"] = KeyboardEvent::KC_MAIL;
    lua["Keys"]["KC_MEDIASELECT"] = KeyboardEvent::KC_MEDIASELECT;
    lua["Keys"]["KC_LALT"] = KeyboardEvent::KC_LALT;
    lua["Keys"]["KC_RALT"] = KeyboardEvent::KC_RALT;

    lua["Modifiers"] = lua.create_table();
    lua["Modifiers"]["LShift"] = KeyboardEvent::LShift;
    lua["Modifiers"]["RShift"] = KeyboardEvent::RShift;
    lua["Modifiers"]["LCtrl"] = KeyboardEvent::LCtrl;
    lua["Modifiers"]["RCtrl"] = KeyboardEvent::RCtrl;
    lua["Modifiers"]["LAlt"] = KeyboardEvent::LAlt;
    lua["Modifiers"]["RAlt"] = KeyboardEvent::RAlt;
    lua["Modifiers"]["LWin"] = KeyboardEvent::LWin;
    lua["Modifiers"]["RWin"] = KeyboardEvent::RWin;
    lua["Modifiers"]["Num"] = KeyboardEvent::Num;
    lua["Modifiers"]["Caps"] = KeyboardEvent::Caps;
    lua["Modifiers"]["Ctrl"] = KeyboardEvent::Ctrl;
    lua["Modifiers"]["Shift"] = KeyboardEvent::Shift;
    lua["Modifiers"]["Alt"] = KeyboardEvent::Alt;
    lua["Modifiers"]["Win"] = KeyboardEvent::Win;

    registerEvent<MouseEvent>("MouseEvent",
        "onMouse",
        sol::base_classes, sol::bases<Event>(),
        "button", sol::readonly(&MouseEvent::button),
        "x", &MouseEvent::mouseX,
        "y", &MouseEvent::mouseY,
        "z", &MouseEvent::mouseZ,
        "relX", sol::readonly(&MouseEvent::relativeX),
        "relY", sol::readonly(&MouseEvent::relativeY),
        "relZ", sol::readonly(&MouseEvent::relativeZ),
        "width", sol::readonly(&MouseEvent::width),
        "height", sol::readonly(&MouseEvent::height),
        "dispatcher", sol::readonly(&MouseEvent::dispatcher),

        "MOUSE_DOWN", sol::var(MouseEvent::MOUSE_DOWN),
        "MOUSE_UP", sol::var(MouseEvent::MOUSE_UP),
        "MOUSE_MOVE", sol::var(MouseEvent::MOUSE_MOVE)
    );

    lua.create_table("Mouse");

    lua["Mouse"]["Left"] = sol::var(MouseEvent::Left);
    lua["Mouse"]["Right"] = sol::var(MouseEvent::Right);
    lua["Mouse"]["Middle"] = sol::var(MouseEvent::Middle);
    lua["Mouse"]["Button3"] = sol::var(MouseEvent::Button3);
    lua["Mouse"]["Button4"] = sol::var(MouseEvent::Button4);
    lua["Mouse"]["Button5"] = sol::var(MouseEvent::Button5);
    lua["Mouse"]["Button6"] = sol::var(MouseEvent::Button6);
    lua["Mouse"]["Button7"] = sol::var(MouseEvent::Button7);
    lua["Mouse"]["None"] = sol::var(MouseEvent::None);

    mInstance->getEngine()->fireEvent(EngineEvent(EngineEvent::LUA_STATE_CHANGE));

    lua["game"] = mInstance;
    lua["core"] = mInstance->getEngine();
    lua["data"] = mInstance->getGameDataManager();

    addCommonBindings(mState);

    return true;
  }

  void LuaInterface::setResourcePath(const std::string& path)
  {
    mResourcePath = path;
    if(mStateView) {
      (*mStateView)["resourcePath"] = mResourcePath;
      mStateView->script("function getResourcePath(path) return resourcePath .. '/' .. path; end");
    }
  }

  sol::state_view* LuaInterface::getSolState() {
    return mStateView;
  }

  lua_State* LuaInterface::getState()
  {
    return mState;
  }

  bool LuaInterface::runPackager(const std::string& path, const DataProxy& dependencies)
  {
    sol::state lua;
    luaL_openlibs(lua.lua_state());
    lua["dependencies"] = dependencies;

    addCommonBindings(lua.lua_state());

    const std::string runPrefix = mResourcePath + GSAGE_PATH_SEPARATOR + "luarocks";
#if GSAGE_PLATFORM == GSAGE_LINUX || GSAGE_PLATFORM == GSAGE_APPLE
    char *resolvedName = NULL;
    char *fullPath = realpath(runPrefix.c_str(), resolvedName);
    if(!fullPath) {
      LOG_IF(resolvedName != NULL, ERROR) << "Failed to resolve path " << resolvedName;
      return false;
    }
    lua["run_prefix"] = std::string(fullPath);
    free(fullPath);

#elif GSAGE_PLATFORM == GSAGE_WIN32
    TCHAR full_path[200];
    GetFullPathName(_T(runPrefix.c_str()), 200, full_path, NULL);
    lua["run_prefix"] = std::string(full_path);

    lua.new_usertype<CommandPipe>("CommandPipe",
      "lines", &CommandPipe::lines,
      "close", &CommandPipe::close,
      "read", &CommandPipe::read,
      sol::meta_function::call, &CommandPipe::next
    );

    lua["os"]["execute"] = [](const std::string& cmd){
      return Gsage::luaCmd(cmd);
    };
    lua["io"]["popen"] = [](const std::string& cmd) {
      return Gsage::luaPopen(cmd);
    };
#else
    lua["run_prefix"] = runPrefix;
#endif
    lua.script("function getResourcePath(path) return resourcePath .. '/' .. path; end");
    return runScript(path, &lua);
  }

  bool LuaInterface::runScript(const std::string& script, sol::state_view* state)
  {
    sol::state_view* targetState = state ? state : mStateView;
    if(!targetState)
      return false;

    try {
      LOG(INFO) << "Executing script " << script;
      auto res = targetState->script_file(script);
      if(!res.valid()) {
        sol::error err = res;
        throw err;
      }
    } catch(sol::error& err) {
      LOG(ERROR) << "Failed to execute lua script: " << err.what();
      return false;
    } catch(std::exception& err) {
      LOG(ERROR) << "Failed to execute lua script: " << err.what();
      return false;
    } catch(...) {
      LOG(ERROR) << "Failed to execute lua script: " << lua_tostring(mStateView->lua_state(), -1);
      return false;
    }

    return true;
  }

  void LuaInterface::addCommonBindings(lua_State* L)
  {
    sol::state_view lua(L);

    lua["resourcePath"] = mResourcePath;

    // Logging
    lua.new_usertype<LogProxy>("LogProxy",
        "subscribe", &LogProxy::subscribe,
        "unsubscribe", &LogProxy::unsubscribe,
        "flushMessages", &LogProxy::flushMessages
    );

    lua.new_usertype<LogProxy::LogMessage>("LogMessage",
        "level", &LogProxy::LogMessage::Level,
        "message", &LogProxy::LogMessage::Message,
        "file", &LogProxy::LogMessage::File,
        "line", &LogProxy::LogMessage::Line,
        "func", &LogProxy::LogMessage::Func,
        "Global", sol::var(el::Level::Global),
        "Trace", sol::var(el::Level::Trace),
        "Debug", sol::var(el::Level::Debug),
        "Error", sol::var(el::Level::Error),
        "Fatal", sol::var(el::Level::Fatal),
        "Warning", sol::var(el::Level::Warning),
        "Verbose", sol::var(el::Level::Verbose),
        "Info", sol::var(el::Level::Info)
    );

    lua["log"] = lua.create_table();
    lua["log"]["info"] = [] (const char* message) { LOG(INFO) << message; };
    lua["log"]["error"] = [] (const char* message) { LOG(ERROR) << message; };
    lua["log"]["debug"] = [] (const char* message) { LOG(DEBUG) << message; };
    lua["log"]["warn"] = [] (const char* message) { LOG(WARNING) << message; };
    lua["log"]["trace"] = [] (const char* message) { LOG(TRACE) << message; };

    lua["log"]["proxy"] = std::shared_ptr<LogProxy>(new LogProxy());

    lua["bit"] = lua.create_table();
    lua["bit"]["brshift"] = [](int n, size_t bits) { return bits >> n; };
    lua["bit"]["blshift"] = [](int n, size_t bits) { return bits << n; };
    lua["bit"]["band"] = [](size_t left, size_t right) { return left & right; };
    lua["bit"]["bor"] = [](size_t left, size_t right) { return left | right; };

    // serialization
    lua["json"] = lua.create_table();
    lua["json"]["load"] = [](const std::string& path) { return load(path, DataWrapper::JSON_OBJECT); };
    lua["json"]["dump"] = [](const std::string& path, const DataProxy& value) { return dump(value, path, DataWrapper::JSON_OBJECT, true); };
    lua["json"]["loads"] = [](const std::string& str) { return loads(str, DataWrapper::JSON_OBJECT); };
    lua["json"]["dumps"] = [](const DataProxy& value) { return dumps(value, DataWrapper::JSON_OBJECT, true); };

    std::string platform;
    std::stringstream ss;
    ss << GSAGE_VERSION_MAJOR << "." << GSAGE_VERSION_MINOR << "." << GSAGE_VERSION_PATCH << "." << GSAGE_VERSION_BUILD;
#if GSAGE_PLATFORM == GSAGE_APPLE
    platform = "apple";
#elif GSAGE_PLATFORM == GSAGE_LINUX
    platform = "linux";
#elif GSAGE_PLATFORM == GSAGE_WIN32
    platform = "win32";
#else
    platform = "other";
#endif

    lua["gsage"] = lua.create_table_with(
      "version", ss.str(),
      "platform", platform
    );

    lua["gsage"]["configure"] = lua.create_table_with(
      "All", GsageFacade::All,
      "Plugins", GsageFacade::Plugins,
      "Managers", GsageFacade::Managers,
      "Systems", GsageFacade::Systems,
      "RestartSystems", GsageFacade::RestartSystems
    );

    // some utility functions
    lua["md5Hash"] = [] (const std::string& value) -> size_t {return std::hash<std::string>()(value);};
    lua["split"] = [](const std::string& s, char delim) -> std::vector<std::string> {
      return split(s, delim);
    };
    lua.script("function getResourcePath(path) return resourcePath .. '/' .. path; end");
  }
}
