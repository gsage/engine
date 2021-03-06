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

#ifndef _ImguiManager_H_
#define _ImguiManager_H_

#include "UIManager.h"
#include "EventSubscriber.h"
#include "KeyboardEvent.h"
#include "ImGuiDockspace.h"
#include "ImguiExportHeader.h"
#include "DataProxy.h"

#include "sol_forward.hpp"

struct lua_State;

namespace Gsage {
  class LuaInterface;
  class RenderEvent;
  class Engine;
  class ImguiManager;

  class ImguiViewCollection {
    public:
      /**
       * Add new lua view to the imgui renderer.
       *
       * @param name view name
       * @param view lua function that will render everything.
       */
      virtual bool addView(const std::string& name, sol::object view);

      /**
       * Remove view from collection
       */
      virtual bool removeView(const std::string& name, sol::object view);
    protected:
      typedef std::function<sol::function_result()> RenderView;

      typedef std::map<std::string, RenderView> Views;

      Views mViews;
  };

  /**
   * Renders window collection into workspace
   */
  class ImguiDockspaceView : public ImguiViewCollection {
    public:
      ImguiDockspaceView(ImguiManager* manager, const std::string& name);
      /**
       * Render views
       * @param x dockspace x
       * @param y dockspace y
       * @param width dockspace width
       * @param height dockspace height
       */
      void render(int x, int y, int width, int height);

      /**
       * Get dockspace state
       */
      sol::table getState();

      /**
       * Set dockspace state
       *
       * @param t sol::table to read state data from
       */
      void setState(sol::table t);

      /**
       * Activate dock
       *
       * @param name Dock name
       * @return true if dock was found
       */
      bool activateDock(const std::string& name);
      /**
       * Add new lua view to the imgui renderer.
       *
       * @param name view name
       * @param view lua function that will render everything.
       */
      virtual bool addView(const std::string& name, sol::object view);

      /**
       * Remove view from dockspace
       */
      virtual bool removeView(const std::string& name, sol::object view);
    private:
      ImguiManager* mManager;
      DataProxy mState;
      std::unique_ptr<ImGuiDockspaceRenderer> mDockspace;
      std::string mName;
  };

  class ImguiRenderer {

    public:

      ImguiRenderer() : mEngine(0), mManager(0) {}
      virtual ~ImguiRenderer() { mContexts.clear(); }

      /**
       * Set engine, setup lua bindings
       *
       * @param engine Gsage CE
       * @param L lua state
       */
      virtual void initialize(Engine* facade, lua_State* L);

      /**
       * Update mouse position for the render target
       *
       * @param name render target name
       * @param position mouse position
       */
      void setMousePosition(const std::string& name, ImVec2 position);

      /**
       * Create font texture in the underlying render system
       *
       * @param pixels Raw texture
       * @param width Texture width
       * @param height Texture height
       */
      virtual void createFontTexture(unsigned char* pixels, int width, int height) = 0;

      /**
       * Imgui context is not shared across plugins, so pass it to renderer
       *
       * @param ctx Context to use
       */
      virtual void setImguiContext(ImGuiContext* ctx) = 0;

      void render();

    protected:
      friend class ImguiManager;

      struct Context
      {
        ImVec2 size;
        ImGuiContext* context;
      };

      virtual Context* getContext(ImGuiContext* context);

      virtual Context* initializeContext(const std::string& name);

      std::map<std::string, ImVec2> mMousePositions;

      Engine* mEngine;

      std::map<std::string, bool> mRenderTargetWhitelist;

      std::mutex mContextLock;

      std::map<std::string, Context> mContexts;
      std::map<ImGuiContext*, std::string> mContextNames;

      ImguiManager* mManager;
  };

  class IMGUI_PLUGIN_API ImguiManager : public UIManager, public EventSubscriber<ImguiManager>, public ImguiViewCollection
  {
    public:
      typedef std::function<ImguiRenderer*()> RendererFactory;

      static const std::string TYPE;

      ImguiManager();
      virtual ~ImguiManager();
      /**
       * Initialize ui manager
       *
       * @param called by gsage facade on setup
       * @param facade Gsage Facade
       * @param L init with lua state
       */
      virtual void initialize(GsageFacade* facade, lua_State* L = 0);
      /**
       * Gets Rocket lua state
       */
      lua_State* getLuaState();

      /**
       * Update load state
       * @param L lua_State
       */
      void setLuaState(lua_State* L);

      /**
       * Configures rendering
       */
      void setUp();

      /**
       * Tear down imgui manager
       */
      void tearDown();

      /**
       * SystemChangeEvent::SYSTEM_ADDED and SystemChangeEvent::SYSTEM_REMOVED handler
       */
      bool handleSystemChange(EventDispatcher* sender, const Event& event);
      /**
       * Handle mouse event from engine
       *
       * @param sender EventDispatcher
       * @param event Event
       */
      bool handleMouseEvent(EventDispatcher* sender, const Event& event);

      const std::string& getType();

      /**
       * This method is called by render system interfaces plugins
       *
       * @param type Type of render system to use for. RenderSystem must properly return "type" field from getSystemInfo call
       * @param f Factory method to create renderables
       */
      void addRendererFactory(const std::string& type, RendererFactory f);

      /**
       * Unregister renderer factory
       */
      void removeRendererFactory(const std::string& type);

      /**
       * Get or create new ImGuiContext instance
       *
       * @param name Context name
       * @param initialSize Initial context size
       */
      ImGuiContext* getImGuiContext(std::string name, const ImVec2& initialSize);

      /**
       * Render a single imgui frame
       */
      virtual void renderViews(ImguiRenderer::Context& ctx);
    private:
      friend class ImguiDockspaceView;
      /**
       * Handle keyboard event from engine
       */
      bool handleKeyboardEvent(EventDispatcher* sender, const Event& event);
      /**
       * Handle input event from engine
       */
      bool handleInputEvent(EventDispatcher* sender, const Event& event);
      /**
       * Check if mouse event can be captured by any rocket element
       */
      bool doCapture();
      /**
       * This function does not handle actual render system rendering, it only updates ImGUI draw list
       */
      bool render(EventDispatcher* dispatcher, const Event& e);

      ImguiRenderer* mRenderer;
      ImFontAtlas* mFontAtlas;

      bool mIsSetUp;

      typedef std::map<std::string, RendererFactory> RendererFactories;

      RendererFactories mRendererFactories;

      std::string mPendingSystemType;
      std::string mUsedRendererType;

      ImVector<ImFont*> mFonts;

      typedef std::vector<ImVector<ImWchar>> GlyphRanges;

      GlyphRanges mGlyphRanges;

      std::map<std::string, ImGuiContext*> mContexts;
      ImGuiDockspaceRenderer* mCurrentDockspace;
  };
}
#endif
