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

#include "ogre/v2/CustomPassProvider.h"

#include <Compositor/OgreCompositorNode.h>

#include "ogre/v2/OverlayPass.h"

namespace Gsage {

  CustomPassProvider::CustomPassProvider()
    : mEngine(0)
  {
    registerPassDef<OverlayPass, OverlayPassDef>("overlay");
  }

  CustomPassProvider::~CustomPassProvider()
  {
  }

  void CustomPassProvider::initialize(Engine* engine)
  {
    mEngine = engine;
  }

  Ogre::CompositorPass* CustomPassProvider::addPass(const Ogre::CompositorPassDef* definition,
      Ogre::Camera* defaultCamera,
      Ogre::CompositorNode* parentNode,
      const Ogre::CompositorChannel& target,
      Ogre::SceneManager* sceneManager
  )
  {
    if(!mEngine) {
      OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
          "CustomPassProvider is not initialized",
          "CustomPassProvider::addPass");
    }

    const std::string customId = static_cast<const CustomPassDef*>(definition)->getID();

    if(mPassFactories.count(customId) > 0) {
      return mPassFactories[customId](definition, target, parentNode);
    }

    std::stringstream ss;
    ss << "Pass \"" << customId << "\" is not supported";
    OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
        ss.str(),
        "CustomPassProvider::addPassDef");
    return 0;
  }

  Ogre::CompositorPassDef* CustomPassProvider::addPassDef(Ogre::CompositorPassType passType,
      Ogre::IdString customId,
      Ogre::CompositorTargetDef* parentTargetDef,
      Ogre::CompositorNodeDef* parentNodeDef
  )
  {
    if(!mEngine) {
      OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
          "CustomPassProvider is not initialized",
          "CustomPassProvider::addPassDef");
    }

    if(mDefFactories.count(customId) > 0) {
      return mDefFactories[customId](parentTargetDef, parentNodeDef);
    }

    std::stringstream ss;
    ss << passType << ", " << customId.getFriendlyText() << " is not supported";
    OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
        ss.str(),
        "CustomPassProvider::addPassDef");
    return nullptr;
  }

}
