.. _custom-input-handler-label:

Custom Input Handler
====================

New handler should be registered in the :cpp:class:`Gsage::GsageFacade` by calling :cpp:func:`Gsage::GsageFacade::registerInputFactory`.
As you can get from the method specialization, you should pass class which implements interface :cpp:class:`Gsage::AbstractInputFactory`.

Any concrete factory should create input handlers which implement interface :cpp:class:`Gsage::InputHandler`.

Input factory will requested to create new handler for each created window. After creation, this handler will receive all resize and close events.

:cpp:class:`Gsage::OisInputFactory` can be taken as a good starting point for a new input implementation.

.. code-block:: cpp

  InputHandler* OisInputFactory::create(size_t windowHandle, Engine* engine)
  {
    return new OisInputListener(windowHandle, engine);
  }

.. warning::
  Due to the fact that only ois system is supported at the moment, interfaces can be unfitting for other input
  handlers, feel free to extend this interface

