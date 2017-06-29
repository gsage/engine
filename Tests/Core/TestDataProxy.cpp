#include "GsageDefinitions.h"
#include <sstream>
#include "DataProxy.h"

#include <json/json.h>
#include <gtest/gtest.h>
#include "sol.hpp"

using namespace Gsage;

class CustomObject
{
  public:
    std::string variable;
};

class TestDataProxy : public ::testing::Test
{

  public:
    TestDataProxy() {
      lua.new_usertype<CustomObject>("CustomObject",
        "variable", &CustomObject::variable
      );
      lua.open_libraries(sol::lib::base, sol::lib::package);
    }
    sol::state lua;
};

namespace Gsage {
  // custom caster
  struct CustomObjectCaster
  {
    bool to(const std::string& str, CustomObject& dst) const
    {
      dst.variable = str;
      return true;
    }

    const std::string from(const CustomObject& value) const
    {
      return value.variable;
    }
  };

  template<>
  struct TranslatorBetween<std::string, CustomObject>
  {
    typedef CustomObjectCaster type;
  };
}

TEST_F(TestDataProxy, TestLuaObject)
{
  sol::table t = lua.create_table();
  t["string"] = "abcd";
  t["int"] = 1;
  t["double"] = 1.1;
  t["custom"] = "builds.from.string";
  sol::optional<std::string> s = t["string"];

  DataProxy dw = DataProxy::create(t);

  std::string strv;
  int intv;
  double doublev;
  CustomObject obj;

  ASSERT_TRUE(dw.read("string", strv));
  ASSERT_TRUE(dw.read("int", intv));
  ASSERT_TRUE(dw.read("double", doublev));
  ASSERT_TRUE(dw.read("custom", obj));

  ASSERT_EQ(strv, "abcd");
  ASSERT_EQ(intv, 1);
  ASSERT_DOUBLE_EQ(doublev, 1.1);
  ASSERT_EQ(obj.variable, "builds.from.string");

  auto pair = dw.get<DataProxy>("string");
  ASSERT_TRUE(pair.second);
  ASSERT_EQ(pair.first.getValue<std::string>().first, strv);

  dw.put("the.key", "works");

  ASSERT_FALSE(dw.get<std::string>("no.such.key").second);

  auto traversedGet = dw.get<std::string>("the.key");
  ASSERT_TRUE(traversedGet.second);
  ASSERT_EQ(traversedGet.first, "works");

  ASSERT_EQ(dw.get<DataProxy>("the").first.get<std::string>("key").first, "works");
  ASSERT_FALSE(dw.get<std::string>("nokey").second);
  ASSERT_FALSE(dw.get<DataProxy>("nokey").second);

  dw.put("object_raw", obj);
  lua["test_table"] = t;

  CustomObject objFromRaw;
  ASSERT_TRUE(dw.read("object_raw", objFromRaw));
  ASSERT_EQ(objFromRaw.variable, obj.variable);

  sol::protected_function_result res = lua.do_string("local v = test_table.object_raw.variable; test_table.object_raw.variable = 'it works!'; return v");
  ASSERT_TRUE(res.valid());
  std::string value = res;
  ASSERT_EQ(value, obj.variable);

  ASSERT_TRUE(dw.read("object_raw", objFromRaw));
  ASSERT_EQ(objFromRaw.variable, "it works!");

  ASSERT_EQ(dw.count("object_raw"), 1);
  ASSERT_EQ(dw.count("nokey"), 0);

  for(auto pair : dw) {
    if(pair.first == "object_raw") {
      CustomObject objectRawIter;
      pair.second.read(objectRawIter);
    } else {
      std::string actual_value;
      sol::object object = t[pair.first];
      if(object == sol::lua_nil) {
        FAIL();
      }

      std::string expected_value = object.as<std::string>();
      ASSERT_TRUE(pair.second.read(actual_value));
      ASSERT_EQ(actual_value, expected_value);
    }
  }
}

