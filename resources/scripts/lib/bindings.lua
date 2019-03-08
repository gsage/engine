require 'lib.class'

function decodeShortcut(value)
  if type(value) == "string" then
    value = tonumber(value)
  end
  local modifiers = bit.band(0x0000FFFF, value)
  local key = bit.brshift(16, bit.band(0xFFFF0000, value))
  return modifiers, key
end

function encodeShortcut(modifiers, key)
  return tostring(bit.bor(bit.blshift(16, key), modifiers))
end

function shortcutToString(value)
  local modifiers, key = decodeShortcut(value)
  local res = {}
  local m = {}

  if modifiers > 0 then
    for name, value in pairs(Modifiers) do
      if bit.band(modifiers, value) ~= 0 then
        m[name] = true

        m["L" .. name] = false
        m["R" .. name] = false
      end
    end
  end

  for name, active in pairs(m) do
    if active then
      table.insert(res, name)
    end
  end

  for name, k in pairs(Keys) do
    if k == key and not Modifiers[name] then
      local val = name:gsub("KC_", "")
      table.insert(res, val)
    end
  end

  return table.concat(res, "+")
end

local function addShortcut(group, modifiers, key, action)
  local m = 0
  for _, modifier in ipairs(modifiers) do
    m = bit.bor(m, modifier)
  end
  local id = encodeShortcut(m, key)
  local str = shortcutToString(id)
  group[id] = {
    action = action,
    str = str
  }
end

local viewport = {}
addShortcut(viewport, {Modifiers.Ctrl}, Keys.KC_Z, "undo")
addShortcut(viewport, {Modifiers.Ctrl}, Keys.KC_Y, "redo")
addShortcut(viewport, {Modifiers.Ctrl, Modifiers.Shift}, Keys.KC_Z, "redo")
addShortcut(viewport, {Modifiers.Ctrl}, Keys.KC_S, "save")
addShortcut(viewport, {Modifiers.Ctrl}, Keys.KC_C, "copy")
addShortcut(viewport, {Modifiers.Ctrl}, Keys.KC_V, "paste")
addShortcut(viewport, {}, Keys.KC_R, "rotate")
addShortcut(viewport, {}, Keys.KC_S, "scale")
addShortcut(viewport, {}, Keys.KC_M, "move")

local script_editor = {}
addShortcut(script_editor, {Modifiers.Ctrl}, Keys.KC_Z, "undo")
addShortcut(script_editor, {Modifiers.Ctrl}, Keys.KC_Y, "redo")
addShortcut(script_editor, {Modifiers.Ctrl, Modifiers.Shift}, Keys.KC_Z, "redo")
addShortcut(script_editor, {Modifiers.Ctrl}, Keys.KC_S, "save")

local defaultBindings = {
  viewport = viewport,
  script_editor = script_editor
}

BindingsGroup = class(function(self, data)
  self.data = data
end)

local otherModifiers = {
  [Modifiers.LCtrl] = Modifiers.Ctrl,
  [Modifiers.RCtrl] = Modifiers.Ctrl,
  [Modifiers.LAlt] = Modifiers.Alt,
  [Modifiers.RAlt] = Modifiers.Alt,
  [Modifiers.LShift] = Modifiers.Shift,
  [Modifiers.RShift] = Modifiers.Shift,
  [bit.bor(Modifiers.RShift, Modifiers.RCtrl)] = bit.bor(Modifiers.Shift, Modifiers.Ctrl),
  [bit.bor(Modifiers.LShift, Modifiers.LCtrl)] = bit.bor(Modifiers.Shift, Modifiers.Ctrl),
  [bit.bor(Modifiers.LAlt, Modifiers.LCtrl)] = bit.bor(Modifiers.Alt, Modifiers.Ctrl),
  [bit.bor(Modifiers.RAlt, Modifiers.RCtrl)] = bit.bor(Modifiers.Alt, Modifiers.Ctrl),
  [bit.bor(Modifiers.LAlt, Modifiers.LShift)] = bit.bor(Modifiers.Alt, Modifiers.Ctrl),
  [bit.bor(Modifiers.RAlt, Modifiers.RShift)] = bit.bor(Modifiers.Alt, Modifiers.Ctrl),
  [Modifiers.LAlt + Modifiers.LShift + Modifiers.LCtrl] = Modifiers.Alt + Modifiers.Shift + Modifiers.Ctrl,
  [Modifiers.RAlt + Modifiers.RShift + Modifiers.RShift] = Modifiers.Alt + Modifiers.Shift + Modifiers.Ctrl,
}

-- search bindings in the bindings group
function BindingsGroup:resolve(event)
  local key = encodeShortcut(event.modifiers, event.key)
  local binding = self.data[key]
  if binding then
    return binding.action
  end

  local m = otherModifiers[event.modifiers]
  if not m then
    return nil
  end
  local key = encodeShortcut(m, event.key)
  binding = self.data[key]
  if binding then
    return binding.action
  end

  return nil
end

BindingsManager = class(function(self)
  self.groups = {}
  self.data = nil
end)

-- configure binding manager
function BindingsManager:configure(data)
  if not data then
    data = defaultBindings
  end

  for name, value in pairs(data) do
    self.groups[name] = BindingsGroup(value)
  end
  self.data = data
end

-- get binding manager data
function BindingsManager:getConfig()
  return self.data
end

-- get bindings group
function BindingsManager:get(name)
  return self.groups[name]
end

local bm = BindingsManager()
return bm
