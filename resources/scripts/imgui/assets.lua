require 'lib.class'
require 'lib.utils'
local fs = require 'lib.filesystem'
local imguiInterface = require 'imgui.base'
local icons = require 'imgui.icons'
local lm = require 'lib.locales'
local projectManager = require 'editor.projectManager'
local event = require 'lib.event'

-- preview renderer for images
local ImageViewer = class(function(self)
end)

-- interface implementation, noop for this class
function ImageViewer:setEnabled()
end

function ImageViewer:run(asset)
  if not asset.texture then
    local tex = core:render():getTexture(asset.fullpath)
    if not tex then
      tex = core:render():createTexture(asset.fullpath, {
        width = 320,
        height = 240,
        path = asset.fullpath
      })
    end
    asset.texture = tex
  else
    if not imgui.Texture(asset.texture) then
      imgui.PushItemWidth(20)
      imgui.Spinner("Loading Image", 10, 4, 0xffaa5119)
      imgui.PopItemWidth()
    end
  end
end

local imageViewer = ImageViewer()

local importers = {
  images = function(filepath, asset)
    -- try load with render system
    asset.viewer = ImageViewer
    asset.install = "models"
    return true
  end,
  scripts = function(filepath, asset)
    local str = data:read(filepath)
    function asset:open(assetManager)
      assetManager.scriptEditor:openFile(asset.fullpath)
    end
    if asset.extension == "json" then
      local success, parsed = pcall(function()
        return json.loads(str)
      end)

      if not success then
        log.error("Failed to parse string " .. str .. " " .. parsed)
        return false
      end

      local install = ""

      if parsed.type == "character" then
        install = "characters"
        asset.group = "characters"
      elseif parsed.type == "scene" then
        install = "scenes"
        asset.group = "scenes"
        asset.actions = {{
          id = "data",
          callback = asset.open
        }}
        function asset:open()
          local sceneEditor = imguiInterface:getView("sceneEditor")
          if sceneEditor then
            sceneEditor:loadScene(self.file)
          end
        end
      end

      asset.install = install
      return true
    elseif asset.extension == "lua" then
      -- run luacheck
      asset.install = "scripts"
      return true
    end

    return true
  end
}

-- automatic resource type detection
local function importResource(filepath, asset)
  if asset.import then
    return asset.import(filepath, asset)
  end

  if importers[asset.group] then
    return importers[asset.group](filepath, asset)
  end

  -- no validation, install to the root directory
  return true
end


-- editor assets
Assets = class(ImguiWindow, function(self, modal, scriptEditor, docked, open)
  ImguiWindow.init(self, "assets", docked, open)
  self.icon = icons.folder_special
  self.assets = {}
  self.modal = modal
  self.scriptEditor = scriptEditor

  self.importSession = {}
  self.defaultAssets = {
    scripts = {
      json = {},
      lua = {}
    },
    images = {
      png = {},
      jpg = {},
      jpeg = {},
      tiff = {},
      dds = {}
    },
    archives = {
      zip = {}
    }
  }

  self.maxInput = 256
  self.supportedAssets = {}
  self.assetFilter = imgui.TextBuffer(self.maxInput)
  self.newAssetNameBuffer = imgui.TextBuffer(self.maxInput)
  self.searchCallback = function()
  end
end)

-- configure assets view, check existing systems
function Assets:configure()
  local assets = deepcopy(self.defaultAssets)
  local pluginFileData = require "editor.plugins"

  local plugins = game:getInstalledPlugins()
  for _, name in ipairs(plugins) do
    local pluginAssets = (pluginFileData.pluginsInfo[name] or {}).assets or {}
    for group, extensions in pairs(pluginAssets) do
      if not assets[group] then
        assets[group] = {}
      end

      for extension, val in pairs(extensions) do
        assets[group][extension] = val
      end
    end
  end

  for group, extensions in pairs(assets) do
    for extension, val in pairs(extensions) do
      val.extension = extension
      val.group = group
      self.supportedAssets[extension] = val
    end
  end

  self:refresh()

  -- fill asset creation combobox
  self.creatableAssets = {}
  self.currentAssetToCreate = 0
  self.newAssetPath = ""

  for extension, asset in pairs(self.supportedAssets) do
    if asset.group == "scripts" then
      table.insert(self.creatableAssets, extension .. " (" .. lm("assets.desc." .. asset.extension) .. ")")
    end
  end
  self.creatableAssetsString = table.concat(self.creatableAssets, "\0") .. "\0\0"
