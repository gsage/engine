-- path loading utils
require 'lib.class'

function split(inputstr, sep)
  if sep == nil then
    sep = "%s"
  end
  local t={}
  for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
    table.insert(t, str)
  end
  return t
end

local PathUtils = class(function(self)
  self.hasDefaultPaths = false
  self.additionalPaths = {}
end)

-- add default script paths
function PathUtils:addDefaultPaths()
  if self.hasDefaultPaths then
    return
  end

  -- adding luarocks dirs
  local version = _VERSION:match("%d+%.%d+")

  self:_addPaths({
    getResourcePath('luarocks/packages/share/lua/' .. version .. '/?.lua'),
    getResourcePath('luarocks/packages/share/lua/' .. version .. '/?/init.lua')
  }, "path")

  self:_addPaths({
    getResourcePath('luarocks/packages/lib/lua/' .. version .. '/?.so'),
    getResourcePath('luarocks/packages/lib/lua/' .. version .. '/?.dll')
  }, "cpath")

  self.hasDefaultPaths = true
end

-- add script folders
-- @param bundleName id to identify this folder pack, can be used to uninstall paths
-- @param folders list of folders to install
-- @param ptype resource path type path or cpath
function PathUtils:addScriptFolders(bundleName, folders, ptype)
  if self.additionalPaths[bundleName] then
    return false
  end

  local str = table.concat(folders)
  self.additionalPaths[bundleName] = folders

  self:_addPaths(folders, ptype)
  return true
end

-- removes path bundle from the resource path
function PathUtils:uninstallPathBundle(bundleName, ptype)
  local folders = self.additionalPaths[bundleName]

  if not folders then
    return
  end

  self:_removePaths(folders, ptype)
end

function PathUtils:_addPaths(paths, ptype)
  local currentPaths = split(package[ptype], ";")

  for _, path in ipairs(paths) do
    currentPaths[#currentPaths + 1] = path
  end

  package[ptype] = table.concat(currentPaths, ";")
end

function PathUtils:_removePaths(paths, ptype)
  local currentPaths = split(package[ptype], ";")

  for _, path in ipairs(paths) do
    for i, currentPath in ipairs(currentPaths) do
      if currentPath == path then
        table.remove(currentPaths, i, 1)
      end
    end
  end

  package[ptype] = table.concat(currentPaths, ";")
end

local pathUtils = PathUtils()
return pathUtils or {}
