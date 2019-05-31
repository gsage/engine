require 'imgui.components.webview'
local imguiInterface = require 'imgui.base'
local lm = require 'lib.locales'
local async = require 'lib.async'
local projectManager = require 'editor.projectManager'
local fs = require 'lib.filesystem'

-- project creation wisard view
ProjectWizard = class(ImguiWindow, function(self, textureID, title, open)
  ImguiWindow.init(self, title, false, open)

  local state = editor:getGlobalState()
  self.fileDialogState = state.fileDialogState or {}
  local dialogFolder = self.fileDialogState.folder or os.getenv("HOME")
  local recent = projectManager:getRecentProjects(5)
  self.data = {
    wizard = core.settings.projectWizard,
    hasRecentProjects = #recent > 0,
    recent = recent
  }

  self.homepage = "client:/projectWizard/index.html.template"
  self.webview = WebView(textureID, self.homepage, self.data)
  self.textureID = textureID

  self.flags = ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoMove + ImGuiWindowFlags_NoScrollbar

  cef:addMessageHandler("resetWizard", function()
    self:goHome()
  end)

  cef:addMessageHandler("selectProjectIcon", function()
    local window = game:getWindowManager()
    local files = window:openDialog(Window.FILE_DIALOG_OPEN, "", dialogFolder, {"png", "jpg"})
  end)

  cef:addMessageHandler("pickFolder", function()
    local window = game:getWindowManager()

    if dialogFolder[#dialogFolder] ~= "/" then
      dialogFolder = dialogFolder .. "/"
    end

    local files = window:openDialog(Window.FILE_DIALOG_OPEN_FOLDER, lm("wizard.create.project_folder"), dialogFolder, {})
    if #files > 0 then
      self:updateProjectsFolder(files[1])
      return files[1]
    end
  end)

  cef:addMessageHandler("createProject", function(settings)
    self.webview:openPage("client:/projectWizard/create.progress.html.template", {settings=settings}, function()
      projectManager:create(settings, function(progress)
        local jsCode = [[updateProgress(]] .. tostring(math.floor(progress.percent)) .. [[, 100,"]] .. progress.msg .. [[", "]] .. tostring(progress.color or "#FFFFFF") .. [[", ]] .. tostring(progress.fatalError or false) .. [[)]]
        self.webview:executeJavascript(jsCode)
      end,
      function(success)
        if success then
          local success, err = pcall(projectManager.open, projectManager, fs.path.join(settings.projectPath, settings.projectName))
          self:updateProjectsFolder(settings.projectPath)
          if not success then
            log.error("Failed to open project " .. err)
          end
        end
      end)
    end)
  end)

  cef:addMessageHandler("browseProjects", function()
    projectManager:browseProjects()
  end)

  cef:addMessageHandler("openProject", function(path)
    projectManager:open(path)
  end)
end)

function ProjectWizard:updateProjectsFolder(folder)
  self.fileDialogState.folder = folder
  editor:putToGlobalState("fileDialogState", self.fileDialogState)
  editor:saveGlobalState()
end

function ProjectWizard:goHome()
  local recent = projectManager:getRecentProjects(5)
  self.data = {
    wizard = core.settings.projectWizard,
    hasRecentProjects = #recent > 0,
    recent = recent
  }
  self.webview:openPage(self.homepage, self.data)
end

-- render wisard
function ProjectWizard:__call()
  if not self.open then
    return
  end

  local sx, sy = imgui.Scale()
  imgui.SetNextWindowPos(90 * sx, 35 * sy)
  local width, height = imgui.DisplaySize()
  imgui.SetNextWindowSize(width - 95 * sx, height - 40 * sy)
  imgui.PushStyleVar_2(ImGuiStyleVar_WindowPadding, 3.0 * sx, 3.0 * sy)
  local drawing = self:imguiBegin()
  imgui.PopStyleVar(ImGuiStyleVar_WindowPadding)

  self.webview.visible = drawing

  if not drawing then
    return
  end

  imguiInterface:captureMouse(not imgui.IsWindowHovered())

  local w, h = imgui.GetContentRegionAvail()
  local x, y = imgui.GetCursorScreenPos()

  self.webview:render(w, h)

  self:imguiEnd()
end

-- start project creation flow
function ProjectWizard:runCreationFlow(project)
  local pageData = {}
  local projectType = nil
  if type(project) == "string" then
    projectType = project

    local renderInfo = core:render().info or {}

    local version = "v1"
    if renderInfo.type == "ogre" and renderInfo.version >= 0x020100 then
      version = "v2"
    end

    local s, loaded = data:loadTemplate(fs.path.join("editor", "projectTemplates", projectType .. ".json"), {version = version})
    if not loaded then
      error("No template found for project type " .. projectType)
    end

    local template = json.loads(s)
    local state = editor:getGlobalState()
    local fileDialogState = state.fileDialogState or {}
    local plugins = {}

    -- loading all plugins information
    local pluginFileData = require "editor.plugins"
    local info = pluginFileData.pluginsInfo

    local enabledPlugins = {}
    for _, name in ipairs(template.plugins) do
      enabledPlugins[name] = true
    end

    for key, value in pairs(info) do

      local available = game:getFullPluginPath(key) ~= ""

      plugins[#plugins + 1] = {
        name = key,
        description = lm("plugins.description." .. key),
        details = value,
        enabled = enabledPlugins[key] == true and available,
        available = available
      }
    end

    pageData.settings = {
      projectPath = fileDialogState.folder or os.getenv("HOME"),
      projectName = projectType .. " project",
      projectType = projectType,
      resources = template.resources,
      plugins = plugins,
      systems = pluginFileData.coreSystems,
      selectedSystems = template.systems or {},
      selectedManagers = template.managers or {}
    }
  elseif type(project) == "table" then
    pageData.settings = project
    projectType = project.projectType
  else
    error("Failed to run creation flow: incorrect parameter " .. tostring(project))
  end

  pageData.projectTemplateName = lm("wizard.projects." .. projectType .. ".name")
  self.webview:openPage("client:/projectWizard/create.html.template", pageData)
end

-- activate webview when added to scene
function ProjectWizard:added()
  self.webview:enable()
end

-- cleanup resources
function ProjectWizard:destroy()
  self.webview:disable()
end
