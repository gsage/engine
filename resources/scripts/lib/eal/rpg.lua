local async = require 'lib.async'
local event = require 'lib.event'

-- rpg extensions

-- mortal mixin:
-- character has health
-- if health goes to zero, then die
local function mortal(cls)
  cls.onCreate(function(self)
    if not self.stats then
      return
    end

    local function checkHp(event)
      if event.id ~= "hp" then
        return
      end

      if self.stats:getNumber("hp") <= 0 then
        self:die()
      end
    end
    event:onStat(self.stats, StatEvent.STAT_CHANGE, checkHp)
  end)

  function cls:die()
    self.render:playAnimation("death1", 1, 1, 0, true)
    async.callLater(0.5, function()
      core:removeEntity(self.id)
    end)
  end
  return cls
end

eal:extend({mixin = "mortal"}, mortal)
