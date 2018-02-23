-- test render system functional

describe("#ogre render system", function()
  describe("animations", function() 
    -- unhappy cases
    local cases = {
      ["wrong model name"] = {
        animations = {
          test = {base = "woop.Walk"},
        }
      },
      ["wrong animation name"] = {
        animations = {
          test2 = {base = "a.c"},
        }
      }
    }

    for id, params in pairs(cases) do
      it("should handle " .. id, function()
        game:reset()
        local entity = {
          id = "animtest",
          render = {
            animations = {
              states = params.animations
            }
          }
        }

        data:createEntity(entity)
      end)
    end
  end)

end)
