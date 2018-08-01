#ifndef _CustomPass_H_
#define _CustomPass_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev and contributors

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

#include <Compositor/Pass/OgreCompositorPass.h>
#include <Compositor/Pass/OgreCompositorPassDef.h>

namespace Gsage {
  class Engine;

  class CustomPass : public Ogre::CompositorPass
  {
    public:
      CustomPass(Engine* engine,
          const Ogre::CompositorPassDef *def,
          const Ogre::CompositorChannel &target,
          Ogre::CompositorNode *parentNode);
      virtual ~CustomPass();
    protected:
      Engine* mEngine;
  };

  class CustomPassDef : public Ogre::CompositorPassDef
  {
    public:
      CustomPassDef(
          const std::string& id,
          Engine* engine,
          Ogre::CompositorTargetDef* target,
          Ogre::CompositorNodeDef* node);
      virtual ~CustomPassDef();

      /**
       * Get pass def identifier
       */
      const std::string& getID() const { return mID; };
    protected:
      const std::string mID;
      Engine* mEngine;
  };
}

#endif
