require 'lib.class'
local hasLFS = pcall(require, 'lfs')

local i18n = require 'i18n'

-- localization manager
LocalizationManager = class(function(self)
  local f = getResourcePath("locales/")
  self.MISSING = "<missing>"
  self.availableLocales = {}
  self.localeData = {}
  local locales = {}
  local files = {}

  if hasLFS then
    for file in lfs.dir(f) do
      local _, extension = string.match(file, "([%w_]+)%.(%w+)$")
      local fullPath = f .. file
      if lfs.attributes(fullPath, "mode") == "file" and extension == "json" then
        files[#files + 1] = file
      end
    end
  else
    files = {
      "en_US.json",
      "ja_JP.json",
      "ru_RU.json"
    }
  end

  for _, file in ipairs(files) do
    local basename, extension = string.match(file, "([%w_]+)%.(%w+)$")
    local fullPath = f .. file
    local data, success = json.load(fullPath)
    if success then
      local localeInfo = {
        id = basename,
        name = data.language
      }

      locales[basename] = data
      self.availableLocales[#self.availableLocales + 1] = localeInfo
      self.localeData = data
    else
      log.error("Failed to parse locale " .. fullPath)
    end

  end
  i18n.load(locales)
  i18n.setLocale('en_US')
  i18n.setFallbackLocale('en_US')
end)

-- init locale context with some specific language
function LocalizationManager:setLocale(locale)
  i18n.setLocale(locale)
end

function LocalizationManager:getLocale()
  return i18n.getLocale()
end

function LocalizationManager:__call(...)
  return (i18n(table.unpack({...})) or self.MISSING)
end

function LocalizationManager:getCurrentLocaleData()
  return self.localeData[self:getLocale()] or {}
end

local lm = LocalizationManager()
return lm
