#ifndef _ImguiMovableObject_H_
#define _ImguiMovableObject_H_

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

#include "OgreMovableObject.h"
#include "imgui.h"

namespace Gsage {
  class ImguiMovableObject : public Ogre::MovableObject
  {
    public:
      ImguiMovableObject(Ogre::IdType id, Ogre::ObjectMemoryManager* objectMemoryManager, Ogre::SceneManager* manager, Ogre::uint8 renderQueue);
      virtual ~ImguiMovableObject();

      virtual const Ogre::String& getMovableType(void) const
      {
        static const Ogre::String type = "ImguiMovableObject";
        return type;
      }
      /**
       * Update vertex data for each underlying renderable
       *
       * @param drawList ImDrawList
       * @param offset drawList list offset (z order)
       */
      void updateVertexData(const ImDrawList* drawList, int offset);

      /**
       * Set datablock to use for renderables
       *
       * @param name Datablock name
       */
      void setDatablock(const Ogre::String& name);
    private:
      Ogre::String mDatablockName;
  };

  /** Factory object for creating ImguiMovableObject instances */
  class ImguiMovableObjectFactory : public Ogre::MovableObjectFactory
  {
    protected:
      virtual Ogre::MovableObject* createInstanceImpl(
          Ogre::IdType id,
          Ogre::ObjectMemoryManager *objectMemoryManager,
          Ogre::SceneManager *manager,
          const Ogre::NameValuePairList* params = 0);
    public:
      ImguiMovableObjectFactory() {}
      ~ImguiMovableObjectFactory() {}

      static Ogre::String FACTORY_TYPE_NAME;

      const Ogre::String& getType(void) const;

      void destroyInstance(Ogre::MovableObject* obj);

  };
  /** @} */
/** @} */
}

#endif
