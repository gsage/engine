local async = require 'lib.async'

return function(self)
  local function checkHp(e)
    local event = StatEvent.cast(e)
    if self:stats():getNumber(event.id) <= 0 then
      local anims = {"death1"}
      self:render():playAnimation(anims[math.random(#anims)], 0, 1, 0, true)
      async.callLater(0.5, function()
        core:removeEntity(self.id)
      end)
    end
  end
  --event:bind(self:stats(), 'statChange', checkHp)
end
