local rocksPath = 'luarocks/lua/5.1/?.lua'
if gsage.platform == "win32" then
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
local site_config = require("luarocks.site_config")

-- activating build environment for Windows
if gsage.platform == "win32" then
  -- TODO: handle that
  local arch = "x64"

  local probe_paths = {}
  local ms_comntools_version = nil
  local try_get_env_variable = function(name)
    local value = os.getenv(name)
    if value then
      probe_paths[#probe_paths + 1] = {path = "\"" .. value .. "..\\..\\VC\\vcvarsall.bat\" " .. arch, version = name}
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

  local function monkey_patch(vcvarsall)
    local props = {"CC", "LD", "MT", "RC"}

    for _, prop in ipairs(props) do
      cfg.variables[prop] = vcvarsall .. " && " .. cfg.variables[prop]
      log.info("Monkey patch " .. prop .. " with vcvarsall")
    end
  end

  local vswhere = site_config.LUAROCKS_PREFIX .. "tools\\vswhere.exe"
  local success, err = pcall(
    function()
      local pipe = io.popen(vswhere .. " -latest -requires Microsoft.Component.MSBuild -format json")
      data = pipe:read("*a")
      pipe:close()

      local info = json.loads(data)
      if #info == 0 then
        error("no MSBuild found")
      end

      ms_comntools_version = info[1].installationVersion
      log.info("Using VS Common Tools version " .. ms_comntools_version)

      pipe = io.popen(vswhere .. " -latest -requires Microsoft.Component.MSBuild -find \"**\\vcvarsall.bat\"")
      local path = pipe:read("*a")
      pipe:close()

      if path == "" then
        error("failed to find vcvarsall.bat")
      end

      monkey_patch("\"" .. path .. "\"" .. " " .. arch)
    end
  )

  if not success then
    log.error("failed to run vswhere, fallback to manual lookup " .. tostring(err))
    for _, tools in ipairs(probe_paths) do
      local vcvarsall = tools.path
      local success = fs.execute_string(vcvarsall)
      if success then
        monkey_patch(vcvarsall)

        log.info("Using VS Common Tools version " .. tools.version)
        ms_comntools_version = tools.version
        break
      end
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
end

local root_dir = fs.absolute_name(gsageTree)
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