TEST_F(TestDataProxy, TestJsonObject)
{
  Json::Value v;
  v["string"] = "abcd";
  v["int"] = 1;
  v["double"] = 1.1;
  v["stringint"] = "100";

  DataProxy dw = DataProxy::create(v);
  std::string strv;
  int intv;
  double doublev;

  ASSERT_TRUE(dw.read("string", strv));
  ASSERT_TRUE(dw.read("int", intv));
  ASSERT_TRUE(dw.read("double", doublev));

  ASSERT_EQ(strv, "abcd");
  ASSERT_EQ(intv, 1);
  ASSERT_DOUBLE_EQ(doublev, 1.1);
  ASSERT_EQ(dw.get("nokey", "default"), "default");

  // truncate double into int
  ASSERT_TRUE(dw.read("double", intv));
  ASSERT_EQ(intv, 1);

  // read int into double
  ASSERT_TRUE(dw.read("int", doublev));
  ASSERT_DOUBLE_EQ(doublev, 1.0);

  // parse int from string
  ASSERT_TRUE(dw.read("stringint", intv));
  ASSERT_EQ(intv, 100);

  // parse int from string which can not be parsed should fail
  ASSERT_FALSE(dw.read("string", intv));
  ASSERT_EQ(intv, 100);

  CustomObject obj;
  // parse into custom object
  ASSERT_TRUE(dw.read("string", obj));
  ASSERT_EQ(obj.variable, "abcd");

  // put tests
  // rewrite string
  dw.put("string", "rewrite");
  ASSERT_TRUE(dw.read("string", strv));
  ASSERT_EQ(strv, "rewrite");

  dw.put("complex", obj);
  ASSERT_TRUE(dw.read("complex", obj));
  ASSERT_EQ(obj.variable, "abcd");

  ASSERT_EQ(dw.size(), 5);
  ASSERT_EQ(dw.count("complex"), 1);
  ASSERT_EQ(dw.count("nokey"), 0);

  Json::Value result;
  ASSERT_TRUE(dw.dump(result));

  int count = 0;
  for(auto pair : dw) {
    count++;
    ASSERT_FALSE(result[pair.first].isNull());

    if(pair.first == "double") {
      double actualDouble;
      ASSERT_TRUE(pair.second.read(actualDouble));
      ASSERT_DOUBLE_EQ(actualDouble, result[pair.first].asDouble());
    } else if (pair.first == "int") {
      int actualInt;
      ASSERT_TRUE(pair.second.read(actualInt));
      ASSERT_EQ(actualInt, result[pair.first].asInt());
    } else if (pair.first == "stringint") {
      int actualIntParse;
      ASSERT_TRUE(pair.second.read(actualIntParse));
      ASSERT_EQ(actualIntParse, std::stoi(result[pair.first].asString()));
    } else if(pair.first == "string") {
      std::string actualString;
      ASSERT_TRUE(pair.second.read(actualString));
      ASSERT_EQ(actualString, result[pair.first].asString());
    }
  }

  ASSERT_EQ(count, dw.size());

  // checking const iteration
  const DataProxy dp = DataProxy::wrap(result);
  for(auto pair : dw) {
    ASSERT_FALSE(result[pair.first].isNull());
  }

  // checking set
  Json::Value testSet;
  DataProxy setDp = DataProxy::wrap(testSet);
  setDp.set("test");

  ASSERT_EQ(testSet.asString(), "test");

  // checking array
  Json::Value array;
  DataProxy arrayDp = DataProxy::wrap(array);
  arrayDp.push("jep");
  arrayDp.push(1);
  arrayDp.push(obj);

  Json::FastWriter writer;
  ASSERT_EQ(writer.write(array), "[\"jep\",1,\"abcd\"]\n");
}

