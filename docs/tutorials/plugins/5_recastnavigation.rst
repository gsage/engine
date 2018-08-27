Recast Navigation Plugin
========================

Plugin Name: :code:`RecastNavigationPlugin`

All render systems should inherit :cpp:class:`Gsage::RenderSystem` and implement :cpp:func:`Gsage::RenderSystem::getGeometry` function.

When :cpp:class:`Gsage::RecastNavigationPlugin` tries to build navmesh, it gets this raw 3D scene information from the :code:`RenderSystem`.

TODO: describe some examples.
