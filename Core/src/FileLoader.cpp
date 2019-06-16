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

#include "FileLoader.h"
#include <assert.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include "ScopedLocale.h"


namespace Gsage {
  inline nlohmann::json jsonContext(const DataProxy& dp)
  {
    nlohmann::json result;
    switch(dp.getStoredType()) {
      case DataWrapper::Object:
        result = nlohmann::json::object();
        for(auto pair : dp) {
          result[pair.first] = jsonContext(pair.second);
        }
        break;
      case DataWrapper::Array:
        result = nlohmann::json::array();
        for(auto pair : dp) {
          result.push_back(jsonContext(pair.second));
        }
        break;
      case DataWrapper::Int:
        result = nlohmann::json(dp.as<int>());
        break;
      case DataWrapper::UInt:
        result = nlohmann::json(dp.as<unsigned int>());
        break;
      case DataWrapper::Float:
        result = nlohmann::json(dp.as<float>());
        break;
      case DataWrapper::Double:
        result = nlohmann::json(dp.as<double>());
        break;
      case DataWrapper::String:
        result = nlohmann::json(dp.as<std::string>());
        break;
      case DataWrapper::Bool:
        result = nlohmann::json(dp.as<bool>());
        break;
      default:
        return nlohmann::json(nullptr);
    }
    return result;
  }

  inline DataProxy jsonToDataProxy(const nlohmann::json& data)
  {
    DataProxy result = DataProxy::create(DataWrapper::JSON_OBJECT);
    switch(data.type()) {
      case nlohmann::json::value_t::object:
        for(auto& pair : data.items()) {
          result.put(pair.key(), jsonToDataProxy(pair.value()));
        }
        break;
      case nlohmann::json::value_t::array:
        for(auto& pair : data.items()) {
          result.push(jsonToDataProxy(pair.value()));
        }
        break;
      case nlohmann::json::value_t::number_integer:
        result.set(data.get<int>());
        break;
      case nlohmann::json::value_t::number_unsigned:
        result.set(data.get<unsigned int>());
        break;
      case nlohmann::json::value_t::number_float:
        result.set(data.get<float>());
        break;
      case nlohmann::json::value_t::string:
        result.set(data.get<std::string>());
        break;
      case nlohmann::json::value_t::boolean:
        result.set(data.get<bool>());
        break;
      default:
        return result;
    }
    return result;
  }

  inline sol::object jsonToLuaObject(lua_State* L, const nlohmann::json& data) {
    sol::object res;
    sol::table t;
    int index = 1;
    switch(data.type()) {
      case nlohmann::json::value_t::number_integer:
        res = sol::make_object(L, data.get<int>());
        break;
      case nlohmann::json::value_t::number_unsigned:
        res = sol::make_object(L, data.get<unsigned int>());
        break;
      case nlohmann::json::value_t::number_float:
        res = sol::make_object(L, data.get<float>());
        break;
      case nlohmann::json::value_t::string:
        res = sol::make_object(L, data.get<std::string>());
        break;
      case nlohmann::json::value_t::boolean:
        res = sol::make_object(L, data.get<bool>());
        break;
      case nlohmann::json::value_t::object:
        t = sol::state_view(L).create_table();
        for(auto& pair : data.items()) {
          t[pair.key()] = jsonToLuaObject(L, pair.value());
        }
        res = t;
        break;
      case nlohmann::json::value_t::array:
        t = sol::state_view(L).create_table();
        for(auto& pair : data.items()) {
          t[index++] = jsonToLuaObject(L, pair.value());
        }
        res = t;
        break;
      default:
        LOG(WARNING) << "Skipped unsupported callback arg type";
        break;
    }

    return res;
  }

  FileLoader* FileLoader::mInstance = 0;

  FileLoader& FileLoader::getSingleton()
  {
    assert(mInstance != 0);
    return *mInstance;
  }

  FileLoader* FileLoader::getSingletonPtr()
  {
    assert(mInstance != 0);
    return mInstance;
  }

  void FileLoader::init(FileLoader::Encoding format, const DataProxy& environment)
  {
    FileLoader::mInstance = new FileLoader(format, environment);
  }

  FileLoader::FileLoader(FileLoader::Encoding format, const DataProxy& environment)
    : mFormat(format)
    , mEnvironment(environment)
    , mInjaEnv(new inja::Environment())
  {
    // add main workdir with low priority
    mResourceSearchFolders[100000] = mEnvironment.get("workdir", ".");

    // add current directory
    mResourceSearchFolders[99999] = Poco::Path::current();
  }

  FileLoader::~FileLoader()
  {
    delete mInjaEnv;
  }

  std::string FileLoader::searchFile(const std::string& file) const
  {
    if(Poco::Path(file).isAbsolute()) {
      Poco::File f(file);
      return f.exists() ? file : "";
    }

    std::vector<std::string> scanned(mResourceSearchFolders.size());

    for(auto& rf : mResourceSearchFolders) {
      std::stringstream ss;
      ss << rf.second << GSAGE_PATH_SEPARATOR << file;
      std::string p = ss.str();
      Poco::File f(p);
      if(f.exists()) {
        return p;
      }
      scanned.push_back(rf.second);
    }

    LOG(ERROR) << "Failed to find file path in any of resource folders " << file << ":" << join(scanned, '\n');
    return "";
  }

