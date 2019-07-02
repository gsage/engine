require 'lib.class'
require 'lib.utils'
local async = require 'lib.async'
local lm = require 'lib.locales'
local event = require 'lib.event'
local path = require 'lib.path'
local fs = require 'lib.filesystem'

local function hasValue(tab, val)
  for index, value in ipairs(tab) do
    if value == val then
      return true
    end
  end

  return false
end

-- represents project file access
local ProjectFile = class(function(self, projectRoot)
  self.path = fs.path.join(projectRoot, "project.gpf")
  self.projectRoot = projectRoot
  self.data = {
    version = 1.0
  }
end)

-- read project file
function ProjectFile:read()
  local success, data = pcall(function()
    local res, loaded = json.load(self.path)
    if not loaded then
      error("Project file " .. self.path .. " does not exist")
    end
    return res
  end)

  if not success then
    error(data)
  end

  self.data = data
end

-- get source root
function ProjectFile:getSourceRoot()
  return fs.path.join(self.projectRoot, "sources")
end

-- write project data back to file
function ProjectFile:write()
  if self.data == nil then
    log.error("Failed to write project file " .. self.path .. ": no data to write")
    return
  end
  if not json.dump(self.path, self.data) then
    error(lm("wizard.create.progress.writing_project_file_failed"))
  end
end

-- set project name
function ProjectFile:setProjectName(name)
  self.data.projectName = name
end

-- get project name
function ProjectFile:getProjectName()
  return self.data.projectName
end

-- save workspace
function ProjectFile:setWorkspace(state)
  self.data.workspace = state
end

-- get workspace
function ProjectFile:getWorkspace(state)
  return self.data.workspace
end

-- adds render system resource folder
function ProjectFile:addRenderablesDir(dir)
  if not self.data.resources then
    self.data.resources = {}
  end

  if not self.data.resources.render then
    self.data.resources.render = {}
  end

  table.insert(self.data.resources.render, dir)
end

-- get render system resource folders
function ProjectFile:getRenderablesDirs()
  local render = (self.data.resources or {}).render
  if not render then
    return {}
  end

  return render
end

-- adds scripts resource folder
function ProjectFile:addScriptsDir(dir)
  if not self.data.resources then
    self.data.resources = {}
  end

  if not self.data.resources.script then
    self.data.resources.script = {}
  end

  table.insert(self.data.resources.script, dir)
end

-- gets scripts folders
function ProjectFile:getScriptsDirs()
  local script = (self.data.resources or {}).script
  if not script then
    return {}
  end

  return deepcopy(script)
end

-- adds plugin (used in release build of the game)
function ProjectFile:enablePlugin(pluginName)
  if not self.data.plugins then
    self.data.plugins = {}
  end

  self.data.plugins[pluginName] = true
end

