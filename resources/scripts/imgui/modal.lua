require 'lib.class'
local icons = require 'imgui.icons'
local lm = require 'lib.locales'

ModalView = class(function(self)
  self.open = false
  self.contentDraw = nil
  self.choices = nil
end)

function ModalView:show(title, contentDraw, choices, width, height)
  if self.open then
    return
  end
  self.title = title
  self.open = true
  self.contentDraw = contentDraw
  self.choices = choices
  self.width = width
  self.height = height
  self.choiceCount = 0
  for _ in pairs(self.choices) do
    self.choiceCount = self.choiceCount + 1
  end
end

function ModalView:showError(text)
  local choices = {
    [lm("modals.ok")] = function() end
  }
  self:show(lm("modals.errors.error"), function()
    imgui.Text(icons.error)
    imgui.SameLine()
    imgui.TextWrapped(text)
  end, choices, 400 * self.sx, 150 * self.sy)
end

function ModalView:__call()
  self.sx, self.sy = imgui.Scale()
  if self.open then
    imgui.OpenPopup(self.title)
  else
    return
  end

  local flags = ImGuiWindowFlags_AlwaysAutoResize + ImGuiWindowFlags_NoMove + ImGuiWindowFlags_NoResize
  if self.width and self.height then
    imgui.SetNextWindowSize(self.width, self.height)
  end

  if imgui.BeginPopupModal(self.title, true, flags) then
    if self.contentDraw then
      self.contentDraw()
    end
    imgui.Dummy(0, 40 * self.sy)
    local buttonWidth = 120 * self.sx
    local w, h = imgui.GetWindowContentRegionMax()
    local x = w / 2 - self.choiceCount * (buttonWidth + 10) / 2
    imgui.SetCursorPos(x, h - imgui.GetItemsLineHeightWithSpacing())
    imgui.Dummy(0, 0)
    imgui.SameLine()
    for key, value in pairs(self.choices) do
      if imgui.Button(key, buttonWidth, 30 * self.sy) then
        value()
        imgui.CloseCurrentPopup()
        self.open = false
      end
      imgui.SameLine()
    end
    imgui.EndPopup()
  else
    self.open = false
  end
end
