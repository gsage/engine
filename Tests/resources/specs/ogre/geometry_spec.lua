describe("#ogre 3D primitives", function()
  local assertQuaternionsEqual = function(ogre, internal)
    assert.equals(ogre.x, internal.x)
    assert.equals(ogre.y, internal.y)
    assert.equals(ogre.z, internal.z)
    assert.equals(ogre.w, internal.w)
  end

  local assertVector3Equal = function(ogre, internal)
    assert.equals(ogre.x, internal.x)
    assert.equals(ogre.y, internal.y)
    assert.equals(ogre.z, internal.z)
  end

  local approximate = function(value, digits)
    local shift = 10 ^ digits
    return math.floor(value * shift) / shift
  end

  describe("Quaternion vs geometry.Quaternion", function()

    it("new creates same object", function()
      local internal = geometry.Quaternion.new(10, 20, 30, 40)
      local ogre = Quaternion.new(10, 20, 30, 40)
      assertQuaternionsEqual(ogre, internal)
    end)

    local internal = geometry.Quaternion.new(1, 10, 20, 1)
    local ogre = Quaternion.new(1, 10, 20, 1)

    describe("operation results equal for", function()
      for _, func in pairs({"getYaw", "getPitch", "getRoll"}) do
        it(func, function()
          local iv = internal[func](internal)
          local ov = ogre[func](ogre)

          assert.equal(approximate(ov.radians, 6), approximate(iv.radians, 6))
          assert.equal(approximate(ov.degrees, 4), approximate(iv.degrees, 4))
        end)
      end

      it("multiplication", function()
        local iv = internal * geometry.Quaternion.new(1, 0, 0.5, 0.5)
        local ov = ogre * Quaternion.new(1, 0, 0.5, 0.5)
        assertQuaternionsEqual(iv, ov)
      end)
    end)
  end)

  describe("Vector3 vs geometry.Vector3", function()
    it("new creates same object", function()
      local internal = geometry.Vector3.new(10, 20, 30)
      local ogre = Vector3.new(10, 20, 30)
      assertVector3Equal(ogre, internal)
    end)

    describe("operation results equal for", function()
      local internal1 = geometry.Vector3.new(10, 20, 30)
      local ogre1 = Vector3.new(10, 20, 30)

      local internal2 = geometry.Vector3.new(30, 0, 6)
      local ogre2 = Vector3.new(30, 0, 6)

      for _, func in pairs({"crossProduct", "squaredDistance", "sum", "multiplication"}) do
        it(func, function()

          local iv, ov
          if func == "multiplication" then
            iv = internal1 * internal2
            ov = ogre1 * ogre2
          elseif func == "sum" then
            iv = internal1 + internal2
            ov = ogre1 + ogre2
          else
            iv = internal1[func](internal1, internal2)
            ov = ogre1[func](ogre1, ogre2)
          end

          if type(iv) == "number" then
            assert.equals(ov, iv)
          else
            assertVector3Equal(ov, iv)
          end
        end)
      end
    end)
  end)

end)
