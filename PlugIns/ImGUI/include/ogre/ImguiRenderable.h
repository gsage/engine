
#ifndef _ImGUIRenderable_H__
#define _ImGUIRenderable_H__

#include "OgrePrerequisites.h"
#include "OgreRenderable.h"
#include <OgreRenderOperation.h>

struct ImDrawData;
namespace Gsage
{
  class SceneManager;
  class ImGUIRenderable : public Ogre::Renderable
  {
    protected:

      Ogre::MaterialPtr mMaterial;

      Ogre::RenderOperation mRenderOp;

      void initImGUIRenderable(void);

    public:
      ImGUIRenderable();
      ~ImGUIRenderable();

      void updateVertexData(ImDrawData* data, unsigned int cmdIndex);

      Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const { 
        (void)cam;
        return 0;
      }

      void setMaterial(const Ogre::String& matName);
      virtual void setMaterial(const Ogre::MaterialPtr & material);
      virtual const Ogre::MaterialPtr& getMaterial(void) const;
      virtual void getWorldTransforms(Ogre::Matrix4* xform) const;
      virtual void getRenderOperation(Ogre::RenderOperation& op);
      virtual const Ogre::LightList& getLights(void) const;

      int mVertexBufferSize;
      int mIndexBufferSize;
  };
}// namespace

#endif
