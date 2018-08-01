/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev

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

#ifndef _UIManager_H_
#define _UIManager_H_

#include <string>

struct lua_State;

namespace Gsage
{
  class Engine;
  class LuaInterface;

  /**
   * Abstract UI implementation
   */
  class UIManager
  {
    public:
      UIManager()
        : mLuaState(0)
        , mEngine(0)
      {
      }
      virtual ~UIManager() {}
      /**
       * Does initial setup of the UIManager
       *
       * @param engine Engine core instance
       * @param L Initialize UI manager with lua state
       */
      virtual void initialize(Engine* engine, lua_State* L = 0) {
        mEngine = engine;
        mLuaState = L;
      };

      /**
       * Get lua state
       */
      virtual lua_State* getLuaState() { return mLuaState; }

      /**
       * Update lua state
       *
       * @param L New lua state
       */
      virtual void setLuaState(lua_State* L) { mLuaState = L; };

      /**
       * Get type id
       */
      virtual const std::string& getType() = 0;
    protected:
      Engine* mEngine;
      lua_State* mLuaState;
  };
}

#endif
