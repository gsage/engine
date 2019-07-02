/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2019 Artem Chernyshev

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

#include "CEFPlugin.h"
#include "Engine.h"
#include "GsageFacade.h"
#include "MouseEvent.h"

#if GSAGE_PLATFORM == GSAGE_APPLE
#define WindowHandle NSView*
#include "OSX/Utils.h"
#elif GSAGE_PLATFORM == GSAGE_LINUX
#define WindowHandle unsigned long long
#include <libgen.h>
#elif GSAGE_PLATFORM == GSAGE_WIN32
#define WindowHandle HWND
#endif

#include <string>
#include <locale>
#include <codecvt>

#include <include/wrapper/cef_helpers.h>
#include <include/wrapper/cef_stream_resource_handler.h>
#include <include/cef_parser.h>

namespace Gsage {
  inline bool endsWith(std::string const & value, std::string const & ending)
  {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
  }

  class ClientSchemeHandlerFactory : public CefSchemeHandlerFactory {
    public:
      ClientSchemeHandlerFactory(GsageFacade* facade)
        : mFacade(facade)
      {
        mWebRoot =  mFacade->getEngine()->settings().get("cef.webRoot", "webview");
        mContentTypes[".js"] = "application/javascript";
        mContentTypes[".css"] = "text/css";
        mContentTypes[".html"] = "text/html";
        mContentTypes[".png"] = "image/png";
        mContentTypes[".jpeg"] = "image/jpeg";
        mContentTypes[".jpg"] = "image/jpeg";
        mContentTypes[".gif"] = "image/gif";
      }

      virtual CefRefPtr<CefResourceHandler> Create(
          CefRefPtr<CefBrowser> browser,
          CefRefPtr<CefFrame> frame,
          const CefString& scheme_name,
          CefRefPtr<CefRequest> request) OVERRIDE
      {
        CEF_REQUIRE_IO_THREAD();
        CefURLParts urlParts;
        CefParseURL(request->GetURL(), urlParts);

        std::string content;
        std::string contentType = "application/octet-stream";

        std::string filepath = mWebRoot + CefString(&urlParts.path).ToString();
        LOG(INFO) << "Loading file " << filepath << " " << CefString(&urlParts.path).ToString();
        volatile bool success = false;
        if(endsWith(filepath, ".template")) {
          CefClient* client = browser->GetHost()->GetClient().get();
          DataProxy pageData;
          if(client) {
            pageData = static_cast<BrowserClient*>(client)->getPageData();
          }
          // Template must be loaded in the main thread as template rendering can call Lua callbacks
          mFacade->getEngine()->executeInMainThread([&](){
              success = FileLoader::getSingletonPtr()->loadTemplate(filepath, content, pageData);
          })->wait();
          contentType = "text/html";
        } else {
          success = FileLoader::getSingletonPtr()->load(filepath, content, std::ios_base::binary);
        }

        if(!success || content.empty()) {
          LOG(WARNING) << "Nothing to load for " << filepath;
          return nullptr;
        }

        for(auto pair : mContentTypes) {
          if(endsWith(filepath, pair.first)) {
            contentType = pair.second;
          }
        }

        // Create a stream reader for |content|.
        CefRefPtr<CefStreamReader> stream =
          CefStreamReader::CreateForData(
              static_cast<void*>(const_cast<char*>(content.c_str())),
              content.size());

        // Constructor for HTTP status code 200 and no custom response headers.
        // There’s also a version of the constructor for custom status code and response headers.
        return new CefStreamResourceHandler(contentType.c_str(), stream);
      }

      IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
    private:
      GsageFacade* mFacade;
      std::string mWebRoot;
      std::map<std::string, std::string> mContentTypes;
  };



  RenderHandler::RenderHandler(TexturePtr texture)
    : mTexture(texture)
  {
  }

  RenderHandler::~RenderHandler()
  {
  }

