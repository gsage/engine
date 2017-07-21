package.path = ';' .. getResourcePath('luarocks/lua/5.1/?.lua') .. ';' .. package.path

run_prefix = run_prefix or getResourcePath('luarocks')
run_prefix = run_prefix .. "/"

local cfg = require("luarocks.cfg")
local path = require('luarocks.path')
local deps = require('luarocks.deps')
local dir = require('luarocks.dir')
local fs = require("luarocks.fs")

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

deps.fulfill_dependencies(spec, "one")
