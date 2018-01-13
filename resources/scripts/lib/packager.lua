local rocksPath = 'luarocks/lua/5.1/?.lua'

if gsage_platform == "win32" then
  rocksPath = 'luarocks/lua/?.lua'
end

package.path = ';' .. getResourcePath(rocksPath) .. ';' .. package.path

run_prefix = run_prefix or getResourcePath('luarocks')
run_prefix = run_prefix .. "/"

local cfg = require("luarocks.cfg")

-- monkey patch configuration
local local_cache = getResourcePath('luarocks/cache')
cfg.local_cache = local_cache

local path = require('luarocks.path')
local deps = require('luarocks.deps')
local dir = require('luarocks.dir')
local fs = require("luarocks.fs")

-- activating build environment for Windows
if gsage_platform == "win32" then
  local probe_versions = {"14.0", "15.0"}
  local ms_build_version = nil

  for _, version in ipairs(probe_versions) do
    local vcvarsall = "\"C:\\Program Files (x86)\\Microsoft Visual Studio " .. version .. "\\VC\\vcvarsall.bat\" x64"
    local success = fs.execute_string(vcvarsall)
    if success then
      local props = {"CC", "LD", "MT", "RC"}

      for _, prop in ipairs(props) do
        cfg.variables[prop] = vcvarsall .. " && " .. cfg.variables[prop]
        log.info("Monkey patch " .. prop .. " with vcvarsall")
      end

      log.info("Using MSBuild version " .. version)
      ms_build_version = version
      break
    end
  end

  if not ms_build_version then
    log.warn("Failed to find MSBuild utils, binary packages assembly won't work")
  end
end

-- constructing fake rockspec
local parsed_deps = {}
for _, value in ipairs(dependencies) do
  table.insert(parsed_deps, deps.parse_dep(value))
end

local spec = {
  name = "gsage",
  version = "1-0",
  dependencies = parsed_deps
}

local gsageTree = getResourcePath("luarocks/packages")

local function replace_tree(tree)
  tree = dir.normalize(tree)
  gsageTree = tree
  path.use_tree(tree)
  fs.make_dir(gsageTree .. "/luarocks")
end

local relative_to = run_prefix
if gsage_platform == "apple" then
  relative_to = nil
end

local root_dir = fs.absolute_name(gsageTree, relative_to)
replace_tree(root_dir)

local success, err = pcall(function()
  local s, err = deps.fulfill_dependencies(spec, "one")
  if err then
    error(err)
  end
end)

if err then
  log.error("failed to get dependencies: " .. err)
end
