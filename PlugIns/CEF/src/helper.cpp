#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>

#ifdef _WIN32
#include "windows.h"
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#endif

class CEFApp;

class JSFunctionHandler : public CefV8Handler {
  public:
    JSFunctionHandler() {}

    virtual bool Execute(const CefString& name,
        CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments,
        CefRefPtr<CefV8Value>& retval,
        CefString& exception) OVERRIDE {

      retval = CefV8Value::CreateObject(NULL, NULL);
      retval->SetValue("ok", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);

      for(auto browser : mBrowsers) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(name);
        CefRefPtr<CefListValue> args = msg->GetArgumentList();
        for(int i = 0; i < arguments.size(); i++) {
          if(arguments[i]->IsString()) {
            args->SetString(i, arguments[i]->GetStringValue());
          }
        }

        browser->SendProcessMessage(PID_BROWSER, msg);
      }
      // Function does not exist.
      return true;
    }

    void registerNewBrowser(CefRefPtr<CefBrowser> browser)
    {
      mBrowsers.push_back(browser);
    }

    // Provide the reference counting implementation for this class.
    IMPLEMENT_REFCOUNTING(JSFunctionHandler);
  private:
    std::vector<CefRefPtr<CefBrowser>> mBrowsers;
};

class CEFApp : public CefApp, public CefRenderProcessHandler {
  public:
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE {
      return this;
    }

    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE {
      static_cast<JSFunctionHandler*>(mHandler.get())->registerNewBrowser(browser);
    }

    void OnWebKitInitialized() OVERRIDE {
      // Define the extension contents.
      std::string extensionCode =
        "var nativeFacade;"
        "if (!nativeFacade)"
        "  nativeFacade = {};"
        "var engine;"
        "if (!engine) {"
        "  engine = new Proxy(nativeFacade, {"
        "    get: function(target, prop) {"
        "      return function() {"
        "        var args = Array.prototype.slice.call(arguments);"
        "        return target.fn.call(target, prop, args);"
        "      };"
        "    }"
        "  });"
        "}"
        "(function() {"
        "  nativeFacade.fn = function(name, args) {"
        "    native function fn(name, data);"
        "    return fn(name, JSON.stringify(args));"
        "  };"
        "})();";

      // Create an instance of my CefV8Handler object.
      mHandler = new JSFunctionHandler();

      // Register the extension.
      CefRegisterExtension("v8/engine", extensionCode, mHandler);
    }

    IMPLEMENT_REFCOUNTING(CEFApp);
  private:
    CefRefPtr<CefV8Handler> mHandler;
    CefRefPtr<CefBrowser> mBrowser;
};

// Program entry-point function.
#if _WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT count) {
#else
int main(int argc, char* argv[]) {
#endif
  // Structure for passing command-line arguments.
  // The definition of this structure is platform-specific.
#ifdef _WIN32
  CefMainArgs main_args(GetModuleHandle(NULL));
#else
  CefMainArgs main_args(argc, argv);
#endif

  CefRefPtr<CefApp> app = new CEFApp();
  // Execute the sub-process logic. This will block until the sub-process should exit.
  return CefExecuteProcess(main_args, app, nullptr);
}
