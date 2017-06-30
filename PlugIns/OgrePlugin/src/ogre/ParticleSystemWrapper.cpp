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

#include "ogre/ParticleSystemWrapper.h"
#include <OgreSceneManager.h>
#include <OgreParticleEmitter.h>
#include <OgreParticleAffector.h>
#include <OgreEntity.h>
#include "Logger.h"

#include "ogre/ManualMovableTextRenderer.h"

namespace Gsage {

  const std::string ParticleSystemWrapper::TYPE = "particleSystem";

  ParticleSystemWrapper::ParticleSystemWrapper()
    : mParticleSystem(0)
  {
    BIND_ACCESSOR_OPTIONAL("template", &ParticleSystemWrapper::setTemplate, &ParticleSystemWrapper::getTemplate);
  }

  ParticleSystemWrapper::~ParticleSystemWrapper()
  {
  }

  bool ParticleSystemWrapper::read(const DataProxy& dict)
  {
    bool res = OgreObject::read(dict);
    if(!mParticleSystem)
      mParticleSystem = mSceneManager->createParticleSystem(mObjectId);
    return res;
  }

  void ParticleSystemWrapper::setTemplate(const std::string& templateName)
  {
    if(mParticleSystem)
      mSceneManager->destroyParticleSystem(mParticleSystem);

    mTemplate = templateName;
    mParticleSystem = mSceneManager->createParticleSystem(mObjectId, mTemplate);
    mParentNode->attachObject(mParticleSystem);
  }

  const std::string& ParticleSystemWrapper::getTemplate() const
  {
    return mTemplate;
  }

  void ParticleSystemWrapper::createParticle(unsigned short index, const std::string& nodeId, const std::string& value)
  {
    createParticle(index, new Ogre::MovableTextValue(value, mSceneManager->getRootSceneNode()), nodeId, Ogre::Quaternion::ZERO);
  }

  void ParticleSystemWrapper::createParticle(unsigned short index, const std::string& nodeId, const std::string& value, const Ogre::Quaternion& rotation)
  {
    createParticle(index, new Ogre::MovableTextValue(value, mSceneManager->getRootSceneNode()), nodeId, rotation);
  }

  void ParticleSystemWrapper::createParticle(unsigned short index, const std::string& nodeId)
  {
    createParticle(index, 0, nodeId, Ogre::Quaternion::ZERO);
  }

  void ParticleSystemWrapper::createParticle(unsigned short index, Ogre::ParticleVisualData* data, const std::string& nodeId, const Ogre::Quaternion& rotation)
  {
    if(mParticleSystem->getNumEmitters() < index)
      return;

    Ogre::ParticleEmitter* emitter = mParticleSystem->getEmitter(index);
    Ogre::Particle* p = mParticleSystem->createParticle();
    if(!p)
      return;

    Ogre::Node* node = mSceneManager->getSceneNode(nodeId);
    if(!node)
      return;

    if(data)
      p->_notifyVisualData(data);

    emitter->_initParticle(p);
    p->position  = 
      (node->_getDerivedOrientation() *
       (node->_getDerivedScale() * p->position))
      + node->_getDerivedPosition();

    p->direction = rotation == Ogre::Quaternion::ZERO ?
      node->_getDerivedOrientation() * p->direction :
      rotation * p->direction;
  }
}