  bool RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
  {
    unsigned int width, height;
    std::tie(width, height) = mTexture->getSize();
    rect = CefRect(0, 0, width, height);
    return true;
  }

  void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
  {
    mTexture->update(buffer, width * height * 4, width, height);
  }

  void RenderHandler::setTexture(TexturePtr texture)
  {
    mTexture = texture;
  }

  BrowserClient::BrowserClient(RenderHandler* renderHandler, CEFPlugin* cef, Webview* webview)
    : mRenderHandler(renderHandler)
    , mCef(cef)
    , mWebview(webview)
  {
  }

  CefRefPtr<CefRenderHandler> BrowserClient::GetRenderHandler()
  {
    return mRenderHandler;
  }

  void BrowserClient::OnBeforeContextMenu(
   CefRefPtr<CefBrowser> browser,
   CefRefPtr<CefFrame> frame,
   CefRefPtr<CefContextMenuParams> params,
   CefRefPtr<CefMenuModel> model) {
   CEF_REQUIRE_UI_THREAD();

   model->Clear();
  }

  bool BrowserClient::OnContextMenuCommand(
     CefRefPtr<CefBrowser> browser,
     CefRefPtr<CefFrame> frame,
     CefRefPtr<CefContextMenuParams> params,
     int command_id,
     EventFlags event_flags) {
     CEF_REQUIRE_UI_THREAD();
     return false;
  }

