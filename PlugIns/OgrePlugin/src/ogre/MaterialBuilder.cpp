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

#include "ogre/MaterialBuilder.h"
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>

namespace Gsage {

  MaterialBuilder::MaterialBuilder()
  {
  }

  MaterialBuilder::~MaterialBuilder()
  {
  }

  Ogre::MaterialPtr MaterialBuilder::parse(const std::string& name, const DataProxy& data)
  {
    if(Ogre::MaterialManager::getSingleton().resourceExists(name)) {
      Ogre::MaterialManager::getSingleton().remove(name);
    }

    Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(name, data.get("resourceGroup", "General"));
    material->setReceiveShadows(data.get("receiveShadows", false));
    int techniqueIndex = 0;
    for(auto t : data.get("techniques", DataProxy::create(DataWrapper::JSON_OBJECT))) {
      Ogre::Technique* tech = material->getTechnique(techniqueIndex);
#if OGRE_VERSION_MAJOR == 1
      tech->setLightingEnabled(t.second.get("lightingEnabled", true));
      tech->setDepthCheckEnabled(t.second.get("depthCheckEnabled", true));
#endif
      int passIndex = 0;
      for(auto p : t.second.get("passes", DataProxy::create(DataWrapper::JSON_OBJECT))) {
        Ogre::Pass* pass = tech->getPass(passIndex++);
        if(!pass) {
          LOG(ERROR) << "Can't get pass " << passIndex << ", material name: " << name;
          continue;
        }

        auto diffuse = p.second.get<Ogre::ColourValue>("colors.diffuse");
        auto ambient = p.second.get<Ogre::ColourValue>("colors.ambient");
        auto selfIllumination = p.second.get<Ogre::ColourValue>("colors.illumination");

        if(diffuse.second) {
          pass->setDiffuse(diffuse.first);
        }

        if(ambient.second) {
          pass->setAmbient(ambient.first);
        }

        if(selfIllumination.second) {
          pass->setSelfIllumination(selfIllumination.first);
        }
      }

      techniqueIndex++;
    }

    return material;
  }

}
