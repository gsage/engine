Introduction
============

First of all thank you for your interest in the Gsage engine! :)
This tutorial will teach you to create some basic scene using Gsage engine.

If you haven't built engine yet, check out these build tutorials:

| :ref:`linux-build`
| :ref:`mac-build`
| :ref:`windows-build`
|

If you've managed to get engine built, then you should have the following:

* :code:`game`/:code:`game.app`/:code:`game.exe` executable file should be
  in the **<your-cmake-build-folder>/bin** folder. This file is the main game executable.

* All resources should be copied to the **/bin** folder on Windows and Linux or to **game.app/Contents/Resources**
  on the Mac OS X systems.

* :code:`editor`/:code:`editor.app`/:code:`editor.exe` should also be in the game folder.

By running :code:`game` executable, you can start the engine, which is configured in isometric rpg mode by default.
This will change in the future, when the **editor** project will become mature enough.

Modifying level
---------------

Now you can open resources folder and find **levels** folder there.

There should be one **exampleLevel.json** file. This file is a basic level example, you can change it in any way.

For example:

1. Add some model to **resources/models** folder (:code:`world.mesh`).
2. Change entities list, and add this model to position :code:`"x: 0, y: 0, z:0"`:

.. code-block:: javascript

  entities: [
  ...
  {
    "id": "hello",
    "render": {
      "root": {
        "position": "0,0,0",
        "rotation": "1,0,-1,0",
        "scale": "1,1,1",
        "children": [{
          "type": "model",
          "mesh": "world.mesh",
          "castShadows": true
        }]
      }
    }
  }
  ...

When you run the **game** again, you should see the model on the scene.
Also if it's placed close enough to the existing walkable area, you will be able to walk on it.

For more information about entities format see :ref:`entity-format-label`.

Creating characters
-------------------

It's also possible to create dynamic level characters.
All characters can be found in characters folder.
