actions = actions or {}

function actions.attackable(self, target)
  if target.id == self.id then
    return false
  end

  if self.stats == nil or target.stats == nil then
    return false
  end

  if target.stats:getNumber("hp", 0) <= 0 then
    return false
  end

  return true
end

function actions.inflictDamage(self, target)
  -- TODO: use damage system
  target.stats:increase("hp", -actions.calculateDamage(self, target))
  if target.movement.speed == 0 then
    return
  end
  local previousSpeed = target.movement.speed
  target.movement.speed = 0
  async.callLater(target.stats:getNumber("stopDelay", 0.5), function()
    target.movement.speed = previousSpeed
  end)

end

function actions.calculateDamage(self, target)
  local damage = self.stats:getNumber("atk") or 0
  damage = math.max(0, math.random() + math.random(0, damage))
  local ps
  if target.id == "sinbad" then
    ps = damageHit
  else
    ps = damageEnemy
  end
  if damage > 0.5 then
    ps:createParticle(0, target.id, tostring(math.floor(damage)), self.render.facingOrientation * Quaternion(Radian(3.14159265), Vector3(0,1,0)))
  else
    damageMiss:createParticle(0, self.id, "Missed")
  end

  return damage
end
