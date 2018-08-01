
#ifndef _ImGUIRenderable_H__
#define _ImGUIRenderable_H__

#include "OgrePrerequisites.h"
#include "OgreRenderable.h"
#include <OgreRenderOperation.h>
#include "imgui.h"

#include "Definitions.h"

struct ImDrawData;
namespace Gsage
{
  class SceneManager;
  class ImGUIRenderable : public Ogre::Renderable
  {
    protected:

      Ogre::MaterialPtr mMaterial;

      OgreV1::RenderOperation mRenderOp;

      void initImGUIRenderable(void);
    public:
      ImGUIRenderable();
      ~ImGUIRenderable();

      void updateVertexData(ImDrawData* data, unsigned int cmdIndex);
      void updateVertexData(const ImDrawVert* vtxBuf, const ImDrawIdx* idxBuf, unsigned int vtxCount, unsigned int idxCount);

      Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const {
        (void)cam;
        return 0;
      }
      void setMaterial(const Ogre::String& matName);
      virtual void setMaterial(const Ogre::MaterialPtr & material);
      virtual const Ogre::MaterialPtr& getMaterial(void) const;
      virtual void getWorldTransforms(Ogre::Matrix4* xform) const;
      virtual void getRenderOperation(OgreV1::RenderOperation& op
#if OGRE_VERSION >= 0x020100
          , bool casterPass
#endif
      );
      virtual const Ogre::LightList& getLights(void) const;

      int mVertexBufferSize;
      int mIndexBufferSize;
#if OGRE_VERSION >= 0x020100
      Ogre::VertexElement2VecVec mVertexElement2VecVec;
#endif
  };
}// namespace

#endif
