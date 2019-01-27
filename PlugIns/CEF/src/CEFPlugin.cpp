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
#elif GSAGE_PLATFORM == GSAGE_LINUX
#define WindowHandle unsigned long long
#elif GSAGE_PLATFORM == GSAGE_WIN32
#define WindowHandle HWND
#endif

#include <string>
#include <locale>
#include <codecvt>

#include <include/wrapper/cef_helpers.h>
#include <include/wrapper/cef_stream_resource_handler.h>
#include <include/cef_parser.h>

#if (!_DLL) && (_MSC_VER >= 1900 /* VS 2015*/) && (_MSC_VER <= 1916 /* VS 2017 */)
std::locale::id std::codecvt<char16_t, char, _Mbstatet>::id;
#endif

namespace Gsage {
  inline bool ends_with(std::string const & value, std::string const & ending)
  {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
  }

  class ClientCefResourceHandler : public CefResourceHandler {
    public:
      ClientCefResourceHandler()
      {

      }

      bool ProcessRequest(CefRefPtr<CefRequest> request,
          CefRefPtr<CefCallback> callback) override {
        CEF_REQUIRE_IO_THREAD();
        CefRefPtr<CefPostData> post = request->GetPostData();
        LOG(INFO) << "Data " << post;
        return true;
      }
      IMPLEMENT_REFCOUNTING(ClientCefResourceHandler);
  };

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
        bool success = false;
        if(ends_with(filepath, ".template")) {
          CefClient* client = browser->GetHost()->GetClient().get();
          DataProxy pageData;
          if(client) {
            pageData = static_cast<BrowserClient*>(client)->getPageData();
          }
          success = FileLoader::getSingletonPtr()->loadTemplate(filepath, content, pageData);
          contentType = "text/html";
        } else {
          success = FileLoader::getSingletonPtr()->load(filepath, content, std::ios_base::binary);
        }

        if(!success || content.empty()) {
          LOG(WARNING) << "Nothing to load for " << filepath;
          return nullptr;
        }


        for(auto pair : mContentTypes) {
          if(ends_with(filepath, pair.first)) {
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
    , mX(0)
    , mY(0)
  {
  }

  RenderHandler::~RenderHandler()
  {
  }

  bool RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
  {
    unsigned int width, height;
    std::tie(width, height) = mTexture->getSize();
    rect = CefRect(mX, mY, width, height);
    return true;
  }

  void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
  {
    if(!mTexture->isValid()) {
      return;
    }

    mTexture->update(buffer, width * height * 4, width, height);
  }

  void RenderHandler::setTexture(TexturePtr texture)
  {
    mTexture = texture;
  }

  BrowserClient::BrowserClient(RenderHandler* renderHandler, CEFPlugin* cef)
    : mRenderHandler(renderHandler)
    , mCef(cef)
  {
  }

  CefRefPtr<CefRenderHandler> BrowserClient::GetRenderHandler()
  {
    return mRenderHandler;
  }

  bool BrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> msg)
  {
    return mCef->handleProcessMessage(msg->Copy());
  }

  const Event::Type Webview::MOUSE_LEAVE = "Webview::MOUSE_LEAVE";

  Webview::Webview(CEFPlugin* cef)
    : mCefWasStopped(false)
    , mCef(cef)
    , mMouseModifiers(0)
    , mMouseX(0)
    , mMouseY(0)
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

    windowInfo.SetAsWindowless((WindowHandle)windowHandle);

    LOG(INFO) << "Rendering webview to texture";
    mRenderHandler = new RenderHandler(texture);
    addEventListener(texture.get(), Texture::RESIZE, &Webview::wasResized, 0);
    mClient = new BrowserClient(mRenderHandler.get(), mCef);
    mClient->setPageData(options.get("pageData", DataProxy()));

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

    CefRefPtr<CefFrame> frame = mBrowser->GetMainFrame();
    frame->ExecuteJavaScript(script, frame->GetURL(), 0);
  }

