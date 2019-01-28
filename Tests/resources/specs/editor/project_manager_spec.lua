local projectManager = require 'editor.projectManager'
local async = require 'lib.async'

describe("#editor project manager", function()
  local projectFolder = "./editortests"
  before_each(function()
    game.filesystem:rmdir(projectFolder, true)
  end)

  after_each(function()
    projectManager:close()
    assert.is_nil(projectManager.openProjectFile)
  end)

  local settings = {
    projectName = "test",
    projectPath = projectFolder,
    plugins = {
      {
        name = "UI",
        enabled = true
      }
    },
    selectedSystems = {
    },
    selectedManagers = {
      input = "SDL"
    },
    resources = {
      {
        source = "folder",
        folder = "bundles/materials/",
        install = "materials",
        type =  "render"
      },
      {
        source = "folder",
        folder = "scripts/imgui/components/webview.lua",
        install = "scripts/imgui/components/webview.lua",
        type = "script"
      }
    }
  }

  if os.getenv("OGRE_ENABLED") ~= "0" then
    table.insert(settings.plugins, {
        name = "OgrePlugin",
        enabled = true,
    })

    table.insert(settings.plugins, {
      name = "RecastNavigationPlugin",
      enabled = true,
    })

    settings.selectedSystems.render = "ogre"
    settings.selectedSystems.navigation = "recast"
  end

  local createProject = function(expectComplete)
    local complete = false

    projectManager:create(settings, function()
    end,
    function(success)
      complete = success
    end)

    for i = 1,10 do
      if complete then
        break
      end
      async.waitSeconds(0.1)
    end

    if expectComplete then
      assert.truthy(complete)
    else
      assert.falsy(complete)
    end
  end
  local pfolder = settings.projectPath .. "/" .. settings.projectName

  it("creating project works", function()
    createProject(true)
    local expectedPlugins = {}
    for _, plugin in ipairs(settings.plugins) do
      expectedPlugins[plugin.name] = plugin.enabled
    end

    local pf, success = json.load(pfolder ..  "/project.gpf")
    assert.truthy(success)
    assert.are.equal(pf.version, 1.0)
    assert.are.equal(pf.projectName, settings.projectName)
    assert.are.same(expectedPlugins, pf.plugins)
    assert.are.same(settings.selectedManagers, pf.managers)
    assert.are.same(settings.selectedSystems, pf.systems)
    assert.are.same({dummy = 1}, pf.workspace)

    local expectedResources = {}
    for _, res in ipairs(settings.resources) do
      if res.type then
        if not expectedResources[res.type] then
          expectedResources[res.type] = {}
        end

        table.insert(expectedResources[res.type], res.install)
      end
      assert.truthy(game.filesystem:exists(pfolder .. "/sources/" .. res.install))
    end
    assert.are.same(expectedResources, pf.resources)
  end)

  it("try to overwrite existing project fails", function()
    createProject(true)
    createProject(false)
  end)

  it("opening/closing project works", function()
    createProject(true)
    assert.has_no.errors(function()
      projectManager:open(pfolder)
    end)
    -- should not uninstall render system
    if settings.selectedSystems.render then
      assert.is_not.is_nil(core:render())
    end
    if settings.selectedSystems.navigation then
      assert.is_not.is_nil(core.navigation)
      assert.is_not.is_nil(core:navigation())
    end

    projectManager:close()
    -- restore previous systems state
    assert.is_not.is_nil(core:getSystem("test"))
    if core.navigation then
      assert.is_nil(core:navigation())
    end
  end)

  for _, path in ipairs({"asdjfoisajfoijaisodfji30493", "."}) do
    it("handles not existing project " .. path, function()
      local success, err = pcall(function()
        projectManager:open(path)
      end)
      assert.falsy(success)
      assert.not_nil(err)
    end)
  end
end)
