-- registers eal that shows lights and camera pictogramms in the editor
--
local lightPictures = {
  directional = "Utility.Sun",
  point = "Utility.Bulb",
  spot = "Utility.Bulb"
}

local hoverObject = nil

local function decorate(cls)

  cls.onCreate(function(self)
    if not self.render or core:render().info.type ~= "ogre" or (self.vars or {}).utility then
      return
    end

    local changed = false

    local function scanChildren(node, path)
      if not node.children then
        return
      end

      for _, value in ipairs(node.children) do
        if not path then
          path = ""
        elseif path ~= "" then
          path = path .. "."
        end

        if value.type == "node" then
          path = path .. value.name
          scanChildren(value, path)
        end

        if value.type == "model" or value.type == "item" then
          self.meshName = path .. value.name
          self.onSelect = function()
            local mesh = self.render.root:getMovableObject(value.type, self.meshName)

            if mesh then
              mesh:setVisibilityFlags(0xF)
            end
          end

          self.onDeselect = function()
            local mesh = self.render.root:getMovableObject(value.type, self.meshName)
            if mesh then
              mesh:resetVisibilityFlags()
            end
          end
        end

        if value.type == "light" then
          local mat = lightPictures[value.lightType]
          local name = value.name .. "Visualization"
          table.insert(node.children, {
            type = "billboard",
            name = name,
            materialName = mat,
            billboardType = "BBT_POINT",
            billboards = {
              {
                position = Vector3.new(0, 0, 0),
                width = 2,
                height = 2,
                rect = "0.0,0.0,1.0,1.0"
              }
            }
          })
          changed = true

          self.bbPath = path .. name

          self.onHover = function()
            local bb = self.render.root:getBillboardSet(self.bbPath)
            if bb then
              bb:setMaterial(mat .. "/Hovered")
            end
          end

          self.onUnhover = function()
            local bb = self.render.root:getBillboardSet(self.bbPath)
            if bb then
              bb:setMaterial(mat)
            end
          end
        end
      end
    end

    local props = self.render.props
    scanChildren(props.root)

    if changed then
      self.render.props = props
    end
  end)

  cls.onDestroy(function(self)
    if hoverObject == self then
      hoverObject = nil
    end
  end)

  function cls:setSelected(value)
    if self.selected == value then
      return
    end

    self.selected = value

    if self.onSelect and self.onDeselect then
      if self.selected then
        self:onSelect()
      else
        self:onDeselect()
      end
    end
  end

  function cls:setHovered(value)
    if self.hovered == value then
      return
    end

    self.hovered = value

    if hoverObject and hoverObject ~= self then
      hoverObject:setHovered(false)
      hoverObject = nil
    end

    if self.onHover and self.onUnhover then
      if self.hovered then
        self:onHover()
      else
        self:onUnhover()
      end
    end

    hoverObject = self
  end

  return cls
end

eal:extend({system="render"}, decorate)
