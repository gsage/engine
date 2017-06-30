#include "Serializable.h"
#include "GsageDefinitions.h"
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

    void setNode(const DataProxy& node)
    {
      this->node = node;
    }

    DataProxy getNode()
    {
      return node;
    }

    bool boolValue;
    float floatValue;
    int notFound;

    bool readByAccessor;

    int intValue;
    DataProxy node;
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
  DataProxy node;
  std::string ss =  "{\"boolValue\": true, \"floatValue\": 0.0001, \"intAccessor\": 1000, \"node\": {\"test\": 1}, \"forNested\":0}";

  ASSERT_TRUE(loads(node, ss, DataWrapper::JSON_OBJECT));
  ASSERT_FALSE(mInstance->read(node));

  ASSERT_EQ(mInstance->boolValue, true);
  ASSERT_FLOAT_EQ(mInstance->floatValue, 0.0001);
  ASSERT_EQ(mInstance->notFound, 100);
  ASSERT_EQ(mInstance->intValue, 1000);
  ASSERT_TRUE(mInstance->readByAccessor);
  ASSERT_EQ(mInstance->node.get<int>("test", -1), 1);

  ASSERT_EQ(mInstance->nested->value, 1);

  std::string s = "world";
  mInstance->node.put("hello", s);

  float newVal = 1.656;
  mInstance->floatValue = newVal;
  mInstance->dump(node);
  ASSERT_EQ(node.get<int>("notFound", -1), 100);
  ASSERT_FLOAT_EQ(node.get<float>("floatValue", -1.0), newVal);
  ASSERT_EQ(node.get<int>("intAccessor", -1), 0);

  ASSERT_EQ(node.get<std::string>("node.hello", "no value"), s);
}

TEST_F(TestSerializable, TestReadErrors)
{
  DataProxy node;
  std::string s = "{\"boolValue\": true, \"floatValue\": 0.0001, \"intAccessor\": 1000}";

  ASSERT_TRUE(loads(node, s, DataWrapper::JSON_OBJECT));
  ASSERT_FALSE(mInstance->read(node));

  s = "{\"boolValue\": true, \"notFound\": 404, \"floatValue\": 0.0001, \"intAccessor\": 1000, \"node\": {\"test\": 1}, \"forNested\":0}";

  ASSERT_TRUE(loads(node, s, DataWrapper::JSON_OBJECT));
  ASSERT_TRUE(mInstance->read(node));
}
