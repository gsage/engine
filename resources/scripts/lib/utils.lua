local fs = require 'lib.filesystem'

-- Save copied tables in `copies`, indexed by original table.
function deepcopy(orig, copies)
    copies = copies or {}
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        if copies[orig] then
            copy = copies[orig]
        else
            copy = {}
            for orig_key, orig_value in next, orig, nil do
                copy[deepcopy(orig_key, copies)] = deepcopy(orig_value, copies)
            end
            copies[orig] = copy
            setmetatable(copy, deepcopy(getmetatable(orig), copies))
        end
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

-- scan directory files
function scanDirectory(folder, recursive, includeDirectories, files)
  local files = files or {}
  for _, file in ipairs(fs.path.ls(folder)) do
    local fullpath = fs.path.join(folder, file)
    local isDir = fs.path.isDirectory(fullpath)

    if isDir and recursive then
      scanDirectory(fullpath, recursive, includeDirectories, files)
    end

    if isDir then
      if includeDirectories then
        table.insert(files, fullpath)
      end
    else
      table.insert(files, fullpath)
    end
  end

  return files
end

-- merge tables recoursively
function tableMerge(t1, t2)
    for k,v in pairs(t2) do
        if type(v) == "table" then
            if type(t1[k] or false) == "table" then
                tableMerge(t1[k] or {}, t2[k] or {})
            else
                t1[k] = v
            end
        else
            t1[k] = v
        end
    end
    return t1
end
