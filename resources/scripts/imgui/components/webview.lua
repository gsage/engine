local event = require 'lib.event'

WebView = class(function(self, textureID, startPage, pageData)
  -- temporarily use ogre view as an image
  self.image = imgui.createOgreView()
  self.textureID = textureID
  self.texture = nil
  self.windowName = core.settings.render.window.name
  self.cefwebview = nil
  self.startPage = startPage or ""
  self.pageData = pageData or {}
  self.oversample = 1
  self.windowMoved = false

  self.x = 0
  self.y = 0

  self.onScreenPositionX = self.x
  self.onScreenPositionY = self.y

  self.width = 0
  self.height = 0
  self.handleInput = false

  self.pendingScripts = {}
  self.visible = false
  self.wasInBounds = false

  local convertCoordinates = function(x, y)
    return (x - self.x) * self.oversample, (y - self.y) * self.oversample
  end

  local isOutOfBounds = function(x, y)
    local right = self.x + self.width
    local bottom = self.y + self.height

    return x > right or x < self.x or y > bottom or y < self.y
  end

  self.focused = false

  self.proxyMouseEvent = function(event)
    if event.dispatcher ~= self.windowName or not self.visible then
      return
    end

    if isOutOfBounds(event.x, event.y) then
      if self.cefwebview and event.type == MouseEvent.MOUSE_UP and self.focused then
        self.cefwebview:executeJavascript('document.activeElement.blur();')
        self.focused = false
      end

      if self.wasInBounds and self.cefwebview then
        self.cefwebview:pushEvent(Event.new(CEFWebview.MOUSE_LEAVE))
      end
      self.wasInBounds = false

      if event.type ~= MouseEvent.MOUSE_UP then
        return
      end
    else
      self.wasInBounds = true
    end


    if event.type == MouseEvent.MOUSE_DOWN then
      self.focused = true
    end

    if self.cefwebview then
      local x, y = convertCoordinates(event.x, event.y)
      event.x = x
      event.y = y
      self.cefwebview:pushEvent(event)
    end
  end

  self.proxyKeyboardEvent = function(event)
    if self.handleInput and self.cefwebview and self.visible then
      self.cefwebview:pushEvent(event)
    end
  end

  self.onWindowMove = function(event)
    self.onScreenPositionX = event.x
    self.onScreenPositionY = event.y
    self.windowMoved = true
  end

  --event:onKeyboard(core, KeyboardEvent.KEY_UP, self.proxyKeyboardEvent)
  event:onKeyboard(core, KeyboardEvent.KEY_DOWN, self.proxyKeyboardEvent)
  event:onInput(core, TextInputEvent.INPUT, self.proxyKeyboardEvent)
  event:onMouse(core, MouseEvent.MOUSE_DOWN, self.proxyMouseEvent)
  event:onMouse(core, MouseEvent.MOUSE_UP, self.proxyMouseEvent)
  event:onMouse(core, MouseEvent.MOUSE_MOVE, self.proxyMouseEvent)

  event:onWindow(core, WindowEvent.MOVE, self.onWindowMove)
end)

function WebView:destroy()
  event:unbind(core, KeyboardEvent.KEY_DOWN, self.proxyKeyboardEvent)
  event:unbind(core, TextInputEvent.INPUT, self.proxyKeyboardEvent)
  event:unbind(core, MouseEvent.MOUSE_DOWN, self.proxyMouseEvent)
  event:unbind(core, MouseEvent.MOUSE_UP, self.proxyMouseEvent)
  event:unbind(core, MouseEvent.MOUSE_MOVE, self.proxyMouseEvent)
  event:unbind(core, WindowEvent.MOVE, self.onWindowMove)

  if cef then
    cef:removeWebview(self.textureID)
    self.cefwebview = nil
  end
end

function WebView:renderPage(page, data)
  if self.cefwebview then
    self.cefwebview:renderPage(page, data)
  else
    self.startPage = page
    self.pageData = data
  end
end

function WebView:executeJavascript(script)
  if self.cefwebview then
    self.cefwebview:executeJavascript(script)
  else
    self.pendingScripts[#self.pendingScripts + 1] = script
  end
end

function WebView:render(width, height)
  if not cef then
    imgui.Text("CEF Unavailable")
    return true
  end

  if width <= 0 or height <= 0 or not self.windowName then
    return false
  end

  if self.texture == nil then
    self.texture = core:render():createTexture(self.textureID, {
      width = width,
      height = height,
    })

    self.image:setTextureID(self.textureID)

    local window = game:getWindowManager():getWindow(self.windowName)
    local windowHandle = window:getWindowHandle()
    self.cefwebview = cef:createWebview(self.textureID)
    self.cefwebview:renderToTexture(
      windowHandle,
      self.texture,
      {
        page = self.startPage,
        pageData = self.pageData,
      }
    )

    for _, script in pairs(self.pendingScripts) do
      self.cefwebview:executeJavascript(script)
    end
    self.onScreenPositionX, self.onScreenPositionY = window:getPosition()
  end

  self.image:render(width, height)
  if self.width ~= width or self.height ~= height then
    self.texture:setSize(math.floor(width * self.oversample), math.floor(height * self.oversample))
  end

  self.width = width
  self.height = height
  local x, y = imgui.GetCursorScreenPos()

  if self.x ~= x or self.y ~= y or self.windowMoved then
    self.x = x
    self.y = y
    self.cefwebview:setScreenPosition(self.x + self.onScreenPositionX, self.y + self.onScreenPositionY)
  end

  self.handleInput = imgui.IsWindowFocused()

  return true
end
