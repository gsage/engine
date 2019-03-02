#ifndef _ManualTextureManager_H_
#define _ManualTextureManager_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev and contributors

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

#include <OgreTexture.h>
#include "systems/RenderSystem.h"

namespace Gsage {
  class OgreRenderSystem;
  /**
   * Implements abstract texture class Texture
   */
  class OgreTexture : public Texture, public Ogre::Texture::Listener
  {
    public:
      class ScalingPolicy
      {
        public:
          ScalingPolicy(OgreTexture& texture);
          virtual ~ScalingPolicy();
          /**
           * Forces rescaling without width/height change
           */
          void invalidate();
          /**
           * Update scaling policy
           */
          void update(int width, int height);

          /**
           * Should be called in the render loop
           *
           * @returns true if the texture was recreated
           */
          bool render();

        protected:
          /**
           * Actual resize
           */
          virtual bool resize() = 0;

          OgreTexture& mTexture;
          int mWidth;
          int mHeight;
          bool mDirty;
      };

      class DefaultScalingPolicy : public ScalingPolicy
      {
        public:
          DefaultScalingPolicy(OgreTexture& texture);
        protected:
          bool resize();
      };

      class AllocateScalingPolicy : public ScalingPolicy
      {
        public:
          AllocateScalingPolicy(OgreTexture& texture, float scalingFactor);
        protected:
          bool resize();
        private:
          float mScalingFactor;
      };

      OgreTexture(const std::string& name, const DataProxy& params, Ogre::PixelFormat pixelFormat);
      virtual ~OgreTexture();

      /**
       * Update texture data
       *
       * @param buffer buffer to use
       * @param size provided buffer size
       * @param width buffer width
       * @param height buffer height
       */
      void update(const void* buffer, size_t size, int width, int height);

      /**
       * Check if the texture has actual data
       */
      inline bool hasData() const { return !mTexture.isNull() && mHasData; };

      /**
       * Set texture size
       *
       * @param width texture width
       * @param height texture height
       */
      void setSize(int width, int height);

      /**
       * Ogre::Texture::Listener implementation
       */
      void unloadingComplete(Ogre::Resource* res);

      /**
       * Create the underlying texture
       *
       * @param width override width
       * @param height override height
       */
      void create(int width = -1, int height = -1);

      /**
       * Texture buffer was changed
       */
      inline bool isDirty() const { return mDirty; }

      /**
       * Update texture using supplied buffer
       */
      void render();

      /**
       * Get underlying OgreTexture object
       */
      inline Ogre::TexturePtr getOgreTexture() { return mTexture; }

      /**
       * Destroy underlying Ogre::TexturePtr
       */
      void destroy();
    private:
      friend class ScalingPolicy;

      std::unique_ptr<OgreTexture::ScalingPolicy> createScalingPolicy(const DataProxy& params);

      Ogre::TexturePtr mTexture;
      std::string mName;
      std::unique_ptr<OgreTexture::ScalingPolicy> mScalingPolicy;

      bool mHasData;
      bool mDirty;
      bool mCreate;
  };

  /**
   * This class is used to handle manual texture management and update
   */
  class ManualTextureManager : public Ogre::Texture::Listener
  {
    public:
      ManualTextureManager(OgreRenderSystem* renderSystem);
      virtual ~ManualTextureManager();

      /**
       * Destroy all texture objects
       */
      void reset();

      /**
       * Create manual texture
       *
       * \verbatim embed:rst:leading-asterisk
       *
       * List of possible texture parameters:
       *
       *  * :code:`group` resource group to use (optional). Defaults to :code:`DEFAULT_RESOURCE_GROUP_NAME`.
       *  * :code:`textureType` texture type (optional). Defaults to :code:`TEX_TYPE_2D`.
       *  * :code:`width` initial texture width (required).
       *  * :code:`height` initial texture height (required).
       *  * :code:`depth` depth	The depth of the texture (optional). Defaults to 0.
       *  * :code:`numMipmaps` The number of pre-filtered mipmaps to generate. If left to MIP_DEFAULT then the TextureManager's default number of mipmaps will be used (see setDefaultNumMipmaps()) If set to MIP_UNLIMITED mipmaps will be generated until the lowest possible level, 1x1x1.
       *  * :code:`pixelFormat` texture pixel format (optional). Defaults to :code:`PF_R8G8B8A8`.
       *  * :code:`usage` usage type (optional). Defaults to :code:`TU_DEFAULT`.
       *  * :code:`hwGammaCorrection` use gamma correction (optional). Defaults to :code:`false`.
       *  * :code:`fsaa` antialiasing (optional). Defaults to 0.
       *  * :code:`fsaaHint` The level of multisampling to use if this is a render target.
       *                     Ignored if usage does not include TU_RENDERTARGET or if the device does not support it. (optional).
       *  * :code:`explicitResolve` Whether FSAA resolves are done implicitly when used as texture, or must be done explicitly. (optional).
       *  * :code:`shareableDepthBuffer` Only valid for depth texture formats. When true, the depth buffer is a "view" of an existing depth texture (e.g. useful for reading the depth buffer contents of a GBuffer pass in deferred rendering). When false, the texture gets its own depth buffer created for itself (e.g. useful for shadow mapping, which is a depth-only pass).
       *
       * \endverbatim
       *
       * @param handle texture handle
       * @param params Texture params, see above
       */
      TexturePtr createTexture(RenderSystem::TextureHandle handle, DataProxy params);

      /**
       * Gets existing texture
       *
       * @param handle texture handle
       */
      TexturePtr getTexture(RenderSystem::TextureHandle handle);

      /**
       * Delete texture by handle
       *
       * @param handle texture id
       * @returns true if succeed
       */
      bool deleteTexture(RenderSystem::TextureHandle handle);

      /**
       * Set default pixelFormat
       *
       * @param format Ogre::PixelFormat
       */
      inline void setDefaultPixelFormat(Ogre::PixelFormat format) { mPixelFormat = format; }

      /**
       * Update dirty textures
       */
      void updateDirtyTexures();
    private:

      std::map<RenderSystem::TextureHandle, TexturePtr> mTextures;
      Ogre::PixelFormat mPixelFormat;
      OgreRenderSystem* mRenderSystem;
  };
}

#endif
