Custom Start Script
===================

.. note::
  Start scripts are still in a mess.
  Common code parts will be bundled in some module later.

It is possible to write any kind of code in the start script.
But it should be set up properly.

Firstly, it is required add other lua scripts folders to :code:`package.path`.

.. code-block:: lua

  package.path = package.path .. ';' .. getResourcePath('scripts/?.lua') ..         -- main scripts folder, contains generic core logic
                                 ';' .. getResourcePath('behaviors/trees/?.lua') .. -- behavior trees folder
                                 ";" .. getResourcePath('behaviors/?.lua') .. ";"


Setting Up ImGUI
----------------

Check if ImGUI UI manager is installed and register a view:

.. code-block:: lua

  local imguiInterface = require 'imgui.base'

  if imguiInterface:available() then

    -- simple function
    imguiInterface:addView("window", function()
      imgui.TextWrapped("Hello world")
    end, true)
  end

Refer to :ref:`imgui-lua-label` for more details about registering ImGui views in Lua.

Setting Up LibRocket
---------------------

Librocket support can be checked by verifying if there is :code:`onRocketContext` event handler.
Then it is possible to subscribe to that callback:

.. code-block:: lua

   -- librocket initialization
   if event.onRocketContext ~= nil then
     event:onRocketContext(core, RocketContextEvent.CREATE, initLibrocket)
   end

Librocket views, fonts loading should be done in :code:`initLibrocket`:

.. code-block:: lua

  function initLibrocket(event)
    local ctx = rocket.contexts[event.name]
    if not rocketInitialized then
      local fonts =
      {
        "Delicious-Roman.otf",
        "Delicious-BoldItalic.otf",
        "Delicious-Bold.otf",
        "Delicious-Italic.otf",
        "lucida.ttf"
      }
      for _, font in pairs(fonts) do
        resource.loadFont(font)
      end
    end

    main = resource.loadDocument(ctx, "minimal.rml")
    cursor = resource.loadCursor(ctx, "cursor.rml")

    main:Show()
  end

| :code:`initLibrocket` will be called for each context initialized.
| :code:`event.name` can be used to distinguish different contexts and render different set of views for each of them.
|

Subscribing For Key Events
---------------------------

.. code-block:: lua

  function handleKeyEvent(e)
    if e.type == KeyboardEvent.KEY_UP then
      -- handle any key up
    end

    if e.key == Keys.KC_T and e.type == KeyboardEvent.KEY_DOWN then
      -- handle pressing T key
    end
  end

  event:onKeyboard(core, KeyboardEvent.KEY_DOWN, handleKeyEvent)
  event:onKeyboard(core, KeyboardEvent.KEY_UP, handleKeyEvent)

Handling Scene Object Selection
-------------------------------

.. code-block:: lua

  local function onSelect(e)
    local target = eal:getEntity(e.entity)
    if not target then
      return
    end

    ogreView:setGizmoTarget(target)
  end

  -- generic approach
  event:bind(core, SelectEvent.OBJECT_SELECTED, onSelect)

Pipeline Setup
--------------

TBD: when ogre 2.1 support is added
