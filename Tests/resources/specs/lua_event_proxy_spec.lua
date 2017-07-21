require 'factories.camera'
require 'lib.eal.manager'

local event = require 'lib.event'

describe("test event proxy #core", function()
  local function createTestEntity()
    return data:createEntity({
      flags = {"dynamic"},
      stats = {
        hp = 1
      }
    })
  end

  setup(function()
    game:reset()
  end)

  teardown(function()
    game:reset()
  end)

  it("should receive events", function()
    local eventType = nil
    local entityID = nil
    local count = 0
    local function handler(e)
      eventType = e.type
      entityID = e.id
      count = count + 1
    end
    local id = event:onEntity(core, EntityEvent.CREATE, handler)
    createTestEntity()

    assert.equals(count, 1)
    assert.equals(eventType, EntityEvent.CREATE)
    assert.is_not.is_nil(entityID)
  end)

  describe("unbind", function()
    local eventType = nil
    it("should work", function()
      local eventType = nil
      local function handler(e)
        eventType = e.type
      end
      event:onEntity(core, EntityEvent.CREATE, handler)
      assert.truthy(event:unbind(core, EntityEvent.CREATE, handler))
      local s = spy.on(event, "handler")
      createTestEntity()
      assert.is_nil(eventType)
      assert.spy(s).was.not_called()
    end)

    it("should remove listener on removal of dispatcher", function()
      local entity1 = createTestEntity()
      local entity2 = createTestEntity()
      local gotEvent = false
      local handler = function(event)
        gotEvent = true
      end
      assert.is_not.is_nil(entity1:stats())
      event:onStat(entity1:stats(), StatEvent.STAT_CHANGE, handler)
      entity1:stats():set("hp", 200)
      assert.truthy(gotEvent)
      assert.truthy(core:removeEntity(entity1.id))
      gotEvent = false
      entity2:stats():set("hp", 200)
      assert.falsy(gotEvent)
    end)

    pending("unbind in handler", function()
      for i = 1, 10 do
        local handle
        local id
        handle = function(e)
          event:unbind(core, EntityEvent.CREATE, handle)
        end
        id = event:onEntity(core, EntityEvent.CREATE, handle)
      end
      local entity = createTestEntity()
    end)
  end)
end)
