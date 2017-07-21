require 'lib.class'

-- table.filter({"a", "b", "c", "d"}, function(o, k, i) return o >= "c" end)  --> {"c","d"}
--
-- @FGRibreau - Francois-Guillaume Ribreau
-- @Redsmin - A full-feature client for Redis http://redsmin.com
table.filter = function(t, filterIter)
  local out = {}

  for k, v in pairs(t) do
    if filterIter(v, k, t) then out[#out + 1] = v end
  end

  return out
end

-- console log entry
local LogEntry = class(function(self, level, text)
  self.level = level
  self.text = text
end)

LogEntry.FATAL = "FATAL"
LogEntry.ERROR = "ERROR"
LogEntry.WARNING = "WARN"
LogEntry.INFO = "INFO"
LogEntry.DEBUG = "DEBUG"
LogEntry.TRACE = "TRACE"

LogEntry.COLORS = {
  [LogEntry.FATAL] = {1.0, 0.0, 0.0, 1.0},
  [LogEntry.ERROR] = {1.0, 0.4, 0.4, 1.0},
  [LogEntry.WARNING] = {1.0, 1.0, 0.5, 1.0},
  [LogEntry.INFO] = {0.0, 5.0, 1.0, 1.0},
  [LogEntry.DEBUG] = {1.0, 1.0, 1.0, 1.0},
  [LogEntry.TRACE] = {1.0, 0.5, 1.0, 1.0}
}

LogEntry.LEVEL_MAP = {
  [LogMessage.Global] = LogEntry.DEBUG,
  [LogMessage.Verbose] = LogEntry.DEBUG,
  [LogMessage.Debug] = LogEntry.DEBUG,
  [LogMessage.Trace] = LogEntry.TRACE,
  [LogMessage.Info] = LogEntry.INFO,
  [LogMessage.Warning] = LogEntry.WARNING,
  [LogMessage.Error] = LogEntry.ERROR,
  [LogMessage.Fatal] = LogEntry.FATAL
}

-- draw log entry
function LogEntry:draw()
  local color = self.COLORS[self.level]
  if not color then
    color = {1.0, 1.0, 1.0, 1.0}
  end
  imgui.PushStyleColor(ImGuiCol_Text, color[1], color[2], color[3], color[4])
  imgui.TextUnformatted(self.text)
  imgui.PopStyleColor()
end

-- lua/logs console
Console = class(function(self, maxInput)
  self.history = {}
  self.historyPos = 0
  self.prevHistoryPos = 0
  self.open = false
  self.title = "Lua Console"
  self.scrollToBottom = true
  self.maxInput = maxInput or 256
  self.commandLine = imgui.TextBuffer(self.maxInput)
  self.commandLine:write("")

  self.commandLineCallback = function(data)
    self:handleCommandLineCallback(data)
  end

  self.log = {}
  self:info("Welcome to lua console")
  self:info("Engine Version: " .. (game.version or "unknown"))

  log.proxy:subscribe("console", function(message)
    local level = LogEntry.LEVEL_MAP[message:level()] or "UNKNOWN"
    self:logMessage(level, "[".. level .."] " .. message:message())
  end)
end)

-- set console open
function Console:setOpen(value)
  self.open = value
end

-- clear logs
function Console:clear()
  self.log = {}
end

function Console:handleCommandLineCallback(data)

  local function setBuf(text)
      data.bufTextLen = data:setBuf(text)
      data.cursorPos = data.bufTextLen
      data.selectionStart = data.cursorPos
      data.selectionEnd = data.cursorPos
      self.prevHistoryPos = self.historyPos
      data.bufDirty = true
      self.commandLine:write(str)
  end

  if data.eventFlag == ImGuiInputTextFlags_CallbackHistory then
    if data.eventKey == ImGuiKey_UpArrow then
      if self.historyPos < 1 then
        self.historyPos = #self.history
      elseif self.historyPos > 1 then
        self.historyPos = self.historyPos - 1
      end
    elseif data.eventKey == ImGuiKey_DownArrow then
      if self.historyPos > 0 then
        self.historyPos = self.historyPos + 1
        if self.historyPos > #self.history then
          self.historyPos = 0
        end
      end
    end

    if self.prevHistoryPos ~= self.historyPos then
      local str = ""
      if self.historyPos > 0 then
        str = self.history[self.historyPos]
      end
      setBuf(str)
    end
  elseif ImGuiInputTextFlags_CallbackCompletion then
    local text = data:getBuf()
    local name, prefix, operator, commonPart, list = self:scan(text)
    local str = (prefix or "") .. (operator or "")
    if #list == 1 then
      str = str .. list[1]
    elseif #list > 1 then
      self:debug(name .. ":")
      for _, value in pairs(list) do
        self:debug("\t" .. value)
      end

      if commonPart ~= "" then
        str = str .. commonPart
      else
        str = nil
      end
    else
      str = nil
    end

    if str then
      setBuf(str)
    end
  end

end

function Console:__call()
  local _, open = imgui.Begin(self.title, self.open)
  if not open then
    imgui.End()
    return
  end

  imgui.TextWrapped("press TAB to use text completion.")
  if imgui.SmallButton("Clear") then
    self:clear()
  end
  imgui.SameLine()

  local scrollToBottom = false
  if imgui.SmallButton("Scroll to bottom") then
    scrollToBottom = true
  end

  imgui.SameLine()
  imgui.PushStyleVar_2(ImGuiStyleVar_FramePadding, 0, 0)
  local _, enabled = imgui.Checkbox("Autoscroll", self.scrollToBottom)
  self.scrollToBottom = enabled
  imgui.PopStyleVar()

  imgui.Separator()

  --self.filter = imgui.ImGuiTextFilter()
  imgui.BeginChild("ScrollingRegion", 0, -imgui.GetItemsLineHeightWithSpacing(), false, ImGuiWindowFlags_HorizontalScrollbar)
  if imgui.BeginPopupContextWindow() then
    if imgui.Selectable("Clear") then
      self:clear()
    end
    imgui.EndPopup()
  end

  imgui.PushStyleVar_2(ImGuiStyleVar_ItemSpacing, 4, 1)
  for _, item in pairs(self.log) do
    -- TODO filter
    item:draw()
  end

  if self.scrollToBottom or scrollToBottom then
    imgui.SetScrollHere()
  end

  imgui.PopStyleVar()
  imgui.EndChild()
  imgui.Separator()

  if imgui.InputText("input", self.commandLine, ImGuiInputTextFlags_EnterReturnsTrue+ImGuiInputTextFlags_CallbackCompletion+ImGuiInputTextFlags_CallbackHistory, self.commandLineCallback) then
    local text = self.commandLine:read()
    if text ~= "" then
      self:debug(text)
      self.history[#self.history + 1] = text
      self.commandLine:write("")
      local res, err = pcall(function() self:executeLua(text) end)
      if not res then
        self:err(err)
      end
      self.historyPos = 0
    end

    if imgui.IsItemHovered() or imgui.IsRootWindowOrAnyChildFocused() and not imgui.IsAnyItemActive() and not imgui.IsMouseClicked(0) then
      imgui.SetKeyboardFocusHere(-1); -- Auto focus previous widget
    end
  end

  imgui.End()
end

-- executeLua string
function Console:executeLua(script)
  local script, err = loadstring(script)
  if script == nil or script == "" then
    error("Failed to parse script " .. err)
  else
    return script()
  end
end

-- log err message
function Console:err(message)
  self:logMessage(LogEntry.ERROR, message)
end

-- log warn message
function Console:warn(message)
  self:logMessage(LogEntry.WARNING, message)
end

-- log info message
function Console:info(message)
  self:logMessage(LogEntry.INFO, message)
end

-- log debug message
function Console:debug(message)
  self:logMessage(LogEntry.DEBUG, message)
end

-- log message
function Console:logMessage(level, message)
  self.log[#self.log + 1] = LogEntry(level, message)
end

-- autocomplete scanner
function Console:scan(text)
  local scanTables = {_G}
  local name = "globals"

  local codeBlock = text

  local function checkBrackets(str)
    local _, closedCount = string.gsub(str, "[%)%]]", "")
    local _, openCount= string.gsub(str, "[%(%[]", "")
    return openCount == closedCount
  end

  -- if there is a comma or any other symbol that separates blocks
  -- process the next block instead
  local prefix, nextBlock = string.match(codeBlock, "^(.*[%(%[,; =%-%+%/]+)(.*)$")
  if nextBlock and checkBrackets(nextBlock) then
    codeBlock = nextBlock
  else
    prefix = ""
  end

  -- getting the last method or property to search
  local operator, field = string.match(codeBlock, "^.*([%.:])([%w\"]*)$")
  -- try to get the part of lua text that can be executed
  local validLua = string.match(codeBlock, "^(.*)[%.:].*$")

  prefix = (prefix or "") .. (validLua or "")

  if not operator and (not field or field == "") then
    field = codeBlock
  end

  local s = {}
  if validLua and validLua ~= "" then
    local res, value = pcall(function() return self:executeLua("return " .. validLua) end)
    if res then
      name = type(value)
      local metatable = getmetatable(value)
      scanTables = {}
      if metatable then
        scanTables[2] = metatable
      end

      if value then
        if type(value) == "table" then
          scanTables[1] = value
          if value.__props then
            local props = value:__props()
            for _, prop in pairs(props) do
              s[#s + 1] = prop.name
            end
          end
        end
      elseif operator then
        return name, validLua, operator, validLua, {}
      end
    else
      return "", validLua, operator, validLua, {}
    end
  end

  if #scanTables == 0 then
    return name, validLua, operator, validLua, {}
  end

  for _, object in pairs(scanTables) do
    for k, value in pairs(object) do
      local addKey = not operator
                     or operator == ":" and type(value) == "function"
                     or operator == "."

      if addKey then
        s[#s + 1] = k
      end
    end
  end
  table.sort(s)

  if field and field ~= "" then
    s = table.filter(s, function(item)
      return string.match(item, "^" .. field .. ".*") ~= nil
    end)
  end


  local commonPart = nil
  for _, key in pairs(s) do
    if commonPart ~= "" then
      if commonPart == nil then
        commonPart = key
      else
        local currentCommon = ""
        for i = 1, #commonPart do
          local c = commonPart:sub(i, i)
          if c == key:sub(i, i) then
            currentCommon = currentCommon .. c
          else
            break
          end
        end
        commonPart = currentCommon
      end
    end
  end

  return name, prefix, operator, commonPart, s
end
