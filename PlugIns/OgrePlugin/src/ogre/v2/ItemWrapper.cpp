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

#include <OgreMesh2.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

#include <OgreMeshManager.h>
#include <OgreMeshManager2.h>

#include "ogre/v2/ItemWrapper.h"

namespace Gsage {
  const std::string ItemWrapper::TYPE = "item";

  ItemWrapper::ItemWrapper()
    : mQuery(STATIC)
  {
    BIND_ACCESSOR_WITH_PRIORITY("mesh", &ItemWrapper::setMesh, &ItemWrapper::getMesh, 1);
    BIND_ACCESSOR("query", &ItemWrapper::setQueryFlags, &ItemWrapper::getQueryFlags);
    BIND_ACCESSOR("datablock", &ItemWrapper::setDatablock, &ItemWrapper::getDatablock);
  }

  ItemWrapper::~ItemWrapper()
  {
  }

  void ItemWrapper::setMesh(const std::string& model)
  {
    mMeshName = model;
    mObject = mSceneManager->createItem(model, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
    mObject->setQueryFlags(mQuery | Ogre::SceneManager::QUERY_ENTITY_DEFAULT_MASK);
    defineUserBindings();
    attachObject(mObject);
  }

  const std::string& ItemWrapper::getMesh() const
  {
    return mMeshName;
  }

  void ItemWrapper::setQueryFlags(const std::string& type)
  {
    mQueryString = type;
    if(type == "static")
      mQuery = STATIC;
    else if(type == "dynamic")
      mQuery = DYNAMIC;
    else
      mQuery = UNKNOWN;

    if(mObject != 0)
    {
      mObject->setQueryFlags(mQuery | Ogre::SceneManager::QUERY_ENTITY_DEFAULT_MASK);
    }
  }

  const std::string& ItemWrapper::getQueryFlags() const
  {
    return mQueryString;
  }

  void ItemWrapper::setDatablock(const std::string& name)
  {
    mDatablock = name;
    if(mObject) {
      mObject->setDatablock(mDatablock);
    }
  }

  const std::string& ItemWrapper::getDatablock() const
  {
    return mDatablock;
  }
}
