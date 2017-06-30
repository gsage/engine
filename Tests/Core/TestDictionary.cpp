
#include <gtest/gtest.h>
#include "Dictionary.h"

using namespace Gsage;

struct TestStruct
{
  std::string variable;
};

namespace Gsage {
  // custom caster
  struct TestStructCaster
  {
    bool to(const std::string& str, TestStruct& dst) const
    {
      dst.variable = str;
      return true;
    }

    const std::string from(const TestStruct& value) const
    {
      return value.variable;
    }
  };

  template<>
  struct TranslatorBetween<std::string, TestStruct>
  {
    typedef TestStructCaster type;
  };
}

class TestJsonSerialization : public ::testing::Test
{

  public:
    void SetUp()
    {
      std::string jsonString = "{"
        "\"bool\": true,"
        "\"double\": 1.40123,"
        "\"int\": 1,"
        "\"nested\": {"
          "\"str\": \"abcd\","
          "\"c\": 100"
        "},"
        "\"a\": \"1\","
        "\"array\": ["
          "1, 2, {\"a\": 123}"
        "]"
      "}";
      auto pair = parseJson(jsonString);
      ASSERT_TRUE(pair.second);
      mDictionary = pair.first;
    }

    void check()
    {
      ASSERT_EQ(mDictionary.get<double>("double").first, 1.40123);
      ASSERT_EQ(mDictionary.get<std::string>("a").first, "1");
      ASSERT_EQ(mDictionary.get<int>("int").first, 1);
      ASSERT_EQ(mDictionary.get<Dictionary>("nested").first.get<int>("c").first, 100);
      ASSERT_EQ(mDictionary.get<Dictionary>("nested").first.get<std::string>("str").first, "abcd");
      ASSERT_EQ(mDictionary.get<bool>("bool").first, true);
      ASSERT_EQ(mDictionary.get<std::string>("nested.str").first, "abcd");
      ASSERT_EQ(mDictionary.get<std::string>("array.0").first, "1");
      ASSERT_EQ(mDictionary.get<int>("array.2.a").first, 123);
    }

    Dictionary mDictionary;
};

TEST(TestDictionary, TestUnion)
{
  Dictionary update;
  Dictionary base;
  base.put("existing", 1);
  base.put("overridden", "was not overridden");

  Dictionary list(true);
  list.put("1", 1);
  list.put("2", 2);
  list.put("3", 3);

  base.put("list", list);

  update.put("new_field", "new");

  Dictionary updatedList(true);
  Dictionary listNode;
  listNode.put("test", 123);

  updatedList.put("4", 4);
  updatedList.put("2", 10);
  updatedList.put("5", 5);
  updatedList.put("6", listNode);

  update.put("list", updatedList);
  update.put("overridden", "(y)");

  Dictionary res = getUnionDict(base, update);
  LOG(INFO) << dumpJson(res);
  ASSERT_EQ(res.get<std::string>("new_field").first, "new");
  ASSERT_EQ(res.get<std::string>("overridden").first, "(y)");
  ASSERT_EQ(res.get<int>("existing").first, 1);

  // check how lists were merged
  int i = 1;
  for(auto pair : res.get<Dictionary>("list").first)
  {
    if(i == 2) {
      EXPECT_EQ(pair.second.getValue<int>().first, 10);
    } else if (i < 6) {
      EXPECT_EQ(pair.second.getValue<int>().first, i);
    } else {
      EXPECT_EQ(pair.second.get<int>("test").first, 123);
    }
    i++;
  }
  EXPECT_EQ(res.get<Dictionary>("list").first.size(), 6);
}

