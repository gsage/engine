#include <gtest/gtest.h>
#include "Dictionary.h"

#include "DictionaryConverters.h"

using namespace Gsage;

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