end

-- browse and import asset
function Assets:browseImport()
  if not projectManager.openProjectFile then
    return
  end
  local state = editor:getGlobalState()
  self.fileDialogState = state.fileDialogState or {}
  local dialogFolder = self.fileDialogState.folder or os.getenv("HOME")
  local wm = game:getWindowManager()
  local files = wm:openDialog(Window.FILE_DIALOG_OPEN_MULTIPLE, lm("assets.import_dialog_title"), dialogFolder, {})
  for _, file in ipairs(files) do
    local info = self:info(file)
    if info then
      self:import(file, info)
    end
  end
end

-- import assets
function Assets:import(file, info)
  local success, err = pcall(function()
    return importResource(file, info)
  end)
  if not success then
    log.error("Can't import asset " .. file .. " " .. err)
    return
  end

  if not self.modal.open then
    local choices = {}
    choices[lm("modals.ok")] = function()
      local root = projectManager.openProjectFile:getSourceRoot()
      for file, asset in pairs(self.importSession) do
        local filename = fs.path.filename(file)
        local parts = {root, filename}
        if asset.install then
          table.insert(parts, 2, asset.install)
        end
        fs.copy(file, fs.path.join(table.unpack(parts)))
        self:refresh()
      end
      self.importSession = {}
    end
    choices[lm("modals.cancel")] = function()
      self.importSession = {}
    end

    self.modal:show(lm("modals.import_assets.title"), function()
      imgui.Text(lm("modals.import_assets.description"))
      imgui.PushStyleColor_U32(ImGuiCol_ChildWindowBg, imgui.GetColorU32(ImGuiCol_FrameBg, 1))
      imgui.PushStyleVar_2(ImGuiStyleVar_WindowPadding, 5, 5)
      imgui.PushStyleColor_U32(ImGuiCol_Separator, imgui.GetColorU32(ImGuiCol_WindowBg, 1))
      imgui.BeginChild("Assets", 800, 600, ImGuiWindowFlags_AlwaysUseWindowPadding)
      imgui.Columns(3)
      imgui.Text(lm("modals.import_assets.file"))
      imgui.NextColumn()
      imgui.Text(lm("modals.import_assets.type"))
      imgui.NextColumn()
      imgui.Text(lm("modals.import_assets.install"))
      imgui.NextColumn()
      imgui.Separator()
      for file, asset in pairs(self.importSession) do
        imgui.PushID(file)
        imgui.Text(asset.filename)
        imgui.NextColumn()
        imgui.Text(lm("assets.desc." .. asset.extension))
        imgui.NextColumn()
        imgui.Text(asset.install or "./")
        imgui.SameLine()
        if imgui.SmallButton("...", 30, 0) then
          local window = game:getWindowManager()
          local folder = projectManager.openProjectFile:getSourceRoot(), (asset.install or "")
          if not fs.path.exists(folder) then
            folder = projectManager.openProjectFile:getSourceRoot()
          end
          local folders = window:openDialog(Window.FILE_DIALOG_OPEN_FOLDER, lm("assets.import_assets.select_folder"), folder, {})
          if #folders == 1 then
            asset.install = folders[1]:gsub(projectManager.openProjectFile:getSourceRoot(), ""):gsub("^%/(.*)%/$", "%1")
          end
        end
        imgui.NextColumn()
        imgui.PopID()
      end
      imgui.Columns(1)
      imgui.EndChild()
      imgui.PopStyleColor(2)
      imgui.PopStyleVar()
    end, choices)
  end

  info.filename = fs.path.filename(file)
  self.importSession[file] = info
