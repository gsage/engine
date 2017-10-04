#ifndef _ImguiLuaInterface_H_
#define _ImguiLuaInterface_H_

/*
-----------------------------------------------------------------------------
Based on https://github.com/patrickriordan/imgui_lua_bindings

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

#include <string>

namespace sol {
  class state_view;
}

namespace Gsage {

  /**
   * Imgui buffer that can be used by lua
   */
  class ImguiTextBuffer {
    public:
      ImguiTextBuffer(int size, const char* initialValue = "");
      virtual ~ImguiTextBuffer();

      /**
       * Read the buffer
       */
      std::string read() const;

      /**
       * Read the buffer
       */
      char* read();

      /**
       * Overwrite the buffer.
       *
       * @param value to write
       */
      bool write(const std::string& value);

      /**
       * Returns buffer size.
       */
      int size() const { return mSize; };

    private:
      char* mBuffer;

      int mSize;
  };

  class ImguiLuaInterface {
    public:
      /**
       * Add IMGUI bindings to the lua state
       *
       * @param lua state to enrich with imgui bindings
       */
      static void addLuaBindings(sol::state_view& lua);
  };

}

#endif