-- get enabled plugins
function ProjectFile:getEnabledPlugins()
  local plugins = self.data.plugins
  if not plugins then
    return {}
  end

  local res = {}
  for name in pairs(plugins) do
    res[#res + 1] = name
  end

  return res
end

-- set managers config
function ProjectFile:setManagers(managers)
  self.data.managers = managers
end

-- get managers config
function ProjectFile:getManagers()
  return self.data.managers or {}
end

-- set systems config
function ProjectFile:setSystems(systems)
  self.data.systems = systems
end

-- get systems config
function ProjectFile:getSystems()
  return self.data.systems or {}
end

-------------------------------------------------------------------------

-- project manager handles project operations like
-- - creation
-- - opening
local ProjectManager = class(function(self)
  self.progressCallbacks = {}
  self.projectOpenCallbacks = {}
  self.projectCloseCallbacks = {}
  self.beforeCloseCallbacks = {}
  self.copyWorkers = {}
  self.onFileCopied = function(event)
    for _, cb in ipairs(self.progressCallbacks) do
      cb(event.id, event.type == FileEvent.COPY_COMPLETE)
    end
  end

  self.onSystemStart = function(event)
    if not self.openingProject then
      return
    end
    self.awaitedSystems[event.systemID] = true
    local ready = true
    for id, started in pairs(self.awaitedSystems) do
      if started ~= true then
        ready = false
        break
      end
    end

    if ready then
      for _, cb in ipairs(self.projectOpenCallbacks) do
        cb(self.openProjectFile)
      end
      self.openingProject = false
      self.awaitedSystems = {}
    end
  end

  event:onFile(game.filesystem, FileEvent.COPY_COMPLETE, self.onFileCopied)
  event:onFile(game.filesystem, FileEvent.COPY_FAILED, self.onFileCopied)
  event:onSystemChange(core, SystemChangeEvent.SYSTEM_STARTED, self.onSystemStart)

  self.openProjectFile = nil
  self.editorWorkdir = core.env.workdir
  self:saveInitialSettings()

  self.openingProject = false
end)

-- create.progress creates folder, copy resources
function ProjectManager:create(settings, onProgress, onComplete)
  local projectPath = fs.path.join(settings.projectPath, settings.projectName)
  local sourcePath = fs.path.join(projectPath, "sources")
  local percent = 0
  local function message(msg, color, fatalError)
    onProgress({
      color = color or "#FFFFFF",
      msg = msg,
      percent = percent,
      fatalError = fatalError,
    })
  end

  local function createFolder(path, recursive)
    return fs.mkdir(path, not not recursive)
  end

  local projectFile = ProjectFile(projectPath)
  local finalize = function(success)
    self.copyWorkers = {}
    self.progressCallbacks = {}
    if success then
      message(lm("wizard.create.progress.writing_project_file"))
      local err = nil
      success, err = pcall(function()
        projectFile:write()
      end)
      if success then
        message(lm("wizard.create.progress.writing_configs"))
        for _, plugin in ipairs(settings.plugins) do
          if plugin.enabled then
            projectFile:enablePlugin(plugin.name)
          end
        end
        projectFile:setManagers(settings.selectedManagers)
        projectFile:setSystems(settings.selectedSystems)
        success, err = pcall(function()
          projectFile:write()
        end)
        message(lm("wizard.create.progress.opening_project"))
      else
        message(tostring(err), "#FF0000", true)
      end
    end
    onComplete(success)
  end

  local success, err = pcall(projectFile.read, projectFile)
  if success then
    percent = 100
    message(lm("wizard.create.progress.project_already_exists"), "#FF0000", true)
    finalize(false)
    return
  end

  createFolder(projectPath, true)
  message(lm("wizard.create.progress.folder_created", {path = projectPath}))
  createFolder(sourcePath, true)
  message(lm("wizard.create.progress.folder_created", {path = sourcePath}))

  -- initial project configuration
  projectFile:setProjectName(settings.projectName)
  projectFile:setWorkspace(editor:getGlobalState().dockState)

  local step = 100 / math.max(#settings.resources, 1)
  filesCount = {}

  self.progressCallbacks[#self.progressCallbacks + 1] = function(id, success)
    local worker = self.copyWorkers[id]
    if not worker then
      return
    end
    local p = step
    if filesCount[id] then
      p = step / filesCount[id]
    end
    percent = percent + p

    local msg = ""
    local color = "#FFFFFF"
    if success then
      msg = lm("wizard.create.progress.file_copied", {flow = worker})
    else
      msg = lm("wizard.create.progress.file_copy_failed", {flow = worker})
      color = "#FF0000"
    end
    message(msg, color)

    if percent >= 100 then
      finalize(true)
    end
  end

  local workdir = core.settings.workdir
  if #settings.resources == 0 then
    percent = 100
    finalize(true)
    return
  end

  local function unzip(src, dst, filecount)
    local unzipped = fs.unzip(
      src,
      dst
    )

    local color = "#FFFFFF"
    local msg = ""
    if unzipped then
      msg = lm("wizard.create.progress.unzip_success", {src = src, dst = dst})
    else
      msg = lm("wizard.create.progress.unzip_failed", {src = src, dst = dst})
      color = "#FF0000"
    end
    message(msg, color)
    percent = percent + step / filecount

    if percent >= 100 then
      finalize(true)
    end
  end

  local function processFolder(resource)
    local folder = fs.path.join(workdir, resource.folder)
    local files = {}
    if not fs.path.isDirectory(folder) then
      files = {folder}
      folder = fs.path.directory(folder)
    else
      files = scanDirectory(folder, true, false)
    end

    for _, fullpath in ipairs(files) do
      local p = fs.path.directory(fullpath):gsub(folder, "")
      local destPath = fs.path.join(sourcePath, resource.install, p)
      local extension = fs.path.extension(fullpath)
      if not createFolder(destPath, true) then
        message(lm("wizard.create.progress.failed_to_create_directory", {directory = destPath}), "#FF0000", true)
        finalize(false)
        break
      end

      if extension:lower() == "zip" then
        unzip(fullpath, destPath .. "/", #files)
      else
        local file = fs.path.filename(fullpath)
        local worker = fs.copytreeAsync(fullpath, fs.path.join(destPath, file))
        filesCount[worker:getID()] = #files
        self.copyWorkers[worker:getID()] = worker
      end
    end
  end

  for _, resource in ipairs(settings.resources) do
    if resource.source == "folder" then
      processFolder(resource)
    elseif parts[1] == "http" then
      percent = percent + step
      log.error("HTTP hosted resources is not supported yet")
    else
      percent = percent + step
      log.error("Unknown resource type")
    end

    if resource.type == "render" then
      projectFile:addRenderablesDir(resource.install)
    elseif resource.type == "script" then
      projectFile:addScriptsDir(resource.install)
    end
  end
end

-- read project file
function ProjectManager:readProjectFile(projectPath)
  local projectFile = ProjectFile(projectPath)
  projectFile:read()
  return projectFile
end

-- open a project
function ProjectManager:open(projectPath)
  if self.openingProject then
    return false
  end

  if self.openProjectFile then
    self:close(false)
  end

  log.info("Opening project " .. projectPath)
  local projectFile = ProjectFile(projectPath)
  local success, err = pcall(projectFile.read, projectFile)
  if not success then
    log.error("Failed to open project " .. err)
    return false
  end

  self.openProjectFile = projectFile
  local sourcesDir = fs.path.join(self.openProjectFile.projectRoot, "sources")

  local scripts = projectFile:getScriptsDirs()
  for i, path in pairs(scripts) do
    scripts[i] = fs.path.join(sourcesDir, path)
  end

  if #scripts > 0 then
    log.info("Adding script folders:\n\t" .. table.concat(scripts))
    -- load script folders
    path:addScriptFolders(projectFile.projectRoot, scripts, "path")
  end

  log.info("Restoring workspace state")
  local state = editor:getGlobalState()
  local projects = state.recentProjects or {}

  for i, project in ipairs(projects) do
    if project.path == projectPath then
      table.remove(projects, i)
    end
  end

  table.insert(projects, 1, {
    opened = os.date("%x %X"),
    path = projectPath,
    name = projectFile:getProjectName()
  })

  editor:putToGlobalState("recentProjects", projects)
  if not editor:saveGlobalState() then
    log.error("Failed to save recent projects")
  end

  -- change workdir to project path
  core:setEnv("workdir", sourcesDir)
  data:addSearchFolder(0, self.openProjectFile.projectRoot)
  data:addSearchFolder(1, self.openProjectFile:getSourceRoot())

  local settings = deepcopy(core.settings)
  settings.dataManager.scenesFolder = "sources/" .. settings.dataManager.scenesFolder
  settings.dataManager.charactersFolder = "sources/" .. settings.dataManager.charactersFolder
  settings.dataManager.savesFolder = "sources/"
  table.insert(settings.pluginsFolders, 1, self.openProjectFile.projectRoot)

  for _, plugin in ipairs(self.openProjectFile:getEnabledPlugins()) do
    local skip = false
    for _, installed in ipairs(settings.plugins) do
      if installed == plugin then
        skip = true
        break
      end
    end

    if not skip then
      settings.plugins[#settings.plugins + 1] = plugin
    end
  end

  self.awaitedSystems = {}
  for system, t in pairs(self.openProjectFile:getSystems()) do
    settings.systemTypes[system] = t
    if not hasValue(settings.systems, system) then
      settings.systems[#settings.systems + 1] = system
    end
    self.awaitedSystems[system] = false
  end


  if self.openProjectFile.data.resources then
    for system, resources in pairs(self.openProjectFile.data.resources) do
      local folders = {}
      for i, folder in ipairs(resources) do
        folders[i] = fs.path.join(self.openProjectFile:getSourceRoot(), folder)
      end

      if not settings[system] then
        settings[system] = {}
      end

      settings[system].resources = folders
    end
  end

  self.openingProject = true
  game:reset()
  if not game:configure(settings, gsage.configure.All + gsage.configure.RestartSystems) then
    log.error("Failed to reconfigure game core")
    error("failed to reconfigure game core")
  end
  return true
end

-- close a project
function ProjectManager:close(withCallback)
  if not self.openProjectFile then
    return
  end

  for _, cb in ipairs(self.beforeCloseCallbacks) do
    cb(self.openProjectFile)
  end

  -- unload script folders
  path:uninstallPathBundle(self.openProjectFile.projectRoot, "path")

  if withCallback == nil or withCallback then
    for _, cb in ipairs(self.projectCloseCallbacks) do
      cb(self.openProjectFile)
    end
  end

  -- revert working directory back to editor root
  core:setEnv("workdir", self.editorWorkdir)
  data:removeSearchFolder(self.openProjectFile:getSourceRoot())
  data:removeSearchFolder(self.openProjectFile.projectRoot)
  self.openProjectFile:write()

  -- reconfigure facade to the initial state
  game:configure(self.initialSettings, gsage.configure.All + gsage.configure.RestartSystems)
  self.openProjectFile = nil
end

-- get recent projects
function ProjectManager:getRecentProjects(limit)
  local state = editor:getGlobalState()
  local projects = state.recentProjects or {}
  local res = {}
  for i = 1,math.min(#projects, limit) do
    res[#res + 1] = projects[i]
  end

  return res
end

function ProjectManager:onProjectOpen(callback)
  self.projectOpenCallbacks[#self.projectOpenCallbacks + 1] = callback
end

function ProjectManager:onProjectClose(callback)
  self.projectCloseCallbacks[#self.projectCloseCallbacks + 1] = callback
end

function ProjectManager:beforeProjectClose(callback)
  self.beforeCloseCallbacks[#self.beforeCloseCallbacks + 1] = callback
end

function ProjectManager:saveInitialSettings()
  self.initialSettings = deepcopy(core.settings)

  local systems = {}
  local plugins = {}

  -- save programmatically installed plugins to the initialSettings
  for i, v in ipairs(game:getInstalledPlugins()) do
    plugins[i] = v
  end
  -- save programmatically installed systems to the initialSettings
  self.initialSettings.plugins = plugins
  for key, value in pairs(core:getSystems()) do
    if value.info.type then
      self.initialSettings.systemTypes[key] = value.info.type
    end
    table.insert(systems, key)
    self.initialSettings[key] = value.config
  end
  self.initialSettings.systems = systems
end

function ProjectManager:browseProjects()
  local state = editor:getGlobalState()
  fileDialogState = state.fileDialogState or {}
  local dialogFolder = fileDialogState.folder or os.getenv("HOME")
  local wm = game:getWindowManager()
  local files, status, err = wm:openDialog(Window.FILE_DIALOG_OPEN_FOLDER, lm("wizard.open_project"), dialogFolder, {})
  if status == Window.FILE_DIALOG_FAILURE then
    log.error("Failed to open file dialog " .. err)
    return
  end

  if #files > 0 then
    self:open(files[1])
  end
end

local projectManager = ProjectManager()

return projectManager