end

-- check asset file information
function Assets:info(file)
  local extension = fs.path.extension(file)
  return deepcopy(self.supportedAssets[extension])
end

-- refresh reloads all assets stored in the sources folder
function Assets:refresh()
  self.assets = {}

  local function addAsset(asset)
    if self.assets[asset.group] == nil then
      self.assets[asset.group] = {}
    end

    if not asset.actions then
      asset.actions = {}
    end
    table.insert(asset.actions, {
      id = "delete",
      callback = function(a)
        local choices = {}
        choices[lm("modals.ok")] = function()
          if fs.rmdir(a.fullpath, true) then
            self:refresh()
          end
        end
        choices[lm("modals.cancel")] = function()
        end
        self.modal:show(lm("modals.confirm_deletion.title", {asset = a.file}), function()
          imgui.TextWrapped(lm("modals.confirm_deletion.desc", {fullpath = a.fullpath}))
        end, choices, 350, 150)
      end
    })

    self.assets[asset.group][asset.fullpath] = asset
  end
  if projectManager.openProjectFile then
    local folder = projectManager.openProjectFile:getSourceRoot()
    local files = scanDirectory(folder, true, false)

    for _, fullpath in ipairs(files) do
      local extension = fs.path.extension(fullpath)
      local file = fs.path.filename(fullpath)

      local asset = self.supportedAssets[extension]
      if asset then
        asset = deepcopy(asset)
        asset.fullpath = fullpath
        asset.file = file
      end
      if asset and importResource(fullpath, asset) then
        addAsset(asset)
      else
        addAsset({
          group = "other",
          file = file,
          fullpath = fullpath
        })
      end
    end

  end


  self.displayedAssets = self.assets
end

function Assets:createAsset(filename, filetype, path, allowSelection, onCreate)
  local choices = {}
  local path = path or "./"
  if filename then
    self.newAssetNameBuffer:write(filename)
  else
    self.newAssetNameBuffer:write("")
  end
  choices[lm("modals.ok")] = function()
    local assetName = self.newAssetNameBuffer:read()
    if assetName == "" then
      error("Failed to create asset: empty asset name")
    end

    local extension = self.creatableAssets[self.currentAssetToCreate + 1]
    extension = split(extension, " ")[1]
    local root = projectManager.openProjectFile:getSourceRoot()
    local fullpath = fs.path.join(root, path, assetName .. "." .. extension)
    fs.mkdir(fs.path.directory(fullpath))
    fs.createFile(fullpath)
    if onCreate then
      onCreate(fullpath)
    else
      self.scriptEditor:openFile(fullpath)
    end
    self:refresh()
  end
  choices[lm("modals.cancel")] = function()
  end

  if filetype then
    for i, value in ipairs(self.creatableAssets) do
      if value:match("^" .. filetype) then
        self.currentAssetToCreate = i - 1
      end
    end
  end

  self.modal:show(lm("modals.create_asset.title"), function()
    imgui.Text(lm("modals.create_asset.description"))
    imgui.InputTextWithCallback(lm("modals.create_asset.name"), self.newAssetNameBuffer, 0, function() end)
    if allowSelection then
      local changed, index = imgui.Combo(lm("modals.create_asset.type"), self.currentAssetToCreate, self.creatableAssetsString)
      if changed then
        self.currentAssetToCreate = index
      end
    else
      imgui.Text(lm("modals.create_asset.type"))
      imgui.SameLine()
      imgui.Text(self.creatableAssets[self.currentAssetToCreate + 1])
    end

    imgui.Text(lm("modals.create_asset.path"))
    imgui.SameLine()
    if self.newAssetPath ~= "" then
      path = self.newAssetPath
    end

    if imgui.Selectable(path) then
      local root = projectManager.openProjectFile:getSourceRoot() .. "/"
      local wm = game:getWindowManager()
      local folders = wm:openDialog(Window.FILE_DIALOG_OPEN_FOLDER, lm("modals.create_asset.title"), root, {})
      if #folders > 0 then
        self.newAssetPath = folders[1]:gsub(root, ""):gsub("^%/(.*)%/$", "%1")
      end
    end
  end, choices)
