.. _serializable-label:

Serialize Classes
=================

Bindings
--------

:cpp:class:`Gsage::Serializable` is a convenient base class which can be used
to tell the Gsage Engine how the cpp object, which does not have any reflection
and dynamic properties/methods lookup, can be converted into a :cpp:class:`Gsage::Dictionary`.

:cpp:class:`Gsage::Dictionary` can be converted to :code:`json` and :code:`msgpack`.

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

* :cpp:func:`Gsage::Serializable::read` - will allow to convert :cpp:class:`Gsage::Dictionary` to the class. It will return :code:`false` if any of non-Optional field is missing from the dict.
* :cpp:func:`Gsage::Serializable::dump` - will allow to convert the class :cpp:class:`Gsage::Dictionary`.
