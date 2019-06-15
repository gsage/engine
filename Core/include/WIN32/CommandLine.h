/*
-----------------------------------------------------------------------------
This file is part of Gsage engine

Copyright (c) 2014-2019 Gsage Authors

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

#include "GsageDefinitions.h"

#if GSAGE_PLATFORM == GSAGE_WIN32
#ifndef __COMMAND_LINE_TOOLS_H__
#define __COMMAND_LINE_TOOLS_H__

#include <memory>
#include "sol_forward.hpp"

namespace Gsage {
  // this package is here to fix built-in Lua behaviour of using standard shell
  // on Windows, which leads to opening countless cmd windows

  /**
   * Represents command line run pipe object
   */
  struct CommandPipe {
    void close();
    sol::object next(sol::this_state s);
    CommandPipe* lines();
    void execute(const std::string& command);
    sol::object read(sol::object arg, sol::this_state s);
    std::string readLine();

    unsigned long exitCode;
    std::stringstream data;
  };

  typedef std::shared_ptr<CommandPipe> CommandPipePtr;

  /**
   * Run shell command and returns status
   */
  int luaCmd(const std::string& command);

  CommandPipePtr luaPopen(const std::string& command);
}

#endif
#endif
