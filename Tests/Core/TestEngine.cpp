#include "Engine.h"
#include <gtest/gtest.h>
#include "EngineSystem.h"
#include "Component.h"
#include "ComponentStorage.h"
#include "Entity.h"

using namespace Gsage;

class SpeedComponent : public EntityComponent
{
  public:
    SpeedComponent()
    {
    }

    virtual ~SpeedComponent()
    {
    }

    double value;
};

class AccelerationComponent : public EntityComponent
{
  public:
    AccelerationComponent()
    {
    }

    virtual ~AccelerationComponent()
    {
    }

    double value;
};

class AccelerationSystem : public ComponentStorage<AccelerationComponent>
{
  public:

    void updateComponent(AccelerationComponent* component, Entity* entity, const double& time)
    {

    }

    bool fillComponentData(AccelerationComponent* c, const DataProxy& data)
    {
      c->value = data.get<double>("acceleration").first;
      return true;
    }
};

class TestSystem : public ComponentStorage<SpeedComponent>
{
  public:
    TestSystem()
    {
    }

    void updateComponent(SpeedComponent* component, Entity* entity, const double& time)
    {
      component->value *= mEngine->getComponent<AccelerationComponent>(*entity, "accelerator")->value;
    }

    bool fillComponentData(SpeedComponent* c, const DataProxy& data)
    {
      c->value = data.get<double>("speed").first;
      return true;
    }
};

class TestEngine : public ::testing::Test
{
  public:
    void SetUp()
    {
      mInstance = new Engine();
    }

    void TearDown()
    {
      delete mInstance;
    }

    Engine* mInstance;
};

TEST_F(TestEngine, TestAddSystem)
{
  TestSystem system;
  mInstance->addSystem("speed", &system);
  mInstance->addSystem("accelerator", new AccelerationSystem());
  DataProxy entityData;
  DataProxy speed;
  DataProxy accelerator;
  speed.put("speed", 2.0);
  accelerator.put("acceleration", 1.5);

  entityData.put("id", "test");
  entityData.put("speed", speed);
  entityData.put("accelerator", accelerator);

  DataProxy entityData2;
  entityData2.put("id", "test2");
  entityData2.put("speed", speed);
  entityData2.put("accelerator", accelerator);

  Entity* e = mInstance->createEntity(entityData);
  Entity* e2 = mInstance->createEntity(entityData2);
  SpeedComponent* c = mInstance->getComponent<SpeedComponent>(*e, "speed");
  ASSERT_FALSE(mInstance->getEntity("test") == NULL);

  ASSERT_EQ(c->getOwner()->getId(), "test");

  ASSERT_EQ(c->value, 2.0);

  mInstance->update(1);

  ASSERT_EQ(c->value, 3.0);

  AccelerationComponent* c2 = mInstance->getComponent<AccelerationComponent>(*e2, "accelerator");
  ASSERT_EQ(c2->getOwner()->getId(), "test2");

  ASSERT_TRUE(mInstance->removeEntity(e));
  ASSERT_TRUE(mInstance->removeEntity(e2));

  ASSERT_EQ(NULL, mInstance->getEntity("test"));
  ASSERT_EQ(0, system.getComponentCount());
}

TEST_F(TestEngine, TestEntityAddFailure)
{
  TestSystem system;
  mInstance->addSystem("speed", &system);
  mInstance->addSystem("accelerator", new AccelerationSystem());
  DataProxy entityData;
  DataProxy speed;
  DataProxy accelerator;
  speed.put("speed", 2.0);
  accelerator.put("acceleration", 1.5);

  entityData.put("speed", speed);
  entityData.put("accelerator", accelerator);

  ASSERT_FALSE(mInstance->createEntity(entityData) == 0);

  entityData.put("id", "test");

  ASSERT_FALSE(mInstance->createEntity(entityData) == 0);
  ASSERT_FALSE(mInstance->createEntity(entityData) == 0);

  ASSERT_TRUE(mInstance->removeEntity("test"));
  ASSERT_FALSE(mInstance->removeEntity("test"));
  ASSERT_FALSE(mInstance->removeEntity("not_exists"));
}
