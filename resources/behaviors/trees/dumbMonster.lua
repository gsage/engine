require 'actions'

local function moveRandomly(self, context)
  local position = Vector3:new(
    self.render.position.x + math.random(30) - 15,
    0,
    self.render.position.z + math.random(30) - 15
  )
  self.movement:go(position)
end

local function findEnemy(self, context, distance)
  local objects = view.getObjectsAround(self.id, distance, OgreSceneNode.DYNAMIC, self.stats:getString("enemy", "none"))
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

  self.render:lookAt(context.target.render.position)
  self.render:playAnimation(anims[math.random(3)], 1, 1, 0, false)
  async.waitSeconds(0.5)
  actions.inflictDamage(self, context.target)
  async.waitSeconds(0.5)
  return true
end

local function follow(self, context)
  if context.target == nil then
    return false
  end
  return self.movement:go(context.target.render.position)
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

