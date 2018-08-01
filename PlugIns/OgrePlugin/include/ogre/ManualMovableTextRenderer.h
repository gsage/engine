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

#ifndef _ManualMovableTextRenderer_H_
#define _ManualMovableTextRenderer_H_

#include <OgreParticleSystemRenderer.h>
#include <OgreParticle.h>

#if OGRE_VERSION_MAJOR == 1
#include "ogre/v1/MovableText.h"
#else
#include "ogre/v2/MovableText.h"
#endif

#include "Logger.h"

namespace Ogre
{
  class RenderQueue;
  class Particle;
  class Node;
  class SceneNode;

  class MovableTextValue;
  /**
   * Class that represents created movable text node
   */
  class TextNode
  {
    public:
#if OGRE_VERSION_MAJOR == 1
      TextNode(const std::string& name);
#else
      TextNode(IdType id, ObjectMemoryManager* memoryManager, SceneManager* sceneManager);
#endif

      virtual ~TextNode();

      /**
       * Activate text node
       * @param parent Node to attach text to
       * @param value Text value to display
       */
      void activate(MovableTextValue* value, const std::string& fontName);

      /**
       * Deactivate text node
       * this deletes movable text value that was attached to this node
       */
      void deactivate();

      /**
       * Set node position
       * @param position Position
       */
      void setPosition(const Vector3& position);

      /**
       * Set text colour
       */
      void setColour(const ColourValue& colour);

      /**
       * Set text height
       */
      void setHeight(const float value);

    private:
      MovableTextValue* mValue;
      SceneNode* mSceneNode;

      MovableText* mView;
#if OGRE_VERSION_MAJOR == 1
      std::string mName;
#else
      IdType mId;
      ObjectMemoryManager* mObjectManager;
      SceneManager* mSceneManager;
#endif

  };

  /**
   * Movable text string
   */
  class MovableTextValue : public ParticleVisualData
  {
    public:
      MovableTextValue(const std::string& value, SceneNode* attachTo);

      /**
       * Get text value for particle
       */
      const std::string& getValue() const;

      /**
       * Set node that is using this movable text value
       * @param node Pointer to the node
       */
      void setNode(TextNode* node);

      /**
       * Node that is using this movable text value
       * @returns 0 if no node present
       */
      TextNode* getNode();

      /**
       * Get parent node to use for the text node
       */
      SceneNode* getNodeToAttachTo();
    private:
      std::string mValue;
      TextNode* mNode;
      SceneNode* mSceneNode;
  };

  /**
   * Renderer that is used to create floating text particles, like damage
   */
  class ManualMovableTextRenderer : public ParticleSystemRenderer
  {

    public:
      class CmdFontName : public ParamCommand
      {
        public:
          std::string doGet(const void* target) const;
          void doSet(void* target, const std::string& val);
      };

#if OGRE_VERSION_MAJOR == 1
      ManualMovableTextRenderer(const std::string& name);
#else
      ManualMovableTextRenderer(const std::string& name, ObjectMemoryManager* memoryManager, SceneManager* sceneManager);
#endif
      virtual ~ManualMovableTextRenderer();

      /**
       * Set font name to use
       * @param name Id of the font
       */
      void setFontName(const std::string& name);

      /**
       * Get used font name
       */
      const std::string& getFontName() const;

      /**
       * Get the type of this renderer
       */
      const std::string& getType() const;

      /**
       * Updates all nodes in the renderer
       */
#if OGRE_VERSION_MAJOR == 1
      void _updateRenderQueue(RenderQueue* queue, list<Particle*>::type& currentParticles, bool cullIndividually);
#else
      void _updateRenderQueue(RenderQueue *queue, Camera *camera, const Camera *lodCamera, list< Particle * >::type &currentParticles, bool cullIndividually, RenderableArray &outRenderables);
#endif

      /**
       * 
       * @param quota Number of nodes to be allocated
       */
      void _notifyParticleQuota(size_t quota);

      /**
       * Not used
       */
      void _notifyDefaultDimensions(Real width, Real height);

      /**
       * Not used
       */
      void setRenderQueueGroup(uint8 queueID);

      /**
       * Not used
       */
      void setRenderQueueGroupAndPriority(uint8 queueID, ushort priority);

      /**
       * Not used
       */
      void setKeepParticlesInLocalSpace(bool keepLocal);

      /**
       * Not used
       */
      SortMode _getSortMode() const;

#if OGRE_VERSION_MAJOR == 1
      /**
       * Not used
       */
      void visitRenderables(Renderable::Visitor* visitor, bool debugRenderables = false);

      /**
       * Not used
       */
      void _setMaterial(MaterialPtr& mat) {};

      /**
       * Not used
       */
      void _notifyCurrentCamera(Camera* cam) {};

      /**
       * Not used
       */
      void _notifyAttached(Node* parent, bool isTagPoint = false) {};
#else
      /**
       * Not used
       */
      void _setDatablock( HlmsDatablock *datablock ) {};

      /**
       * Not used
       */
      void _setMaterialName( const String &matName, const String &resourceGroup ) {};

      /**
       * Not used
       */
      void _notifyCurrentCamera(const Camera* cam) {};

      /**
       * Not used
       */
      void _notifyAttached(Node* parent) {};

      /**
       * Not used
       */
      void setRenderQueueSubGroup(uint8 queueID) {};
#endif

      /**
       * Not used
       */
      void _destroyVisualData(ParticleVisualData* data) { }
    private:
      /**
       * Adjust particle count to the quota
       */
      void adjustNodeCount();

      typedef std::vector<TextNode> TextNodes;
      typedef std::deque<TextNode*> TextNodesPtrs;
      typedef std::map<Particle*, TextNode*> TextNodesMap;

      TextNodesMap mBusyNodes;
      TextNodesPtrs mFreeNodes;
      TextNodes mNodePool;

      size_t mQuota;

      std::string mName;
      std::string mFontName;

      int mPrevousParticleCount;
#if OGRE_VERSION_MAJOR == 2
      ObjectMemoryManager* mObjectManager;
      SceneManager* mSceneManager;
#endif
    protected:
      static CmdFontName msFontNameCmd;
  };

  class ManualMovableTextRendererFactory : public ParticleSystemRendererFactory
  {
    public:
      ManualMovableTextRendererFactory();
      virtual ~ManualMovableTextRendererFactory();
      /// @copydoc FactoryObj::getType
      const String& getType() const;
      /// @copydoc FactoryObj::createInstance
      ParticleSystemRenderer* createInstance( const String& name );
      /// @copydoc FactoryObj::destroyInstance
      void destroyInstance(ParticleSystemRenderer* ptr);
    private:
      int mCreatedRenderersCounter;
#if OGRE_VERSION_MAJOR == 2
      ObjectMemoryManager *mDummyObjectMemoryManager;
#endif
  };
}

#endif
