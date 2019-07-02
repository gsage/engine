package.path = package.path .. ';' .. getResourcePath('scripts/?.lua')

local path = require 'lib.path'
path:addDefaultPaths()

require 'math'
require 'helpers.base'
require 'lib.behaviors'
require 'factories.camera'
require 'factories.emitters'
local eal = require 'lib.eal.manager'
local event = require 'lib.event'
local lm = require 'lib.locales'
local async = require 'lib.async'
local bindings = require 'lib.bindings'

local imguiInterface = require 'imgui.base'
local icons = require 'imgui.icons'
local projectManager = require 'editor.projectManager'

if imguiInterface:available() then
  require 'imgui.menu'
  require 'imgui.console'
  require 'imgui.sceneEditor'
  require 'imgui.stats'
  require 'imgui.transform'
  require 'imgui.sceneExplorer'
  require 'imgui.settings'
  require 'imgui.tools'
  require 'imgui.scriptEditor'
  require 'imgui.projectWizard'
  require 'imgui.assets'
  require 'imgui.modal'
else
  log.error("editor requires ImGUI plugin to be installed")
  game:shutdown(1)
  return
end

require 'editor.ogre.objects'

-- create main dockspace view
local dockspace = imgui.createDockspace("workspace")

-- create project wizard
local wizard = ProjectWizard("wizard", "wizard", true)

local globalEditorState = editor:getGlobalState()
if globalEditorState.settings and globalEditorState.settings.locale then
  lm:setLocale(globalEditorState.settings.locale)
end

bindings:configure(globalEditorState.bindings)

local rocketInitialized = false

function initLibrocket(event)
  local ctx = rocket.contexts[event.name]
  if not rocketInitialized then
    local fonts =
    {
      "Delicious-Roman.otf",
      "Delicious-BoldItalic.otf",
      "Delicious-Bold.otf",
      "Delicious-Italic.otf",
      "lucida.ttf"
    }
    for _, font in pairs(fonts) do
      resource.loadFont(font)
    end
  end
end

-- librocket initialization
if event.onRocketContext ~= nil then
  event:onRocketContext(core, RocketContextEvent.CREATE, initLibrocket)
end

local selectTransform = false

function handleKeyEvent(e)
  if e.type == KeyboardEvent.KEY_UP then
    selectTransform = false
    return
  end

  if e.key == Keys.KC_T and e.type == KeyboardEvent.KEY_DOWN then
    selectTransform = true
  end
end

event:onKeyboard(core, KeyboardEvent.KEY_DOWN, handleKeyEvent)
event:onKeyboard(core, KeyboardEvent.KEY_UP, handleKeyEvent)

local initialized = false
local dockstates = {}
local mode = "world"

local function saveSettings()
  log.info("Saving imgui dock state")
  dockstates[mode] = dockspace:getState()
  if projectManager.openProjectFile then
    projectManager.openProjectFile:setWorkspace(dockstates)
    projectManager.openProjectFile:write()
  end
  editor:putToGlobalState("bindings", bindings:getConfig())
  editor:saveGlobalState()
end

local views = {}
local modalView = ModalView()
local viewsMenu = {}

