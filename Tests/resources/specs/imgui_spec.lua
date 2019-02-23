local camera = require 'factories.camera'
local async = require "lib.async"
local imguiInterface = require 'imgui.base'

local testView = function()
  imgui.Begin("Test Window")
  imgui.Text("works")
  imgui.End()
end

if imgui then
  after_each(function()
    game:reset()
    async.waitSeconds(0.2)
  end)

  describe("imgui tests #core #imgui", function()
    it("should work after engine reset", function()
      camera:createAndAttach('free', 'free')
      assert.truthy(imguiInterface:addView(imgui.manager, "test", testView, true))
    end)
  end)
end
