.. _custom-windowing-label:

Custom Window Manager
=====================

| Gsage Engine allows you to implement new windowing managers.
| Current main windowing system is SDL.
| :code:`WindowManager` is optional, it is possible to let render system create windows on it's own.
|

Steps to register new :code:`WindowManager`:

1. Inherit :cpp:class:`Gsage::WindowManager` interface. Implement abstract methods:

   * :cpp:func:`Gsage::WindowManager::initialize`.
   * :cpp:func:`Gsage::WindowManager::createWindow`.
   * :cpp:func:`Gsage::WindowManager::destroyWindow`.

2. Inherit :cpp:class:`Gsage::Window` interface. Implement abstract methods:

   * :cpp:func:`Gsage::WindowManager::getWindowHandle` should return native OS window handle.
   * :cpp:func:`Gsage::WindowManager::getGLContext` should return open GL pointer. Return 0 if not needed.

3. Add register, unregister methods in the plugin:

.. code-block:: cpp

   ...

   bool MyPlugin::installImpl() {
     mFacade->registerWindowManager<MyWindowManager>("MyWindowManager");
     return true;
   }

   ...

   void MyPlugin::uninstallImpl() {
     mFacade->removeWindowManager("MyWindowManager");
   }

4. Make Gsage load the Plugin and switch to the new WindowManager:

.. code-block:: json

  ...
  "windowManager": {
    "type": "MyWindowManager"
  }

  ...
  "plugins":
  [
  ...
  "MyPlugin"
  ...
  ]
