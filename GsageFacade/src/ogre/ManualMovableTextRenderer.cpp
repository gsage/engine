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

#include "ogre/ManualMovableTextRenderer.h"
#include <OgreRenderQueue.h>
#include <OgreParticle.h>
#include <OgreNode.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

namespace Ogre
{

  const std::string manualMovableTextRendererName = "manual_text";

  TextNode::TextNode(const std::string& name)
    : mValue(0)
    , mView(0)
    , mName(name)
    , mSceneNode(0)
  {
  }

  TextNode::~TextNode()
  {
    if(mValue)
      delete mValue;

    if(mView)
      delete mView;

    if(mSceneNode)
      mSceneNode->getCreator()->destroySceneNode(mSceneNode);
  }

  void TextNode::activate(MovableTextValue* value, const std::string& fontName)
  {
    if(value->getNodeToAttachTo() == 0)
      return;

    mValue = value;
    mValue->setNode(this);
    if(!mView)
    {
      mView = new MovableText(mName, mValue->getValue(), fontName, 1, ColourValue(1.0, 1.0, 1.0));
      mSceneNode = mValue->getNodeToAttachTo()->createChildSceneNode(mName);
      mSceneNode->attachObject(mView);
    }
    else
    {
      mValue->getNodeToAttachTo()->addChild(mSceneNode);
    }

    mView->showOnTop(true);
    mView->setCaption(mValue->getValue());
  }

  void TextNode::deactivate()
  {
    mValue->getNodeToAttachTo()->removeChild(mSceneNode);
    delete mValue;
    mValue = 0;
  }

  void TextNode::setPosition(const Vector3& position)
  {
    if(!mSceneNode)
      return;
    mSceneNode->setPosition(position);
  }

  void TextNode::setColour(const ColourValue& colour)
  {
    mView->setColor(colour);
  }

  void TextNode::setHeight(const float value)
  {
    mView->setCharacterHeight(value);
  }

  MovableTextValue::MovableTextValue(const std::string& value, SceneNode* attachTo)
    : mValue(value)
    , mNode(0)
    , mSceneNode(attachTo)
  {
  }

  const std::string& MovableTextValue::getValue() const
  {
    return mValue;
  }

  void MovableTextValue::setNode(TextNode* node)
  {
    mNode = node;
  }

  TextNode* MovableTextValue::getNode()
  {
    return mNode;
  }

  SceneNode* MovableTextValue::getNodeToAttachTo()
  {
    return mSceneNode;
  }

  ManualMovableTextRenderer::CmdFontName ManualMovableTextRenderer::msFontNameCmd;

  ManualMovableTextRenderer::ManualMovableTextRenderer(const std::string& name)
    : mName(name)
    , mQuota(0)
    , mPrevousParticleCount(0)
  {
    if(createParamDictionary("ManualMovableTextRenderer"))
    {
      ParamDictionary* dict = getParamDictionary();
      dict->addParameter(ParameterDef("font_name", 
            "fontdef to use, should be loaded by the font manager",
            PT_STRING),
          &msFontNameCmd);
    }
  }

  ManualMovableTextRenderer::~ManualMovableTextRenderer()
  {
  }

  void ManualMovableTextRenderer::setFontName(const std::string& name)
  {
    mFontName = name;
    // TODO: update all nodes with new font
  }

  const std::string& ManualMovableTextRenderer::getFontName() const
  {
    return mFontName;
  }

  const std::string& ManualMovableTextRenderer::getType() const
  {
    return manualMovableTextRendererName; 
  }