local function createViews()
  views.luaConsole = Console(256, true)
  views.sceneEditor = SceneEditor(modalView, "viewport", "viewport", true)
  views.scriptEditor = ScriptEditor("script", "script_editor", true)
  views.transform = Transform(views.sceneEditor, "transform", true)
  views.stats = Stats("stats", true)
  views.sceneExplorer = SceneExplorer("scene explorer", true)
  views.settings = SettingsView(true)
  views.assets = Assets(modalView, views.scriptEditor, true)

  local function createEditorView(systemType)
    if not systemType then
      return
    end

    local success, editorView = pcall(function() return require('imgui.systems.' .. systemType) end)
    if not success then
      log.warn("Failed to load editor for system " .. systemType)
    else
      views[systemType] = editorView(true)
      imguiInterface:addView(dockspace, systemType, views[systemType])
    end
  end

  local function onSystemUpdate(event)
    local systemType = event.system.info.type

    if event.type == SystemChangeEvent.SYSTEM_ADDED then
      createEditorView(systemType)
    else
      if views[systemType] then
        imguiInterface:removeView(dockspace, systemType, views[systemType])
      end
    end
  end

  event:onSystemChange(core, SystemChangeEvent.SYSTEM_ADDED, onSystemUpdate)
  event:onSystemChange(core, SystemChangeEvent.SYSTEM_REMOVED, onSystemUpdate)

  local systems = core:getSystems()
  for name, system in pairs(systems) do
    if system.info then
      local systemType = system.info.type
      if systemType then
        createEditorView(systemType)
      end
    end
  end

  for name, view in pairs(views) do
    viewsMenu[#viewsMenu+1] = MenuItem("window_title." .. view.label,
      function() view.open = not view.open end,
      function() return view.open end
    )
  end

  for name, view in pairs(views) do
    imguiInterface:addView(dockspace, name, view)
  end

end

local fileMenu = {
  MenuItem("menu.file.new", function()
    showWizard()
  end),
  MenuItem("menu.file.open", function()
    projectManager:browseProjects()
  end),
  MenuItem("menu.file.close", function()
    projectManager:close()
  end),
  MenuItem("menu.file.save", function() end),
  Separator,
  MenuItem("menu.file.import", function()
    views.assets:browseImport()
  end)
}

local menus = {
  List("top_menu.file", fileMenu),
  List("top_menu.views", viewsMenu)
}

local renderDockspace = false
imguiInterface:addView(imgui.manager, "mainMenu", Menu(menus))
local tools = ToolsView(true, function()
  local width, height = imgui.DisplaySize()
  local sx, sy = imgui.Scale()
  return 90 * sx, 35 * sy, width - 95 * sx, 40 * sy
end)
imgui.manager:addView("modal", modalView)
imgui.manager:addView("tools", tools)
imgui.manager:addView("workspace", {
  __call = function()
    if not renderDockspace then
      return
    end

    local sx, sy = imgui.Scale()
    local width, height = imgui.DisplaySize()
    dockspace:render(90 * sx, 35 * sy + 43 *sy, width - 95 * sx, height - 83 * sy)
  end
})

local flags = ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoMove + ImGuiWindowFlags_NoScrollbar

imgui.manager:addView("sidePanel", {
  __call = function()
    local width, height = imgui.DisplaySize()
    local sx, sy = imgui.Scale()
    imgui.SetNextWindowPos(5 * sx, 35 * sy)
    imgui.SetNextWindowSize(82 * sx, height - 40 * sy)
    imgui.PushStyleColor_U32(ImGuiCol_WindowBg, imgui.GetColorU32(ImGuiCol_FrameBg, 1))
    imgui.PushStyleVar_2(ImGuiStyleVar_WindowPadding, 0, 0)
    local drawing = imgui.Begin("sidePanel", true, flags)
    if drawing then
      imgui.PushStyleVar_2(ImGuiStyleVar_ItemSpacing, 0, 3 * sy)
      imgui.PushStyleVar(ImGuiStyleVar_FrameRounding, 0)
      local size = imgui.GetContentRegionAvailWidth()
      if imgui.Button("   " .. icons.home .. "\n" .. lm("side_panel.home"), size, size) then
        showWizard()
      end
      if projectManager.openProjectFile then
        if imgui.Button("   " .. icons.world .. "\n" .. lm("side_panel.world"), size, size) then
          hideWizard()
          dockstates[mode] = dockspace:getState()
          mode = "world"
          dockspace:setState(dockstates[mode] or {})
        end
        if imgui.Button("    " .. icons.directions_run .. "\n" .. lm("side_panel.models"), size, size) then
          hideWizard()
          dockstates[mode] = dockspace:getState()
          mode = "character"
          dockspace:setState(dockstates[mode] or {})
        end
      end
      imgui.PopStyleVar(2)
      imgui.End()
    end
    imgui.PopStyleColor()
    imgui.PopStyleVar()
  end
})

