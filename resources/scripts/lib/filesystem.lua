
local cfs = game.filesystem
local fs = {
  path = {
    isDirectory = function(path)
      return cfs:isDirectory(path)
    end,
    join = function(...)
      return cfs:join({...})
    end,
    ls = function(folder)
      return cfs:ls(folder)
    end,
    filename = function(path)
      return cfs:filename(path)
    end,
    directory = function(path)
      return cfs:directory(path)
    end,
    extension = function(path)
      return cfs:extension(path)
    end,
    exists = function(path)
      return cfs:exists(path)
    end
  },
  rmdir = function(path, recursive)
    return cfs:rmdir(path, recursive)
  end,
  mkdir = function(folder, recursive)
    return cfs:mkdir(folder, not not recursive)
  end,
  createFile = function(path)
    return cfs:createFile(path)
  end,
  copy = function(src, dest)
    return cfs:copy(src, dest)
  end,
  copytreeAsync = function(src, dest)
    return cfs:copytreeAsync(src, dest)
  end,
  unzip = function(src, dst)
    return cfs:unzip(src, dst)
  end,
}

return fs

