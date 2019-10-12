local recast = require 'helpers.recast'
local lm = require 'lib.locales'
require 'lib.class'
require 'imgui.base'
local event = require 'lib.event'
local icons = require 'imgui.icons'

-- imgui view for recast navigation editor
local RecastEditorView = class(ImguiWindow, function(self, open, dockable)
  if dockable == nil then
    dockable = true
  end
  ImguiWindow.init(self, "recast", dockable, open)
  if not core:navigation() then
    error("Navigation system is not loaded")
  end
  self.icon = icons.timeline
  self.buildOptions = core:navigation().defaultOptions
  self.buildOptions.merge = false

  self.mode = {"All", "Selection"}

  self.cellDimensions = {self.buildOptions.cellSize, self.buildOptions.cellHeight}

  self.floatInputs = {
    ["recast.group.walkable"] = {
      walkableSlopeAngle = {"recast.walkable.slope", 1, 0.01, 90, "%.1f", 1},
      walkableHeight = {"recast.walkable.height", 1, 0.01, 90, "%.1f", 1},
      walkableClimb = {"recast.walkable.climb", 0.2, 0.2, 2000000, "%.1f", 1},
      walkableRadius = {"recast.walkable.radius", 0.2, 0.01, 2000000, "%.1f", 1},
    },
    ["recast.group.area"] = {
      mergeRegionArea = {"recast.area.merge_region", 0.5, 0.0, 100000, "%.1f", 1},
      minRegionArea = {"recast.area.min_region", 0.5, 0.0, 100000, "%.1f", 1},
    },
    ["recast.group.misc"] = {
      maxEdgeLen = {"recast.misc.max_edge_len", 0.5, 0.0, 1000000, "%.1f", 1},
      maxSimplificationError = {"recast.misc.max_simplification_error", 0.5, 0.0, 1000000, "%.1f", 1},
      maxVertsPerPoly = {"recast.misc.max_verts_per_poly", 0.5, 0.0, 1000000, "%.1f", 1},
      detailSampleDist = {"recast.misc.detail_sample_dist", 0.5, 0.0, 1000000, "%.1f", 1},
      detailSampleMaxError = {"recast.misc.detail_sample_max_error", 0.5, 0.0, 1000000, "%.1f", 1},
      tileSize = {"recast.misc.tile_size", 0.5, 0.0, 1000000, "%.1f", 1},
      trisPerChunk = {"recast.misc.tris_per_chunk", 1, 0.0, 1000000, "%.0f", 1},
    }
  }

  self.navmeshDraw = nil

  self.flags = ImGuiWindowFlags_NoScrollbar + ImGuiWindowFlags_NoScrollWithMouse

  self.flagsCb = {
    ["recast.group.filter"] = {
      filterLowHangingObstacles = {"recast.flags.low_hanging_obstacles"},
      filterLedgeSpans = {"recast.flags.ledge_spans"},
      filterWalkableLowHeightSpans = {"recast.flags.walkable_low_height_spans"},
    },
    ["recast.group.flags"] = {
      dynamicRebuild = {"recast.flags.dynamic_rebuild"},
      merge = {"recast.flags.merge"}
    },
    ["recast.group.visualize"] = {
      showNavMesh = {"recast.visualize.show_navmesh", function(enabled)
        if enabled then
          self.navmeshDraw = recast.visualizeNavmesh()
        elseif self.navmeshDraw then
          core:removeEntity(self.navmeshDraw.id)
          self.navmeshDraw = nil
        end
      end},
      showRawGeom = {"recast.visualize.show_rawgeom", function(enabled)
        if enabled then
          self.geomDraw = recast.visualizeGeom()
        elseif self.geomDraw then
          core:removeEntity(self.geomDraw.id)
          self.geomDraw = nil
        end
      end},
      showPath = {"recast.visualize.show_path", function(enabled)
        self:setShowNavPathEnabled(enabled)
      end},
    },
  }
  self.dynamicUpdateFrequency = 20
  self.updateCounter = 0
  self.paths = {}

  self.navpathDraw = function(event)
    if event.path then
      local points = event.path:points()
      local pos = eal:getEntity(event.entityID).render.position
      table.insert(points, 1, geometry.Vector3.new(pos.x, pos.y, pos.z))
      self.paths[event.entityID] = recast.visualizePath(points, true, event.entityID)
    end
  end
end)

-- render editor view
function RecastEditorView:__call()

  if self:imguiBegin() then
    local anyChange = false
    imgui.Text(lm("recast.rebuild_settings"))
    imgui.Separator()
    if imgui.BeginChild("rebuildSettings", 0, -imgui.GetItemsLineHeightWithSpacing() - 3, false, 0) then
      anyChange = imgui.DragFloatN(lm("recast.cell_dimensions"), self.cellDimensions, 2, 0.02, 0.1, 200, "%.3f", 1)
      for group, inputs in pairs(self.floatInputs) do
        imgui.Text(lm(group))
        for name, params in pairs(inputs) do
          local changed, newVal = imgui.DragFloat(
          lm(params[1]),
          self.buildOptions[name],
          params[2],
          params[3],
          params[4],
          params[5],
          params[6]
          )
          if changed then
            anyChange = true
            self.buildOptions[name] = newVal
          end
        end
        imgui.Separator()
      end

      for group, cbs in pairs(self.flagsCb) do
        imgui.Text(lm(group))
        for name, params in pairs(cbs) do
          local changed, newVal = imgui.Checkbox(
          lm(params[1]),
          self.buildOptions[name]
          )
          if changed then
            if params[2] then
              params[2](newVal)
            end
            self.buildOptions[name] = newVal
          end
        end
        imgui.Separator()
      end
    end
    imgui.EndChild()

    imgui.Separator()

    local update = false

    if anyChange and self.buildOptions.dynamicRebuild then
      self.scheduleUpdate = true
    end

    if self.scheduleUpdate and self.updateCounter % self.dynamicUpdateFrequency == 0 then
      update = true
      self.scheduleUpdate = false
    end

    self.updateCounter = self.updateCounter + 1

    if imgui.Button(lm("recast.rebuild")) or update then
      self.buildOptions.cellSize = self.cellDimensions[1]
      self.buildOptions.cellHeight = self.cellDimensions[2]
      core:navigation():rebuildNavMesh(self.buildOptions)

      if self.buildOptions.showNavMesh then
        if self.navmeshDraw then
          core:removeEntity(self.navmeshDraw.id)
          self.navmeshDraw = nil
        end
        self.navmeshDraw = recast.visualizeNavmesh()
      end
      if self.buildOptions.showRawGeom then
        if self.geomDraw then
          core:removeEntity(self.geomDraw.id)
          self.geomDraw = nil
        end
        self.geomDraw = recast.visualizeGeom()
      end
    end
    self:imguiEnd()
  end
end

function RecastEditorView:setShowNavPathEnabled(value)
  if self.buildOptions.showPath and not value then
    event:unbind(core, RecastEvent.NAVIGATION_START, self.navpathDraw)
  elseif not self.buildOptions.showPath and value then
    event:onRecastEvent(core, RecastEvent.NAVIGATION_START, self.navpathDraw)
  end
end

return RecastEditorView
