local common = require 'lib.eal.common'
require 'helpers.base'

local event = require 'lib.event'

-- engine abstraction layer
-- allow building high level game objects based on top of low level component based architecture
--
-- combine a lua wrapper on the fly, depending on:
--  entity components set
--  system types installed
local EALManager = class(function(self)
  self.systems = {}
  self.entities = {}
  self.classCache = {}
  self.extensions = {}
  self.classes = {}
  self.mixins = {}
  local currentSystems = core:getSystemNames()
  for i = 1, #currentSystems do
    self:addSystem(currentSystems[i])
  end

  self.systemUpdateHandler = function(event)
    if event.type == SystemChangeEvent.SYSTEM_ADDED then
      self:addSystem(event.systemID)
    else
      self:removeSystem(event.systemID)
    end
  end

  self.entityUpdateHandler = function(event, sender)
    if event.type == EntityEvent.CREATE then
      -- create and cache created entity in the eal
      log.trace("EAL: initialize lua extensions for entity " .. event.id)
      e = self:getEntity(event.id)
      e:__configure()
    elseif event.type == EntityEvent.REMOVE then
      -- invalidate linked entity
      local e = self.entities[event.id]
      if e then
        e:__destroy()
        e.entity = nil
        self.entities[event.id] = nil
      end
    end
    return true
  end

  event:onSystemChange(core, SystemChangeEvent.SYSTEM_ADDED, self.systemUpdateHandler)
  event:onSystemChange(core, SystemChangeEvent.SYSTEM_REMOVED, self.systemUpdateHandler)
  event:onEntity(core, EntityEvent.CREATE, self.entityUpdateHandler)
  event:onEntity(core, EntityEvent.REMOVE, self.entityUpdateHandler)
end)

-- add system to the eal
function EALManager:addSystem(name)
  self.systems[name] = core:getSystem(name)

  -- todo: rebuild all entities
end

-- remove system from eal
function EALManager:removeSystem(name)
  self.systems[name] = nil

  -- todo: rebuild all entities
end

-- get entity using eal
function EALManager:getEntity(name)
  if not self.entities[name] then
    self.entities[name] = self:assemble(name)
  end

  return self.entities[name]
end

-- assemble entity wrapper
function EALManager:assemble(name)
  local e = core:getEntity(name)
  if e == nil then
    return nil
  end

  local components = e.componentNames
  local hashString = ""
  local parts = {}
  local info = {
    components = {},
    types = {}
  }
  local cls = e.vars.class
  if cls then
    parts[#parts + 1] = cls
    info.class = cls
  end

  if e.vars.mixins then
    info.mixins = e.vars.mixins
    local mixinsSorted = info.mixins
    table.sort(mixinsSorted)
    hashString = table.concat(mixinsSorted)
  end

  for i = 1, #components do
    local componentName = components[i]
    local t = self.systems[componentName].info.type
    parts[#parts + 1] = componentName .. (t or "")
    info.components[componentName] = true
    if t then
      info.types[t] = true
    end
  end
  table.sort(parts)

  hashString = hashString .. table.concat(parts)

  local hash = md5Hash(hashString)
  -- check if should assemble new type of class
  if not self.classCache[hash] then
    self.classCache[hash] = self:assembleNew(info)
  end

  return self.classCache[hash](e)
end

-- assemble new class
-- @param info entity metadata
-- @return new class, decorated with extensions
function EALManager:assembleNew(info)
  local cls = common.base()
  local function walkTree(tree, id)
    if type(tree) == "function" then
      cls = tree(cls)
      if cls == nil then
        error("Nil return value from " .. id)
      end
      return
    end

    if type(tree) ~= "table" then
      return
    end

    for name, value in pairs(tree) do
      if info[id][name] then
        walkTree(value, "types")
      end
    end
  end

  for key in pairs(info.components) do
    if self.extensions[key] then
      for _, tree in pairs(self.extensions[key]) do
        walkTree(tree, "types")
      end
    end
  end

  if info.class then
    tree = self.classes[info.class]
    if tree and type(tree) == "table" and tree["system"] then
      walkTree(tree["system"], "components")
    else
      walkTree(tree, "components")
    end
  end

  if info.mixins then
    for _, mixin in pairs(info.mixins) do
      local decorator = self.mixins[mixin]
      if decorator then
        cls = decorator(cls)
        if cls == nil then
          error("Nil return value from mixin decorator " .. mixin)
        end
      else
        log.warn("Undefined mixin \"" .. mixin .. "\"")
      end
    end
  end
  return cls
end

-- register an extension
function EALManager:extend(info, decorator)
  if info.class then
    local name = info.class.name
    if not name then
      error("No class name defined in the extension " .. table.toString(info))
    end

    local res = {}
    if info.class.requires then
      for requirement, subtype in pairs(info.class.requires) do
        local leaf = decorator
        if subtype then
          leaf = {[subtype] = decorator}
        end
        res[requirement] = leaf
      end
    else
      res = decorator
    end
    self.classes[info.class.name] = res
  end

  if info.mixin then
    self.mixins[info.mixin] = decorator
  end

  if info.system then
    local leaf = decorator
    if info.type then
      leaf = {[info.type] = decorator}
    end

    local extension = self.extensions[info.system] or {}
    if not self.extensions[info.system] then
      self.extensions[info.system] = extension
    end
    extension[#extension+1] = leaf
  end
end

if not eal then
  eal = EALManager()
  require 'lib.eal.extensions'
end

return eal
