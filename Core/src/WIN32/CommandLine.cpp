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

#include <iostream>
#include "WIN32/CommandLine.h"

#include <windows.h>
#include <tchar.h>

#include "Logger.h"
#include "sol.hpp"

#define BUFSIZE 4096

namespace Gsage {

  sol::object CommandPipe::read(sol::object arg, sol::this_state s)
  {
    sol::state_view lua(s);
    if(!data) {
      return sol::make_object(lua, sol::lua_nil);
    }

    if(arg.get_type() == sol::type::number) {
      std::string res;
      size_t count = arg.as<size_t>();
      res.resize(count);
      data.read(&res[0], count);
      return sol::make_object(lua, res);
    } else if (arg.get_type() == sol::type::string || arg.get_type() == sol::type::lua_nil){
      std::string query = arg.as<std::string>();
      if(query == "*a" || trim(query).empty()) {
        std::string res = data.str();
        if(res[res.size() - 1] == '\n') {
          res = res.substr(0, res.size() - 1);
        }

        return sol::make_object(lua, res);
      } else if(query == "*l") {
        return sol::make_object(lua, readLine());
      } else {
        LOG(ERROR) << "Unsupported parameter " << query;
        throw std::runtime_error("pipe query not supported");
      }
    } else {
      std::stringstream ss;
      ss << "Incorrect argument type " << (int)arg.get_type();
      throw std::runtime_error(ss.str());
    }

    return sol::make_object(lua, sol::lua_nil);
  }

  void CommandPipe::execute(const std::string& command)
  {
    if(command.empty()) {
      return;
    }

    HANDLE childStdOutRead = NULL;
    HANDLE childStdOutWrite = NULL;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&childStdOutRead, &childStdOutWrite, &saAttr, 0)) {
      throw std::runtime_error("failed to create pipe");
    }

    if (!SetHandleInformation(childStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
      throw std::runtime_error("failed to set handle information");
    }

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = childStdOutWrite;
    siStartInfo.hStdOutput = childStdOutWrite;
    siStartInfo.hStdInput = NULL;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    std::string cmd = std::string("/C ") + command;
    cmd.erase(std::remove(cmd.begin(), cmd.end(), '\r'), cmd.end());

    if(!CreateProcess("c:\\Windows\\System32\\cmd.exe", (LPSTR)cmd.c_str(), NULL, NULL, TRUE,
              CREATE_NO_WINDOW | CREATE_PRESERVE_CODE_AUTHZ_LEVEL, NULL, NULL, &siStartInfo, &piProcInfo)) {
      std::stringstream ss;
      exitCode = (unsigned long)GetLastError();
      ss << "failed to create child process, error code: " << exitCode;
      throw std::runtime_error(ss.str());
    }

    bool readPipeSucceed = false;

    std::vector<std::string> lines;
    DWORD dwRead, bytesAvailable, left;
    CHAR chBuf[BUFSIZE];
    chBuf[0] ='\0';

    bool processFinished = false;

    for(;;) {
      PeekNamedPipe(childStdOutRead, NULL, 0, &dwRead, &bytesAvailable, &left);
      if(bytesAvailable > 0) {
        DWORD read = bytesAvailable;
        if(read > BUFSIZE - 1) {
          read = BUFSIZE - 1;
        }

        if(!ReadFile(childStdOutRead, chBuf, read, &dwRead, NULL)) {
          if(GetLastError() == ERROR_BROKEN_PIPE) {
            break;
          }
          throw std::runtime_error("failed to read pipe");
        }

        if(dwRead == 0) {
          break;
        }

        chBuf[dwRead] = '\0';
        data << chBuf;
      } else if (processFinished) {
        CloseHandle(childStdOutWrite);
        break;
      }

      if(WaitForSingleObject(piProcInfo.hProcess, 20) == WAIT_OBJECT_0) {
        processFinished = true;
      }
    }

    DWORD code;
    if(!GetExitCodeProcess(piProcInfo.hProcess, &code)) {
      exitCode = (unsigned long)GetLastError();
    } else {
      exitCode = (unsigned long)code;
    }

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    LOG(DEBUG) << "CMD call from Lua: " << cmd << ", exit code: " << exitCode;
    CloseHandle(childStdOutRead);
  }

  sol::object CommandPipe::next(sol::this_state s)
  {
    sol::state_view lua(s);
    if(!data) {
      return sol::make_object(lua, sol::lua_nil);
    }
    return sol::make_object(lua, readLine());
  }

  CommandPipe* CommandPipe::lines()
  {
    return this;
  }

  std::string CommandPipe::readLine()
  {
    std::string res;
    std::getline(data, res);
    return trim(res);
  }

  void CommandPipe::close()
  {
      // does nothing
  }

  /**
   * Run shell command and returns status
   */
  int luaCmd(const std::string& command)
  {
    CommandPipePtr pipe = std::make_shared<CommandPipe>();
    pipe->exitCode = 0;
    try {
      pipe->execute(command);
    } catch(const std::runtime_error& err) {
      LOG(ERROR) << "Failed to run command " << command << ", error: " << err.what();
    }

    return pipe->exitCode;
  }

  CommandPipePtr luaPopen(const std::string& command)
  {
    CommandPipePtr pipe = std::make_shared<CommandPipe>();
    pipe->exitCode = 0;

    try {
      pipe->execute(command);
    } catch(const std::runtime_error& err) {
      LOG(ERROR) << "Failed to run command " << command << ", error: " << err.what();
      throw err;
    }
    return pipe;
  }

}

#endif
