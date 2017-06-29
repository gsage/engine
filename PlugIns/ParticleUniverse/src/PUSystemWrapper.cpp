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

#include "PUSystemWrapper.h"
#include <OgreSceneNode.h>
#include "ParticleUniverseSystemManager.h"

#include "Logger.h"

namespace Gsage
{

  const std::string PUSystemWrapper::TYPE = "puSystem";

  PUSystemWrapper::PUSystemWrapper()
    : mParticleSystem(0)
    , mStartOnCreate(false)
  {
    BIND_PROPERTY("startOnCreate", &mStartOnCreate);
    BIND_ACCESSOR("template", &PUSystemWrapper::setTemplateName, &PUSystemWrapper::getTemplateName);
  }

  PUSystemWrapper::~PUSystemWrapper()
  {
  }

  bool PUSystemWrapper::read(const DataProxy& node)
  {
    ParticleUniverse::ParticleSystemManager* psManager = ParticleUniverse::ParticleSystemManager::getSingletonPtr();
    OgreObject::read(node, "name");
    mParticleSystem = psManager->createParticleSystem(mObjectId, mSceneManager);

    OgreObject::read(node, "startOnCreate");
    OgreObject::read(node, "template");
    mParticleSystem->setScale(1 / mParentNode->getScale());
    mParentNode->attachObject(mParticleSystem);
    if(mStartOnCreate)
      mParticleSystem->start();
  }

  void PUSystemWrapper::setTemplateName(const std::string& templateName)
  {
    ParticleUniverse::ParticleSystem* pTemplate = ParticleUniverse::ParticleSystemManager::getSingletonPtr()->getParticleSystemTemplate(templateName);
    *mParticleSystem = *pTemplate;
    pTemplate->copyAttributesTo(mParticleSystem);
    mParticleSystem->setTemplateName(templateName);
  }

  const std::string& PUSystemWrapper::getTemplateName() const
  {
    return mParticleSystem->getTemplateName();
  }

  void PUSystemWrapper::start()
  {
    mParticleSystem->start();
  }

  void PUSystemWrapper::stop()
  {
    mParticleSystem->stop();
  }

  void PUSystemWrapper::pause()
  {
    mParticleSystem->pause();
  }

  void PUSystemWrapper::pause(const float& time)
  {
    mParticleSystem->pause(time);
  }

  void PUSystemWrapper::resume()
  {
    mParticleSystem->resume();
  }

  void PUSystemWrapper::destroy()
  {
    ParticleUniverse::ParticleSystemManager* psManager = ParticleUniverse::ParticleSystemManager::getSingletonPtr();
    OgreObject::destroy();
    if(!psManager || !mParticleSystem)
      return;

    psManager->destroyParticleSystem(mParticleSystem, mSceneManager);
  }
}
