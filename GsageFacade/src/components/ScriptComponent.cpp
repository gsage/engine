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

#include "components/ScriptComponent.h"
#include "Logger.h"
#include "Entity.h"
#include <iostream>

namespace Gsage {

  const std::string ScriptComponent::SYSTEM = "script";

  ScriptComponent::ScriptComponent()
    : mSetupExecuted(false)
    , mTearDownExecuted(false)
    , mHasBehavior(false)
  {
    BIND_ACCESSOR_OPTIONAL("behavior", &ScriptComponent::setBehavior, &ScriptComponent::getBehavior);
    BIND_ACCESSOR_OPTIONAL("setupScript", &ScriptComponent::setSetupScript, &ScriptComponent::getSetupScript);
    BIND_ACCESSOR_OPTIONAL("tearDownScript", &ScriptComponent::setTearDownScript, &ScriptComponent::getTearDownScript);
  }

  ScriptComponent::~ScriptComponent()
  {
  }

  void ScriptComponent::readScript(const std::string& path)
  {
    std::ifstream stream(path);
    std::string res;
    stream.seekg(0, std::ios::end);
    long int size = stream.tellg();
    if(size == -1)
    {
      LOG(ERROR) << "Failed to read script file: " << path;
      return;
    }

    res.reserve(size);
    stream.seekg(0, std::ios::beg);
    try
    {
      res.assign((std::istreambuf_iterator<char>(stream)),
                  std::istreambuf_iterator<char>());
    }
    catch(std::ifstream::failure e)
    {
      LOG(ERROR) << "Failed to read script file: " << path;
      return;
    }

    stream.close();
  }

  void ScriptComponent::setBehavior(const std::string& value)
  {
    mBehavior = value;
  }

  const std::string& ScriptComponent::getBehavior() const
  {
    return mBehavior;
  }

  void ScriptComponent::setSetupScript(const std::string& value)
  {
    mSetupScript = value;
  }

  const std::string& ScriptComponent::getSetupScript() const
  {
    return mSetupScript;
  }

  void ScriptComponent::setTearDownScript(const std::string& value)
  {
    mTearDownScript = value;
  }

  const std::string& ScriptComponent::getTearDownScript() const
  {
    return mTearDownScript;
  }

  void ScriptComponent::setData(luabind::object& data)
  {
    mData = data;
  }

  luabind::object& ScriptComponent::getData()
  {
    return mData;
  }

  void ScriptComponent::setSetupExecuted(bool value)
  {
    mSetupExecuted = value;
  }

  bool ScriptComponent::getSetupExecuted()
  {
    return mSetupExecuted;
  }

  void ScriptComponent::setTearDownExecuted(bool value)
  {
    mTearDownExecuted = value;
  }

  bool ScriptComponent::getTearDownExecuted()
  {
    return mTearDownExecuted;
  }

  void ScriptComponent::setBtree(const luabind::object& object)
  {
    mBtree = object;
    mHasBehavior = true;
  }

  luabind::object& ScriptComponent::getBtree()
  {
    return mBtree;
  }

  bool ScriptComponent::hasBehavior() const
  {
    return mHasBehavior;
  }
}
