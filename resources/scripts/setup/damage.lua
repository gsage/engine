return function(self)
  damageHit = self:render().root:getParticleSystem('damageHit')
  damageMiss = self:render().root:getParticleSystem('damageMiss')
  damageEnemy = self:render().root:getParticleSystem('damageEnemy')
end