  void FileLoader::addSearchFolder(int index, const std::string& path)
  {
    mResourceSearchFolders[index] = path;
  }

  bool FileLoader::removeSearchFolder(const std::string& path)
  {
    FileLoader::ResourceFolders::iterator it = std::find_if(
          mResourceSearchFolders.begin(),
          mResourceSearchFolders.end(),
          [path](const auto& mo) {return mo.second == path;
    });

    if (it == mResourceSearchFolders.end()) {
      return false;
    }

    mResourceSearchFolders.erase(it);
    return true;
  }

  bool FileLoader::load(const std::string& path, std::string& dest, std::ios_base::openmode mode) const
  {
    auto pair = loadFile(path, mode);
    if(!pair.second) {
      return false;
    }

    dest = pair.first;
    return true;
  }

  bool FileLoader::loadTemplate(const std::string& path, std::string& dest, const DataProxy& context)
  {
    std::string data;
    if(!load(path, data)) {
      LOG(ERROR) << "Failed to load file " << path;
      return false;
    }

    nlohmann::json ctx = jsonContext(context);
    try {
      dest = mInjaEnv->render(data, ctx);
    } catch (std::runtime_error err) {
      LOG(ERROR) << "Failed to render template " << err.what();
      return false;
    }
    return true;
  }

  void FileLoader::addTemplateCallback(const std::string& name, int argsNumber, sol::function function)
  {
    mInjaEnv->add_callback(name, argsNumber, [name, function] (inja::Arguments& args) {
      std::vector<sol::object> luaArgs;
      for(auto& a : args) {
        luaArgs.push_back(jsonToLuaObject(function.lua_state(), *a));
      }

      DataProxy result;
      try {
        sol::object res;
        res = function(sol::as_args(luaArgs));
        result = DataProxy::create(res);
      } catch(...) {
        LOG(ERROR) << "Failed to call filter function " << name;
      }

      inja::json res = jsonContext(result);
      return res;
    });
  }

  std::ifstream FileLoader::stream(const std::string& path) const
  {
    std::stringstream ss;
    ss << mEnvironment.get("workdir", ".") << GSAGE_PATH_SEPARATOR << path;
    return std::ifstream(ss.str());
  }

  bool FileLoader::load(const std::string& path, const DataProxy& params, DataProxy& dest) const
  {
    DataProxy p = merge(mEnvironment, params);
    auto pair = loadFile(path);
    if (!pair.second) {
      return false;
    }

    if(!parse(pair.first, dest))
      return false;

    mergeInto(dest, p);
    return true;
  }

  std::pair<DataProxy, bool> FileLoader::load(const std::string& path, const DataProxy& params) const
  {
    DataProxy res;
    bool success = load(path, params, res);
    return std::make_pair(res, success);
  }

  std::pair<DataProxy, bool> FileLoader::load(const std::string& path) const
  {
    return load(path, DataProxy());
  }

  void FileLoader::dump(const std::string& path, const DataProxy& value) const
  {
    DataWrapper::WrappedType type;
    switch(mFormat) {
      case Json:
        type = DataWrapper::JSON_OBJECT;
        break;
      case Msgpack:
        type = DataWrapper::MSGPACK_OBJECT;
        break;
    }
    std::string str = Gsage::dumps(value, type);
    dump(path, str);
  }

  bool FileLoader::dump(const std::string& path, const std::string& str, const std::string& rootDir) const
  {
    std::stringstream ss;
    if(!rootDir.empty() && !Poco::Path(path).isAbsolute()) {
      ss << rootDir << GSAGE_PATH_SEPARATOR;
    }
    ss << path;
    std::ofstream os(ss.str());
    if(!os)
      return false;

    os << str;
    os.close();
    return true;
  }

  std::pair<std::string, bool> FileLoader::loadFile(const std::string& path, std::ios_base::openmode mode) const
  {
    ScopedCLocale l(true);
    std::string fullPath;
    if(Poco::Path(path).isAbsolute()) {
      fullPath = path;
    } else {
      fullPath = searchFile(path);
    }

    if(fullPath.empty()) {
      LOG(ERROR) << "Failed to read file: " << path;
      return std::make_pair("", false);
    }

    std::string res;
    bool success = true;
    std::ifstream stream(fullPath, mode);

    try
    {
      stream.seekg(0, std::ios::end);
      res.reserve(stream.tellg());
      stream.seekg(0, std::ios::beg);
      res.assign((std::istreambuf_iterator<char>(stream)),
                  std::istreambuf_iterator<char>());
    }
    catch(std::exception& e)
    {
      success = false;
      LOG(ERROR) << "Failed to read file: " << fullPath << ", reason: " << e.what();
    }

    stream.close();

    return std::make_pair(res, success);
  }

  bool FileLoader::parse(const std::string& data, DataProxy& dest) const
  {
    bool success = false;
    DataWrapper::WrappedType type;

    switch(mFormat) {
      case Json:
        type = DataWrapper::JSON_OBJECT;
        break;
      case Msgpack:
        type = DataWrapper::MSGPACK_OBJECT;
        break;
    }
    return loads(dest, data, type);
  }

}
