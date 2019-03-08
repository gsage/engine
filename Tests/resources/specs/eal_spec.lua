local eal = require 'lib.eal.manager'

-- registering test system assembly
local function decorator(cls)
  -- also setup configure and destroy chains
  cls.onCreate(function(self)
    self.calledOnCreate = true
  end, 100)

  -- also setup configure and destroy chains
  cls.onDestroy(function(self)
    self.calledOnDestroy = true
  end, 100)

  function cls:ping()
    return self.test.prop
  end
  return cls
end

-- registering test system assembly
local function stronglyDefinedDecorator(cls)
  function cls:ok()
    return true
  end
  return cls
end

-- dummy class extension
local function dummyDecorator(cls)
  function cls:isDummy()
    return true
  end
  return cls
end

-- burple class extension
local function burpleDecorator(cls)
  function cls:isBurple()
    return true
  end
  return cls
end

eal:extend({system="test"}, decorator)
eal:extend({system="test", type="testSystem"}, stronglyDefinedDecorator)

eal:extend({class={name="dummy"}}, dummyDecorator)
eal:extend({class={name="burple", requires={system="test"}}}, burpleDecorator)

local function firstMixinDecorator(cls)
  cls.onCreate(function(self)
    self.firstCreate = true
  end)

  cls.onDestroy(function(self)
    self.firstDestroy = true
  end)

  function cls:firstMixinAdded()
    return true
  end
  return cls
end

local function secondMixinDecorator(cls)
  cls.onCreate(function(self)
    self.sequenceValid = self.firstCreate == true
  end)

  cls.onDestroy(function(self)
    self.sequenceValid = self.firstDestroy == true
  end)

  function cls:checkSequence()
    return self.sequenceValid
  end
  return cls
end

eal:extend({mixin="second"}, secondMixinDecorator)
eal:extend({mixin="first"}, firstMixinDecorator)

describe("test engine abstraction layer #eal", function()
  setup(function()
    game:reset()
  end)

  teardown(function()
    game:reset()
  end)

  it("reset should invalidate all eal entities", function()
    local e = data:createEntity({
      id = "abcd",
      test = {
        prop = "pong"
      }
    })
    local wrapper = eal:getEntity(e.id)
    game:reset()
    assert.is_nil(wrapper.entity)
    assert.truthy(wrapper.calledOnDestroy)
    assert.equals(#eal.entities, 0)
    assert.is_nil(eal.entities[e.id])

    -- create the entity with the same id
    local e = data:createEntity({
      id = "abcd",
      test = {
        prop = "pong"
      }
    })
    assert.equals(wrapper:ping(), "pong")

    -- ensure that cache was cleared
    local wrapper = eal:getEntity(e.id)
    assert.is_not.is_nil(wrapper.entity)
    assert.truthy(wrapper.calledOnCreate)
    assert.equals(wrapper:ping(), "pong")
  end)

  describe("mixins", function()
    local composition = data:createEntity({
      vars = {
        mixins = {"first", "second"},
      },
      test = {
        prop = "ok"
      },
      stats = {
      }
    })

    it("simple case works", function()
      local wrapper = eal:getEntity(composition.id)
      assert.truthy(wrapper:firstMixinAdded())
      assert.truthy(wrapper:checkSequence())
    end)
  end)

  describe("assemble entity", function()
    it("check test system", function()
      local entity = data:createEntity({
        test = {
          prop = "pong"
        }
      })
      assert.equals(type(entity), "userdata")
      local wrapper = eal:getEntity(entity.id)
      assert.is_not.equals(wrapper.ping, nil)
      assert.equals(wrapper:ping(), "pong")
      assert.truthy(wrapper.calledOnCreate)
      assert.truthy(core:removeEntity(entity.id))
      assert.truthy(wrapper.calledOnDestroy)
      -- entity was removed, so the wrapper should be invalid now
      -- should raise errors, but no app crash expected
      assert.has.errors(function() wrapper:ping() end)
    end)

    it("should cache assembled entity", function()
      local entity = data:createEntity({
        test = {
          prop = "pongpong"
        }
      })
      local s = spy.on(eal, "assemble")
      local wrapper = eal:getEntity(entity.id)
      assert.spy(s).was.not_called()
    end)

    it("should reuse assembled class", function()
      local s = spy.on(eal, "assemble")
      local entity = data:createEntity({
        test = {
          prop = "pongpong"
        }
      })
      assert.spy(s).was.called(1)
      s = spy.on(eal, "assembleNew")
      assert.equals(type(entity), "userdata")
      local wrapper = eal:getEntity(entity.id)
      assert.is_not.equals(wrapper.ping, nil)
      assert.spy(s).was.not_called()
      assert.equals(wrapper:ping(), "pongpong")
    end)

    it("should return nil for not existing entity", function()
      local wrapper = eal:getEntity("no one here")
      assert.is_nil(wrapper)
    end)

    it("should also have strongly defined extension", function()
      local s = spy.on(eal, "getEntity")
      local entity = data:createEntity({
        test = {
          prop = "ok"
        }
      })
      assert.spy(s).was.called(1)
      local wrapper = eal:getEntity(entity.id)
      assert.truthy(wrapper:ok())
    end)

    local dummy = data:createEntity({
      vars = {
        class = "dummy"
      }
    })

    it("should extend class", function()
      local wrapper = eal:getEntity(dummy.id)
      assert.is_not.is_nil(wrapper.isDummy, "dummy class was not extended")
      assert.truthy(wrapper:isDummy())
    end)

    local dummyPlus = data:createEntity({
      vars = {
        class = "dummy",
      },
      test = {
        prop = "ok"
      }
    })

    it("should extend both by class and systems decorators", function()
      local wrapper = eal:getEntity(dummyPlus.id)
      assert.is_not.is_nil(wrapper.ping, "dummy class did not get system decorators")
      assert.is_not.is_nil(wrapper.isDummy, "dummy class was not extended")
      assert.truthy(wrapper:isDummy())
    end)

    local burple = data:createEntity({
      vars = {
        class = "burple",
      },
      test = {
        prop = "ok"
      },
      stats = {
      }
    })

    it("should assemble class with the system dependency", function()
      local wrapper = eal:getEntity(burple.id)
      assert.is_not.is_nil(wrapper.ping, "system extension did not work")
      assert.is_not.is_nil(wrapper.isBurple, "class extension did not work")
      assert.truthy(wrapper:isBurple(), "class extension did not work")
    end)

    -- required system not defined, so it should not be defined
    local noBurple = data:createEntity({
      vars = {
        class = "burple"
      }
    })

    it("should consider class system requirements", function()
      local wrapper = eal:getEntity(noBurple.id)
      assert.is_nil(wrapper.ping, "requirements were ignored (system)")
      assert.is_nil(wrapper.isBurple, "requirements were ignored (class)")
    end)
  end)
end)
