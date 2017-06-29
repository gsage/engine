return function(self)
  local model = self:render().root:getEntity('sinbad')
  local sword = {
    type = 'model',
    mesh = 'Sword.mesh',
    castShadows = true
  }

  model:attachToBone('Handle.R', 's1', sword)
  model:attachToBone('Handle.L', 's2', sword)
  self:render():playAnimation('closeHands', 1, 0, 0, false)
  if rocket ~= nil then
    function initStatusBar()
      statusBar.setMaximumHP(self:stats():getNumber('maxHP', 1))
      statusBar.setCurrentHP(self:stats():getNumber('hp', 1))
    end

    function updateHp(event)
      if event.id == 'maxHP' then
        statusBar.setMaximumHP(self:stats():getNumber('maxHP', 1))
      end

      if event.id == 'hp' then
        statusBar.setCurrentHP(self:stats():getNumber('hp', 1))
      end
    end

    initStatusBar()
    event:onStat(self:stats(), 'statChange', updateHp)
  end
end
