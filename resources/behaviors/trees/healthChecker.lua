local function isDead(self, context)
  local dead = self.stats:getNumber("hp", 0) <= 0
  return false
end

local function die(self, context)
  context.waitSeconds(0.5)
  core:removeEntity(self.id)
  game:reset()
  game:loadSave("gameStart")
end

local function inRange(self, context)
  if context.target == nil then
    return false
  end

  return self.render.position:squaredDistance(context.target.render.position) < self.stats:getNumber("attackDistance", 5) * 2
end

local function attack(self, context)
  if not actions.attackable(self, context.target) then
    context.target = nil
    return false
  end

  if context.target == nil then
    return false
  end
  local aspd = self.stats:getNumber('aspd', 1)

  self.render:lookAt(context.target.render.position, geometry.Y_AXIS, geometry.TS_WORLD)
  local anims = {"attack1", "attack2"}
  self.render:playAnimation(anims[math.random(2)], 1, 1*aspd, 0, false)
  context.waitSeconds(0.43/aspd)
  actions.inflictDamage(self, context.target)
  context.waitSeconds(0.2/aspd)
  return true
end

local function follow(self, context)
  if context.target == nil then
    return false
  end
  return self.movement:go(context.target.render.position)
end

local function createTree()
  return
    Repeat(
      Sequence(
        Successor(
          Selector(
            Sequence(
              Invertor(Leaf(inRange)),
              Leaf(follow)
            ),
            Sequence(
              Leaf(inRange),
              Leaf(function(self) self.movement:stop() end),
              Leaf(attack)
            )
          )
        ),
        Leaf(isDead),
        Leaf(die)
      )
    )
end

btree.register('healthChecker', createTree)
