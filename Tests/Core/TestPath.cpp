#include <gtest/gtest.h>
#include "Path.h"
#include "DataProxy.h"

using namespace Gsage;

TEST(TestPath, TestSimple) {
  Path3D::Vector points;
  points.push_back(Vector3(0.0f, 1.0f, 2.0f));
  points.push_back(Vector3(0.0f, 5.0f, 20.0f));
  points.push_back(Vector3(0.0f, 30.0f, 50.0f));
  points.push_back(Vector3(0.0f, 10.0f, 10.0f));

  Path3D path(points);

  Gsage::Vector3* point = nullptr;
  int index = 0;
  while(path.next()) {
    point = path.current();
    ASSERT_EQ(*point, points[index++]);
  }

  point = path.current();
  ASSERT_EQ(point, nullptr);

  Path3D::Vector updatedPoints;
  updatedPoints.push_back(Vector3(3.0f, 1.0f, 2.0f));
  updatedPoints.push_back(Vector3(5.0f, 5.0f, 20.0f));
  updatedPoints.push_back(Vector3(6.0f, 30.0f, 50.0f));
  updatedPoints.push_back(Vector3(8.0f, 10.0f, 10.0f));
  updatedPoints.push_back(Vector3(10.0f, 10.0f, 10.0f));

  // now try same, but update the path in the middle
  path.reset();
  index = 0;
  bool updated = false;
  while(path.next()) {
    if(index == 2 && !updated) {
      path.update(updatedPoints);
      index = 0;
      updated = true;
    }

    point = path.current();
    ASSERT_EQ(*point, (updated ? updatedPoints : points)[index++]);
  }
}

TEST(TestPath, TestDumpAndLoad) {
  DataProxy data = DataProxy::create(DataWrapper::JSON_OBJECT);

  Path3D::Vector points;
  points.push_back(Vector3(0.0f, 1.0f, 2.0f));
  points.push_back(Vector3(0.0f, 5.0f, 20.0f));
  points.push_back(Vector3(0.0f, 30.0f, 50.0f));
  points.push_back(Vector3(0.0f, 10.0f, 10.0f));

  Path3D path(points);

  ASSERT_TRUE(path.dump(data));

  std::string json = dumps(data, DataWrapper::JSON_OBJECT);

  ASSERT_EQ(json, "[\"0,1,2\",\"0,5,20\",\"0,30,50\",\"0,10,10\"]\n");

  Path3D restored;
  ASSERT_TRUE(restored.read(data));

  while(restored.next() && path.next()) {
    ASSERT_EQ(*restored.current(), *path.current());
  }

  // now check malformed elements
  json = "[\"0,1,2\",\"0,5,20\",\"0,50\",\"0,10,30\"]\n";

  auto malformed = loads(json, DataWrapper::JSON_OBJECT);

  ASSERT_TRUE(restored.read(malformed));

  int index = 0;
  while(restored.next()) {
    Gsage::Vector3 expected = malformed[(index > 1 ? index+1 : index)].as<Gsage::Vector3>();
    ASSERT_EQ(*restored.current(), expected);
    index++;
  }
  ASSERT_EQ(index, 3);
}
