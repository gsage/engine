#ifndef _CustomPassProvider_H_
#define _CustomPassProvider_H_

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

#include <Compositor/Pass/OgreCompositorPassProvider.h>
#include "ogre/v2/CustomPass.h"

namespace Gsage {
  class Engine;

  class CustomPassProvider : public Ogre::CompositorPassProvider
  {
    public:
      CustomPassProvider();
      virtual ~CustomPassProvider();

      void initialize(Engine* engine);

      Ogre::CompositorPass* addPass(const Ogre::CompositorPassDef* definition,
          Ogre::Camera* defaultCamera,
          Ogre::CompositorNode* parentNode,
          const Ogre::CompositorChannel& target,
          Ogre::SceneManager* sceneManager
      );

      Ogre::CompositorPassDef* addPassDef(Ogre::CompositorPassType passType,
          Ogre::IdString customId,
          Ogre::CompositorTargetDef* parentTargetDef,
          Ogre::CompositorNodeDef* parentNodeDef
      );

      template<class C, class D>
      bool registerPassDef(const std::string& customId)
      {
        if(!std::is_base_of<CustomPass, C>::value) {
          return false;
        }

        if(!std::is_base_of<CustomPassDef, D>::value) {
          return false;
        }

        mDefFactories[customId] = [&](Ogre::CompositorTargetDef* target, Ogre::CompositorNodeDef* node) {
          return new D(mEngine, target, node);
        };

        mPassFactories[customId] = [&](const Ogre::CompositorPassDef *def, const Ogre::CompositorChannel &target, Ogre::CompositorNode *parentNode) {
          return new C(mEngine, def, target, parentNode);
        };

        return true;
      }
    private:
      Engine* mEngine;

      typedef std::function<Ogre::CompositorPassDef*(Ogre::CompositorTargetDef*, Ogre::CompositorNodeDef*)> DefFactoryFunc;

      typedef std::map<Ogre::IdString, DefFactoryFunc> DefFactoryFuncs;

      DefFactoryFuncs mDefFactories;

      typedef std::function<Ogre::CompositorPass*(const Ogre::CompositorPassDef *definition, const Ogre::CompositorChannel &target, Ogre::CompositorNode *parentNode)> PassFactoryFunc;

      typedef std::map<std::string, PassFactoryFunc> PassFactoryFuncs;

      PassFactoryFuncs mPassFactories;
  };
}

#endif
