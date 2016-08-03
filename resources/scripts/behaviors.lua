btree = btree or {}

btree.factories = {}

function btree.initialize(id, behaviorId)
  if btree[id] ~= nil then
    return btree[id]
  end

  local tree = btree.getBehavior(behaviorId)
  if tree == nil then
    return nil
  end

  btree[id] = BehaviorTree(RunContext(id))
  btree[id]:start(tree)
  log.info("Started btree for object " .. id)
  return btree[id]
end

function btree.deinitialize(id)
  if btree[id] == nil then
    log.warn("Tried to stop non-existing tree with id " .. id)
    return
  end
  btree[id]:stop()
  log.info("Stopped btree for object " .. id)
  btree[id] = nil
end

function btree.getBehavior(behaviorId)
  if not btree.factories[behaviorId] then
    local succeed, err = pcall(function() require(behaviorId) end)
    if not succeed then
      log.error("Failed to initialize behavior of type " .. behaviorId .. ": " ..  err)
    end
  end

  if btree.factories[behaviorId] == nil then
    return nil
  end

  return btree.factories[behaviorId]()
end

function btree.register(behaviorId, tree)
  btree.factories[behaviorId] = tree
end

--------------------------------------------------------------------------------
-- BehaviorTree class
--------------------------------------------------------------------------------

class 'BehaviorTree'

function BehaviorTree:__init(context)
  self.currentBehavior = nil
  self.context = context

  self.mainCoroutine = nil
end

function BehaviorTree:start(rootNode)
  self.mainCoroutine = coroutine.create(
    function()
      rootNode:run(self.context)
      self:stop()
    end
  )
end

function BehaviorTree:update(time)
  if async.isSuspended(self.mainCoroutine) then
    return
  end

  coroutine.resume(self.mainCoroutine)
end

function BehaviorTree:stop()
  self.mainCoroutine = nil
end

--------------------------------------------------------------------------------
-- Timer class
--------------------------------------------------------------------------------

class 'Timer'

function Timer:__init()
  self.running = false
end

function Timer:run(delay, callback)
  self.delay = delay
  self.callback = callback
  self.thread = coroutine.create(function() self:_run() end)
  coroutine.resume(self.thread)
end

function Timer:_run()
  self.running = true
  async.waitSeconds(self.delay)
  if self.callback then
    pcall(self.callback)
  end
  self.running = false
end

--------------------------------------------------------------------------------
-- RunContext class
--------------------------------------------------------------------------------

class 'RunContext'

function RunContext:__init(id)
  self.id = id
  self.entity = engine:get(id)
  self.valid = true

  self.stacks = {}
end

function RunContext:pushToStack(stack, item)
  if self.stacks[stack] == nil then
    self.stacks[stack] = {}
  end

  table.insert(self.stacks[stack], item)
end

function RunContext:popFromStack(stack, dest)
  if self.stacks[stack] == nil then
    return nil
  end

  self[dest] = table.remove(self.stacks[stack])
  return self[dest]
end

function RunContext:stackEmpty(stack)
  return self.stacks[stack] == nil or #self.stacks[stack] == 0
end

--------------------------------------------------------------------------------
-- BaseBehavior class
--------------------------------------------------------------------------------

class 'BaseBehavior'

function BaseBehavior:__init(children)
  self.children = {}

  for _, child in pairs(children) do
    table.insert(self.children, child)
  end
end

function BaseBehavior:run(context)
  error({message='trying to use abstract class BaseBehavior'})
end

--------------------------------------------------------------------------------
-- Repeat behavior class
--------------------------------------------------------------------------------

class 'Repeat' (BaseBehavior)

function Repeat:__init(...) BaseBehavior.__init(self, ({...}))
  self.running = true
end

function Repeat:run(context)
  self.running = true
  local co = coroutine.running()
  while self.running do
    for _, child in pairs(self.children) do
      child:run(context)
      coroutine.yield(co)
    end
  end
  return false
end

--------------------------------------------------------------------------------
-- RepeatUntil behavior class
--------------------------------------------------------------------------------

class 'RepeatUntil' (Repeat)

function RepeatUntil:__init(...) BaseBehavior.__init(self, ({...}))
end

function RepeatUntil:run(context)
  self.running = true
  local co = coroutine.running()
  while self.running do
    for _, child in pairs(self.children) do
      if not child:run(context) then
        self.running = false
        break
      end
      coroutine.yield(co)
    end
  end
  return false
end

--------------------------------------------------------------------------------
-- Sequence behavior class
--------------------------------------------------------------------------------

class 'Sequence' (BaseBehavior)

function Sequence:__init(...) BaseBehavior.__init(self, ({...}))
end

function Sequence:run(context)
  local success = true
  for _, child in pairs(self.children) do
    if not child:run(context) then
      success = false
      break
    end
  end
  return success
end

--------------------------------------------------------------------------------
-- Selector behavior class
--------------------------------------------------------------------------------

class 'Selector' (BaseBehavior)

function Selector:__init(...) BaseBehavior.__init(self, ({...}))
end

function Selector:run(context)
  local success = false
  for _, child in pairs(self.children) do
    if child:run(context) then
      success = true
      break
    end
  end
  return success
end

--------------------------------------------------------------------------------
-- Invertor behavior class
--------------------------------------------------------------------------------

class 'Invertor' (BaseBehavior)

function Invertor:__init(child)
  self.child = child
end

function Invertor:run(context)
  return not self.child:run(context)
end

--------------------------------------------------------------------------------
-- Successor behavior class
--------------------------------------------------------------------------------

class 'Successor' (BaseBehavior)

function Successor:__init(child)
  self.child = child
end

function Successor:run(context)
  if self.child then
    self.child:run(context)
  end
  return true
end

--------------------------------------------------------------------------------
-- Leaf behavior class
--------------------------------------------------------------------------------

class 'Leaf' (BaseBehavior)

function Leaf:__init(callback)
  self.callback = callback
end

function Leaf:run(context)
  if not context.valid then
    return false
  end

  local succeed, res = pcall(function() return self.callback(context.entity, context) end)
  if failed then
    log.error("Failed to call leaf: " .. res)
    return false
  end

  if res == nil then
    return true
  end
  return res
end

--------------------------------------------------------------------------------
-- Delay behavior class
--------------------------------------------------------------------------------

class 'Delay' (BaseBehavior)

function Delay:__init(child, delayClosure)
  self.child = child
  self.timer = Timer()
  self.delayClosure = delayClosure
end

function Delay:run(context)
  if not self.timer.running then
    self.child:run(context)
    self.timer:run(self.delayClosure())
  end
end
