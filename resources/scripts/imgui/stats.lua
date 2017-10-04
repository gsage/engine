require 'lib.class'
local time = require 'lib.time'

-- imgui engine stats view
Stats = class(function(self)
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
  imgui.Begin("Stats", true, ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoTitleBar)
  imgui.Text("FPS:" .. self.fps)
  imgui.Text("CPU:" .. math.floor(self.stats.lastCPU * 100))
  imgui.End()
end
