local rocksPath = 'luarocks/lua/5.1/?.lua'

if gsage_platform == "win32" then
  rocksPath = 'luarocks/share/lua/5.1/?.lua'
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
  local probe_paths = {}
  local ms_comntools_version = nil
  local try_get_env_variable = function(name)
    local value = os.getenv(name)
    if value then
      probe_paths[#probe_paths + 1] = {path = "\"" .. value .. "..\\..\\VC\\vcvarsall.bat\" x64", version = name}
    end
  end

  local value = os.getenv("VCVARSALL_PATH")
  if value then
    probe_paths[#probe_paths + 1] = {path = "\"" .. value .. "\"", version = "set manually"}
  else
    try_get_env_variable("VS150COMNTOOLS")
    try_get_env_variable("VS140COMNTOOLS")
    try_get_env_variable("VS130COMNTOOLS")
  end

  -- TODO we definetely need vswhere there
  for _, tools in ipairs(probe_paths) do
    local vcvarsall = tools.path
    local success = fs.execute_string(vcvarsall)
    if success then
      local props = {"CC", "LD", "MT", "RC"}

      for _, prop in ipairs(props) do
        cfg.variables[prop] = vcvarsall .. " && " .. cfg.variables[prop]
        log.info("Monkey patch " .. prop .. " with vcvarsall")
      end

      log.info("Using VS Common Tools version " .. tools.version)
      ms_comntools_version = tools.version
      break
    end
  end

  if not ms_comntools_version then
    log.warn("Failed to find VS Common Tools utils, binary packages assembly won't work")
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
