-- test render system functional

describe("#ogre render system", function()
  describe("manual texture", function()
    it("should handle bad config", function()
      -- bad texture definition
      local texture = core:render():createTexture("bad", {})

      assert.is_nil(texture)
    end)

    --[[
       *  * :code:`group` resource group to use (optional). Defaults to :code:`DEFAULT_RESOURCE_GROUP_NAME`.
       *  * :code:`textureType` texture type (optional). Defaults to :code:`TEX_TYPE_2D`.
       *  * :code:`width` initial texture width (required).
       *  * :code:`height` initial texture height (required).
       *  * :code:`depth` depth	The depth of the texture (optional). Defaults to 0.
       *  * :code:`numMipmaps` The number of pre-filtered mipmaps to generate. If left to MIP_DEFAULT then the TextureManager's default number of mipmaps will be used (see setDefaultNumMipmaps()) If set to MIP_UNLIMITED mipmaps will be generated until the lowest possible level, 1x1x1.
       *  * :code:`pixelFormat` texture pixel format (optional). Defaults to :code:`PF_R8G8B8A8`.
       *  * :code:`usage` usage type (optional). Defaults to :code:`TU_DEFAULT`.
       *  * :code:`hwGammaCorrection` use gamma correction (optional). Defaults to :code:`false`.
       *  * :code:`fsaa` antialiasing (optional). Defaults to 0.
       *  * :code:`fsaaHint` The level of multisampling to use if this is a render target.
       *                     Ignored if usage does not include TU_RENDERTARGET or if the device does not support it. (optional).
       *  * :code:`explicitResolve` Whether FSAA resolves are done implicitly when used as texture, or must be done explicitly. (optional).
       *  * :code:`shareableDepthBuffer` Only valid for depth texture formats. When true, the depth buffer is a "view" of an existing depth texture (e.g. useful for reading the depth buffer contents of a GBuffer pass in deferred rendering). When false, the texture gets its own depth buffer created for itself (e.g. useful for shadow mapping, which is a depth-only pass).
    ]]--
    definitions = {
      minimal = {width = 320, height = 240},
      maximal = {
        width = 320,
        height = 240,
        group = "General",
        textureType = ogre.TEX_TYPE_2D,
        depth = 0,
        numMipmaps = 0,
        pixelFormat = ogre.PF_R8G8B8A8,
        usage = TU_RENDERTARGET,
        hwGammaCorrection = true,
        fsaa = 0
      }
    }

    for name, config in pairs(definitions) do
      it(name .. " definition", function()
        -- update config
        local texture = core:render():createTexture(name, config)
        assert.is_not.is_nil(texture)
        assert.truthy(texture.valid)
        assert.truthy(core:render():deleteTexture(name))
      end)
    end
  end)

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
