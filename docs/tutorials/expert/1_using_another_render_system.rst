Using Another Render System
===========================

Writing the plugin which introduces another :code:`RenderSystem` is pretty complicated thing to do, because
many other plugins usually depend on :code:`RenderSystem` implementation.

The most annoying thing is to write all UIManager adapters for newly implemented :code:`RenderSystem`.
Besides that, it is required to wrap all scene objects into serializable classes.

TODO: describe what interfaces to implement
TODO: describe how to use generic 3D primitives

Implementing 2D Render System
-----------------------------

TODO: never done that yet, so hard to describe it yet

Using EAL
---------

EAL can help to simplify migration from one render system to another.
Or make Lua code written for one :code:`RenderSystem` work with another :code:`RenderSystem` without any changes.

TODO: more details

Implement Factories
-------------------

If it is impossible to use the same data structure for render component of the new system, it is possible to
achieve compatibility by using different set of factories for different systems.

This way it will be possible to make it use the same interface, having different implementation underneath.