end

-- render assets browser
function Assets:__call()
  if not self.open then
    return
  end

  local drawing = self:imguiBegin()

  if not drawing then
    return
  end
  local _, height = imgui.GetContentRegionAvail()

  local expandAll = false
  local search = self.assetFilter:read()
  imgui.InputTextWithCallback(lm("assets.search"), self.assetFilter, 0, self.searchCallback)
  if self.search ~= search then
    self.search = search
    if search ~= "" then
      self.displayedAssets = {}
      for group, assets in pairs(self.assets) do
        self.displayedAssets[group] = {}
        for path, asset in pairs(assets) do
          local success, res = pcall(string.find, asset.file, self.assetFilter:read())
          if success and res then
            self.displayedAssets[group][path] = asset
          end
        end
      end
      expandAll = true
    else
      self.displayedAssets = self.assets
    end
  end

  if self.draggedAsset then
    imgui.BeginTooltip("draggedAsset")
    imgui.Text(self.draggedAsset.file)
    imgui.EndTooltip()
  end

  if imgui.IsMouseReleased(0) and self.draggedAsset then
    local view = imguiInterface:getViewDragged()
    if view and view.label == "viewport" then
      local rt = view:getRenderTarget()
      local position = Vector3.ZERO
      if rt then
        position = rt:raycast(30, 0.1, 0xFF)
      end

      self.draggedAsset:addToScene(self, position)
    end
    self.draggedAsset = nil
  end

  local bottomMenuHeight = imgui.GetItemsLineHeightWithSpacing()
  if height > bottomMenuHeight and imgui.BeginChild("assets", 0, -bottomMenuHeight, false, 0) then
    for group, assets in pairs(self.displayedAssets) do

      local hasAssets = false
      for _ in pairs(assets) do
        hasAssets = true
        break
      end

      if hasAssets then
        if expandAll then
          imgui.SetNextTreeNodeOpen(true)
        end

        local groupName = lm("assets.groups." .. group)
        if groupName == lm.MISSING then
          groupName = group
        end
        if imgui.CollapsingHeader(groupName) then
          imgui.PushID(group)
          for path, asset in pairs(assets) do
            imgui.PushID(path)
            imgui.Selectable(asset.file)

            local hovered = imgui.IsItemHovered()

            if hovered and imgui.IsMouseDragging(0, 0.1) and asset.addToScene then
              self.draggedAsset = asset
            end

            if asset.viewer then
              asset.viewer:setEnabled(not self.draggedAsset)
              if hovered and not self.draggedAsset then
                if asset.viewer.getSize then
                  local w, h = asset.viewer:getSize()
                  imgui.SetNextWindowSize(w, h)
                end
                imgui.BeginTooltip()
                asset.viewer:run(asset)
                imgui.EndTooltip()
              end
            end

            if asset.open then
              if hovered and imgui.IsMouseDoubleClicked(0) then
                asset:open(self)
              end
            end

            if asset.actions then
              if imgui.BeginPopupContextItem(asset.fullpath) then
                for _, action in ipairs(asset.actions) do
                  if imgui.Selectable(lm("assets.actions." .. action.id)) then
                    action.callback(asset, self)
                  end
                end
                imgui.EndPopup()
              end
            end

            imgui.PopID()
          end
          imgui.PopID()
        end
      end
    end
    imgui.EndChild()
    if imgui.Button(icons.add_box) then
      self:browseImport()
    end
    imgui.SameLine(0, 2)
    if imgui.Button(icons.refresh) then
      self:refresh()
    end
    imgui.SameLine(0, 2)
    if imgui.Button(icons.insert_drive_file) then
      self:createAsset()
    end
  end

  self:imguiEnd()
end