TEST_F(TestDataProxy, TestNestedDataProxy)
{
  // create lua table and put it as child to the json wrapped dp
  sol::table t = lua.create_table("nestedTest");
  t["string"] = "abcd";
  t["int"] = 1;
  t["double"] = 1.1;
  t["custom"] = "builds.from.string";

  DataProxy l = DataProxy::create(t);
  DataProxy js = DataProxy::create(DataWrapper::JSON_OBJECT);
  js.put("nested", l);
  Json::FastWriter writer;

  auto pair = js.get<DataProxy>("nested");
  ASSERT_TRUE(pair.second);

  ASSERT_EQ(pair.first.get<std::string>("string").first, "abcd");

  l.put("moreLevels", js);

  DataProxy nestedJson = DataProxy::create(DataWrapper::JSON_OBJECT);
  DataProxy nestedLua = DataProxy::create(lua.create_table());
  nestedJson.put("int.value", 1);
  nestedLua.put("thestring", "lua_works");

  DataProxy array = l.createChild("array");

  // push lua table to lua
  array.push(nestedJson);
  array.push(nestedLua);

  std::vector<std::pair<std::string, std::string>> cases{
    std::make_pair("return type(nestedTest.moreLevels.nested.int)", "number"),
    std::make_pair("return nestedTest.moreLevels.nested.int", "1"),
    std::make_pair("return nestedTest.array[1].int.value", "1"),
    std::make_pair("return nestedTest.array[2].thestring", "lua_works")
  };

  for (auto pair : cases) {
    sol::protected_function_result res = lua.do_string(pair.first);
    ASSERT_TRUE(res.valid());
    std::string type_info = res;
    ASSERT_EQ(type_info, pair.second);
  }

}

TEST_F(TestDataProxy, TestDump)
{
  sol::table t = lua.create_table("fromJson");
  Json::Value jsonValue;

  jsonValue["string"] = "abcd";
  jsonValue["int"] = 1;
  jsonValue["double"] = 1.1;
  jsonValue["bool"] = false;

  Json::Value nestedJson;

  nestedJson["int"] = 20;
  nestedJson["string"] = "hi";

  jsonValue["nested"] = nestedJson;

  jsonValue["array"] = Json::Value();

  jsonValue["array"][0] = 1;
  jsonValue["array"][1] = 5;

  DataProxy dpLua = DataProxy::create(t);
  DataProxy dpJson = DataProxy::create(jsonValue);

  // json to lua
  ASSERT_TRUE(dpJson.dump(dpLua));

  std::vector<std::pair<std::string, std::string>> cases{
    std::make_pair("return type(fromJson.int)", "number"),
    std::make_pair("return type(fromJson.string)", "string"),
    std::make_pair("return type(fromJson.double)", "number"),
    std::make_pair("return type(fromJson.bool)", "boolean"),
    std::make_pair("return type(fromJson.nested.int)", "number"),
    std::make_pair("return type(fromJson.nested.string)", "string"),
    std::make_pair("return fromJson.nested.string", "hi"),
    std::make_pair("return type(fromJson.array[1])", "number")
  };

  for (auto pair : cases) {
    sol::protected_function_result res = lua.do_string(pair.first);
    ASSERT_TRUE(res.valid());
    std::string type_info = res;
    ASSERT_EQ(type_info, pair.second);
  }

  Json::Value fromLuaJson;
  DataProxy fromLua = DataProxy::create(fromLuaJson);

  // lua to json
  ASSERT_TRUE(dpLua.dump(fromLua));

  Json::FastWriter writer;
  Json::Value result;
  fromLua.dump(result);

  LOG(INFO) << "From lua result:\n\t" << writer.write(result);

  std::string stringResult;
  ASSERT_TRUE(fromLua.read("string", stringResult));
  ASSERT_EQ(stringResult, jsonValue["string"].asString());
  int intResult;
  ASSERT_TRUE(fromLua.read("int", intResult));

  ASSERT_EQ(fromLua.size(), 6);
}

class TestSerialization : public TestDataProxy,
                          public ::testing::WithParamInterface<DataWrapper::WrappedType> {
};

TEST_P(TestSerialization, TestDumpRead)
{
  sol::table t = lua.create_table();
  t["string"] = "abcd";
  t["int"] = 1;
  t["double"] = 1.1;

  DataProxy l = DataProxy::wrap(t);
  DataProxy nested = l.createChild("nested");
  nested.push(1);
  nested.push(2);

  DataWrapper::WrappedType type = GetParam();

  std::string data = dumps(l, type);
  DataProxy read = loads(data, type);

  auto validate = [] (DataProxy& read) {
    ASSERT_EQ(read["string"].as<std::string>(), "abcd");
    ASSERT_EQ(read["int"].as<int>(), 1);
    ASSERT_FLOAT_EQ(read["double"].as<double>(), 1.1);
    ASSERT_EQ(read["nested"][0].as<int>(), 1);
    ASSERT_EQ(read["nested"][1].as<int>(), 2);
  };
  LOG(INFO) << dumps(read, DataWrapper::JSON_OBJECT);

  validate(read);

  data = dumps(read, type);
  read = loads(data, type);

  validate(read);
}

