.. _serializable-label:

Serialize Classes
=================

Bindings
--------

:cpp:class:`Gsage::Serializable` is a convenient base class which can be used
to tell Gsage Engine how the cpp object, which does not have any reflection
and dynamic properties/methods lookup, can be converted into a :cpp:class:`Gsage::DataProxy`.

:cpp:class:`Gsage::DataProxy` can be converted to :code:`json`, :code:`msgpack` and :code:`sol::table`.
Lua can pass :code:`sol::table` directly into the engine and it will be wrapped by the :cpp:class:`Gsage::DataProxy`.
:code:`json` and :code:`msgpack` serialization will automatically try to convert all complex types to primitive type.
When converting to :code:`sol::table`, no conversion will take place, so if it is required to use field
that stores a complex type in lua, this type should be registered in lua bindings.
When converting :code:`sol::table` to :cpp:class:`Gsage::DataProxy`, it will try convert primitive types
to complex. However, it is better to initialize the fields as complex types in Lua, as it will work faster.

When you derive class from the :cpp:class:`Gsage::Serializable`, you should tell it what
kind of properties you want to dump and read and how.

There are several macros for that:

.. doxygendefine:: BIND_PROPERTY
.. doxygendefine:: BIND_PROPERTY_WITH_PRIORITY
.. doxygendefine:: BIND_PROPERTY_OPTIONAL
.. doxygendefine:: BIND_ACCESSOR
.. doxygendefine:: BIND_ACCESSOR_WITH_PRIORITY
.. doxygendefine:: BIND_ACCESSOR_OPTIONAL
.. doxygendefine:: BIND_GETTER
.. doxygendefine:: BIND_GETTER_OPTIONAL
.. doxygendefine:: BIND_SETTER_OPTIONAL
.. doxygendefine:: BIND_READONLY_PROPERTY
.. doxygendefine:: BIND_WRITEONLY_PROPERTY

There are various places in code, where you can check out how these methods are used.
For example, from :cpp:class:`Gsage::RenderComponent`:

.. code-block:: cpp

    BIND_ACCESSOR_OPTIONAL("resources", &RenderComponent::setResources, &RenderComponent::getResources);
    BIND_GETTER("root", &RenderComponent::getRootNode);
    BIND_GETTER("animations", &RenderComponent::getAnimations);

Or :cpp:class:`Gsage::SceneNodeWrapper`:

.. code-block:: cpp

    BIND_PROPERTY("offset", &mOffset);

    BIND_ACCESSOR("orientationVector", &SceneNodeWrapper::setOrientationVector, &SceneNodeWrapper::getOrientationVector);
    BIND_ACCESSOR("name", &SceneNodeWrapper::createNode, &SceneNodeWrapper::getId);
    BIND_ACCESSOR("position", &SceneNodeWrapper::setPosition, &SceneNodeWrapper::getPosition);
    BIND_ACCESSOR("scale", &SceneNodeWrapper::setScale, &SceneNodeWrapper::getScale);
    BIND_ACCESSOR("rotation", &SceneNodeWrapper::setOrientation, &SceneNodeWrapper::getOrientation);
    BIND_ACCESSOR("children", &SceneNodeWrapper::readChildren, &SceneNodeWrapper::writeChildren);

Read and Dump
-------------

After the bindings are defined, it will be possible to use :cpp:func:`Gsage::Serializable::dump` and :cpp:func:`Gsage::Serializable::read`
functions.

* :cpp:func:`Gsage::Serializable::read` - will allow to convert :cpp:class:`Gsage::DataProxy` to the class. It will return :code:`false` if any of non-Optional field is missing from the dict.
* :cpp:func:`Gsage::Serializable::dump` - will allow to convert the class :cpp:class:`Gsage::DataProxy`.
