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

  bool BillboardWrapper::initialize(const DataProxy& dict, Ogre::BillboardSet* billboardSet)
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

  Ogre::BillboardType BillboardSetWrapper::mapBillboardType(const std::string& type)
  {
    Ogre::BillboardType res;
    if(type == BBT_POINT_ID)
      res = Ogre::BillboardType::BBT_POINT;
    else if(type == BBT_ORIENTED_COMMON_ID)
      res = Ogre::BillboardType::BBT_ORIENTED_COMMON;
    else if(type == BBT_ORIENTED_SELF_ID)
      res = Ogre::BillboardType::BBT_ORIENTED_SELF;
    else if(type == BBT_PERPENDICULAR_COMMON_ID)
      res = Ogre::BillboardType::BBT_PERPENDICULAR_COMMON;
    else if(type == BBT_PERPENDICULAR_SELF_ID)
      res = Ogre::BillboardType::BBT_PERPENDICULAR_SELF;
    else
      res = Ogre::BillboardType::BBT_POINT;
    return res;
  }

  std::string BillboardSetWrapper::mapBillboardType(const Ogre::BillboardType type)
  {
    std::string res;
    switch(type)
    {
      case Ogre::BillboardType::BBT_POINT:
        res = BBT_POINT_ID;
        break;
      case Ogre::BillboardType::BBT_ORIENTED_COMMON:
        res = BBT_ORIENTED_COMMON_ID;
        break;
      case Ogre::BillboardType::BBT_ORIENTED_SELF:
        res = BBT_ORIENTED_SELF_ID;
        break;
      case Ogre::BillboardType::BBT_PERPENDICULAR_SELF:
        res = BBT_PERPENDICULAR_SELF_ID;
        break;
      case Ogre::BillboardType::BBT_PERPENDICULAR_COMMON:
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
    mObject = mSceneManager->createBillboardSet();
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
    mObject->setMaterialName(id);
  }

  const std::string& BillboardSetWrapper::getMaterialName() const
  {
    return mObject->getMaterialName();
  }

  void BillboardSetWrapper::setBillboards(const DataProxy& dict)
  {
    for(auto& pair : dict)
    {
      mBillboards.emplace_back();
      if(!mBillboards.back().initialize(pair.second, mObject))
        mBillboards.pop_back();
    }
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
