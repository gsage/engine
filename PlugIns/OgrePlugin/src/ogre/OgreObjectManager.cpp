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

#include "ogre/OgreObjectManager.h"
#include <OgreSceneManager.h>

#include "ogre/OgreObject.h"
#include "Logger.h"

namespace Gsage {

  const std::string OgreObjectManagerEvent::FACTORY_UNREGISTERED = "factoryUnregistered";

  OgreObjectManager::OgreObjectManager()
  {
  }

  OgreObjectManager::~OgreObjectManager()
  {
  }

  OgreObject* OgreObjectManager::create(const DataProxy& dict, const std::string& owner, Ogre::SceneManager* sceneManager, const std::string& boneId, Ogre::Entity* parentEntity )
  {
    std::string type = dict.get("type", "no_type_defined!");
    if(mObjects.count(type) == 0)
    {
      LOG(ERROR) << "Failed to create element of type \"" << type << "\": no factory of such type exists";
      return 0;
    }

    OgreObject* object = mObjects[type]->allocate();
    if(!object->initialize(
        this,
        dict,
        owner,
        type,
        sceneManager,
        boneId,
        parentEntity))
    {
      LOG(ERROR) << "Failed to create object of type \"" << type << "\": failed to build object";
      object->destroy();
      return 0;
    }
    return object;
  }

  OgreObject* OgreObjectManager::create(const DataProxy& dict, const std::string& owner, Ogre::SceneManager* sceneManager, const std::string& type, Ogre::SceneNode* parent)
  {
    if(mObjects.count(type) == 0)
    {
      LOG(ERROR) << "Failed to create element of type \"" << type << "\": no factory of such type exists";
      return 0;
    }

    OgreObject* object = mObjects[type]->allocate();
    if(!object->initialize(
        this,
        dict,
        owner,
        type,
        sceneManager,
        parent == 0 ? sceneManager->getRootSceneNode() : parent))
    {
      LOG(ERROR) << "Failed to create object of type \"" << type << "\": failed to build object";
      object->destroy();
      return 0;
    }
    return object;
  }

  OgreObject* OgreObjectManager::create(const DataProxy& dict, const std::string& owner, Ogre::SceneManager* sceneManager, Ogre::SceneNode* parent)
  {
    if(dict.count("type") == 0)
    {
      LOG(ERROR) << "Failed to create element, dict is in incorrect format: type field missing";
      return 0;
    }

    return create(dict, owner, sceneManager, dict.get("type", "no type defined!"), parent);
  }

  void OgreObjectManager::destroy(OgreObject* object)
  {
    const std::string& type = object->getType();
    if(mObjects.count(type) == 0)
    {
      LOG(WARNING) << "Failed to destroy object of type \""<< type << "\": no pool of such type exists";
      return;
    }

    mObjects[type]->remove(object);
  }
}
