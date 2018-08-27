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

#include "sol.hpp"
#include <imgui.h>

#include "MouseEvent.h"
#include "Engine.h"
#include "EngineSystem.h"
#include "EngineEvent.h"
#include "ImGuiDockspaceState.h"
#include "ImGuiConverters.h"
#include "ImguiEvent.h"

namespace Gsage {

  void ImguiRenderer::initialize(Engine* engine, lua_State* L)
  {
    mEngine = engine;
    auto nodes = mEngine->settings().get<DataProxy>("imgui.renderTargetWhitelist");

    if(nodes.second) {
      for(auto pair : nodes.first) {
        std::string name = pair.second.as<std::string>();
        mRenderTargetWhitelist[name] = true;
      }
    }
  }

  ImGuiDockspaceRenderer* ImguiRenderer::getDockspace(ImGuiContext* imctx)
  {
    if(mContextNames.count(imctx) == 0) {
      return nullptr;
    }

    Context& ctx = mContexts[mContextNames[imctx]];

    return ctx.dockspace.get();
  }

  ImguiRenderer::Context* ImguiRenderer::initializeContext(const std::string& name)
  {
    Context ctx;
    memset(&ctx, 0, sizeof(ctx));
    mContexts[name] = std::move(ctx);
    return &mContexts[name];
  }

  void ImguiRenderer::setMousePosition(const std::string& name, ImVec2 position)
  {
    mMousePositions[name] = position;
  }

  void ImguiRenderer::render()
  {
    mContextLock.lock();
    for(auto& pair : mContexts) {
      if(mRenderTargetWhitelist.count(pair.first) == 0) {
        continue;
      }

      // inject imgui context
      if(pair.second.context == NULL) {
        pair.second.context = mManager->getImGuiContext(pair.first, pair.second.size);
        ImGui::SetCurrentContext(pair.second.context);
        pair.second.dockspace = std::make_unique<ImGuiDockspaceRenderer>();
        pair.second.dockspace->setState(mManager->getDockState(pair.first));
        mContextNames[pair.second.context] = pair.first;
      } else  {
        ImGui::SetCurrentContext(pair.second.context);
      }

      ImGuiIO& io = ImGui::GetIO();
      if(mMousePositions.count(pair.first) != 0) {
        io.MousePos.x = mMousePositions[pair.first].x;
        io.MousePos.y = mMousePositions[pair.first].y;
      }

      mFrameEnded = false;

      // Setup display size (every frame to accommodate for window resizing)
      io.DisplaySize = ImVec2(pair.second.size.x, pair.second.size.y);

      // Start the frame
      ImGui::NewFrame();
      setImguiContext(pair.second.context);
      mManager->renderViews(pair.second);
      ImGui::EndFrame();
      ImGui::Render();
    }
    mContextLock.unlock();
  }

  ImguiManager::ImguiManager()
    : mRenderer(0)
    , mIsSetUp(false)
    , mFontAtlas(NULL)
  {
  }

