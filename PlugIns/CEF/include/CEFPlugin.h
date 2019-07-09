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
    private:
      TexturePtr mTexture;
    public:
      IMPLEMENT_REFCOUNTING(RenderHandler);
  };

  class Webview;

  class BrowserClient : public CefClient, public CefLoadHandler, public CefDialogHandler, public CefContextMenuHandler
  {
    public:
      BrowserClient(RenderHandler* renderHandler, CEFPlugin* cef, Webview* webview);

      virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE;

      CefRefPtr<CefDialogHandler> GetDialogHandler() OVERRIDE {
        return this;
      }

      // Allow overwrite of context menu functions
      virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE {
        return this;
      }

      void OnBeforeContextMenu( CefRefPtr< CefBrowser > browser, CefRefPtr< CefFrame > frame, CefRefPtr< CefContextMenuParams > params, CefRefPtr< CefMenuModel > model ) OVERRIDE;

      bool OnContextMenuCommand( CefRefPtr< CefBrowser > browser, CefRefPtr< CefFrame > frame, CefRefPtr< CefContextMenuParams > params, int command_id, CefContextMenuHandler::EventFlags event_flags ) OVERRIDE;

      bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message) OVERRIDE;

      /**
       * Sets next page template data
       *
       * @param data lua table or json object to use for template rendering
       */
      inline void setPageData(const DataProxy& data) {
        std::lock_guard<std::mutex> lock(mPageDataLock);
        mPageData = data;
      }

      /**
       * Returns next page template data
       */
      inline const DataProxy& getPageData() {
        std::lock_guard<std::mutex> lock(mPageDataLock);
        return mPageData;
      }

      inline CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; };

      void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) OVERRIDE;

      void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::TransitionType transitionType) OVERRIDE;

      bool OnFileDialog(CefRefPtr<CefBrowser> browser, CefDialogHandler::FileDialogMode mode, const CefString& title, const CefString& defaultFilePath, const std::vector<CefString>& acceptFilters, int selectedAcceptFilter, CefRefPtr<CefFileDialogCallback> callback) OVERRIDE;
    private:
      CefRefPtr<CefRenderHandler> mRenderHandler;
      CEFPlugin* mCef;
      Webview* mWebview;
      DataProxy mPageData;
      std::mutex mPageDataLock;

      IMPLEMENT_REFCOUNTING(BrowserClient);
  };


  /**
   * Single CEF page
   */
  class Webview : public EventSubscriber<Webview>, public EventDispatcher
  {
    public:
      static const Event::Type PAGE_LOADED;

      static const Event::Type PAGE_LOADING;

      static const Event::Type MOUSE_LEAVE;

      static const Event::Type UNDO;

      static const Event::Type REDO;

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
        if(mBrowser.get()) {
          mBrowser->GetHost()->CloseBrowser(false);
        }
        mBrowser = nullptr;
        mClient = nullptr;
        mRenderHandler = nullptr;
      }

      void renderPage(const std::string& page, const DataProxy& data);

      /**
       * Events input
       */

      /**
       * Receive basic event
       * Should be called by the wrapper
       * Event is pushed to the event queue and consumed on the next Webview::processEvents call
       */
      void pushEvent(Event event);
      /**
       * Receive keyboard event
       * Should be called by the wrapper
       * Event is pushed to the event queue and consumed on the next Webview::processEvents call
       */
      void pushEvent(KeyboardEvent event);
      /**
       * Receive text input event
       * Should be called by the wrapper
       * Event is pushed to the event queue and consumed on the next Webview::processEvents call
       */
      void pushEvent(TextInputEvent event);
      /**
       * Receive text input event
       * Should be called by the wrapper
       * Event is pushed to the event queue and consumed on the next Webview::processEvents call
       */
      void pushEvent(MouseEvent event);

      /**
       * Consume all pushed events
       */
      void processEvents();

      /**
       * Queue outgoing events to be send out during the next Webview::sendEvents call
       */
      void queueEvent(Event event);

      /**
       * Send out all queued events
       */
      void sendEvents();

      void executeJavascript(const std::string& script);

      void setZoom(float factor);

      /**
       * Get window this webview rendering to
       */
      WindowPtr getWindow();
    private:
      void handleMouseEvent(const MouseEvent& event);

      void handleKeyboardEvent(const KeyboardEvent& event);

      void handleInput(const TextInputEvent& event);

      bool wasResized(EventDispatcher* sender, const Event& e);

      bool mCefWasStopped;

      CefRefPtr<BrowserClient> mClient;
      CefRefPtr<CefBrowser> mBrowser;
      CefRefPtr<RenderHandler> mRenderHandler;

      CEFPlugin* mCef;

      Events mEvents;
      MouseEvents mMouseEvents;
      KeyboardEvents mKeyboardEvents;
      TextInputEvents mTextInputEvents;
      float mZoom;

      Events mOutgoingEvents;
      unsigned long long mWindowHandle;

      std::string mPage;

      int mMouseX;
      int mMouseY;
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
        std::string result;
      };

      /**
       * RenderProcess abstract message handler
       */
      class MessageHandler {
        public:
          virtual std::string execute(CefRefPtr<CefProcessMessage> msg) = 0;
      };

      class LuaMessageHandler : public MessageHandler {
        public:
          LuaMessageHandler(sol::function callback);
          std::string execute(CefRefPtr<CefProcessMessage> msg);
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
       * Runs lambda in the render thread
       *
       * @param cb function
       */
      void runInRenderThread(CEFPlugin::Task cb);

      /**
       * Handles a message from another process
       *
       * @param msg Message
       *
       * @return json, true if succeed
       */
      std::tuple<std::string, bool> handleProcessMessage(CefRefPtr<CefProcessMessage> msg);

      /**
       * UpdateListener implementation
       */
      void update(double time);

      /**
       * Used in webview
       */
      inline GsageFacade* getFacade() { return mFacade; }

    private:
      bool initialize();
      void stop();
      void doMessageLoop();
      std::tuple<std::string, bool> handleProcessMessageSync(CefRefPtr<CefProcessMessage> msg);

      std::map<std::string, WebviewPtr> mViews;
      std::vector<char*> mArgs;
      std::thread mCefRunner;
      std::atomic<bool> mRunning;
      std::mutex mViewsLock;

      Tasks mTasks;
      Tasks mRenderThreadTasks;
      Messages mMessages;

      typedef std::map<std::string, MessageHandlerPtr> MessageHandlers;
      MessageHandlers mMessageHandlers;

      bool mAsyncMode;
  };
}

#endif
