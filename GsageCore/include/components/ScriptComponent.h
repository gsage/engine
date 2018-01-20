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

#ifndef _ScriptComponent_H_
#define _ScriptComponent_H_

#include "sol_forward.hpp"
#include "Component.h"

struct lua_State;

namespace Gsage {

  class ScriptComponent : public EntityComponent
  {
    public:
      static const std::string SYSTEM;

      ScriptComponent();
      virtual ~ScriptComponent();

      /**
       * Read script from the file
       *
       * @param value Script file path
       */
      void readScript(const std::string& path);

      /**
       * Set behavior id for this component
       * @param value Id of behavior
       */
      void setBehavior(const std::string& value);

      /**
       * Get behavior id
       */
      const std::string& getBehavior() const;

      /**
       * Set set up script for the component
       * @param value Script string
       */
      void setSetupScript(const std::string& value);

      /**
       * Get set up script string from the component
       */
      const std::string& getSetupScript() const;
      /**
       * Set tear down script for the component
       * @param value Script string
       */
      void setTearDownScript(const std::string& value);

      /**
       * Get script string from the component
       */
      const std::string& getTearDownScript() const;

      /**
       * Set context data
       *
       * @param data Lua table
       */
      void setData(sol::table& data);

      /**
       * Get data from the context
       */
      sol::table& getData();

      /**
       * Set setup script execution state
       * @param value Done/not done
       */
      void setSetupExecuted(bool value);

      /**
       * Get setup script execution state
       */
      bool getSetupExecuted();

      /**
       * Set tear down script execution state
       * @param value Done/not done
       */
      void setTearDownExecuted(bool value);

      /**
       * Get tear down script execution state
       */
      bool getTearDownExecuted();

      /**
       * Check if script has a behavior tree set
       */
      bool hasBehavior() const;

      /**
       * Set script behavior lua table
       * @param value sol::table
       */
      void setBtree(const sol::table& object);

      /**
       * Get script behavior lua object
       */
      sol::table& getBtree();

      /**
       * Build initial context state
       *
       * @param context to update
       */
      void setContext(const DataProxy& context);

      /**
       * Get actual context
       */
      DataProxy getContext();

      /**
       * Set setup function
       *
       * @param function sol::protected_function
       */
      void setSetupFunction(const sol::protected_function& function);

      /**
       * Set teardown function
       *
       * @param function sol::protected_function
       */
      void setTearDownFunction(const sol::protected_function& function);

      /**
       * Get setup function
       */
      sol::protected_function getSetupFunction();

      /**
       * Get tear down function
       */
      sol::protected_function getTearDownFunction();

    private:

      std::string mSetupScript;
      std::string mTearDownScript;

      sol::protected_function mSetupFunction;
      sol::protected_function mTearDownFunction;

      std::string mBehavior;

      sol::table mData;
      sol::table mBtree;
      DataProxy mUpdatedContext;

      bool mSetupExecuted;
      bool mTearDownExecuted;
      bool mHasBehavior;
  };
}

#endif
