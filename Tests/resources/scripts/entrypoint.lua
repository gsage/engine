package.path =  TRESOURCES .. '/scripts/?.lua' ..
                               ';' .. getResourcePath('scripts/?.lua') ..
                               ';' .. getResourcePath('behaviors/trees/?.lua') ..
                               ';' .. getResourcePath('behaviors/?.lua') .. package.path

local version = _VERSION:match("%d+%.%d+")
package.path = ';' .. getResourcePath('luarocks/packages/share/lua/' .. version .. '/?.lua') ..
               ';' .. getResourcePath('luarocks/packages/share/lua/' .. version .. '/?/init.lua') .. ';' .. package.path
package.cpath = getResourcePath('luarocks/packages/lib/lua/' .. version .. '/?.so') .. ';' .. package.cpath

local async = require 'lib.async'

local timing = function(state, arguments)
  local func = arguments[1]
  if not type(func) == "function" then
    error("arg #1 must be a function")
  end

  local expected = arguments[2]
  if not type(expected) == "number" then
    error("arg #2 must be a number")
  end

  local args = {}
  for i = 3, #arguments do
    args[#args+1] = arguments[i]
  end
  local start = os.clock()
  func(table.unpack(args))
  return os.clock() - start < expected
end

local runTests = function()
  local runner = require 'busted.runner'
  local assert = require 'luassert'
  local say = require 'say.init'

  say:set("assertion.timing.positive", "Expected function to execute faster than: %s")
  assert:register("assertion", "timing", timing, "assertion.timing.positive")

  res, err = pcall(runner, ({standalone=false}))
  async.signal("TestsComplete")
  return res, err
end

function main()
  game:loadPlugin(PLUGINS_DIR .. "/Input")
  if os.getenv("OGRE_ENABLED") ~= "0" then
    local hasOgre = game:loadPlugin(PLUGINS_DIR .. "/OgrePlugin")
    if hasOgre then
      game:loadPlugin(PLUGINS_DIR .. "/ImGUI")
      game:createSystem("ogre")
    end
  else
    if arg then
      local index
      local found = false
      for i, v in ipairs(arg) do
        local tags = string.match(v, "%-%-exclude%-tags=\"?(.*)\"?")
        if tags then
          if not string.match(tags, "ogre") then
            tags = tags .. "," .. "ogre"
            arg[i] = "--exclude-tags=" .. tags
          end
          found = true
          break
        end
      end
      if not found then
          arg[#arg+1] = "--exclude-tags=ogre"
      end
    end
  end

  if not arg then
    arg = {}
  end

  arg[#arg+1] = TRESOURCES .. '/specs/'
  testsCoroutine = coroutine.create(runTests)
  return coroutine.resume(testsCoroutine)
end

local shutdown = function()
  async.waitSignal("TestsComplete")
  game:shutdown(exitCode)
end

local shutdownCoroutine = coroutine.create(shutdown)
coroutine.resume(shutdownCoroutine)

local success, exitCode = pcall(main)
if not success then
  print("Tests error: " .. tostring(exitCode))
  exitCode = 1
end
