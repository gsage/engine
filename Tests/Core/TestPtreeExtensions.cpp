
#include <sstream>
#include <gtest/gtest.h>
#include <boost/property_tree/json_parser.hpp>
#include "PtreeExtensions.h"
#include "Logger.h"

class TestPtreeExtensions : public ::testing::Test
{

  public:
    void SetUp()
    {
      std::stringstream stream;
      stream << "{\"colour\": \"0x010F5030\", \"position\": \"-1.0, 100,31.123451\", \"rotation\": \"1.0,40.0,1.4,1\", \"degree\": 10.1, \"fr\": \"1.1,1.2,1.3,1.4\"}";
      boost::property_tree::read_json(stream, mPtree);
    }

    boost::property_tree::ptree mPtree;
};

TEST_F(TestPtreeExtensions, TestVector3)
{
  Ogre::Vector3 res = mPtree.get<Ogre::Vector3>("position");
  ASSERT_FLOAT_EQ(-1.0, res.x);
  ASSERT_FLOAT_EQ(100.0, res.y);
  ASSERT_FLOAT_EQ(31.123451, res.z);
  res = Ogre::Vector3(1.0, 0.1, 0.5);
  mPtree.put("position_m", res);

  ASSERT_EQ(mPtree.get<std::string>("position_m"), "1,0.1,0.5");
}

TEST_F(TestPtreeExtensions, TestQuaternion)
{
  Ogre::Quaternion res = mPtree.get<Ogre::Quaternion>("rotation");
  ASSERT_FLOAT_EQ(1.0, res.w);
  ASSERT_FLOAT_EQ(40.0, res.x);
  ASSERT_FLOAT_EQ(1.4, res.y);
  ASSERT_FLOAT_EQ(1.0, res.z);
  res = Ogre::Quaternion(1.0, 0.1, 0.5, 0);
  mPtree.put("rotation_m", res);

  ASSERT_EQ(mPtree.get<std::string>("rotation_m"), "1,0.1,0.5,0");
}

TEST_F(TestPtreeExtensions, TestColourValue)
{
  Ogre::ColourValue res = mPtree.get<Ogre::ColourValue>("colour");
  ASSERT_FLOAT_EQ(1.0 / 255.0, res.a);
  ASSERT_FLOAT_EQ(15.0 / 255.0, res.r);
  ASSERT_FLOAT_EQ(80.0 / 255.0, res.g);
  ASSERT_FLOAT_EQ(48.0 / 255.0, res.b);

  res.a = 0.4;
  res.r = 0.4;
  res.g = 0.4;
  res.b = 0.4;
  mPtree.put("colour_modified", res);
  ASSERT_EQ("0x66666666", mPtree.get<std::string>("colour_modified"));
}

TEST_F(TestPtreeExtensions, TestDegree)
{
  Ogre::Degree res = mPtree.get<Ogre::Degree>("degree");
  ASSERT_FLOAT_EQ(10.1, res.valueDegrees());
  res = Ogre::Degree(50.5);
  mPtree.put("degree_m", res);

  ASSERT_EQ(mPtree.get<std::string>("degree_m"), "50.5");
}

TEST_F(TestPtreeExtensions, TestFloatRect)
{
  Ogre::FloatRect res = mPtree.get<Ogre::FloatRect>("fr");
  ASSERT_FLOAT_EQ(res.left, 1.1);
  ASSERT_FLOAT_EQ(res.top, 1.2);
  ASSERT_FLOAT_EQ(res.right, 1.3);
  ASSERT_FLOAT_EQ(res.bottom, 1.4);
  res.left = 1.5;
  mPtree.put("fr_m", res);

  ASSERT_EQ(mPtree.get<std::string>("fr_m"), "1.5,1.2,1.3,1.4");
}

TEST(TestPtreeUtils, TestMerge)
{
  DataNode update;
  DataNode base;
  base.put("existing", 1);
  base.put("overridden", "was not overridden");

  DataNode list;
  list.push_back(std::make_pair("", "1"));
  list.push_back(std::make_pair("", "2"));
  list.push_back(std::make_pair("", "3"));

  base.put_child("list", list);

  update.put("new_field", "new");

  DataNode updatedList;
  DataNode listNode;
  listNode.put("test", 123);

  updatedList.push_back(std::make_pair("", "4"));
  updatedList.push_back(std::make_pair("", listNode));

  update.put_child("list", updatedList);
  update.put("overridden", "(y)");

  DataNode res = getMergedDataNode(base, update);
  ASSERT_EQ(res.get<std::string>("new_field"), "new");
  ASSERT_EQ(res.get<std::string>("overridden"), "(y)");
  ASSERT_EQ(res.get<int>("existing"), 1);

  // check how lists were merged
  int i = 1;
  for(auto pair : res.get_child("list"))
  {
    ASSERT_EQ(pair.first, "");
    if(i < 5)
      ASSERT_EQ(pair.second.get_value<int>(), i);
    else
      ASSERT_EQ(pair.second.get<int>("test"), 123);
    i++;
  }
  ASSERT_EQ(res.get_child("list").size(), 5);
}
