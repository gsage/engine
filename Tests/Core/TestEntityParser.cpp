#include "JsonEntityParser.h"
#include <stdexcept>
#include <gtest/gtest.h>

using namespace Gsage;

class TestJsonEntityParser : public ::testing::Test
{

  public:
    void SetUp()
    {
      mInstance = new JsonEntityParser();
    }

    void TearDown()
    {
      delete mInstance;
    }

    JsonEntityParser* mInstance;
};


TEST_F(TestJsonEntityParser, TestDecodeJson)
{
  JsonEntityParser::Entities entities;
  ASSERT_EQ(mInstance->load("resources/entities.json", entities), 0);

  ASSERT_EQ(entities.size(), 2);

  EntityData& e = entities[0];

  ASSERT_FALSE(e.mData.count("testComponent") == 0);
  const boost::property_tree::ptree& c = e.mData["testComponent"];

  ASSERT_EQ(c.get<bool>("booleanValue"), false);
  ASSERT_EQ(c.get<int>("intValue"), 1);
  ASSERT_EQ(c.get<double>("doubleValue"), 2.56);
  ASSERT_EQ(c.get<std::string>("stringValue"), "Hello");

  std::string stringValue = c.get_child("nestedComponent").get<std::string>("stringValue");
  ASSERT_EQ(stringValue, "World");
}

TEST_F(TestJsonEntityParser, TestFailures)
{
  EntityParser::Entities entities;
  int res;

  std::stringstream badFormat("{");
  std::stringstream emptyJson("{}");

  res = mInstance->decode(badFormat, entities);
  ASSERT_EQ(res, EntityParser::BAD_JSON_EXCEPTION);
  ASSERT_EQ(entities.size(), 0);

  res = mInstance->load("I_AM_NO_MORE", entities);
  ASSERT_EQ(res, EntityParser::FILE_NOT_FOUND);
  ASSERT_EQ(entities.size(), 0);

  res = mInstance->decode(emptyJson, entities);
  ASSERT_EQ(res, EntityParser::FORMAT_EXCEPTION);
  ASSERT_EQ(entities.size(), 0);
}

