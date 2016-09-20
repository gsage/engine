#include <gtest/gtest.h>
#include "FileLoader.h"

using namespace Gsage;

TEST(FileLoader, FileLoaderLoadAndDump)
{
  Dictionary environment;
  environment.put("test", 123);

  std::map<FileLoader::Encoding, std::string> cases;
  cases[FileLoader::Json] = "resources/templated.json";

  // Using ctemplate with msgpack is generally a bad idea
//  cases[FileLoader::Msgpack] = "resources/templated.msgpack";

  for(auto p : cases)
  {
    FileLoader instance(p.first, environment);
    Dictionary params;
    params.put("param1", "works");
    auto pair = instance.load(p.second, params);
    EXPECT_TRUE(pair.second);
    EXPECT_EQ(pair.first.get<std::string>("fromParam", ""), "works");
    EXPECT_EQ(pair.first.get("envVariable", 0), 123);
    EXPECT_EQ(pair.first.get<std::string>("some", ""), "hello");
  }
}

