
describe("validate primitive", function()
  local expected = {
    ZERO = {0, 0, 0},
    UNIT_X = {1, 0, 0},
    UNIT_Y = {0, 1, 0},
    UNIT_Z = {0, 0, 1},
    NEGATIVE_UNIT_X = {-1, 0, 0},
    NEGATIVE_UNIT_Y = {0, -1, 0},
    NEGATIVE_UNIT_Z = {0, 0, -1},
    UNIT_SCALE = {1, 1, 1},
  }

  describe("Quaternion", function()
    local q1 = geometry.Quaternion.new(1, 4, 0, 1)
    local q2 = geometry.Quaternion.new(1, 4, 0, 1)
    local q3 = geometry.Quaternion.new(1, 4, 4, 1)

    it("equals", function()
      assert.truthy(q1 == q2)
      assert.falsy(q1 == q3)
    end)

    describe("*", function()
      it("Quaternion", function()
        local q = q1 * q3
        assert.equals(q.w, -16)
        assert.equals(q.x, 4)
        assert.equals(q.y, 4)
        assert.equals(q.z, 18)
      end)

      it("Vector3", function()
        local vector = geometry.Vector3.new(1, 4, 4)
        local v = q1 * vector
        assert.equals(v, geometry.Vector3.new(40, -94, -16))
      end)
    end)
  end)

  describe("Vector3", function()
    local v1 = geometry.Vector3.new(1, 4, 0)
    local v2 = geometry.Vector3.new(1, 4, 0)
    local v3 = geometry.Vector3.new(1, 4, 4)

    describe("check constants", function()
      for c, value in pairs(expected) do
        it(c, function()
          local v = geometry.Vector3[c]
          assert.is_not.is_nil(v)
          assert.equals(v.x, value[1])
          assert.equals(v.y, value[2])
          assert.equals(v.z, value[3])
        end)
      end
    end)

    it("equals", function()
      assert.truthy(v1 == v2)
      assert.falsy(v1 == v3)
    end)

    describe("*", function()
      it("double", function()
        local v = geometry.Vector3.UNIT_SCALE * 5.5
        assert.equals(v, geometry.Vector3.new(5.5, 5.5, 5.5))
      end)

      it("Vector3", function()
        local v = v1 * v3
        assert.equals(v, geometry.Vector3.new(v1.x * v3.x, v1.y * v3.y, v1.z * v3.z))
      end)
    end)
  end)
end)
