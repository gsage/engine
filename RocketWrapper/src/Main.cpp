/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include <Ogre.h>

#include "GsageFacade.h"
#include "RocketUIManager.h"
#include "lua/LuaInterface.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include "components/ScriptComponent.h"
#include "systems/OgreRenderSystem.h"
#include "systems/RecastMovementSystem.h"
#include "systems/CombatSystem.h"
#include "systems/LuaScriptSystem.h"

#include "input/OisInputListener.h"

#include <Rocket/Core/Lua/Interpreter.h>
#include <stdio.h>

#include "Logger.h"

#if GSAGE_PLATFORM == GSAGE_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifndef RESOURCES_FOLDER
#define RESOURCES_FOLDER "./resources"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if GSAGE_PLATFORM == GSAGE_WIN32
  INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT count)
#else
    int main(int argc, char *argv[])
#endif
    {
      Gsage::GsageFacade facade;
      std::string coreConfig = "gameConfig.json";

      Gsage::RocketUIManager uiManager;
      facade.setUIManager(&uiManager);
      // add systems to the engine
      Gsage::OgreRenderSystem* render = facade.addSystem<Gsage::OgreRenderSystem>();
      facade.addSystem<Gsage::RecastMovementSystem>();
      facade.addSystem<Gsage::CombatSystem>();

      if(!facade.initialize(coreConfig, RESOURCES_FOLDER))
      {
        LOG(ERROR) << "Failed to initialize game engine";
        return 1;
      }

      // initialize input listener
      Gsage::OisInputListener inputListener(render->getRenderWindow(), facade.getEngine());
      facade.addUpdateListener(&inputListener);
      facade.addSystem<Gsage::LuaScriptSystem>()->setLuaState(Rocket::Core::Lua::Interpreter::GetLuaState());

      while(facade.update())
      {
      }
      return 0;
    }
#ifdef __cplusplus
}
#endif