  const std::string ImguiManager::TYPE;

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
    // mouse events
    addEventListener(mEngine, MouseEvent::MOUSE_DOWN, &ImguiManager::handleMouseEvent, -100);
    addEventListener(mEngine, MouseEvent::MOUSE_UP, &ImguiManager::handleMouseEvent, -100);
    addEventListener(mEngine, MouseEvent::MOUSE_MOVE, &ImguiManager::handleMouseEvent, -100);
    // keyboard events
    addEventListener(mEngine, KeyboardEvent::KEY_DOWN, &ImguiManager::handleKeyboardEvent, -100);
    addEventListener(mEngine, KeyboardEvent::KEY_UP, &ImguiManager::handleKeyboardEvent, -100);
    // input event
    addEventListener(mEngine, TextInputEvent::INPUT, &ImguiManager::handleInputEvent, -100);
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
      tearDown();
      LOG(INFO) << "Render system was removed, system wrapper was destroyed";
    }
    return true;
  }

  void ImguiManager::setUp()
  {
    if(mIsSetUp)
      return;

    EngineSystem* render = mEngine->getSystem("render");
    if(render == 0 || !render->isReady()) {
      return;
    }

    const std::string type = render->getSystemInfo().get("type", "");

    if(type.empty()) {
      LOG(ERROR) << "Render system type is empty, can't select appropriate renderer for it";
      return;
    }

    if(mRendererFactories.count(type) > 0)
    {
      if(mRenderer) {
        delete mRenderer;
      }

      LOG(INFO) << "Initialize for render system " << type;
      mRenderer = mRendererFactories[type]();
    } else {
      LOG(INFO) << "Can't initialize on render system " << type << ", waiting for interface to be registered";
      mPendingSystemType = type;
      return;
    }

    mRenderer->mManager = this;
    mRenderer->initialize(mEngine, mLuaState);
    setLuaState(mLuaState);

    mIsSetUp = true;

    // subscribe for engine updates to handle UI
    addEventListener(mEngine, EngineEvent::UPDATE, &ImguiManager::render);
  }

  void ImguiManager::tearDown()
  {
    removeEventListener(mEngine, EngineEvent::UPDATE, &ImguiManager::render);
    mIsSetUp = false;
    delete mRenderer;
    mRenderer = 0;
    mUsedRendererType = "";
    for(auto pair : mContexts) {
      ImGui::SetCurrentContext(pair.second);
      ImGui::DestroyContext();
    }
    mContexts.clear();
    mFontAtlas = NULL;
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
    lua["ImGuiDock_NoTitleBar"] = ImGuiDock_NoTitleBar;
    lua["ImGuiDock_NoResize"] = ImGuiDock_NoResize;

    lua["imgui"]["BeginDock"] = [this](std::string label, int flags) -> bool {
      return mRenderer->getDockspace(ImGui::GetCurrentContext())->begin(label.c_str(), NULL, flags);
    };

    lua["imgui"]["BeginDockOpen"] = [this](std::string label, bool opened, int flags) -> std::tuple<bool, bool> {
      bool active = mRenderer->getDockspace(ImGui::GetCurrentContext())->begin(label.c_str(), &opened, flags);
      return std::make_tuple(active, opened);
    };

    lua["imgui"]["BeginDockTitleOpen"] = [this](std::string label, std::string title, bool opened, int flags, int dockFlags) -> std::tuple<bool, bool> {
      bool active = mRenderer->getDockspace(ImGui::GetCurrentContext())->begin(label.c_str(), title.c_str(), &opened, flags, dockFlags);
      return std::make_tuple(active, opened);
    };

    lua["imgui"]["EndDock"] = [this]() {
      return mRenderer->getDockspace(ImGui::GetCurrentContext())->end();
    };

    lua["imgui"]["GetDockState"] = [this] () -> sol::table {
      if(mRenderer) {
        mRenderer->getDockState(mDockspaceStates);
      }

      sol::state_view lua(mLuaState);
      sol::table t = lua.create_table();
      mDockspaceStates.dump(t);
      return t;
    };

    lua["imgui"]["SetDockState"] = [this] (sol::table t) {
      DataProxy dp = DataProxy::wrap(t);
      dp.dump(mDockspaceStates);
      if(mRenderer) {
        mRenderer->setDockState(dp);
      }
    };
  }

  bool ImguiManager::handleMouseEvent(EventDispatcher* sender, const Event& event)
  {
    if(!mRenderer || mContexts.size() == 0)
      return true;

    const MouseEvent& e = static_cast<const MouseEvent&>(event);
    mRenderer->setMousePosition(e.dispatcher, ImVec2(e.mouseX, e.mouseY));
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel += e.relativeZ / 100;

    io.MouseDown[e.button] = e.getType() == MouseEvent::MOUSE_DOWN;
    return !doCapture() || e.getType() == MouseEvent::MOUSE_UP;
  }

  const std::string& ImguiManager::getType()
  {
    return ImguiManager::TYPE;
  }

  void ImguiManager::addRendererFactory(const std::string& type, RendererFactory f)
  {
    if(mRendererFactories.count(type) != 0) {
      LOG(ERROR) << "Renderable factory of type " << type << " already exists. Remove conflicting interfaces plugins";
      return;
    }

    LOG(INFO) << "Render interface for render system " << type << " was installed";
    mRendererFactories[type] = f;
    if(mPendingSystemType == type) {
      setUp();
      mPendingSystemType = "";
      mUsedRendererType = type;
    }
  }

  void ImguiManager::removeRendererFactory(const std::string& type)
  {
    mRendererFactories.erase(type);
    if(mUsedRendererType == type && mRenderer) {
      tearDown();
      LOG(INFO) << "Renderer plugin \"" << type <<  "\" was removed, renderer was destroyed";
    }
  }

  bool ImguiManager::handleKeyboardEvent(EventDispatcher* sender, const Event& event)
  {
    if(!mRenderer || mContexts.size() == 0)
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
    io.KeySuper = e.isModifierDown(KeyboardEvent::Win);

    return !ImGui::IsAnyItemActive() || e.getType() == KeyboardEvent::KEY_UP;
  }

  bool ImguiManager::handleInputEvent(EventDispatcher* sender, const Event& event)
  {
    if(!mRenderer || mContexts.size() == 0)
      return true;

    const TextInputEvent& e = static_cast<const TextInputEvent&>(event);

    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharactersUTF8(e.getText());

    return true;
  }

  bool ImguiManager::doCapture()
  {
    return ImGui::GetIO().WantCaptureMouse;
  }

  bool ImguiManager::render(EventDispatcher* dispatcher, const Event& e)
  {
    mRenderer->render();
    return true;
  }

  void ImguiRenderer::getDockState(DataProxy& dest)
  {
    for(auto& pair : mContexts) {
      dest.put(pair.first, dumpState(pair.second.dockspace->getState()));
    }
  }

  void ImguiRenderer::setDockState(const DataProxy& state)
  {
    for(auto pair : state) {
      if(mContexts.count(pair.first) > 0) {
        ImGuiDockspaceState s = loadState(pair.second);
        mContexts[pair.first].dockspace->setState(s);
      }
    }
  }

  ImGuiContext* ImguiManager::getImGuiContext(std::string name, const ImVec2& initialSize)
  {
    if(mContexts.count(name) != 0) {
      return mContexts[name];
    }

    LOG(INFO) << "Creating ImGui context " << name;
    ImGuiContext* ctx = ImGui::CreateContext(mFontAtlas);
    ImGui::SetCurrentContext(ctx);

    DataProxy theme = DataProxy::create(DataWrapper::JSON_OBJECT);
    if(!mEngine->settings().read(std::string("imgui.theme.") + name, theme)) {
      mEngine->settings().read("imgui.theme", theme);
    }

    ImGuiStyle& style = ImGui::GetStyle();

    DataProxy colors = theme.get("colors", DataProxy::create(DataWrapper::JSON_OBJECT));

    style.Colors[ImGuiCol_Text]                  = colors.get("text", ImVec4(0.73f, 0.73f, 0.73f, 1.00f));
    style.Colors[ImGuiCol_TextDisabled]          = colors.get("textDisabled", ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
    style.Colors[ImGuiCol_WindowBg]              = colors.get("windowBg", ImVec4(0.26f, 0.26f, 0.26f, 0.95f));
    style.Colors[ImGuiCol_ChildWindowBg]         = colors.get("childWindowBg", ImVec4(0.28f, 0.28f, 0.28f, 1.00f));
    style.Colors[ImGuiCol_PopupBg]               = colors.get("popupBg", ImVec4(0.26f, 0.26f, 0.26f, 1.00f));
    style.Colors[ImGuiCol_Border]                = colors.get("border", ImVec4(0.11f, 0.11f, 0.11f, 1.00f));
    style.Colors[ImGuiCol_BorderShadow]          = colors.get("borderShadow", ImVec4(0.11f, 0.11f, 0.11f, 1.00f));
    style.Colors[ImGuiCol_FrameBg]               = colors.get("frameBg", ImVec4(0.16f, 0.16f, 0.16f, 1.00f));
    style.Colors[ImGuiCol_FrameBgHovered]        = colors.get("frameBgHovered", ImVec4(0.16f, 0.16f, 0.16f, 1.00f));
    style.Colors[ImGuiCol_FrameBgActive]         = colors.get("frameBgActive", ImVec4(0.16f, 0.16f, 0.16f, 1.00f));
    style.Colors[ImGuiCol_TitleBg]               = colors.get("titleBg", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_TitleBgCollapsed]      = colors.get("titleBgCollapsed", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_TitleBgActive]         = colors.get("titleBgActive", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_MenuBarBg]             = colors.get("menuBarBg", ImVec4(0.26f, 0.26f, 0.26f, 1.00f));
    style.Colors[ImGuiCol_ScrollbarBg]           = colors.get("scrollbarBg", ImVec4(0.21f, 0.21f, 0.21f, 1.00f));
    style.Colors[ImGuiCol_ScrollbarGrab]         = colors.get("scrollbarGrab", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = colors.get("scrollbarGrabHovered", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = colors.get("scrollbarGrabActive", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_CheckMark]             = colors.get("checkMark", ImVec4(0.78f, 0.78f, 0.78f, 1.00f));
    style.Colors[ImGuiCol_SliderGrab]            = colors.get("sliderGrab", ImVec4(0.74f, 0.74f, 0.74f, 1.00f));
    style.Colors[ImGuiCol_SliderGrabActive]      = colors.get("sliderGrabActive", ImVec4(0.74f, 0.74f, 0.74f, 1.00f));
    style.Colors[ImGuiCol_Button]                = colors.get("button", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_ButtonHovered]         = colors.get("buttonHovered", ImVec4(0.43f, 0.43f, 0.43f, 0.90f));
    style.Colors[ImGuiCol_ButtonActive]          = colors.get("buttonActive", ImVec4(0.11f, 0.11f, 0.11f, 1.00f));
    style.Colors[ImGuiCol_Header]                = colors.get("header", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_HeaderHovered]         = colors.get("headerHovered", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_HeaderActive]          = colors.get("headerActive", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_Column]                = colors.get("column", ImVec4(0.39f, 0.39f, 0.39f, 1.00f));
    style.Colors[ImGuiCol_ColumnHovered]         = colors.get("columnHovered", ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    style.Colors[ImGuiCol_ColumnActive]          = colors.get("columnActive", ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    style.Colors[ImGuiCol_ResizeGrip]            = colors.get("resizeGrip", ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    style.Colors[ImGuiCol_ResizeGripHovered]     = colors.get("resizeGripHovered", ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    style.Colors[ImGuiCol_ResizeGripActive]      = colors.get("resizeGripActive", ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    style.Colors[ImGuiCol_PlotLines]             = colors.get("plotLines", ImVec4(0.39f, 0.39f, 0.39f, 1.00f));
    style.Colors[ImGuiCol_PlotLinesHovered]      = colors.get("plotLinesHovered", ImVec4(1.00f, 0.43f, 0.35f, 1.00f));
    style.Colors[ImGuiCol_PlotHistogram]         = colors.get("plotHistogram", ImVec4(0.90f, 0.70f, 0.00f, 1.00f));
    style.Colors[ImGuiCol_PlotHistogramHovered]  = colors.get("plotHistogramHovered", ImVec4(1.00f, 0.60f, 0.00f, 1.00f));
    style.Colors[ImGuiCol_TextSelectedBg]        = colors.get("textSelectedBg", ImVec4(0.32f, 0.52f, 0.65f, 1.00f));
    style.Colors[ImGuiCol_ModalWindowDarkening]  = colors.get("modalWindowDarkening", ImVec4(0.20f, 0.20f, 0.20f, 0.50f));
    style.Colors[ImGuiCol_NavHighlight]          = colors.get("navHighlight", ImVec4(1.00f, 1.00f, 1.00f, 1.00f));

    style.WindowTitleAlign      = theme.get("windowTitleAlign", ImVec2(0.5f,0.5f));
    style.WindowPadding         = theme.get("windowPadding", ImVec2(15, 15));
    style.WindowRounding        = theme.get("windowRounding", 5.0f);
    style.ChildRounding         = theme.get("childRounding", 5.0f);
    style.FramePadding          = theme.get("framePadding", ImVec2(5.0f, 5.0f));
    style.FrameBorderSize       = theme.get("frameBorderSize", 0.0f);
    style.ButtonTextAlign       = theme.get("buttonTextAlign", ImVec2(0.5f,0.3f));
    style.FrameRounding         = theme.get("frameRounding", 4.0f);
    style.ItemSpacing           = theme.get("itemSpacing", ImVec2(12.0f, 8.0f));
    style.ItemInnerSpacing      = theme.get("itemInnerSpacing", ImVec2(8.0f, 6.0f));
    style.IndentSpacing         = theme.get("indentSpacing", 25.0f);
    style.ScrollbarSize         = theme.get("scrollbarSize", 15.0f);
    style.ScrollbarRounding     = theme.get("scrollbarRounding", 10.0f);
    style.GrabMinSize           = theme.get("grabMinSize", 20.0f);
    style.GrabRounding          = theme.get("grabRounding", 3.0f);
    style.CurveTessellationTol  = theme.get("curveTessellationTol", 1.25f);

    std::string workdir = mEngine->env().get("workdir", ".");
    auto additionalFonts = mEngine->settings().get<DataProxy>("imgui.fonts");

    ImGuiIO& io = ImGui::GetIO();
    if(!mFontAtlas) {
      // Build texture atlas
      unsigned char* pixels;
      int width, height;
      if(additionalFonts.second) {
        int i = 0;
        mGlyphRanges.clear();
        mGlyphRanges.reserve(additionalFonts.first.size());
        for(auto pair : additionalFonts.first) {
          mGlyphRanges.emplace_back();
          auto file = pair.second.get<std::string>("file");
          if(!file.second) {
            LOG(ERROR) << "Malformed font syntax, \'file\' field is mandatory";
            continue;
          }

          auto size = pair.second.get<float>("size");
          if(!size.second) {
            LOG(ERROR) << "Malformed font syntax, \'size\' field is mandatory";
            continue;
          }

          ImFontConfig config;
          config.MergeMode = pair.second.get("merge", false);
          pair.second.read("oversampleH", config.OversampleH);
          pair.second.read("oversampleV", config.OversampleV);

          ImFontAtlas::GlyphRangesBuilder builder;
          auto glyphRanges = pair.second.get<DataProxy>("glyphRanges");
          if(glyphRanges.second) {
            for(auto range : glyphRanges.first) {
              for(int j = range.second[0].as<int>(); j < range.second[1].as<int>(); j++) {
                builder.AddChar((ImWchar)j);
              }
            }
          }

          if(!pair.second.get("noDefaultRanges", false)) {
            builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
            builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
            builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
          }

          auto glyphOffset = pair.second.get<ImVec2>("glyphOffset");
          if(glyphOffset.second) {
            config.GlyphOffset = glyphOffset.first;
          }
          builder.BuildRanges(&mGlyphRanges[i]);
          config.PixelSnapH = pair.second.get("pixelSnapH", false);

          ImFont* pFont = io.Fonts->AddFontFromFileTTF((workdir + GSAGE_PATH_SEPARATOR + file.first).c_str(), size.first, &config, mGlyphRanges[i].Data);
          mFonts.push_back(pFont);
          i++;
        }
      }

      io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
      mRenderer->createFontTexture(pixels, width, height);
      mFontAtlas = io.Fonts;
    }

    io.MouseDrawCursor = mEngine->settings().get("imgui.drawCursor", true);

    io.DisplaySize = initialSize;
    ImGui::NewFrame();
    ImGui::EndFrame();

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
    io.KeyMap[ImGuiKey_Space] = KeyboardEvent::KC_SPACE;
    mContexts[name] = ctx;

    mEngine->fireEvent(ImguiEvent(ImguiEvent::CONTEXT_CREATED, name));
    return ctx;
  }

  bool ImguiManager::addView(const std::string& name, sol::object view, bool docked)
  {
    sol::function callback;
    sol::table t = view.as<sol::table>();
    sol::optional<sol::function> call = t[sol::meta_function::call];
    RenderView render;

    if (call) {
      render = [t, call] () -> sol::function_result { return call.value()(t); };
      docked = t.get_or("docked", docked);
    } else if(view.is<sol::function>()) {
      callback = view.as<sol::function>();
      render = [callback] () -> sol::function_result { return callback(); };
    } else {
      LOG(ERROR) << "Failed to add lua object as lua view " << name << ": must be either callable or function";
      return false;
    }

    if(docked){
      mDockedViews[name] = render;
    } else {
      mViews[name] = render;
    }

    return true;
  }

  bool ImguiManager::removeView(const std::string& name, sol::object view)
  {
    bool docked = false;
    auto views = &mViews;
    if(view != sol::lua_nil) {
      sol::table t = view.as<sol::table>();
      if(t.get_or("docked", docked)) {
        views = &mDockedViews;
      }
    }

    if(views->count(name) == 0) {
      return false;
    }

    views->erase(name);
    return true;
  }

  void ImguiManager::renderViews(ImguiRenderer::Context& ctx)
  {
    ImGuiIO& io = ImGui::GetIO();
    if(mDockedViews.size() > 0) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));
      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetColorU32(ImGuiCol_Border));
      ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
      ImVec2 size = ctx.size;

      ImGui::SetNextWindowSize(size);
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::Begin(
          "###dockspace",
          NULL,
          ImGuiWindowFlags_NoTitleBar |
          ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoScrollWithMouse |
          ImGuiWindowFlags_NoBringToFrontOnFocus |
          ImGuiWindowFlags_NoSavedSettings
      );
      ImGui::PopStyleVar();
      if(ctx.dockspace) {
        ctx.dockspace->beginWorkspace(ImVec2(5, 35), size - ImVec2(10, 40));
      }
      ImGui::PopStyleColor(2);
      for(auto pair : mDockedViews) {
        try {
          auto res = pair.second();
          if(!res.valid()) {
            sol::error e = res;
            LOG(ERROR) << "Failed to render view " << pair.first << ": " << e.what();
          }
        } catch(sol::error e) {
          LOG(ERROR) << "Exception in view " << pair.first << ": " << e.what();
        }
      }
      if(ctx.dockspace) {
        ctx.dockspace->endWorkspace();
      }
      ImGui::End();
    }

    for(auto pair : mViews) {
      try {
        auto res = pair.second();
        if(!res.valid()) {
          sol::error e = res;
          LOG(ERROR) << "Failed to render view " << pair.first << ": " << e.what();
        }
      } catch(sol::error e) {
        LOG(ERROR) << "Exception in view " << pair.first << ": " << e.what();
      }
    }
  }

  ImGuiDockspaceState ImguiManager::getDockState(const std::string& name)
  {
    ImGuiDockspaceState s;
    DataProxy dp;
    mDockspaceStates.read(name, dp);
    return loadState(dp);
  }
}
