require 'lib.class'

-- easing
Easing = class(function(self, func, startVal, endVal, duration)
  self.func = func
  self.startVal = startVal
  self.endVal = endVal
  self.duration = duration
  self.time = 0
end)

-- get next value and increase current time delta
function Easing:next(delta)
  self.time = self.time + delta

  if self.time >= self.duration then
    self.time = 0
    return nil
  end

  return self.func(self.time, self.startVal, self.endVal, self.duration)
end

-- export
local easing = {}

-- easing functions
function easing.inOutQuad(t, b, c, d)
  t = t / d * 2
  if t < 1 then
    return c / 2 * math.pow(t, 2) + b
  else
    return -c / 2 * ((t - 1) * (t - 3) - 1) + b
  end
end

function easing.inQuad(t, b, c, d)
  t = t / d
  return c * math.pow(t, 2) + b
end

function easing.outQuad(t, b, c, d)
  t = t / d
  return -c * t * (t - 2) + b
end

function easing.outInQuad(t, b, c, d)
  if t < d / 2 then
    return easing.outQuad (t * 2, b, c / 2, d)
  else
    return easing.inQuad((t * 2) - d, b + c / 2, c / 2, d)
  end
end

function easing.init(func, startVal, endVal, duration)
  return Easing(func, startVal, endVal, duration)
end

return easing