INSTANTIATE_TEST_CASE_P(TestDumpRead,
                        TestSerialization,
                        ::testing::Values(DataWrapper::JSON_OBJECT, DataWrapper::MSGPACK_OBJECT));

class TestMerge : public TestDataProxy,
                  public ::testing::WithParamInterface<std::tuple<DataWrapper::WrappedType, DataWrapper::WrappedType>> {
};

TEST_P(TestMerge, TestMergeOperations)
{
  DataWrapper::WrappedType typeBase;
  DataWrapper::WrappedType typeUpdate;

  std::tie(typeBase, typeUpdate) = GetParam();

  auto createDataProxy = [&] (DataWrapper::WrappedType value) -> DataProxy {
    switch(value) {
      case DataWrapper::LUA_TABLE:
        return DataProxy::create(lua.create_table());
        break;
      case DataWrapper::JSON_OBJECT:
        return DataProxy::create(value);
        break;
      default:
        return DataProxy::create(DataWrapper::JSON_OBJECT);
    }
  };

  DataProxy update = createDataProxy(typeBase);
  DataProxy base = createDataProxy(typeUpdate);
  base.put("existing", 1);
  base.put("overridden", "was not overridden");

  DataProxy list = createDataProxy(typeBase);
  DataProxy n = createDataProxy(typeBase);
  n.put("test", 1.1);
  n.put("exists", 2);
  list.push(1);
  list.push(2);
  list.push(3);
  list.push(4);
  list.push(n);
  list.push(6);
  list.push(7);

  base.put("list", list);

  update.put("new_field", "new");

  DataProxy updatedList = createDataProxy(typeUpdate);
  DataProxy listNode = createDataProxy(typeUpdate);
  listNode.put("test", 123);
  listNode.put("new", "works");

  updatedList.push(4);
  updatedList.push(10);
  updatedList.push(5);
  updatedList.push(listNode);
  updatedList.push(listNode);

  update.put("list", updatedList);
  update.put("overridden", "(y)");

  DataProxy res = merge(base, update);
  LOG(INFO) << dumps(res, DataWrapper::JSON_OBJECT);
  ASSERT_EQ(res["new_field"].as<std::string>(), "new");
  ASSERT_EQ(res["overridden"].as<std::string>(), "(y)");
  ASSERT_EQ(res["existing"].as<int>(), 1);

  // check how lists were merged
  int i = 0;
  for(auto pair : res["list"])
  {
    if(i < updatedList.size() - 2) {
      EXPECT_EQ(pair.second.getValue<int>().first, updatedList[i].as<int>());
    } else if (i == 3) {
      EXPECT_EQ(pair.second["test"].as<int>(), 123);
    } else if (i == 4) {
      EXPECT_EQ(pair.second["test"].as<int>(), 123);
      EXPECT_EQ(pair.second["new"].as<std::string>(), "works");
      EXPECT_EQ(pair.second["exists"].as<int>(), 2);
    } else {
      EXPECT_EQ(pair.second.as<int>(), list[i].as<int>());
    }
    i++;
  }
  EXPECT_EQ(res.get<DataProxy>("list").first.size(), list.size());
}

INSTANTIATE_TEST_CASE_P(TestMergeOperations,
                        TestMerge,
                        ::testing::Values(
                          std::make_tuple(DataWrapper::JSON_OBJECT, DataWrapper::JSON_OBJECT),
                          std::make_tuple(DataWrapper::LUA_TABLE, DataWrapper::LUA_TABLE),
                          std::make_tuple(DataWrapper::JSON_OBJECT, DataWrapper::LUA_TABLE),
                          std::make_tuple(DataWrapper::LUA_TABLE, DataWrapper::JSON_OBJECT)
                        ));
