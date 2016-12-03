#include "GsageFacade.h"
#include <gtest/gtest.h>
#include "EngineSystem.h"
#include "Component.h"
#include "ComponentStorage.h"
#include "Entity.h"

using namespace Gsage;

class TestGsageFacade : public ::testing::Test
{
  public:
    void SetUp()
    {
      mInstance = new GsageFacade();
#if GSAGE_PLATFORM == GSAGE_APPLE
      mPluginDirectory = "../PlugIns";
#else
      mPluginDirectory = "PlugIns";
#endif
    }

    void TearDown()
    {
      delete mInstance;
    }

    GsageFacade* mInstance;
    std::string mPluginDirectory;
};

/**
 * Test how facade handles plugin loading and unloading
 */
TEST_F(TestGsageFacade, TestPlugins)
{
  std::string input = mPluginDirectory + GSAGE_PATH_SEPARATOR + "Input";
  std::string empty = mPluginDirectory + GSAGE_PATH_SEPARATOR + "Empty";

  ASSERT_TRUE(mInstance->initialize("testConfig.json", RESOURCES_FOLDER));
  // Loading after initialize
  ASSERT_TRUE(mInstance->loadPlugin(empty));
  ASSERT_TRUE(mInstance->loadPlugin(input));
  ASSERT_TRUE(mInstance->update());
  // Unloading before shutdown
  mInstance->unloadPlugin(input);
}
