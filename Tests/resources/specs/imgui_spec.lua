local camera = require 'factories.camera'
local async = require "lib.async"

local testView = function()
  imgui.Begin("Test Window")
  imgui.Text("works")
  imgui.End()
end

if imgui then
  describe("imgui tests #core #imgui", function()
    it("should work after engine reset", function()
      camera:createAndAttach('free', 'free')
      imgui.render:addView("test", view)
      game:reset()
      async.waitSeconds(0.2)
    end)
  end)
end
