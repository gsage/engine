local function decorate(cls)
  -- rotate root node
  -- @param quaternion Ogre::Quaternion
  function cls:rotate(quaternion)
    self.render:rotate(quaternion)
  end

  -- translate root node
  -- @param vector Ogre::Vector3
  function cls:translate(vector)
    self.render.root:translate(vector, OgreNode.TS_LOCAL)
  end

  -- get node from root node
  -- @param id id of the node, "." can be used to access nodes deeper in root
  function cls:getSceneNode(id)
    return self.render.root:getSceneNode(id)
  end

  function cls:getObjectsAround(distance, flags, targetId)
    if targetId == nil then
      return {}
    end

    local entities = {}
    local objects = core:render():getObjectsInRadius(self.render.position, distance, flags, targetId)
    for i = 1, #objects do
      table.insert(entities, eal:getEntity(objects[i].id))
    end
    return entities
  end

  return cls
end

eal:extend({system="render", type="ogre"}, decorate)
