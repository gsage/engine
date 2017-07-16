require 'lib.async'

return function(self)
  local function checkHp(e)
    local event = StatEvent.cast(e)
    if self:stats():getNumber(event.id) <= 0 then
      async.callLater(0.1, function()
        core:removeEntity(self.id)
      end)
    end
  end
  event:bind(self:stats(), 'statChange', checkHp)
end