TEST(TestDictionary, TestDictionaryOperations)
{
  Dictionary d;
  d.put("a", "b");
  d.put("test123", 1);

  Dictionary nested;
  nested.put("c", "d");
  d.put("nested", nested);

  d.put("nested.dictionary", nested);
  d.put("nested.value", 120);


  auto pair = d.get<std::string>("a");
  // checking success
  ASSERT_TRUE(pair.second);
  ASSERT_EQ(pair.first, "b");

  auto pair2 = d.get<int>("test123");
  ASSERT_TRUE(pair2.second);
  ASSERT_EQ(pair2.first, 1);

  auto pair3 = d.get<float>("test123");
  ASSERT_TRUE(pair3.second);
  ASSERT_EQ(pair3.first, 1.0);

  auto pair4 = d.get<Dictionary>("nested");
  ASSERT_TRUE(pair4.second);
  ASSERT_EQ(pair4.first.get<std::string>("c").first, "d");

  auto pair5 = d.get<TestStruct>("a");
  ASSERT_TRUE(pair5.second);
  ASSERT_EQ(pair5.first.variable, "b");

  auto pair6 = d.get<std::string>("nested.dictionary.c");
  ASSERT_TRUE(pair6.second);
  ASSERT_EQ(pair6.first, "d");

  pair2 = d.get<int>("nested.value");
  ASSERT_TRUE(pair2.second);
  ASSERT_EQ(pair2.first, 120);

  d.put("_nested.dictionary", nested);
  d.put("_nested.value", 120);

  pair6 = d.get<std::string>("_nested.dictionary.c");
  ASSERT_TRUE(pair6.second);
  ASSERT_EQ(pair6.first, "d");

  pair2 = d.get<int>("_nested.value");
  ASSERT_TRUE(pair2.second);
  ASSERT_EQ(pair2.first, 120);
}

TEST_F(TestJsonSerialization, TestIteration)
{
  std::string expectedValues[] = {
    "a",
    "array",
    "bool",
    "double",
    "int",
    "nested"
  };

  int index = 0;
  for(auto pair : mDictionary)
  {
    ASSERT_EQ(pair.first, expectedValues[index++]);
  }
  ASSERT_TRUE(index > 0);
}

TEST_F(TestJsonSerialization, TestCreateFromJson)
{
  check();
}

TEST_F(TestJsonSerialization, TestDumpToJson)
{
  std::string json = dumpJson(mDictionary);
  auto pair = parseJson(json);
  ASSERT_TRUE(pair.second);
  LOG(INFO) << json;
  mDictionary = pair.first;
  check();
}

TEST_F(TestJsonSerialization, TestMsgpack)
{
  std::string str = dumpMsgPack(mDictionary);
  std::pair<Dictionary, bool> pair;
  try{
    pair = parseMsgPack(str);
    ASSERT_TRUE(pair.second);
    mDictionary = pair.first;
  } catch(std::exception e) {
    LOG(INFO) << e.what();
  }
  check();
  // check file write + read
  std::ofstream os("resources/test.msgpack");
  ASSERT_TRUE(dumpMsgPack(os, mDictionary));
  os.close();

  std::ifstream is("resources/test.msgpack");

  pair = parseMsgPack(is);
  ASSERT_TRUE(pair.second);
  check();
}

class TestAccessPolicies : public ::testing::Test
{
  public:
    /**
     * Write to void* and then read
     */
    template<typename P, typename T>
    void* writeAndRead(T value)
    {
      detail::AccessPolicy* policy = detail::AccessPolicyFactory<P>::getPolicy();

      // copy
      policy->copyFromValue(value, &mValue);

      return policy->getValue(&mValue);
    }

    void* mValue;
};

TEST_F(TestAccessPolicies, TestConstChar)
{
  // create const char
  const char* c = "abcd";
  ASSERT_EQ(std::string(reinterpret_cast<const char*>(writeAndRead<const char*>(c))), "abcd");
}

TEST_F(TestAccessPolicies, TestArray)
{
  detail::AccessPolicy* policy = detail::AccessPolicyFactory<char[5]>::getPolicy();

  // copy
  policy->copyFromValue("abcd", &mValue);

  ASSERT_EQ(std::string(reinterpret_cast<const char*>(mValue)), "abcd");
}

TEST_F(TestAccessPolicies, TestInt)
{
  // create int variable
  int i = 1000;
  ASSERT_EQ(*reinterpret_cast<int*>(writeAndRead<int>(&i)), i);
}

TEST_F(TestAccessPolicies, TestBigAccessPolicy)
{
  // create string variable
  std::string s = "aaaa";
  ASSERT_EQ(*reinterpret_cast<std::string*>(writeAndRead<std::string>(&s)), s);
}

class Fake
{
  public:
    Fake(int a) : a(a) {}
    virtual ~Fake() {}
    int a;
};

TEST_F(TestAccessPolicies, TestPointers)
{
  // create string variable
  Fake* f = new Fake(1);
  detail::AccessPolicy* policy = detail::AccessPolicyFactory<Fake*>::getPolicy();

  // copy
  policy->copyFromValue(&f, &mValue);

  ASSERT_EQ(reinterpret_cast<Fake*>(mValue), f);
}
