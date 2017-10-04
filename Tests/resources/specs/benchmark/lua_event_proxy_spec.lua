require 'os'

local event = require 'lib.event'

describe("lua event proxy #benchmark", function()
  it("check one million handlers", function()
    local count = 0
    local handler = function(event)
      count = count + 1
    end

    for i = 1, 1000000 do
      event:bind(core, Facade.BEFORE_RESET, handler)
    end

    assert.timing(function()
      game:reset()
    end, 0.6)
    assert.equals(1000000, count)
  end)
end)