  void Webview::renderPage(const std::string& page, const DataProxy& data)
  {
    if(page != mPage) {
      mBrowser->GetMainFrame()->LoadURL(page);
    }
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

  void Webview::handleMouseEvent(const MouseEvent& event)
  {
    CefMouseEvent evt;
    mMouseX = evt.x = event.mouseX;
    mMouseY = evt.y = event.mouseY;

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
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(event.getText());

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

  DataProxy CEFPlugin::LuaMessageHandler::execute(CefRefPtr<CefProcessMessage> msg)
  {
    DataProxy result;
    if(!mCallback.valid())
    {
      LOG(ERROR) << "Failed to call lua callback";
      return result;
    }

    auto args = msg->GetArgumentList();
    DataProxy arguments = loads(args->GetString(1).ToString(), DataWrapper::JSON_OBJECT);

    std::vector<sol::object> mObjects;
    sol::table t = sol::state_view(mCallback.lua_state()).create_table();
    arguments.dump(t);

    for(auto iter = t.begin();
        iter != t.end();
        ++iter
       ) {
      mObjects.push_back((*iter).second);
    }

    try {
      sol::function_result res = mCallback(sol::as_args(mObjects));
      sol::object value = res;
      if(value.get_type() == sol::type::table) {
        result = DataProxy::create(value.as<sol::table>());
      }
      if(!res.valid()) {
        sol::error err = res;
        LOG(ERROR) << "Failed to call lua callback " << err.what();
        return result;
      }
    } catch(...) {
      LOG(ERROR) << "Failed to handle CEF message";
      return result;
    }


    return result;
  }

  CEFPlugin::CEFPlugin()
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
    std::shared_ptr<SignalChannel> channel = std::make_shared<SignalChannel>();
    bool succeed = false;
    mCefRunner = std::thread([&] () {
      {
        // save ref in this scope
        auto c = channel;
        mRunning.store(true);
#if GSAGE_PLATFORM == GSAGE_WIN32
        CefMainArgs args(0);
#else
        CefMainArgs args(0, nullptr);
#endif
        {
          int result = CefExecuteProcess(args, nullptr, nullptr);
          if(result >= 0) {
            channel->send(ChannelSignal::FAILURE);
            return;
          }
        }

        CefSettings settings;
        settings.windowless_rendering_enabled = true;
        settings.multi_threaded_message_loop = false;
        settings.external_message_pump = true;
#if GSAGE_PLATFORM == GSAGE_APPLE
        char path[PATH_MAX];
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef frameworksURL = CFBundleCopyPrivateFrameworksURL(mainBundle);
        if (!CFURLGetFileSystemRepresentation(frameworksURL, TRUE, (UInt8 *)path, PATH_MAX))
        {
          channel->send(ChannelSignal::FAILURE);
          return;
        }

        std::stringstream ss;
        ss << path << "/cef.helper.app/Contents/MacOS/cef.helper";
        CefString(&settings.browser_subprocess_path).FromASCII(ss.str().c_str());
#elif GSAGE_PLATFORM == GSAGE_LINUX
        char path[1024];
        std::stringstream ss;
        if(getcwd(path, 1024))
          ss << path;

        ss << "/" << "cef.helper";
        CefString(&settings.browser_subprocess_path).FromASCII(ss.str().c_str());
#elif GSAGE_PLATFORM == GSAGE_WIN32
        char filename[] = "cef.helper.exe";
        char fullFilename[MAX_PATH];
        GetFullPathName(filename, MAX_PATH, fullFilename, nullptr);

        LOG(INFO) << fullFilename;
        CefString(&settings.browser_subprocess_path).FromASCII(fullFilename);
#endif
        bool result = CefInitialize(args, settings, nullptr, nullptr);
        if(!result) {
          channel->send(ChannelSignal::FAILURE);
          return;
        }

        succeed = true;
        channel->send(ChannelSignal::DONE);
      }

      CefRegisterSchemeHandlerFactory("client", "views", new ClientSchemeHandlerFactory(mFacade));
      std::chrono::milliseconds waitTime(10);
      Task task;
      while(mRunning.load()) {
        CefDoMessageLoopWork();

        if(mTasks.get(task)) {
          task();
        }

        {
          std::lock_guard<std::mutex> lock(mViewsLock);
          for(auto pair : mViews) {
            pair.second->processEvents();
          }
        }
        std::this_thread::sleep_for(waitTime);
      }
      {
        std::lock_guard<std::mutex> lock(mViewsLock);
        for(auto pair : mViews) {
          pair.second->setCefWasStopped();
        }
        mViews.clear();
      }
      CefShutdown();
    });

    channel->recv();
    mFacade->addUpdateListener(this);

    return succeed;
  }

  void CEFPlugin::uninstallImpl()
  {
    mFacade->removeUpdateListener(this);
    if (mLuaInterface && mLuaInterface->getState())
    {
      sol::state_view& lua = *mLuaInterface->getSolState();

      lua["cef"] = sol::lua_nil;
    }
    mRunning.store(false);
    mCefRunner.join();
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
        "new", sol::no_constructor,
        "renderToTexture", &Webview::renderToTexture,
        "updateTexture", &Webview::updateTexture,
        "executeJavascript", &Webview::executeJavascript,
        "setScreenPosition", &Webview::setScreenPosition,
        "renderPage", &Webview::renderPage,
        "pushEvent", sol::overload(
          (void(Webview::*)(MouseEvent))&Webview::pushEvent,
          (void(Webview::*)(KeyboardEvent))&Webview::pushEvent,
          (void(Webview::*)(TextInputEvent))&Webview::pushEvent,
          (void(Webview::*)(Event))&Webview::pushEvent
        ),
        "MOUSE_LEAVE", sol::var(Webview::MOUSE_LEAVE)
      );

      lua["cef"] = this;
    }
  }

  void CEFPlugin::runInMainThread(CEFPlugin::Task cb)
  {
    mTasks << cb;
  }

  bool CEFPlugin::handleProcessMessage(CefRefPtr<CefProcessMessage> msg)
  {
    PendingMessagePtr pm = std::make_shared<PendingMessage>(msg);
    mMessages << pm;
    ChannelSignal result = pm->channel.recv();
    return result == ChannelSignal::DONE;
  }

  void CEFPlugin::update(double time)
  {
    int count = mMessages.size();
    for(int i = 0; i < count; i++) {
      PendingMessagePtr pm;
      if(mMessages.get(pm)) {
        std::string name = pm->msg->GetName().ToString();
        if(name == "fn") {
          auto args = pm->msg->GetArgumentList();
          name = args->GetString(0).ToString();
          if(mMessageHandlers.count(name) != 0) {
            DataProxy result = mMessageHandlers[name]->execute(pm->msg);
          }
        }
      }
      pm->channel.send(ChannelSignal::DONE);
    }
  }

  WebviewPtr CEFPlugin::createWebview(const std::string& name)
  {
    std::lock_guard<std::mutex> lock(mViewsLock);
    if(mViews.count(name) > 0)
    {
      return mViews[name];
    }

    mViews[name] = std::make_shared<Webview>(this);
    return mViews[name];
  }

  bool CEFPlugin::removeWebview(const std::string& name)
  {
    std::lock_guard<std::mutex> lock(mViewsLock);
    if(mViews.count(name) > 0)
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
