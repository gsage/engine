#include "Serializable.h"
#include "GsageDefinitions.h"
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

#include <gtest/gtest.h>

using namespace Gsage;

class StubSerializable : public Serializable<StubSerializable>
{
  public:
    class NestedSerializable
    {
      public:
        NestedSerializable() : value(-1) {};
        int setValue(const int& v)
        {
          return value = v + 1;
        }

        int getValue()
        {
          return value;
        }
        int value;
    };

    StubSerializable() :
      boolValue(false),
      floatValue(-1),
      notFound(100),
      readByAccessor(false),
      intValue(-1)
    {
      nested = new NestedSerializable();
      BIND_PROPERTY("boolValue", &boolValue);
      BIND_PROPERTY("floatValue", &floatValue);
      BIND_PROPERTY("notFound", &notFound);

      BIND_ACCESSOR("intAccessor", &StubSerializable::setInt, &StubSerializable::getInt);

      BIND_ACCESSOR("node", &StubSerializable::setNode, &StubSerializable::getNode);

      registerProperty("forNested", nested, &NestedSerializable::setValue, &NestedSerializable::getValue);

      BIND_ACCESSOR_OPTIONAL("optionalOne", &StubSerializable::setInt, &StubSerializable::getInt);
    }

    virtual ~StubSerializable()
    {
      delete nested;
    }

    void setInt(const int& value)
    {
      readByAccessor = true;
      intValue = value;
    }

    int getInt()
    {
      return 0;
    }

    void setNode(const DataNode& node)
    {
      this->node = node;
    }

    DataNode getNode()
    {
      return node;
    }

    bool boolValue;
    float floatValue;
    int notFound;

    bool readByAccessor;

    int intValue;
    DataNode node;
    NestedSerializable* nested;

};

class TestSerializable : public ::testing::Test
{

  public:
    void SetUp()
    {
      mInstance = new StubSerializable();
    }

    void TearDown()
    {
      delete mInstance;
    }

    StubSerializable* mInstance;
};

TEST_F(TestSerializable, TestSetProperty)
{
  DataNode node;
  std::stringstream ss;
  ss << "{\"boolValue\": true, \"floatValue\": 0.0001, \"intAccessor\": 1000, \"node\": {\"test\": 1}, \"forNested\":0}";

  boost::property_tree::read_json(ss, node);
  ASSERT_FALSE(mInstance->read(node));

  ASSERT_EQ(mInstance->boolValue, true);
  ASSERT_FLOAT_EQ(mInstance->floatValue, 0.0001);
  ASSERT_EQ(mInstance->notFound, 100);
  ASSERT_EQ(mInstance->intValue, 1000);
  ASSERT_TRUE(mInstance->readByAccessor);
  ASSERT_EQ(mInstance->node.get<int>("test"), 1);

  ASSERT_EQ(mInstance->nested->value, 1);

  std::string s = "world";
  mInstance->node.put("hello", s);

  float newVal = 1.656;
  mInstance->floatValue = newVal;
  mInstance->dump(node);
  ASSERT_EQ(node.get<int>("notFound"), 100);
  ASSERT_FLOAT_EQ(node.get<float>("floatValue"), newVal);
  ASSERT_EQ(node.get<int>("intAccessor"), 0);

  ASSERT_EQ(node.get<std::string>("node.hello"), s);
}

TEST_F(TestSerializable, TestReadErrors)
{
  DataNode node;
  std::stringstream ss;
  ss << "{\"boolValue\": true, \"floatValue\": 0.0001, \"intAccessor\": 1000}";

  boost::property_tree::read_json(ss, node);
  ASSERT_FALSE(mInstance->read(node));

  ss.clear();
  ss << "{\"boolValue\": true, \"notFound\": 404, \"floatValue\": 0.0001, \"intAccessor\": 1000, \"node\": {\"test\": 1}, \"forNested\":0}";

  boost::property_tree::read_json(ss, node);
  ASSERT_TRUE(mInstance->read(node));
}
