
-- This helpers are wrapping basic functional of the engine

-- Resource loading helpers
resource = resource or {}

local eal = 'lib.eal.manager'

if rocket ~= nil then
  function resource.loadFont(fileName)
    rocket:LoadFontFace(getResourcePath('fonts/' .. fileName))
  end

  function resource.loadDocument(context, documentName)
    return context:LoadDocument(getResourcePath('ui/' .. documentName))
  end

  function resource.loadCursor(context, cursor)
    return context:LoadMouseCursor(getResourcePath('ui/' .. cursor))
  end
end

-- View related functions

view = view or {}

function table.toString(t)
  local s = {}
  for k, v in pairs(t) do
    if type(v) == "table" then
      v = table.toString(v)
    else
      v = tostring(v)
    end
    s[#s+1] = tostring(k) .. " = " .. v
  end
  s = table.concat(s, ', ')
  return "{" .. s .. "}"
end
