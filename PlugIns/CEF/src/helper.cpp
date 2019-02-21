#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include "include/cef_parser.h"

#ifdef _WIN32
#include "windows.h"
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#endif
#include <iostream>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>

class CEFApp;

CefRefPtr<CefV8Value> CefValueToCefV8Value(CefRefPtr<CefValue> value) {
  CefRefPtr<CefV8Value> result;
  switch (value->GetType()) {
    case VTYPE_INVALID:
      {
        result = CefV8Value::CreateNull();
      }
      break;
    case VTYPE_NULL:
      {
        result = CefV8Value::CreateNull();
      }
      break;
    case VTYPE_BOOL:
      {
        result = CefV8Value::CreateBool(value->GetBool());
      }
      break;
    case VTYPE_INT:
      {
        result = CefV8Value::CreateInt(value->GetInt());
      }
      break;
    case VTYPE_DOUBLE:
      {
        result = CefV8Value::CreateDouble(value->GetDouble());
      }
      break;
    case VTYPE_STRING:
      {
        result = CefV8Value::CreateString(value->GetString());
      }
      break;
    case VTYPE_BINARY:
      {
        result = CefV8Value::CreateNull();
      }
      break;
    case VTYPE_DICTIONARY:
      {
        result = CefV8Value::CreateObject(NULL, NULL);
        CefRefPtr<CefDictionaryValue> dict = value->GetDictionary();
        CefDictionaryValue::KeyList keys;
        dict->GetKeys(keys);
        for (unsigned int i = 0; i < keys.size(); i++) {
          CefString key = keys[i];
          result->SetValue(key, CefValueToCefV8Value(dict->GetValue(key)), V8_PROPERTY_ATTRIBUTE_NONE);
        }
      }
      break;
    case VTYPE_LIST:
      {
        CefRefPtr<CefListValue> list = value->GetList();
        int size = list->GetSize();
        result = CefV8Value::CreateArray(size);
        for (int i = 0; i < size; i++) {
          result->SetValue(i, CefValueToCefV8Value(list->GetValue(i)));
        }
      }
      break;
  }
  return result;
}


class JSCallbacks
{
  public:
    bool execute(int id, const CefString& json)
    {
      std::lock_guard<std::mutex> lock(mMapLock);
      if(mCallbacks.count(id) == 0) {
        return false;
      }

      CefRefPtr<CefValue> value = CefParseJSON(json, JSON_PARSER_RFC);
      if(!value || value->GetType() != VTYPE_DICTIONARY) {
        mCallbacks.erase(id);
        return false;
      }

      CefRefPtr<CefV8Value> object, func;
      CefRefPtr<CefV8Context> context;
      std::tie(object, context, func) = mCallbacks[id];

      context->Enter();
      CefRefPtr<CefV8Value> data = CefValueToCefV8Value(value->GetDictionary()->GetValue("retval"));
      CefV8ValueList args;
      args.push_back(data);

      func->ExecuteFunction(object, args);
      mCallbacks.erase(id);
      context->Exit();
      return true;
    }

    void set(int id, CefRefPtr<CefV8Value> thisObj, CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> value)
    {
      mMapLock.lock();
      mCallbacks[id] = std::make_tuple(thisObj, context, value);

      if(mCallbacks.size() > 1024) {
        mCallbacks.erase(mCallbacks.begin()->first);
      }
      mMapLock.unlock();
    }
  private:
    std::mutex mMapLock;
    std::map<int, std::tuple<CefRefPtr<CefV8Value>, CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>> mCallbacks;
};

class JSFunctionHandler : public CefV8Handler {
  public:
    JSFunctionHandler(JSCallbacks* callbacks)
      : mJSCallbacks(callbacks)
      , mCounter(0)
    {
    }

    virtual bool Execute(const CefString& name,
        CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments,
        CefRefPtr<CefV8Value>& retval,
        CefString& exception) OVERRIDE {

      int id = mCounter.fetch_add(1);
      if(id == INT_MAX - 1) {
        mCounter.store(0);
      }

      for(auto browser : mBrowsers) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(name);
        CefRefPtr<CefListValue> args = msg->GetArgumentList();
        args->SetInt(0, id);
        for(int i = 2; i < arguments.size(); i++) {
          if(arguments[i]->IsString()) {
            args->SetString(args->GetSize(), arguments[i]->GetStringValue());
          }
        }

        browser->SendProcessMessage(PID_BROWSER, msg);
      }

      retval = CefV8Value::CreateInt(id);
      if(arguments[1]->IsFunction()) {
        mJSCallbacks->set(id, arguments[0], CefV8Context::GetCurrentContext(), arguments[1]);
      }
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
    JSCallbacks* mJSCallbacks;
    std::atomic<int> mCounter;
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
        "    native function fn(thisObj, handler, name, data);"
        "    var promise = {"
        "      then: function(handler) { if(this.succeed) handler(this.result); this.handler = handler; },"
        "      callback: function(result) { this.succeed = true; this.result = result; if(this.handler) { this.handler(result); } }"
        "    };"
        "    var handler = function(result) { promise.callback(result); };"
        "    fn(this, handler, name, JSON.stringify(args));"
        "    return promise;"
        "  };"
        "})();";

      // Create an instance of my CefV8Handler object.
      mHandler = new JSFunctionHandler(&mJSCallbacks);

      // Register the extension.
      CefRegisterExtension("v8/engine", extensionCode, mHandler);
    }

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message) OVERRIDE
    {
      CefRefPtr<CefListValue> args = message->GetArgumentList();
      if(message->GetName() == "JSResponse") {
        mJSCallbacks.execute(args->GetInt(0), args->GetString(1));
        return true;
      }
      return false;
    }

    IMPLEMENT_REFCOUNTING(CEFApp);
  private:
    CefRefPtr<CefV8Handler> mHandler;
    CefRefPtr<CefBrowser> mBrowser;

    JSCallbacks mJSCallbacks;
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
  CefMainArgs mainArgs(GetModuleHandle(NULL));
#else
  CefMainArgs mainArgs(argc, argv);
#endif

  CefRefPtr<CefApp> app = new CEFApp();
  // Execute the sub-process logic. This will block until the sub-process should exit.
  int result = CefExecuteProcess(mainArgs, app, nullptr);
  if(result > 0)
    std::cout << "helper.cpp exited with code " << result << "\n";
  return result;
}