  void ManualMovableTextRenderer::_updateRenderQueue(RenderQueue* queue, list<Particle*>::type& currentParticles, bool cullIndividually)
  {

    for (list<Particle*>::type::iterator i = currentParticles.begin();
        i != currentParticles.end(); ++i)
    {
      Particle* p = *i;
      if(p->getVisualData() == 0)
        continue;

      MovableTextValue* value = static_cast<MovableTextValue*>(p->getVisualData());
      if(value->getNode() == 0)
      {
        if(mFreeNodes.size() != 0)
        {
          mFreeNodes.front()->activate(value, mFontName);
          mBusyNodes[p] = mFreeNodes.front();
          mFreeNodes.pop_front();
        }
        else
        {
          continue;
        }
      }
      value->getNode()->setPosition(p->position);
      value->getNode()->setColour(p->colour);
      if(p->mHeight > 0)
        value->getNode()->setHeight(p->mHeight);
    }


    if(mPrevousParticleCount > currentParticles.size())
    {
      TextNodesMap::iterator iter = mBusyNodes.begin();
        while(iter != mBusyNodes.end())
        {
          if(std::find(currentParticles.begin(), currentParticles.end(), (*iter).first) == currentParticles.end())
          {
            mFreeNodes.push_back((*iter).second);
            (*iter).second->deactivate();
            TextNodesMap::iterator tmp = iter++;
            mBusyNodes.erase(tmp);
          }
          else
          {
            iter++;
          }
        }
    }
    mPrevousParticleCount = currentParticles.size();
  }

  void ManualMovableTextRenderer::_setMaterial(MaterialPtr& mat)
  {

  }

  void ManualMovableTextRenderer::_notifyCurrentCamera(Camera* cam)
  {
    // iterate over all movable object and do notify current camera
  }

  void ManualMovableTextRenderer::_notifyAttached(Node* parent, bool isTagPoint)
  {
  }

  void ManualMovableTextRenderer::_notifyParticleQuota(size_t quota)
  {
    mQuota = quota;
    adjustNodeCount();
  }

  void ManualMovableTextRenderer::_notifyDefaultDimensions(Real width, Real height)
  {

  }

  void ManualMovableTextRenderer::setRenderQueueGroup(uint8 queueID)
  {

  }

  void ManualMovableTextRenderer::setRenderQueueGroupAndPriority(uint8 queueID, ushort priority)
  {

  }

  void ManualMovableTextRenderer::setKeepParticlesInLocalSpace(bool keepLocal)
  {
  }

  SortMode ManualMovableTextRenderer::_getSortMode() const
  {
    return SM_DISTANCE;
  }

  void ManualMovableTextRenderer::visitRenderables(Renderable::Visitor* visitor, bool debugRenderables)
  {
  }

  void ManualMovableTextRenderer::adjustNodeCount()
  {
    if(mQuota == mNodePool.size())
      return;

    mNodePool.reserve(mQuota * 2);

    if(mQuota < mNodePool.size())
    {
      mNodePool.erase(mNodePool.begin() + mQuota, mNodePool.end());
      // TODO: iterate over free nodes and check if active
    }
    else
    {
      for(int i = mNodePool.size(); i < mQuota; i++)
      {
        std::stringstream ss;
        ss << mName << "textNode" << i;
        mNodePool.emplace_back(ss.str());
        mFreeNodes.push_back(&mNodePool.back());
      }
    }
  }

  ManualMovableTextRendererFactory::ManualMovableTextRendererFactory()
    : mCreatedRenderersCounter(0)
  {
  }

  const String& ManualMovableTextRendererFactory::getType() const
  {
    return manualMovableTextRendererName;
  }

  ParticleSystemRenderer* ManualMovableTextRendererFactory::createInstance(const String& name)
  {
    std::stringstream ss;
    ss << name << (mCreatedRenderersCounter++);
    return OGRE_NEW ManualMovableTextRenderer(ss.str());
  }

  void ManualMovableTextRendererFactory::destroyInstance(ParticleSystemRenderer* ptr)
  {
    OGRE_DELETE ptr;
  }

  std::string ManualMovableTextRenderer::CmdFontName::doGet(const void* target) const
  {
    return static_cast<const ManualMovableTextRenderer*>(target)->getFontName();
  }

  void ManualMovableTextRenderer::CmdFontName::doSet(void* target, const std::string& val)
  {
    static_cast<ManualMovableTextRenderer*>(target)->setFontName(val);
  }
}

