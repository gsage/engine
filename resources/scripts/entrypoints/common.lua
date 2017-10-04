package.path = package.path .. ';' .. getResourcePath('scripts/?.lua') ..
                               ';' .. getResourcePath('behaviors/trees/?.lua') .. 
                               ";" .. getResourcePath('behaviors/?.lua') .. ";"
