
package.path =  TRESOURCES .. '/scripts/?.lua' ..
                               ';' .. getResourcePath('scripts/?.lua');

local path = require 'lib.path'
path:addDefaultPaths()

-- fake editor interface
local editorState = {
  dockState = {
    dummy = 1
  }
}
editor = {
  getGlobalState = function()
    return editorState
  end,

  putToGlobalState = function(key, value)
    editorState[key] = value
  end,

  saveGlobalState = function()
    return true
  end
}

local async = require 'lib.async'
local exitCode = 0

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

local truncateFloat = function(exact, precision)
  return tonumber(string.format("%." .. tostring(precision) .. "f", exact))
end

local equals_float = function(state, arguments)
  local precision = arguments[3] or 5

  local a1 = truncateFloat(arguments[1], precision)
  local a2 = truncateFloat(arguments[2], precision)
  if a1 ~= a2 then
    return false, {a1, a2}
  end
  return true
end

local close_enough = function(state, arguments)
  local percent = arguments[3] or 1
  local a1 = arguments[1]
  local a2 = arguments[2]

  local t = math.max(math.abs(a1), math.abs(a2))

  local delta = math.max(arguments[4] or 0.2, math.abs(t / 100 * percent))
  arguments[3] = delta
  if math.abs(a1 - a2) > delta then
    return false, {a1, a2, delta}
  end
  return true
end

local runTests = function()
  local s, runner, assert
  local res, err = pcall(function()
    runner = require 'busted.runner'
    assert = require 'luassert'
    s = require 'say'
  end)

  if err then
    print("tests failed " .. tostring(err))
    async.signal("TestsComplete")
    return err
  end

  s:set("assertion.timing.positive", "Expected function to execute faster than: %s")
  assert:register("assertion", "timing", timing, "assertion.timing.positive")

  s:set("assertion.equals_float.positive", "Expected %s to be equal to %s")
  assert:register("assertion", "equals_float", equals_float, "assertion.equals_float.positive")

  s:set("assertion.close_enough.positive", "Expected %s and %s difference to be less than: %s")
  assert:register("assertion", "close_enough", close_enough, "assertion.close_enough.positive")

  res, err = pcall(runner, ({standalone=false}))
  async.signal("TestsComplete")
  if not res then
    exitCode = 1
    return err
  end

  return nil
end

function main()
  game:loadPlugin("Input")
  if os.getenv("OGRE_ENABLED") ~= "0" then
    local hasOgre = game:loadPlugin("OgrePlugin")
    if hasOgre then
      game:createSystem("ogre")
      core:render():configure({
        colourAmbient = "0x403030",
        resources = {
          workdir = TRESOURCES,
          TestResources = {
            "FileSystem;models/",
          }
        }
      })

      if core:render():getRenderer().name ~= "NULL_RS" then
        game:loadPlugin("ImGUIPlugin")
        game:loadPlugin("ImGUIOgreRenderer")
      end
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
  local res, err = coroutine.resume(testsCoroutine)
  if err and type(err) ~= 'thread' then
    error(err)
  end
end

local shutdown = function()
  async.waitSignal("TestsComplete")
  game:shutdown(exitCode)
end

local shutdownCoroutine = coroutine.create(shutdown)
coroutine.resume(shutdownCoroutine)

local success, err = pcall(main)
if not success then
  print("Tests error, exit code: " .. tostring(exitCode) .. ", err:", err)
  exitCode = 1
  game:shutdown(exitCode)
end