  bool BrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> msg)
  {
    std::string json;
    bool success;
    std::tie(json, success) = mCef->handleProcessMessage(msg->Copy());

    if(msg->GetName() == "fn") {
      CefRefPtr<CefProcessMessage> response = CefProcessMessage::Create("JSResponse");
      CefRefPtr<CefListValue> args = response->GetArgumentList();
      args->SetInt(0, msg->GetArgumentList()->GetInt(0));
      args->SetString(1, json);
      browser->SendProcessMessage(sourceProcess, response);
    }
    return success;
  }

  void BrowserClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
  {
    if(httpStatusCode < 400) {
      mWebview->queueEvent(Event(Webview::PAGE_LOADED));
      browser->GetHost()->SetFocus(true);
      browser->GetHost()->SendFocusEvent(true);
    }
  }

  void BrowserClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::TransitionType transitionType)
  {
    mWebview->queueEvent(Event(Webview::PAGE_LOADING));
  }

  bool BrowserClient::OnFileDialog(CefRefPtr<CefBrowser> browser, CefDialogHandler::FileDialogMode mode, const CefString& title, const CefString& defaultFilePath, const std::vector<CefString>& acceptFilters, int selectedAcceptFilter, CefRefPtr<CefFileDialogCallback> callback)
  {
    std::shared_ptr<SignalChannel> channel = std::make_shared<SignalChannel>();
    mCef->runInRenderThread([&] () {
      auto c = channel;
      Window::DialogMode dialogMode;
      switch(mode & FILE_DIALOG_TYPE_MASK) {
        case FILE_DIALOG_OPEN:
          dialogMode = Window::DialogMode::FILE_DIALOG_OPEN;
          break;
        case FILE_DIALOG_OPEN_MULTIPLE:
          dialogMode = Window::DialogMode::FILE_DIALOG_OPEN_MULTIPLE;
          break;
        case FILE_DIALOG_OPEN_FOLDER:
          dialogMode = Window::DialogMode::FILE_DIALOG_OPEN_FOLDER;
          break;
        case FILE_DIALOG_SAVE:
          dialogMode = Window::DialogMode::FILE_DIALOG_SAVE;
          break;
        default:
          LOG(ERROR) << "Failed to handle dialog mode " << (mode & FILE_DIALOG_TYPE_MASK);
          c->send(ChannelSignal::FAILURE);
          return;
      }

      std::vector<std::string> filters;
      for(auto& s : acceptFilters) {
        filters.push_back(s.ToString());
      }

      WindowManagerPtr windowManager = mCef->getFacade()->getWindowManager();
      std::string err;
      std::vector<std::string> files;

      Window::DialogStatus status;

      std::tie(files, status, err) = windowManager->openDialog(
          dialogMode,
          title.ToString(),
          defaultFilePath.ToString(),
          filters
      );
      std::vector<CefString> filePaths;
      std::vector<std::string>::const_iterator it = files.begin();
      for (; it != files.end(); ++it) {
        filePaths.push_back(CefString((*it).c_str()));
      }

      switch(status) {
        case Window::DialogStatus::OKAY:
          callback->Continue(selectedAcceptFilter, filePaths);
          break;
        case Window::DialogStatus::FAILURE:
          LOG(ERROR) << "Failed to run file dialog " << err;
        case Window::DialogStatus::CANCEL:
          callback->Cancel();
          break;
      }

      c->send(ChannelSignal::DONE);
    });

    if(channel->recv() != ChannelSignal::DONE) {
      LOG(ERROR) << "Failed to open dialog";
    }
    return true;
  }

  const Event::Type Webview::PAGE_LOADED = "Webview::PAGE_LOADED";

  const Event::Type Webview::PAGE_LOADING = "Webview::PAGE_LOADING";

  const Event::Type Webview::MOUSE_LEAVE = "Webview::MOUSE_LEAVE";

  const Event::Type Webview::UNDO = "Webview::UNDO";

  const Event::Type Webview::REDO = "Webview::REDO";

  Webview::Webview(CEFPlugin* cef)
    : mCefWasStopped(false)
    , mCef(cef)
    , mMouseModifiers(0)
    , mMouseX(0)
    , mMouseY(0)
    , mZoom(0)
    , mWindowHandle(0)
  {
  }

  Webview::~Webview()
  {
    mBrowser = nullptr;
    mClient = nullptr;
    mRenderHandler = nullptr;
  }

  bool Webview::renderToTexture(unsigned long long windowHandle, TexturePtr texture, DataProxy options)
  {
    if(mCefWasStopped) {
      return false;
    }

    std::string page = options.get("page", "");
    mPage = page;
    CefWindowInfo windowInfo;
    CefBrowserSettings browserSettings;
    browserSettings.windowless_frame_rate = 60;

    mWindowHandle = windowHandle;
    windowInfo.SetAsWindowless((WindowHandle)windowHandle);

    LOG(INFO) << "Rendering webview to texture";
    mRenderHandler = new RenderHandler(texture);
    EventSubscriber::addEventListener(texture.get(), Texture::RESIZE, &Webview::wasResized, 0);
    mClient = new BrowserClient(mRenderHandler.get(), mCef, this);
    mClient->setPageData(options.get("pageData", DataProxy()));

    // TODO: replace main thread runners with some generic implementation
    std::shared_ptr<SignalChannel> channel = std::make_shared<SignalChannel>();
    mCef->runInMainThread([&] () {
      auto c = channel;
      mBrowser = CefBrowserHost::CreateBrowserSync(windowInfo, mClient.get(), page.c_str(), browserSettings, nullptr);
      c->send(ChannelSignal::DONE);
    });

    channel->recv();
    return true;
  }

  void Webview::updateTexture(TexturePtr texture)
  {
    mRenderHandler->setTexture(texture);
    pushEvent(Event(Texture::RESIZE));
  }

  bool Webview::wasResized(EventDispatcher* sender, const Event& e)
  {
    pushEvent(Event(Texture::RESIZE));
    return true;
  }

  void Webview::executeJavascript(const std::string& script)
  {
    LOG(INFO) << "Execute javascript from native";
    if(!mBrowser) {
      return;
    }

    // JS should be excuted from the main thread only
    std::shared_ptr<SignalChannel> channel = std::make_shared<SignalChannel>();
    mCef->runInMainThread([&] () {
      auto c = channel;
      CefRefPtr<CefFrame> frame = mBrowser->GetMainFrame();
      frame->ExecuteJavaScript(script, frame->GetURL(), 0);
      c->send(ChannelSignal::DONE);
    });

    channel->recv();
  }

  void Webview::setZoom(float zoom)
  {
    mZoom = zoom;
    mBrowser->GetHost()->SetZoomLevel(zoom);
  }

  WindowPtr Webview::getWindow()
  {
    if(mWindowHandle == 0) {
      return nullptr;
    }

    return mCef->getFacade()->getWindowManager()->getWindow(mWindowHandle);
  }

  void Webview::renderPage(const std::string& page, const DataProxy& data)
  {
    mBrowser->GetMainFrame()->LoadURL(page);
    mClient->setPageData(data);
  }

  void Webview::pushEvent(Event event)
  {
    mEvents << event;
  }

  void Webview::pushEvent(KeyboardEvent event)
  {
    mKeyboardEvents << event;
  }

  void Webview::pushEvent(TextInputEvent event)
  {
    mTextInputEvents << event;
  }

  void Webview::pushEvent(MouseEvent event)
  {
    mMouseEvents << event;
  }

  void Webview::processEvents()
  {
    if (!mBrowser)
      return;

    {
      size_t count = mEvents.size();
      Event event;
      for(size_t i = 0; i < count; i++) {
        if(mEvents.get(event)) {
          if(event.getType() == Texture::RESIZE) {
            mBrowser->GetHost()->WasResized();
          } else if(event.getType() == Webview::MOUSE_LEAVE) {
            CefMouseEvent evt;
            evt.x = mMouseX;
            evt.y = mMouseY;
            evt.modifiers = mMouseModifiers;
            mBrowser->GetHost()->SendMouseMoveEvent(evt, true);
          } else if(event.getType() == Webview::UNDO) {
            mBrowser->GetMainFrame()->Undo();
          } else if(event.getType() == Webview::REDO) {
            mBrowser->GetMainFrame()->Redo();
          } else {
            LOG(WARNING) << "Unhandled event type " << event.getType();
          }
        }
      }
    }

    {
      size_t count = mKeyboardEvents.size();
      KeyboardEvent event;
      for(size_t i = 0; i < count; i++) {
        if(mKeyboardEvents.get(event)) {
          handleKeyboardEvent(event);
        }
      }
    }

    {
      size_t count = mTextInputEvents.size();
      TextInputEvent event;
      for(size_t i = 0; i < count; i++) {
        if(mTextInputEvents.get(event)) {
          handleInput(event);
        }
      }
    }

    {
      size_t count = mMouseEvents.size();
      MouseEvent event;
      for(size_t i = 0; i < count; i++) {
        if(mMouseEvents.get(event)) {
          handleMouseEvent(event);
        }
      }
    }
  }

  void Webview::queueEvent(Event event)
  {
    mOutgoingEvents << event;
    if(event.getType() == Webview::PAGE_LOADED) {
      mBrowser->GetHost()->SetZoomLevel(mZoom);
    }
  }

  void Webview::sendEvents()
  {
    size_t count = mOutgoingEvents.size();
    Event event;
    for(size_t i = 0; i < count; i++) {
      if(mOutgoingEvents.get(event)) {
        fireEvent(event);
      }
    }
  }

  void Webview::handleMouseEvent(const MouseEvent& event)
  {
    CefMouseEvent evt;
    mMouseX = evt.x = (int)event.mouseX;
    mMouseY = evt.y = (int)event.mouseY;

    int mouseModifier = 0;

    if(event.getType() == MouseEvent::MOUSE_MOVE) {
      if(event.relativeZ != 0) {
        mBrowser->GetHost()->SendMouseWheelEvent(evt, 0, event.relativeZ * 5);
      }
      evt.modifiers = mMouseModifiers;
      mBrowser->GetHost()->SendMouseMoveEvent(evt, false);
    } else {
      CefBrowserHost::MouseButtonType mappedBtn;
      switch(event.button) {
        case MouseEvent::ButtonType::Right:
          mappedBtn = CefBrowserHost::MouseButtonType::MBT_RIGHT;
          mouseModifier = EVENTFLAG_RIGHT_MOUSE_BUTTON;
          break;
        case MouseEvent::ButtonType::Left:
          mappedBtn = CefBrowserHost::MouseButtonType::MBT_LEFT;
          mouseModifier = EVENTFLAG_LEFT_MOUSE_BUTTON;
          break;
        case MouseEvent::ButtonType::Middle:
          mappedBtn = CefBrowserHost::MouseButtonType::MBT_MIDDLE;
          mouseModifier = EVENTFLAG_MIDDLE_MOUSE_BUTTON;
          break;
        default:
          LOG(WARNING) << "Unhandled mouse button " << event.button;
          return;
      }

      if(event.getType() == MouseEvent::MOUSE_DOWN) {
          mMouseModifiers |= mouseModifier;
      } else if(event.getType() == MouseEvent::MOUSE_UP) {
          mMouseModifiers &= ~mouseModifier;
      }

      mBrowser->GetHost()->SendMouseClickEvent(evt, mappedBtn, event.getType() == MouseEvent::MOUSE_UP, 1);
    }
  }

  void Webview::handleKeyboardEvent(const KeyboardEvent& event)
  {
    CefKeyEvent evt;
    int key = event.getNativeKey();
    if(key != -1) {
      evt.native_key_code = key;
      evt.windows_key_code = key;
    }

    if(event.text != 0) {
      evt.character = event.text;
    }

    uint32 modifiersCode = 0;
    if(event.isModifierDown(KeyboardEvent::Shift)) {
      modifiersCode |= EVENTFLAG_SHIFT_DOWN;
    }
    if(event.isModifierDown(KeyboardEvent::Win)) {
      modifiersCode |= EVENTFLAG_COMMAND_DOWN;
    }
    if(event.isModifierDown(KeyboardEvent::Ctrl)) {
      modifiersCode |= EVENTFLAG_CONTROL_DOWN;
    }
    if(event.isModifierDown(KeyboardEvent::Alt)) {
      modifiersCode |= EVENTFLAG_ALT_DOWN;
    }
    if(event.isModifierDown(KeyboardEvent::Num)) {
      modifiersCode |= EVENTFLAG_NUM_LOCK_ON;
    }
    if(event.isModifierDown(KeyboardEvent::Caps)) {
      modifiersCode |= EVENTFLAG_CAPS_LOCK_ON;
    }
    evt.modifiers = modifiersCode;
    evt.type = event.getType() == KeyboardEvent::KEY_DOWN ? KEYEVENT_KEYDOWN : KEYEVENT_KEYUP;
    mBrowser->GetHost()->SendKeyEvent(evt);
  }

  void Webview::handleInput(const TextInputEvent& event)
  {
#if GSAGE_PLATFORM == GSAGE_WIN32
    auto str = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(event.getText());
#else
    auto str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(event.getText());
#endif

    for(int i = 0; i < str.size(); i++) {
      CefKeyEvent evt;
      int key = KeyboardEvent::getNativeKey(event.key);
      if(key != -1) {
        evt.native_key_code = key;
      }
#if GSAGE_PLATFORM == GSAGE_WIN32
      evt.windows_key_code = str[i];
#endif
      evt.character = str[i];
      evt.unmodified_character = str[i];
      evt.type = KEYEVENT_CHAR;
      mBrowser->GetHost()->SendKeyEvent(evt);
    }
  }

  const std::string PLUGIN_NAME = "CEFPlugin";

  CEFPlugin::LuaMessageHandler::LuaMessageHandler(sol::function callback)
    : mCallback(callback)
  {

  }

  std::string CEFPlugin::LuaMessageHandler::execute(CefRefPtr<CefProcessMessage> msg)
  {
    if(!mCallback.valid())
    {
      LOG(ERROR) << "Failed to call lua callback";
      return "null";
    }

    auto args = msg->GetArgumentList();
    DataProxy arguments = loads(args->GetString(2).ToString(), DataWrapper::JSON_OBJECT);

    std::vector<sol::object> mObjects;
    sol::table t = sol::state_view(mCallback.lua_state()).create_table();
    arguments.dump(t);

    for(auto iter = t.begin();
        iter != t.end();
        ++iter
       ) {
      mObjects.push_back((*iter).second);
    }

    std::string result = "null";
    try {
      sol::function_result res = mCallback(sol::as_args(mObjects));

      if(!res.valid()) {
        sol::error err = res;
        LOG(ERROR) << "Failed to call lua callback " << err.what();
        return "null";
      }

      sol::object value = res;
      DataProxy data;
      switch(value.get_type()) {
        case sol::type::table:
          data.put("retval", DataProxy::create(value.as<sol::table>()));
          break;
        case sol::type::string:
          data.put("retval", value.as<std::string>());
          break;
        case sol::type::boolean:
          data.put("retval", value.as<bool>());
          break;
        case sol::type::number:
          data.put("retval", value.as<double>());
          break;
        default:
          // skipped
          break;
      }
      result = dumps(data, DataWrapper::JSON_OBJECT);
    } catch(sol::error& err) {
      LOG(ERROR) << "Failed to handle CEF message: " << err.what();
    } catch(const std::exception& e) {
      LOG(ERROR) << "Failed to handle CEF message: " << e.what();
    } catch(...) {
      LOG(ERROR) << "Failed to handle CEF message: unknown error";
    }

    return result;
  }

  CEFPlugin::CEFPlugin()
    : mAsyncMode(false)
  {
  }

  CEFPlugin::~CEFPlugin()
  {
  }

  const std::string& CEFPlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  bool CEFPlugin::installImpl()
  {
    mAsyncMode = mFacade->getEngine()->settings().get("cef.dedicatedThread", false);

    bool success = false;
    if(mAsyncMode) {
      std::shared_ptr<SignalChannel> channel = std::make_shared<SignalChannel>();
      mCefRunner = std::thread([&] () {
          // save ref in this scope
          auto c = channel;
          mRunning.store(true);
          if(initialize()) {
            channel->send(ChannelSignal::DONE);
          } else {
            channel->send(ChannelSignal::FAILURE);
          }
          std::chrono::milliseconds waitTime(10);
          while(mRunning.load()) {
            doMessageLoop();
            std::this_thread::sleep_for(waitTime);
          }
          stop();
      });

      success = channel->recv() == ChannelSignal::DONE;
    } else {
      success = initialize();
    }

    if(success) {
      mFacade->addUpdateListener(this);
    }

    return success;
  }

  void CEFPlugin::uninstallImpl()
  {
    mFacade->removeUpdateListener(this);
    if (mLuaInterface && mLuaInterface->getState())
    {
      sol::state_view& lua = *mLuaInterface->getSolState();

      lua["cef"] = sol::lua_nil;
    }
    if(mAsyncMode && mRunning.load()) {
      mRunning.store(false);
      mCefRunner.join();
    } else {
      stop();
    }
  }

  void CEFPlugin::setupLuaBindings() {
    if (mLuaInterface && mLuaInterface->getState())
    {
      sol::state_view lua(mLuaInterface->getState());

      lua.new_usertype<CEFPlugin>("CEFPlugin",
        "new", sol::no_constructor,
        "createWebview", &CEFPlugin::createWebview,
        "removeWebview", &CEFPlugin::removeWebview,
        "addMessageHandler", &CEFPlugin::addLuaMessagehandler
      );

      lua.new_usertype<Webview>("CEFWebview",
        sol::base_classes, sol::bases<EventDispatcher>(),
        "new", sol::no_constructor,
        "renderToTexture", &Webview::renderToTexture,
        "updateTexture", &Webview::updateTexture,
        "executeJavascript", &Webview::executeJavascript,
        "setZoom", &Webview::setZoom,
        "renderPage", &Webview::renderPage,
        "pushEvent", sol::overload(
          (void(Webview::*)(MouseEvent))&Webview::pushEvent,
          (void(Webview::*)(KeyboardEvent))&Webview::pushEvent,
          (void(Webview::*)(TextInputEvent))&Webview::pushEvent,
          (void(Webview::*)(Event))&Webview::pushEvent
        ),
        "MOUSE_LEAVE", sol::var(Webview::MOUSE_LEAVE),
        "UNDO", sol::var(Webview::UNDO),
        "REDO", sol::var(Webview::REDO),
        "PAGE_LOADING", sol::var(Webview::PAGE_LOADING),
        "PAGE_LOADED", sol::var(Webview::PAGE_LOADED)
      );

      lua["cef"] = this;
    }
  }

  void CEFPlugin::runInMainThread(CEFPlugin::Task cb)
  {
    if(mAsyncMode) {
      mTasks << cb;
    } else {
      cb();
    }
  }

  void CEFPlugin::runInRenderThread(CEFPlugin::Task cb)
  {
    if(mAsyncMode) {
      mRenderThreadTasks << cb;
    } else {
      cb();
    }
  }

  std::tuple<std::string, bool> CEFPlugin::handleProcessMessage(CefRefPtr<CefProcessMessage> msg)
  {
    if(mAsyncMode) {
      PendingMessagePtr pm = std::make_shared<PendingMessage>(msg);
      mMessages << pm;
      ChannelSignal result = pm->channel.recv();
      return std::make_tuple(pm->result, result == ChannelSignal::DONE);
    }

    return handleProcessMessageSync(msg);
  }

  void CEFPlugin::update(double time)
  {
    if(mAsyncMode) {
      int count = mMessages.size();
      for(int i = 0; i < count; i++) {
        PendingMessagePtr pm;
        if(mMessages.get(pm)) {
          bool success;
          std::string result;
          std::tie(result, success) = handleProcessMessageSync(pm->msg);
          if(success) {
            pm->result = result;
          }
        }
        pm->channel.send(ChannelSignal::DONE);
      }
    }

    for(auto& pair : mViews) {
      pair.second->sendEvents();
    }

    Task task;
    if(mRenderThreadTasks.get(task)) {
      task();
    }

    if(!mAsyncMode) {
      doMessageLoop();
    }
  }

  WebviewPtr CEFPlugin::createWebview(const std::string& name)
  {
    std::lock_guard<std::mutex> lock(mViewsLock);
    if(contains(mViews, name))
    {
      return mViews[name];
    }

    mViews[name] = std::make_shared<Webview>(this);
    return mViews[name];
  }

  bool CEFPlugin::removeWebview(const std::string& name)
  {
    std::lock_guard<std::mutex> lock(mViewsLock);
    if(contains(mViews, name))
    {
      mViews[name]->setCefWasStopped();
      mViews.erase(name);
      return true;
    }
    return false;
  }

  void CEFPlugin::addLuaMessagehandler(const std::string& name, sol::function callback)
  {
    mMessageHandlers[name] = MessageHandlerPtr(new LuaMessageHandler(callback));
  }

  bool CEFPlugin::initialize()
  {
#if GSAGE_PLATFORM == GSAGE_WIN32
    CefMainArgs args(0);
#else
    //int argc = 2;
    //char* argv[2] = {"--off-screen-rendering-enabled", "--v=0"};
    //CefMainArgs args(argc, &argv[0]);
    CefMainArgs args(0, nullptr);
#endif
    {
      int result = CefExecuteProcess(args, nullptr, nullptr);
      if(result >= 0) {
        return false;
      }
    }

    CefSettings settings;
    settings.windowless_rendering_enabled = true;
    settings.multi_threaded_message_loop = false;
    settings.external_message_pump = true;
    settings.no_sandbox = true;
#if GSAGE_PLATFORM == GSAGE_APPLE
    char path[PATH_MAX];
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef frameworksURL = CFBundleCopyPrivateFrameworksURL(mainBundle);
    if (!CFURLGetFileSystemRepresentation(frameworksURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
      return false;
    }

    std::stringstream ss;
    ss << path << "/cef.helper.app/Contents/MacOS/cef.helper";
    CefString(&settings.browser_subprocess_path).FromASCII(ss.str().c_str());
#elif GSAGE_PLATFORM == GSAGE_LINUX
    char path[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", path, sizeof(path)-1);
    if (len == -1) {
      path[len] = '\0';
      LOG(ERROR) << "Failed to get executable path";
      return false;
    }

    std::stringstream ss;
    ss << dirname(path) << "/" << "cef.helper";
    CefString(&settings.browser_subprocess_path).FromASCII(ss.str().c_str());

    if(!getcwd(path, sizeof(path)-1)) {
      LOG(ERROR) << "Failed to get current working directory";
      return false;
    }
    ss = std::stringstream();
    ss << path << "/locales";
    CefString(&settings.locales_dir_path) = ss.str().c_str();
#elif GSAGE_PLATFORM == GSAGE_WIN32
    char filename[] = "cef.helper.exe";
    char fullFilename[MAX_PATH];
    GetFullPathName(filename, MAX_PATH, fullFilename, nullptr);

    CefString(&settings.browser_subprocess_path).FromASCII(fullFilename);
#endif
    CefString(&settings.cache_path) = (mFacade->filesystem()->getCacheHome() + "/gsage").c_str();

    if(!mFacade->filesystem()->exists(CefString(&settings.browser_subprocess_path).ToString())) {
      LOG(ERROR) << "Failed to locate cef.helper executable " << CefString(&settings.browser_subprocess_path).ToString();
      return false;
    }

    {
      bool result = CefInitialize(args, settings, nullptr, nullptr);
      if(!result) {
        return false;
      }
    }

    CefRegisterSchemeHandlerFactory("client", "views", new ClientSchemeHandlerFactory(mFacade));
    return true;
  }

  void CEFPlugin::stop()
  {
    {
      std::lock_guard<std::mutex> lock(mViewsLock);
      for(auto pair : mViews) {
        pair.second->setCefWasStopped();
      }
      mViews.clear();
    }
    CefShutdown();
  }

  void CEFPlugin::doMessageLoop()
  {
    CefDoMessageLoopWork();

    Task task;
    if(mTasks.get(task)) {
      task();
    }

    {
      std::lock_guard<std::mutex> lock(mViewsLock);
      for(auto pair : mViews) {
        pair.second->processEvents();
      }
    }
  }

  std::tuple<std::string, bool> CEFPlugin::handleProcessMessageSync(CefRefPtr<CefProcessMessage> msg)
  {
    std::string name = msg->GetName().ToString();
    std::string result;
    bool success = false;
    if(name == "fn") {
      auto args = msg->GetArgumentList();
      name = args->GetString(1).ToString();
      if(contains(mMessageHandlers, name)) {
        result = mMessageHandlers[name]->execute(msg);
        success = true;
      }
    }
    return std::make_tuple(result, success);
  }

  CEFPlugin* cefPlugin = NULL;

  extern "C" bool PluginExport dllStartPlugin(GsageFacade* facade)
  {
    if(cefPlugin != NULL)
    {
      return false;
    }
    cefPlugin = new CEFPlugin();
    return facade->installPlugin(cefPlugin);
  }

  extern "C" bool PluginExport dllStopPlugin(GsageFacade* facade)
  {
    if(cefPlugin == NULL)
      return true;

    bool res = facade->uninstallPlugin(cefPlugin);
    if(!res)
      return false;
    delete cefPlugin;
    cefPlugin = NULL;
    return true;
  }
}
