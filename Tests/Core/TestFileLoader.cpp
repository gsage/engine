#include <gtest/gtest.h>
#include "FileLoader.h"

using namespace Gsage;

TEST(FileLoader, FileLoaderLoadAndDump)
{
  Dictionary environment;
  environment.put("envVariable", 123);

  std::map<FileLoader::Encoding, std::string> cases;
  cases[FileLoader::Json] = "resources/templated.json";
  cases[FileLoader::Msgpack] = "resources/templated.msgpack";

  for(auto p : cases)
  {
    FileLoader instance(p.first, environment);
    Dictionary params;
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
