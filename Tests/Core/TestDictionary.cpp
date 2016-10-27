
#include <gtest/gtest.h>
#include "Dictionary.h"

#include "DictionaryConverters.h"

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
  auto pair = parseMsgPack(str);
  ASSERT_TRUE(pair.second);
  mDictionary = pair.first;
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

// -----------------------------------------------------------------------------

class TestDictionaryConverters : public ::testing::Test
{

  public:
    void SetUp()
    {
      std::string jsonString = "{\"colour\": \"0x010F5030\", \"position\": \"-1.0, 100,31.123451\", \"rotation\": \"1.0,40.0,1.4,1\", \"degree\": 10.1, \"fr\": \"1.1,1.2,1.3,1.4\"}";
      auto pair = parseJson(jsonString);
      ASSERT_TRUE(pair.second);
      mDictionary = pair.first;
    }

    Dictionary mDictionary;
};

TEST_F(TestDictionaryConverters, TestVector3)
{
  auto pair = mDictionary.get<Ogre::Vector3>("position");
  ASSERT_TRUE(pair.second);
  Ogre::Vector3 res = pair.first;
  ASSERT_FLOAT_EQ(-1.0, res.x);
  ASSERT_FLOAT_EQ(100.0, res.y);
  ASSERT_FLOAT_EQ(31.123451, res.z);
  res = Ogre::Vector3(1.0, 0.1, 0.5);
  mDictionary.put("position_m", res);

  ASSERT_EQ(mDictionary.get<std::string>("position_m").first, "1,0.1,0.5");
}

TEST_F(TestDictionaryConverters, TestQuaternion)
{
  auto pair = mDictionary.get<Ogre::Quaternion>("rotation");
  ASSERT_TRUE(pair.second);
  Ogre::Quaternion res = pair.first;
  ASSERT_FLOAT_EQ(1.0, res.w);
  ASSERT_FLOAT_EQ(40.0, res.x);
  ASSERT_FLOAT_EQ(1.4, res.y);
  ASSERT_FLOAT_EQ(1.0, res.z);
  res = Ogre::Quaternion(1.0, 0.1, 0.5, 0);
  mDictionary.put("rotation_m", res);

  ASSERT_EQ(mDictionary.get<std::string>("rotation_m").first, "1,0.1,0.5,0");
}

TEST_F(TestDictionaryConverters, TestColourValue)
{
  auto pair = mDictionary.get<Ogre::ColourValue>("colour");
  ASSERT_TRUE(pair.second);
  Ogre::ColourValue res = pair.first;
  ASSERT_FLOAT_EQ(1.0 / 255.0, res.a);
  ASSERT_FLOAT_EQ(15.0 / 255.0, res.r);
  ASSERT_FLOAT_EQ(80.0 / 255.0, res.g);
  ASSERT_FLOAT_EQ(48.0 / 255.0, res.b);

  res.a = 0.4;
  res.r = 0.4;
  mDictionary.put("colour_modified", res);
  ASSERT_EQ("0x66665030", mDictionary.get<std::string>("colour_modified").first);
}

TEST_F(TestDictionaryConverters, TestDegree)
{
  Ogre::Degree res = mDictionary.get<Ogre::Degree>("degree").first;
  ASSERT_FLOAT_EQ(10.1, res.valueDegrees());
  res = Ogre::Degree(50.5);
  mDictionary.put("degree_m", res);

  ASSERT_EQ(mDictionary.get<std::string>("degree_m").first, "50.500000");
}

TEST_F(TestDictionaryConverters, TestFloatRect)
{
  Ogre::FloatRect res = mDictionary.get<Ogre::FloatRect>("fr").first;
  ASSERT_FLOAT_EQ(res.left, 1.1);
  ASSERT_FLOAT_EQ(res.top, 1.2);
  ASSERT_FLOAT_EQ(res.right, 1.3);
  ASSERT_FLOAT_EQ(res.bottom, 1.4);
  res.left = 1.5;
  mDictionary.put("fr_m", res);

  ASSERT_EQ(mDictionary.get<std::string>("fr_m").first, "1.5,1.2,1.3,1.4");
}
