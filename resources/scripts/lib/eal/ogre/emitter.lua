local function decorate(cls)
  cls.onCreate(function(self)
    local function initPS(name)
      if self.props[name] then
        self[name] = self.render.root:getParticleSystem(self.props[name])
      end
    end

    initPS("pc")
    initPS("miss")
    initPS("hostile")
  end)

  function cls:onDamage(actor, target, amount)
    local ps
    if target.props.type == "npc" and target.props.hostile then
      ps = self.hostile or self.pc
    else
      ps = self.pc
    end

    if amount == 0 then
      if self.miss then
        self.miss:createParticle(0, actor.id, "miss")
      end
      return
    end

    if ps then
      ps:createParticle(0, target.id, tostring(math.floor(amount)), self.render.facingOrientation * Quaternion.new(Radian.new(3.14159265), Vector3.UNIT_Y))
    end
  end

  return cls
end

eal:extend({class = {name = "damageEmitter", requires = {render = "ogre"}}}, decorate)
