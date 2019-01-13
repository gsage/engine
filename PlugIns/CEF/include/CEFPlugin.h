#ifndef _CEFPlugin_H_
#define _CEFPlugin_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2019 Gsage Authors

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

#include "IPlugin.h"
#include "systems/RenderSystem.h"
#include "UpdateListener.h"

#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>

#include "GsageDefinitions.h"
#include "KeyboardEvent.h"

#include <atomic>

#if !defined(OVERRIDE)

#if GSAGE_PLATFORM == GSAGE_APPLE
#define OVERRIDE override
#else
#define OVERRIDE
#endif

#endif

namespace Gsage {
  class Engine;
  class CEFPlugin;

  class RenderHandler : public CefRenderHandler
  {
    // CefRenderHandler interface
    public:
      RenderHandler(TexturePtr texture);
      virtual ~RenderHandler();
      bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE;
      void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) OVERRIDE;
      void setTexture(TexturePtr texture);
      inline void setScreenPosition(int x, int y) { mX = x; mY = y; }
    private:
      TexturePtr mTexture;
    public:
      IMPLEMENT_REFCOUNTING(RenderHandler);
      int mX;
      int mY;
  };

  class BrowserClient : public CefClient
  {
    public:
      BrowserClient(RenderHandler* renderHandler, CEFPlugin* cef);

      virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE;

      bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE;
    private:
      CefRefPtr<CefRenderHandler> mRenderHandler;
      CEFPlugin* mCef;

      IMPLEMENT_REFCOUNTING(BrowserClient);
  };


  /**
   * Single CEF page
   */
  class Webview : public EventSubscriber<Webview>
  {
    public:
      static const Event::Type MOUSE_LEAVE;

      typedef ThreadSafeQueue<Event> Events;
      typedef ThreadSafeQueue<KeyboardEvent> KeyboardEvents;
      typedef ThreadSafeQueue<MouseEvent> MouseEvents;
      typedef ThreadSafeQueue<TextInputEvent> TextInputEvents;

      Webview(CEFPlugin* cef);
      virtual ~Webview();

      /**
       * Start webview rendering
       *
       * \verbatim embed:rst:leading-asterisk
       *
       * List of possible options:
       *
       *  * :code:`page` starting page (required).
       *  * :code:`opaque` use site background color when rendering (optional). Default `true`.
       *
       * \endverbatim
       *
       * @param windowHandle Window handle to use for rendering
       * @param texture Texture to render to
       * @param options Various options, see above
       */
      bool renderToTexture(unsigned long long windowHandle, TexturePtr texture, DataProxy options);

      /**
       * Update texture only
       *
       * @param texture Texture to swap to
       */
      void updateTexture(TexturePtr texture);

      inline void setCefWasStopped() { 
        mCefWasStopped = true;
        mBrowser = nullptr;
        mClient = nullptr;
        mRenderHandler = nullptr;
      }

      void pushEvent(Event event);
      void pushEvent(KeyboardEvent event);
      void pushEvent(TextInputEvent event);
      void pushEvent(MouseEvent event);

      void processEvents();

      void handleMouseEvent(const MouseEvent& event);

      void handleKeyboardEvent(const KeyboardEvent& event);

      void handleInput(const TextInputEvent& event);

      void setScreenPosition(unsigned int x, unsigned int y) { mRenderHandler->setScreenPosition(x, y); }

      void executeJavascript(const std::string& script);

    private:
      bool wasResized(EventDispatcher* sender, const Event& e);

      bool mCefWasStopped;

      int mMouseX;
      int mMouseY;

      CefRefPtr<BrowserClient> mClient;
      CefRefPtr<CefBrowser> mBrowser;
      CefRefPtr<RenderHandler> mRenderHandler;

      CEFPlugin* mCef;

      Events mEvents;
      MouseEvents mMouseEvents;
      KeyboardEvents mKeyboardEvents;
      TextInputEvents mTextInputEvents;

      int mMouseModifiers;
  };

  typedef std::shared_ptr<Webview> WebviewPtr;

  class CEFPlugin : public IPlugin, public UpdateListener
  {
    public:
      /**
       * RenderProcess message pending callback
       */
      struct PendingMessage {
        PendingMessage(CefRefPtr<CefProcessMessage> pMsg) : msg(pMsg) {}
        SignalChannel channel;
        CefRefPtr<CefProcessMessage> msg;
      };

      /**
       * RenderProcess abstract message handler
       */
      class MessageHandler {
        public:
          virtual DataProxy execute(CefRefPtr<CefProcessMessage> msg) = 0;
      };

      class LuaMessageHandler : public MessageHandler {
        public:
          LuaMessageHandler(sol::function callback);
          DataProxy execute(CefRefPtr<CefProcessMessage> msg);
        private:
          sol::function mCallback;
      };

      typedef std::unique_ptr<MessageHandler> MessageHandlerPtr;

      typedef std::shared_ptr<PendingMessage> PendingMessagePtr;

      typedef std::function<void()> Task;
      typedef ThreadSafeQueue<Task> Tasks;
      typedef ThreadSafeQueue<PendingMessagePtr> Messages;

      CEFPlugin();
      virtual ~CEFPlugin();

      /**
       * Get plugin name
       */
      const std::string& getName() const;

      /**
       * Starts CEF
       */
      bool installImpl();

      /**
       * Stops CEF
       */
      void uninstallImpl();

      /**
       * Set up lua bindings
       */
      void setupLuaBindings();

      /**
       * Create webview
       *
       * @param name Unique webview name
       *
       * @returns created webview
       */
      WebviewPtr createWebview(const std::string& name);

      /**
       * Deletes webview
       *
       * @param name webview name
       *
       * @returns true if succeed
       */
      bool removeWebview(const std::string& name);

      /**
       * Add render process handler from lua
       *
       * @param name Handler name
       * @param callback Lua callback
       */
      void addLuaMessagehandler(const std::string& name, sol::function callback);

      /**
       * Runs lambda in the main thread
       *
       * @param cb function
       */
      void runInMainThread(CEFPlugin::Task cb);

      /**
       * Handles a message from another process
       *
       * @param msg Message
       *
       * @return true if succeed
       */
      bool handleProcessMessage(CefRefPtr<CefProcessMessage> msg);

      /**
       * UpdateListener implementation
       */
      void update(double time);

    private:
      std::map<std::string, WebviewPtr> mViews;
      std::thread mCefRunner;
      std::atomic<bool> mRunning;
      std::mutex mViewsLock;

      Tasks mTasks;
      Messages mMessages;

      typedef std::map<std::string, MessageHandlerPtr> MessageHandlers;
      MessageHandlers mMessageHandlers;

  };
}

#endif
