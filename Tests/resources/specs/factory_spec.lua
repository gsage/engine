require 'factories.base'
local eal = require 'lib.eal.manager'

local TestFactory = class(EntityFactory, function(self)
  EntityFactory.init(self, "testObject")
end)

local testFactory = TestFactory()
testFactory.createObject = function(name, settings)
  local res = data:createEntity({
    id = name,
    props = settings,
    test = {
      prop = settings.testProp or false
    }
  })
  if not res then
    return nil
  end

  return eal:getEntity(res.id)
end

testFactory:register("obj", testFactory.createObject)

describe("test entity factory #core", function()
  teardown(function()
    game:reset()
  end)

  it("should not do anything for NOTHING policy", function()
    local res = testFactory:create("obj", "object")
    assert.is_not.is_nil(res)

    -- now create again, should be nil
    assert.is_nil(testFactory:create("obj", "object", {policy = EntityFactory.NOTHING}))
  end)

  it("should reuse existing entity for REUSE policy", function()
    local res = testFactory:create("obj", "object2")
    assert.is_not.is_nil(res)

    local s = spy.on(eal, "assemble")
    -- now create again, should reuse value from the eal cache
    local reuse = testFactory:create("obj", "object2", {policy = EntityFactory.REUSE})
    assert.spy(s).was.not_called()
    assert.is_not.is_nil(reuse)
  end)

  it("should replace existing entity for REPLACE policy", function()
    local res = testFactory:create("obj", "object3")
    assert.is_not.is_nil(res)

    local s = spy.on(eal, "assemble")
    -- now create again, should re-create an entity
    local reuse = testFactory:create("obj", "object3", {policy = EntityFactory.REPLACE})
    assert.spy(s).was.called()
    assert.is_not.is_nil(reuse)
  end)

  it("should reset on game reset", function()
    local check = function()
      local res = testFactory:create("obj", "object4")
      assert.is_not.is_nil(res)
      assert.is_not_nil(testFactory.createdObjects["object4"])
      game:reset()
      assert.is_nil(testFactory.createdObjects["object4"])

      local s = spy.on(eal, "assemble")
      local create = spy.on(testFactory, "createObject")
      -- now create again, should not reuse cache
      local reuse = testFactory:create("obj", "object4", {policy = EntityFactory.REUSE})
      assert.spy(s).was.called()
      assert.is_not.is_nil(reuse)
      game:reset()
    end

    -- check it twice
    check()
    check()
  end)
end)
