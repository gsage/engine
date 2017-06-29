#include <gtest/gtest.h>
#include "FileLoader.h"
#include "TestDefinitions.h"

using namespace Gsage;

TEST(FileLoader, FileLoaderLoadAndDump)
{
  DataProxy environment;
  environment.put("envVariable", 123);

  std::map<FileLoader::Encoding, std::string> cases;
  cases[FileLoader::Json] = std::string(TEST_RESOURCES) + GSAGE_PATH_SEPARATOR + "templated.json";
  cases[FileLoader::Msgpack] = std::string(TEST_RESOURCES) + GSAGE_PATH_SEPARATOR + "templated.msgpack";

  for(auto p : cases)
  {
    FileLoader instance(p.first, environment);
    DataProxy params;
    params.put("fromParam", "works");
    auto pair = instance.load(p.second, params);
    EXPECT_TRUE(pair.second);
    EXPECT_EQ(pair.first.get<std::string>("fromParam", ""), "works");
    EXPECT_EQ(pair.first.get("envVariable", 0), 123);
    EXPECT_EQ(pair.first.get<std::string>("some", ""), "hello");

    instance.dump(p.second + ".dumped", pair.first);
    pair = instance.load(p.second, params);
    EXPECT_TRUE(pair.second);
    EXPECT_EQ(pair.first.get<std::string>("fromParam", ""), "works");
    EXPECT_EQ(pair.first.get("envVariable", 0), 123);
    EXPECT_EQ(pair.first.get<std::string>("some", ""), "hello");
  }
}
