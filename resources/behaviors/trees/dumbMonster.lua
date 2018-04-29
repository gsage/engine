
require 'actions'

local function moveRandomly(self, context)
  self.navigation:go(
    self.render.position.x + math.random(30) - 15,
    0,
    self.render.position.z + math.random(30) - 15
  )
end

local function findEnemy(self, context, distance)
  local objects = self:getObjectsAround(distance, RenderComponent.DYNAMIC, self.stats:getString("enemy", "none"))
  context.target = nil
  for _, object in pairs(objects) do
    if actions.attackable(self, object) then
      context.target = object
      break
    end
  end
  return context.target ~= nil
end

local function targetInRange(self, context)
  if context.target == nil then
    return false
  end
  return self.render.position:squaredDistance(context.target.render.position) < self.stats:getNumber("attackDistance", 5) * 2
end

local function attack(self, context)
  local anims = {'attack1', 'attack2', 'attack3'}
  if not actions.attackable(self, context.target) then
    return false
  end

  self.render:lookAt(context.target.render.position, geometry.Y_AXIS, geometry.TS_WORLD)
  self.render:playAnimation(anims[math.random(3)], 1, 1, 0, false)
  context.waitSeconds(0.5)
  actions.inflictDamage(self, context.target)
  context.waitSeconds(0.5)
  return true
end

local function follow(self, context)
  if context.target == nil then
    return false
  end
  local pos = context.target.render.position
  return self.navigation:go(pos.x, pos.y, pos.z)
end

local function createTree()
  return Repeat(
    Sequence(
      Invertor(
        Sequence(
          Leaf(function(self, context) return findEnemy(self, context, self.stats:getNumber("aggroDistance", 0)) end),
          Leaf(follow),
          RepeatUntil(
            Selector(
              Sequence(
                Leaf(targetInRange),
                Leaf(function(self, context) self.movement:stop() end),
                Leaf(attack)
              ),
              Sequence(
                Leaf(function(self, context) return findEnemy(self, context, self.stats:getNumber("followDistance", 0)) end),
                Leaf(follow)
              )
            )
          )
        )
      ),
      Delay(Leaf(moveRandomly), function() return math.random(700)/100 + 3 end)
    )
  )
end

btree.register("dumbMonster", createTree)
