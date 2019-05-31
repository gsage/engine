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

#include "ogre/BillboardWrapper.h"

#include "OgreConverters.h"
#include <OgreSceneManager.h>

namespace Gsage {

  BillboardWrapper::BillboardWrapper()
    : mBillboardSet(0)
    , mBillboard(0)
    , mWidth(-1)
    , mHeight(-1)
  {
    BIND_ACCESSOR("position", &BillboardWrapper::setPosition, &BillboardWrapper::getPosition);
    BIND_ACCESSOR_OPTIONAL("width", &BillboardWrapper::setWidth, &BillboardWrapper::getWidth);
    BIND_ACCESSOR_OPTIONAL("height", &BillboardWrapper::setHeight, &BillboardWrapper::getHeight);
    BIND_ACCESSOR_OPTIONAL("colour", &BillboardWrapper::setColour, &BillboardWrapper::getColour);
    BIND_ACCESSOR_OPTIONAL("rotation", &BillboardWrapper::setRotation, &BillboardWrapper::getRotation);
    BIND_ACCESSOR_OPTIONAL("rect", &BillboardWrapper::setTexcoordRect, &BillboardWrapper::getTexcoordRect);
    BIND_ACCESSOR_OPTIONAL("index", &BillboardWrapper::setTexcoordIndex, &BillboardWrapper::getTexcoordIndex);
  }

  bool BillboardWrapper::initialize(const DataProxy& dict, OgreV1::BillboardSet* billboardSet)
  {
    mBillboardSet = billboardSet;
    return Serializable<BillboardWrapper>::read(dict);
  }

  void BillboardWrapper::setPosition(const Ogre::Vector3& position)
  {
    mBillboard = mBillboardSet->createBillboard(position);
  }

  const Ogre::Vector3& BillboardWrapper::getPosition() const
  {
    return mBillboard->getPosition();
  }

  void BillboardWrapper::setWidth(const float& value)
  {
    mWidth = value;
    mBillboard->setDimensions(mWidth, mHeight != -1 ? mHeight : mBillboardSet->getDefaultHeight());
  }

  float BillboardWrapper::getWidth()
  {
    return mBillboard->getOwnWidth();
  }

  void BillboardWrapper::setHeight(const float& value)
  {
    mHeight = value;
    mBillboard->setDimensions(mWidth != -1 ? mWidth : mBillboardSet->getDefaultWidth(), mHeight);
  }

  float BillboardWrapper::getHeight()
  {
    return mBillboard->getOwnHeight();
  }

  void BillboardWrapper::setColour(const Ogre::ColourValue& value)
  {
    mBillboard->setColour(value);
  }

  const Ogre::ColourValue& BillboardWrapper::getColour() const
  {
    return mBillboard->getColour();
  }

  void BillboardWrapper::setRotation(const Ogre::Degree& value)
  {
    mBillboard->setRotation(Ogre::Radian(value));
  }

  Ogre::Degree BillboardWrapper::getRotation()
  {
    return Ogre::Degree(mBillboard->getRotation());
  }

  void BillboardWrapper::setTexcoordRect(const Ogre::FloatRect& rect)
  {
    mBillboard->setTexcoordRect(rect);
  }

  const Ogre::FloatRect& BillboardWrapper::getTexcoordRect() const
  {
    return mBillboard->getTexcoordRect();
  }

  void BillboardWrapper::setTexcoordIndex(const unsigned int& index)
  {
    mBillboard->setTexcoordIndex(index);
  }

  unsigned int BillboardWrapper::getTexcoordIndex()
  {
    return mBillboard->getTexcoordIndex();
  }

  const std::string BillboardSetWrapper::TYPE = "billboard";

  OgreV1::BillboardType BillboardSetWrapper::mapBillboardType(const std::string& type)
  {
    OgreV1::BillboardType res;
    if(type == BBT_POINT_ID)
      res = OgreV1::BillboardType::BBT_POINT;
    else if(type == BBT_ORIENTED_COMMON_ID)
      res = OgreV1::BillboardType::BBT_ORIENTED_COMMON;
    else if(type == BBT_ORIENTED_SELF_ID)
      res = OgreV1::BillboardType::BBT_ORIENTED_SELF;
    else if(type == BBT_PERPENDICULAR_COMMON_ID)
      res = OgreV1::BillboardType::BBT_PERPENDICULAR_COMMON;
    else if(type == BBT_PERPENDICULAR_SELF_ID)
      res = OgreV1::BillboardType::BBT_PERPENDICULAR_SELF;
    else
      res = OgreV1::BillboardType::BBT_POINT;
    return res;
  }

  std::string BillboardSetWrapper::mapBillboardType(const OgreV1::BillboardType type)
  {
    std::string res;
    switch(type)
    {
      case OgreV1::BillboardType::BBT_POINT:
        res = BBT_POINT_ID;
        break;
      case OgreV1::BillboardType::BBT_ORIENTED_COMMON:
        res = BBT_ORIENTED_COMMON_ID;
        break;
      case OgreV1::BillboardType::BBT_ORIENTED_SELF:
        res = BBT_ORIENTED_SELF_ID;
        break;
      case OgreV1::BillboardType::BBT_PERPENDICULAR_SELF:
        res = BBT_PERPENDICULAR_SELF_ID;
        break;
      case OgreV1::BillboardType::BBT_PERPENDICULAR_COMMON:
        res = BBT_PERPENDICULAR_COMMON_ID;
        break;
    }
    return res;
  }

