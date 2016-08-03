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

extern "C" {
  #include "lua.h"
  #include "lauxlib.h"
  #include "lualib.h"
}

#include <luabind/luabind.hpp>
#include <vector>

#include "Entity.h"

namespace Gsage {
  typedef std::vector<Entity*> EntityList;
}

namespace luabind
{
  template <>
    struct default_converter<Gsage::EntityList>
    : native_converter_base<Gsage::EntityList>
    {
      static int compute_score(lua_State* L, int index)
      {
        return lua_type(L, index) == LUA_TTABLE ? 0 : -1;
      }

      Gsage::EntityList from(lua_State* L, int index)
      {
        // not implemented yet
        // TODO
        return Gsage::EntityList();
      }

      void to(lua_State* L, Gsage::EntityList const& items)
      {
        object res = newtable(L);
        int i = 1;
        for(auto& entity : items)
        {
          res[i++] = entity;
        }
        res.push(L);
      }
    };

  template <>
    struct default_converter<Gsage::EntityList const&>
    : default_converter<Gsage::EntityList>
    {};
}
