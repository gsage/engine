local utils = {}
local fs = require 'lib.filesystem'

-- install all factories from the folder
function utils.loadAll(directory)
  local info = debug.getinfo(1,'S')
  directory = directory or fs.path.directory(info.source:gsub("@", ""))
  local thisFile = fs.path.filename(info.source)

  for _, file in ipairs(fs.path.ls(directory)) do
    if file ~= "base.lua" and file ~= thisFile then
      dofile(fs.path.join(directory, file))
    end
  end
end

return utils