  BillboardSetWrapper::BillboardSetWrapper()
  {
    BIND_ACCESSOR("commonDirection", &BillboardSetWrapper::setCommonDirection, &BillboardSetWrapper::getCommonDirection);
    BIND_ACCESSOR("commonUpVector", &BillboardSetWrapper::setCommonUpVector, &BillboardSetWrapper::getCommonUpVector);
    BIND_ACCESSOR("billboardType", &BillboardSetWrapper::setBillboardType, &BillboardSetWrapper::getBillboardType);
    BIND_ACCESSOR("materialName", &BillboardSetWrapper::setMaterialName, &BillboardSetWrapper::getMaterialName);

    BIND_ACCESSOR("billboards", &BillboardSetWrapper::setBillboards, &BillboardSetWrapper::getBillboards);
  }

  BillboardSetWrapper::~BillboardSetWrapper()
  {
  }

  bool BillboardSetWrapper::read(const DataProxy& dict)
  {
    if(mObject) {
      mObject->detachFromParent();
      mSceneManager->destroyBillboardSet(mObject);
    }

    mObject = mSceneManager->createBillboardSet();
#if OGRE_VERSION >= 0x020100
    mObject->setRenderQueueGroup(51);
    mObject->setQueryFlags(Ogre::SceneManager::QUERY_FX_DEFAULT_MASK);
#else
    mObject->setQueryFlags(Ogre::SceneManager::FX_TYPE_MASK);
#endif
    defineUserBindings();
    bool res = OgreObject::read(dict);
    attachObject(mObject);
    return res;
  }

  void BillboardSetWrapper::setCommonUpVector(const Ogre::Vector3& vector)
  {
    mObject->setCommonUpVector(vector);
  }

  const Ogre::Vector3& BillboardSetWrapper::getCommonUpVector() const
  {
    return mObject->getCommonUpVector();
  }

  void BillboardSetWrapper::setCommonDirection(const Ogre::Vector3& vector)
  {
    mObject->setCommonDirection(vector);
  }

  const Ogre::Vector3& BillboardSetWrapper::getCommonDirection() const
  {
    return mObject->getCommonDirection();
  }

  void BillboardSetWrapper::setBillboardType(const std::string& type)
  {
    mObject->setBillboardType(mapBillboardType(type));
  }

  std::string BillboardSetWrapper::getBillboardType()
  {
    return mapBillboardType(mObject->getBillboardType());
  }

  void BillboardSetWrapper::setMaterialName(const std::string& id)
  {
    Ogre::String group = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
    Ogre::String materialName;
    auto parts = split(id, '.');
    if(parts.size() == 2) {
      group = parts[0];
      materialName = parts[1];
    } else {
      materialName = id;
    }
    mMaterialName = group + "." + materialName;

#if OGRE_VERSION >= 0x020100
    mObject->setDatablockOrMaterialName(materialName, group);
#else
    mObject->setMaterialName(materialName, group);
#endif
    mObject->setSortingEnabled(true);
  }

  const std::string& BillboardSetWrapper::getMaterialName() const
  {
    return mMaterialName;
  }

  void BillboardSetWrapper::setBillboards(const DataProxy& dict)
  {
    for(int i = 0; i < mObject->getNumBillboards(); i++) {
      mObject->removeBillboard(i);
    }

    mBillboards.clear();
    Ogre::Vector3 min;
    Ogre::Vector3 max;
    for(auto& pair : dict)
    {
      mBillboards.emplace_back();
      BillboardWrapper& billboard = mBillboards.back();
      if(!billboard.initialize(pair.second, mObject)) {
        mBillboards.pop_back();
        continue;
      }

      Ogre::Vector3 pos = billboard.getPosition();
      if(mBillboards.size() == 1) {
        min = max = pos;
      }
      // cubic bounds for the BBT_POINT billboard set
      if(mObject->getBillboardType() == OgreV1::BBT_POINT) {
        Ogre::Real bounds = std::max(billboard.getWidth(), billboard.getHeight());
        min.makeFloor(
            pos
#if OGRE_VERSION < 0x020100
            - bounds / 2
#endif
        );
        max.makeCeil(pos + bounds / 2);
      }
    }
#if OGRE_VERSION >= 0x020100
    Ogre::Aabb aabb(min, max);
    Ogre::Real radius = aabb.getRadiusOrigin();
#else
    Ogre::AxisAlignedBox aabb(min, max);
    Ogre::Real radius = Ogre::Math::boundingRadiusFromAABB(aabb);
#endif
    mObject->setBounds(aabb, radius);
  }

  DataProxy BillboardSetWrapper::getBillboards()
  {
    DataProxy res;
    for(auto& bb : mBillboards)
    {
      DataProxy bbData;
      bb.dump(bbData);
      res.push(bbData);
    }
    return res;
  }

}
