local eal = require 'lib.eal.manager'
require 'factories.base'
require 'lib.utils'
local fs = require 'lib.filesystem'

local CharacterFactory = class(EntityFactory, function(self)
  EntityFactory.init(self, "character")
  self.templates = {}
end)

characters = characters or CharacterFactory()

function CharacterFactory:getTemplates()
  local charactersDirectory = fs.path.join(core.env.workdir, "characters")
  if self.charactersFolder ~= charactersDirectory then
    self.charactersFolder = charactersDirectory
    for _, file in ipairs(fs.path.ls(self.charactersFolder)) do
      if fs.path.extension(file) == "json" then
        local type = fs.path.basename(file)
        self.templates[type] = function(name, settings)
          local e = data:createEntity(fs.path.join(self.charactersFolder, file), settings)
          if not e then
            return nil
          end

          return eal:getEntity(e.id)
        end
      end
    end
    self:updateAvailableTypes()
  end
end

function CharacterFactory:getAvailableTypes()
  self:getTemplates()
  return EntityFactory.getAvailableTypes(self)
end

function CharacterFactory:updateAvailableTypes()
  EntityFactory.updateAvailableTypes(self)
  for key in pairs(self.templates) do
    table.insert(self.availableTypes, key)
  end
end

-- override create method to implement dynamic character scan
function CharacterFactory:getConstructor(type)
  self:getTemplates()
  return self.templates[type] or self.definitions[type]
end

return characters
