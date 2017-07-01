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

#include "ImguiManager.h"

#ifdef OGRE_INTERFACE
#include "ogre/ImguiOgreWrapper.h"
#endif

#include <imgui.h>

#include "MouseEvent.h"
#include "Engine.h"
#include "EngineSystem.h"
#include "EngineEvent.h"

namespace Gsage {

  void ImguiRenderer::addView(const std::string& name, sol::object view)
  {
    sol::protected_function callback;
    sol::table t = view.as<sol::table>();
    sol::optional<sol::protected_function> call = t[sol::meta_function::call];
    RenderView render;

    if (call) {
      render = [t, call] () -> sol::protected_function_result { return call.value()(t); };
    } else if(view.is<sol::protected_function>()) {
      callback = view.as<sol::protected_function>();
      render = [callback] () -> sol::protected_function_result { return callback(); };
    } else {
      LOG(ERROR) << "Failed to add lua object as lua view " << name << ": must be either callable or function";
      return;
    }

    mViews[name] = render;
  }

  bool ImguiRenderer::removeView(const std::string& name)
  {
    if(mViews.count(name) == 0) {
      return false;
    }

    mViews.erase(name);
    return true;
  }

  void ImguiRenderer::renderViews()
  {
    for(auto pair : mViews) {
      try {
        auto res = pair.second();
      } catch(sol::error e) {
        LOG(ERROR) << "Failed to render view " << pair.first << ": " << e.what();
      }
    }
  }

  ImguiManager::ImguiManager()
    : mRenderer(0)
    , mIsSetUp(false)
  {
  }

  ImguiManager::~ImguiManager()
  {
    if(mRenderer)
      delete mRenderer;
  }

  void ImguiManager::initialize(Engine* engine, lua_State* luaState)
  {
    UIManager::initialize(engine, luaState);
    setUp();
    addEventListener(engine, SystemChangeEvent::SYSTEM_ADDED, &ImguiManager::handleSystemChange);
    addEventListener(engine, SystemChangeEvent::SYSTEM_REMOVED, &ImguiManager::handleSystemChange);
  }

  bool ImguiManager::handleSystemChange(EventDispatcher* sender, const Event& event)
  {
    const SystemChangeEvent& e = static_cast<const SystemChangeEvent&>(event);
    if(e.mSystemId != "render") {
      return true;
    }

    if(e.getType() == SystemChangeEvent::SYSTEM_ADDED)
    {
      setUp();
    }

    if(e.getType() == SystemChangeEvent::SYSTEM_REMOVED && mRenderer != 0)
    {
      mIsSetUp = false;
      delete mRenderer;
      LOG(INFO) << "Render system was removed, system wrapper was destroyed";
      mRenderer = 0;
    }
    return true;
  }

  void ImguiManager::setUp()
  {
    if(mIsSetUp)
      return;

    EngineSystem* render = mEngine->getSystem("render");
    if(render == 0) {
      return;
    }

    const std::string type = render->getSystemInfo().get("type", "unknown");
    bool initialized = false;
#ifdef OGRE_INTERFACE
    if(type == "ogre") {
      LOG(INFO) << "Initialize for render system ogre3d";
      mRenderer = new ImguiOgreWrapper(mEngine);
      initialized = true;
    }

    setLuaState(mLuaState);
#endif

    if(!initialized) {
      LOG(ERROR) << "Failed to initialize imgui ui manager with the render system of type \"" << type << "\"";
      return;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = KeyboardEvent::KC_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = KeyboardEvent::KC_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = KeyboardEvent::KC_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = KeyboardEvent::KC_UP;
    io.KeyMap[ImGuiKey_DownArrow] = KeyboardEvent::KC_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = KeyboardEvent::KC_PGUP;
    io.KeyMap[ImGuiKey_PageDown] = KeyboardEvent::KC_PGDOWN;
    io.KeyMap[ImGuiKey_Home] = KeyboardEvent::KC_HOME;
    io.KeyMap[ImGuiKey_End] = KeyboardEvent::KC_END;
    io.KeyMap[ImGuiKey_Delete] = KeyboardEvent::KC_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = KeyboardEvent::KC_BACK;
    io.KeyMap[ImGuiKey_Enter] = KeyboardEvent::KC_RETURN;
    io.KeyMap[ImGuiKey_Escape] = KeyboardEvent::KC_ESCAPE;
    io.KeyMap[ImGuiKey_A] = KeyboardEvent::KC_A;
    io.KeyMap[ImGuiKey_C] = KeyboardEvent::KC_C;
    io.KeyMap[ImGuiKey_V] = KeyboardEvent::KC_V;
    io.KeyMap[ImGuiKey_X] = KeyboardEvent::KC_X;
    io.KeyMap[ImGuiKey_Y] = KeyboardEvent::KC_Y;
    io.KeyMap[ImGuiKey_Z] = KeyboardEvent::KC_Z;
    // mouse events
    addEventListener(mEngine, MouseEvent::MOUSE_DOWN, &ImguiManager::handleMouseEvent, -100);
    addEventListener(mEngine, MouseEvent::MOUSE_UP, &ImguiManager::handleMouseEvent, -100);
    addEventListener(mEngine, MouseEvent::MOUSE_MOVE, &ImguiManager::handleMouseEvent, -100);
    // keyboard events
    addEventListener(mEngine, KeyboardEvent::KEY_DOWN, &ImguiManager::handleKeyboardEvent, -100);
    addEventListener(mEngine, KeyboardEvent::KEY_UP, &ImguiManager::handleKeyboardEvent, -100);
    mIsSetUp = true;
  }

  lua_State* ImguiManager::getLuaState()
  {
    return mLuaState;
  }

  void ImguiManager::setLuaState(lua_State* L)
  {
    mLuaState = L;
    sol::state_view lua(L);
    lua["imgui"]["render"] = mRenderer;
  }

  bool ImguiManager::handleMouseEvent(EventDispatcher* sender, const Event& event)
  {
    if(!mRenderer)
      return true;

    const MouseEvent& e = static_cast<const MouseEvent&>(event);

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = e.mouseX;
    io.MousePos.y = e.mouseY;
    io.MouseWheel += e.relativeZ / 100;

    io.MouseDown[e.button] = e.getType() == MouseEvent::MOUSE_DOWN;
    return !doCapture() || e.getType() == MouseEvent::MOUSE_UP;
  }

  bool ImguiManager::handleKeyboardEvent(EventDispatcher* sender, const Event& event)
  {
    if(!mRenderer)
      return true;

    const KeyboardEvent& e = static_cast<const KeyboardEvent&>(event);

    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[e.key] = e.getType() == KeyboardEvent::KEY_DOWN;

    if(e.text > 0)
    {
        io.AddInputCharacter((unsigned short)e.text);
    }

    // Read keyboard modifiers inputs
    io.KeyCtrl = e.isModifierDown(KeyboardEvent::Ctrl);
    io.KeyShift = e.isModifierDown(KeyboardEvent::Shift);
    io.KeyAlt = e.isModifierDown(KeyboardEvent::Alt);
    io.KeySuper = false;

    return !ImGui::IsAnyItemActive() || e.getType() == KeyboardEvent::KEY_UP;
  }

  bool ImguiManager::doCapture()
  {
    return ImGui::IsMouseHoveringAnyWindow();
  }
}