local function onAreaLoad(event)
  views.sceneEditor:createCamera("free", "editorMain", {utility = true, policy = EntityFactory.REUSE})
end

local function openProject(event)
  local f = event.file
  if f:match("^.*%.gpf$") ~= nil then
    f = game.filesystem:directory(f)
  elseif not game.filesystem:exists(f .. "/project.gpf") then
    local d = views.assets:info(f)
    if d then
      views.assets:import(f, d)
    end
    return
  end
  local pf = projectManager:readProjectFile(f)
  local choices = {}
  choices[lm("modals.ok")] = function()
    projectManager:open(f)
  end
  choices[lm("modals.cancel")] = function()
  end

  modalView:show(lm("modals.open_project.title"), function()
    imgui.Text(lm("modals.open_project.question", {name = pf:getProjectName()}))
  end, choices)
end

event:bind(core, Facade.LOAD, onAreaLoad)
event:onFileDrop(core, DropFileEvent.DROP_FILE, openProject)

-- used for editor window initial set up
local function setDefaultEditorDimensions(onResizeFinished)
  local window = game:getWindowManager():getWindow(core.settings.render.window.name)
  local x, y, width, height = window:getDisplayBounds()
  -- scale it to 90% of the screen
  local ww = width * 0.9
  local wh = height * 0.9
  local wx = width / 2 - ww / 2
  local wy = height / 2 - wh / 2

  window:setPosition(math.floor(wx), math.floor(wy))
  window:setSize(math.floor(ww), math.floor(wh))
end

local wizardShown = false

function showWizard()
  if wizardShown then
    return
  end

  wizardShown = true
  imguiInterface:addView(imgui.manager, "wizard", wizard)
  wizard:goHome()
  renderDockspace = false
end

function hideWizard()
  if not wizardShown then
    return
  end

  wizardShown = false
  imguiInterface:removeView(imgui.manager, "wizard", wizard)
  renderDockspace = true
end

if imguiInterface:available() then

  local function getLocalizedString(...)
    return lm(table.concat({...}, "."))
  end

  -- configure localization for template rendering engine
  for i = 1,10 do
    local name = "lm"
    if i > 1 then
      name = name .. tostring(i)
    end

    data:addTemplateCallback(name, i, getLocalizedString)
  end

  -- render object as js object with single quotes
  data:addTemplateCallback("js", 1, function(object)
    return json.dumps(object):gsub("\"", "'")
  end)

  -- render object as js object with single quotes
  data:addTemplateCallback("escapePath", 1, function(str)
    return str:gsub('\\', '\\\\')
  end)

  -- starting from project creation wizard
  if not cef then
    log.error("editor requires CEF plugin to be installed")
    game:shutdown(1)
  end

  -- expose method to request localized strings from JS code
  cef:addMessageHandler("requestLocalization", function(strings)
    local result = {}
    for _, s in pairs(strings) do
      result[s] = lm(s)
    end

    return result
  end)

  cef:addMessageHandler("startCreateWizard", function(project)
    local success, err = pcall(wizard.runCreationFlow, wizard, project)
    if not success then
      log.error(err)
      error(err)
    end
  end)

  setDefaultEditorDimensions()
  showWizard()
  createViews()

  projectManager:onProjectOpen(function(projectFile)
    hideWizard()
    local dockstates = (projectFile:getWorkspace() or (globalEditorState or {}).dockState) or {
      world = {},
      character = {},
    }

    log.info("Restoring imgui dock state")
    dockspace:setState(dockstates[mode])
    views.assets:configure()
    event:bind(core, EngineEvent.STOPPING, saveSettings)
  end)

  projectManager:beforeProjectClose(function(projectFile)
    log.info("Saving imgui dock state")
    game:reset()
    dockstates[mode] = dockspace:getState()
    projectManager.openProjectFile:setWorkspace(dockstates)
  end)

  projectManager:onProjectClose(function(projectFile)
    showWizard()
    views.assets:configure()
  end)
else
  log.error("editor requires ImGUI plugin to be installed")
  game:shutdown(1)
end
