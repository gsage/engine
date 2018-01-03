require 'lib.class'
local time = require 'lib.time'
require 'imgui.base'

-- imgui engine stats view
Stats = class(ImguiWindow, function(self, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
  self.fps = 0
  self.frames = 0
  self.elapsedTime = 0
  -- update each second
  self.monitor = ResourceMonitor.new(0.1)
  self.stats = self.monitor.stats

  self.handleTime = function(delta)
    self.frames = self.frames + 1
    self.elapsedTime = self.elapsedTime + delta
    if self.elapsedTime >= 1 then
      self.fps = self.frames
      self.frames = 0
      self.elapsedTime = 0
    end
  end
  game:addUpdateListener(self.monitor)
  time.addHandler("stats", self.handleTime, true)
end)

-- render stats
function Stats:__call()
  if self:imguiBegin() then
    imgui.Text("FPS:" .. self.fps)
    imgui.Text("CPU:" .. math.floor(self.stats.lastCPU * 100))
    self:imguiEnd()
  end
end
