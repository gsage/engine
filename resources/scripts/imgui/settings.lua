require 'lib.class'
require 'imgui.base'
local lm = require 'lib.locales'
local icons = require 'imgui.icons'
local bindings = require 'lib.bindings'
local cm = require 'lib.context'
local event = require 'lib.event'

-- imgui view for editor settings
SettingsView = class(ImguiWindow, function(self, open)
  ImguiWindow.init(self, "settings", true, open)
  self.icon = icons.tune
  self.settings = editor:getGlobalState().settings or {}
end)

local ignoredKeys = {}
ignoredKeys[Keys.KC_LCONTROL] = true
ignoredKeys[Keys.KC_RCONTROL] = true
ignoredKeys[Keys.KC_LALT] = true
ignoredKeys[Keys.KC_RALT] = true
ignoredKeys[Keys.KC_LSHIFT] = true
ignoredKeys[Keys.KC_RSHIFT] = true
ignoredKeys[Keys.KC_LWIN] = true
ignoredKeys[Keys.KC_RWIN] = true

function SettingsView:listenKeys()
  if not self.onKey then
    self.onKey = function(event)
      if ignoredKeys[event.key] then
        return
      end

      if self.changingKey then
        local value = encodeShortcut(event.modifiers, event.key)
        local str = shortcutToString(value)

        self.changingKey.scStr = str;
        self.changingKey.scValue = value;
      end
    end
  else
    return
  end

  event:onKeyboard(core, KeyboardEvent.KEY_DOWN, self.onKey)
end

function SettingsView:stopListenKeys()
  if not self.onKey then
    return
  end

  event:unbind(core, KeyboardEvent.KEY_DOWN, self.onKey)
  self.onKey = nil
end

-- settings view
function SettingsView:__call()
  if self:imguiBegin() then
    local currentLocale = lm:getLocale()
    local w = imgui.GetContentRegionAvail()
    imgui.Text(lm("settings.lang"))
    if imgui.ListBoxHeader("language", w, 200) then
      for _, info in pairs(lm.availableLocales) do
        if imgui.Selectable(info.name, info.id == currentLocale) then
          lm:setLocale(info.id)
          editor:putToGlobalState("settings", {
            locale = info.id
          })
        end
      end
      imgui.ListBoxFooter()
    end
    imgui.Separator()
    imgui.Text(lm("settings.bindings"))
    local bcfg = bindings:getConfig()
    if bcfg then
      for name, group in pairs(bcfg) do
        imgui.PushID(name)
        if imgui.CollapsingHeader(lm("window_title." .. name)) then
          imgui.Columns(2)
          for shortcut, binding in pairs(group) do
            imgui.Dummy(0, 0)
            local desc = lm(name .. ".actions." .. binding.action)
            if desc == lm.MISSING then
              imgui.Text(binding.action)
            else
              imgui.Text(binding.action .. " (" .. desc .. ")")
            end
            imgui.NextColumn()
            local changing = self.changingKey and self.changingKey.group == group and self.changingKey.shortcut == shortcut
            self:renderKeyBinding(changing, group, shortcut, binding)
            imgui.NextColumn()
          end
          imgui.Columns(1)
          local context = cm:getContext(name)
          if context then
            if self.addingKey and self.addingGroup == name then
              imgui.Columns(2)
              local changed = false
              changed, self.actionSelected = imgui.Combo(lm("settings.bindings_select"), self.actionSelected, self.actionsView)
              if changed then
                self.addingKey.action = self.actions[self.actionSelected + 1]
              end
              imgui.NextColumn()
              self.changingNewKeyBinding = self:renderKeyBinding(self.changingNewKeyBinding, group, nil, self.addingKey, false)
              imgui.SameLine()
              local done = false
              if imgui.Button(lm("settings.bindings_create")) and self.addingKey.action and self.addingKey.shortcut then
                group[self.addingKey.shortcut] = self.addingKey
                self.addingKey.shortcut = nil
                self.addingKey = nil
              end
              imgui.SameLine()
              if imgui.Button(lm("settings.bindings_cancel")) then
                self.addingKey = nil
              end

              imgui.Columns(1)
            else
              if imgui.Button(icons.plus) then
                self.addingKey = {}
                self.addingGroup = name
                self.actionSelected = 0
                self.actions = {}
                for action in pairs(context.callbacks) do
                  table.insert(self.actions, action)
                end
                self.actionsView = table.concat(self.actions, "\0") .. "\0\0"
              end
            end
          end
        end
        imgui.PopID()
      end
    else
      imgui.Text(lm("settings.bindings_error", {icon = icons.warning}))
    end
    self:imguiEnd()
  end
end

function SettingsView:changeKeyBinding(group, shortcut)
  self.changingKey = {
    group = group,
    shortcut = shortcut,
    scValue = nil,
    scStr = ""
  }
end

function SettingsView:renderKeyBinding(changing, group, shortcut, binding, save)
  if changing then
    imgui.Button(self.changingKey.scStr or "...")
    imgui.SameLine()
    done = false

    hovered = false
    if imgui.Button(lm("settings.bindings_save")) then
      if self.changingKey.scStr ~= nil then
        self:stopListenKeys()
        binding.str = self.changingKey.scStr
        if save == nil or save then
          group[shortcut] = nil
          self.changingKey.group[self.changingKey.scValue] = binding
        else
          binding.shortcut = self.changingKey.scValue
        end
        done = true
      end
    end

    hovered = imgui.IsItemHovered()

    if shortcut then
      imgui.SameLine()
      if imgui.Button(lm("settings.bindings_remove")) then
        group[shortcut] = nil
        done = true
      end
    end

    hovered = imgui.IsItemHovered() or hovered

    if imgui.IsMouseClicked(0) and not hovered or done then
      self:stopListenKeys()
      self.changingKey = nil
      return false
    end
    return true
  else
    local s = binding.str or "..."

    if imgui.Button(s) then
      self.changingKey = {
        group = group,
        shortcut = shortcut,
        scStr = nil,
        scValue = nil
      }
      self:listenKeys()
      return true
    end
  end
end

return SettingsView
