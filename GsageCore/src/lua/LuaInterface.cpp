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

#include "GameDataManager.h"
#include "EngineEvent.h"
#include "KeyboardEvent.h"
#include "MouseEvent.h"
#include "ResourceMonitor.h"

#include "components/StatsComponent.h"
#include "components/ScriptComponent.h"

#include "systems/CombatSystem.h"
#include "systems/LuaScriptSystem.h"

#include "lua/LuaEventProxy.h"
#include "lua/LuaEventConnection.h"

#if GSAGE_PLATFORM == GSAGE_LINUX || GSAGE_PLATFORM == GSAGE_APPLE
#include <limits.h>
#include <stdlib.h>
#elif GSAGE_PLATFORM == GSAGE_WIN32
#include <windows.h>
#include <tchar.h>
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
    el::Helpers::logDispatchCallback<LogSubscriber>(name)->setWrappedFunction(function);
  }

  void LogProxy::unsubscribe(const std::string& name)
  {
    el::Helpers::uninstallLogDispatchCallback<LogSubscriber>(name);
  }

  void LogProxy::LogSubscriber::handle(const el::LogDispatchData* data)
  {
    if(!mReady || !mWrapped.valid()) {
      return;
    }

    mWrapped(data->logMessage());
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
        "shutdown", &GsageFacade::shutdown,
        "reset", &GsageFacade::reset,
        "loadSave", &GsageFacade::loadSave,
        "dumpSave", &GsageFacade::dumpSave,
        "loadArea", &GsageFacade::loadArea,
        "loadPlugin", &GsageFacade::loadPlugin,
        "unloadPlugin", &GsageFacade::unloadPlugin,
        "createSystem", &GsageFacade::createSystem,
        "addUpdateListener", &GsageFacade::addUpdateListener,
        "BEFORE_RESET", sol::var(GsageFacade::BEFORE_RESET),
        "RESET", sol::var(GsageFacade::RESET),
        "LOAD", sol::var(GsageFacade::LOAD)
    );

    lua.new_usertype<Reflection>("Reflection",
        "props", sol::property(&Reflection::getProps, &Reflection::setProps)
    );

    // --------------------------------------------------------------------------------
    // Systems

    lua.new_usertype<EngineSystem>("EngineSystem",
        "enabled", sol::property(&EngineSystem::isEnabled, &EngineSystem::setEnabled),
        "info", sol::property(&EngineSystem::getSystemInfo)
    );

    lua.new_usertype<LuaScriptSystem>("ScriptSystem",
        "addUpdateListener", &LuaScriptSystem::addUpdateListener,
        "removeUpdateListener", &LuaScriptSystem::removeUpdateListener
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
          (const float(StatsComponent::*)(const std::string&)) &StatsComponent::getStat<float>,
          (const float(StatsComponent::*)(const std::string&, const float&)) &StatsComponent::getStat<float>
        ),
        "data", sol::property(&StatsComponent::data),
        "set", sol::overload(
          &StatsComponent::setStat<bool>,
          &StatsComponent::setStat<std::string>,
          &StatsComponent::setStat<float>
        ),
        "increase", &StatsComponent::increase
    );

    lua.new_usertype<ScriptComponent>("ScriptComponent",
        sol::base_classes, sol::bases<Reflection>(),
        "state", sol::property(&ScriptComponent::getData)
    );

    lua.new_usertype<Entity>("Entity",
        "id", sol::property(&Entity::getId),
        "class", sol::property(&Entity::getClass),
        "props", sol::property(&Entity::getProps, &Entity::setProps),
        "getProps", &Entity::getProps,
        "hasComponent", &Entity::hasComponent,
        "hasComponents", &Entity::hasComponents,
        "componentNames", sol::property(&Entity::getComponentNames)
    );

    lua["Entity"]["script"] = &Entity::getComponent<ScriptComponent>;
    lua["Entity"]["stats"] = &Entity::getComponent<StatsComponent>;
    lua["Entity"]["getProps"] = &Entity::getProps;

    lua.new_usertype<Engine>("Engine",
        sol::base_classes, sol::bases<EventDispatcher>(),
        "settings", sol::property(&Engine::settings)
    );

    lua["Engine"]["removeEntity"] = (bool(Engine::*)(const std::string& id))&Engine::removeEntity;
    lua["Engine"]["getEntity"] = &Engine::getEntity;
    lua["Engine"]["getSystem"] = (EngineSystem*(Engine::*)(const std::string& name))&Engine::getSystem;
    lua["Engine"]["getSystems"] = &Engine::getSystems;
    lua["Engine"]["hasSystem"] = (bool(Engine::*)(const std::string& name))&Engine::hasSystem;
    lua["Engine"]["script"] = sol::property(&Engine::getSystem<LuaScriptSystem>);
    lua["Engine"]["getSystemNames"] = [](Engine* e) -> std::vector<std::string>{
      std::vector<std::string> res;
      for(auto& pair : e->getSystems()) {
        res.push_back(pair.first);
      }
      return res;
    };
    lua["Engine"]["getEntities"] = &Engine::getEntities;

    lua.new_usertype<GameDataManager>("DataManager",
        "createEntity", sol::overload(
          (Entity*(GameDataManager::*)(const std::string&))&GameDataManager::createEntity,
          (Entity*(GameDataManager::*)(const std::string&, const DataProxy&))&GameDataManager::createEntity,
          [] (GameDataManager* self, const std::string& name, DataProxy params)
          {
            return self->createEntity(name, params);
          },
          (Entity*(GameDataManager::*)(DataProxy))&GameDataManager::createEntity
        )
    );

    lua.new_usertype<LuaEventProxy>("LuaEventProxy",
        "new", sol::constructors<void()>()
    );
    lua["LuaEventProxy"]["bind"] = (bool(LuaEventProxy::*)(EventDispatcher*, const std::string&, const sol::object&))&LuaEventProxy::addEventListener;
    lua["LuaEventProxy"]["unbind"] = &LuaEventProxy::removeEventListener;

    lua.new_usertype<LuaEventConnection>("LuaEventConnection",
        "new", sol::constructors<sol::types<sol::protected_function>>()
    );
    lua["LuaEventConnection"]["bind"] = (long(LuaEventConnection::*)(EventDispatcher*, const std::string&))&LuaEventConnection::bind;
    lua["LuaEventConnection"]["unbind"] = &LuaEventConnection::unbind;

    // events

    lua.new_usertype<Event>("BaseEvent",
        "type", sol::property(&Event::getType)
    );

    registerEvent<SystemChangeEvent>("SystemChangeEvent",
        "onSystemChange",
        sol::base_classes, sol::bases<Event>(),
        "systemID", &SystemChangeEvent::mSystemId,
        "SYSTEM_ADDED", sol::var(SystemChangeEvent::SYSTEM_ADDED),
        "SYSTEM_REMOVED", sol::var(SystemChangeEvent::SYSTEM_REMOVED)
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
        "isModifierDown", &KeyboardEvent::isModifierDown,
        "KEY_DOWN", sol::var(KeyboardEvent::KEY_DOWN),
        "KEY_UP", sol::var(KeyboardEvent::KEY_UP)
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

    registerEvent<MouseEvent>("MouseEvent",
        "onMouse",
        sol::base_classes, sol::bases<Event>(),
        "button", sol::readonly(&MouseEvent::button),
        "x", sol::readonly(&MouseEvent::mouseX),
        "y", sol::readonly(&MouseEvent::mouseY),
        "z", sol::readonly(&MouseEvent::mouseZ),
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

    // Logging
    lua.new_usertype<LogProxy>("LogProxy",
        "subscribe", &LogProxy::subscribe,
        "unsubscribe", &LogProxy::unsubscribe
    );

    lua.new_usertype<el::LogMessage>("LogMessage",
        "level", &el::LogMessage::level,
        "message", &el::LogMessage::message,
        "file", &el::LogMessage::file,
        "line", &el::LogMessage::line,
        "func", &el::LogMessage::func,
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

    lua["resourcePath"] = mResourcePath;
    lua.script("function getResourcePath(path) return resourcePath .. '/' .. path; end");
    lua["game"] = mInstance;
    lua["core"] = mInstance->getEngine();
    lua["data"] = mInstance->getGameDataManager();

    // some utility functions
    lua["md5Hash"] = [] (const std::string& value) -> size_t {return std::hash<std::string>()(value);};
    lua["split"] = [](const std::string& s, char delim) -> std::vector<std::string> {
      return split(s, delim);
    };

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
    lua["resourcePath"] = mResourcePath;

    lua["log"] = lua.create_table();
    lua["log"]["info"] = [] (const char* message) { LOG(INFO) << message; };
    lua["log"]["error"] = [] (const char* message) { LOG(ERROR) << message; };
    lua["log"]["debug"] = [] (const char* message) { LOG(DEBUG) << message; };
    lua["log"]["warn"] = [] (const char* message) { LOG(WARNING) << message; };
    lua["log"]["trace"] = [] (const char* message) { LOG(TRACE) << message; };

#if GSAGE_PLATFORM == GSAGE_APPLE
    lua["gsage_platform"] = "apple";
#elif GSAGE_PLATFORM == GSAGE_LINUX
    lua["gsage_platform"] = "linux";
#elif GSAGE_PLATFORM == GSAGE_WIN32
    lua["gsage_platform"] = "win32";
#else
    lua["gsage_platform"] = "other";
#endif
    const std::string runPrefix = mResourcePath + GSAGE_PATH_SEPARATOR + "luarocks";
#if GSAGE_PLATFORM == GSAGE_LINUX || GSAGE_PLATFORM == GSAGE_APPLE
    char *full_path = realpath(runPrefix.c_str(), NULL);
    lua["run_prefix"] = std::string(full_path);
    free(full_path);

#elif GSAGE_PLATFORM == GSAGE_WIN32
    TCHAR full_path[200];
    GetFullPathName(_T(runPrefix.c_str()), 200, full_path, NULL);
    lua["run_prefix"] = std::string(full_path);
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
}
