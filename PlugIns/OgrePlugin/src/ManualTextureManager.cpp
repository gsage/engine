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

#include "ManualTextureManager.h"
#include "Definitions.h"
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>

#if OGRE_VERSION >= 0x020100
#define _BLANKSTRING Ogre::BLANKSTRING
#else
#define _BLANKSTRING Ogre::StringUtil::BLANK
#endif

namespace Gsage {
  OgreTexture::OgreTexture(const std::string& name, const DataProxy& params, Ogre::PixelFormat pixelFormat)
    : Texture()
    , mName(name)
    , mParams(params)
    , mHasData(false)
    , mDirty(false)
  {
    params.read("width", mWidth);
    params.read("height", mHeight);
    if(mParams.count("pixelFormat") == 0) {
      mParams.put("pixelFormat", pixelFormat);
    }
    create();
    mTexture->addListener(this);
  }

  void OgreTexture::create()
  {
    mValid = false;
    Ogre::TextureManager& texManager = Ogre::TextureManager::getSingleton();
    Ogre::String group = mParams.get("group", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::TextureType textureType = mParams.get("textureType", Ogre::TEX_TYPE_2D);

    unsigned int depth = mParams.get("depth", 1);
    int numMipmaps = mParams.get("numMipmaps", 0);
    Ogre::PixelFormat pixelFormat = mParams.get("pixelFormat", Ogre::PF_R8G8B8A8);
    int usage = mParams.get("usage", Ogre::TU_DEFAULT);
    bool hwGammaCorrection = mParams.get("hwGammaCorrection", true);
    unsigned int fsaa = mParams.get("fsaa", 0);
    std::string fsaaHint = mParams.get("fsaaHint", _BLANKSTRING);
#if OGRE_VERSION >= 0x020100
    bool explicitResolve = mParams.get("explicitResolve", false);
    bool shareableDepthBuffer = mParams.get("shareableDepthBuffer", true);
#endif

    Ogre::ResourcePtr resource = texManager.getResourceByName(mName, group);
    if(!resource.isNull()) {
      texManager.remove(resource);
    }

    mHasData = false;
    mTexture = texManager.createManual(
      mName,
      group,
      textureType,
      mWidth,
      mHeight,
      depth,
      numMipmaps,
      pixelFormat,
      usage,
      0,
      hwGammaCorrection,
      fsaa,
      fsaaHint
#if OGRE_VERSION >= 0x020100
      ,
      explicitResolve,
      shareableDepthBuffer
#endif
    );
    OgreV1::HardwarePixelBufferSharedPtr texBuf = mTexture->getBuffer();
    texBuf->lock(OgreV1::HardwareBuffer::HBL_DISCARD);
    memset(texBuf->getCurrentLock().data, 0, mWidth * mHeight * Ogre::PixelUtil::getNumElemBytes(mTexture->getFormat()));
    texBuf->unlock();
    LOG(TRACE) << "Created texture with size " << mWidth << "x" << mHeight;
    mValid = true;
  }

  void OgreTexture::update(const void* buffer, size_t size, int width, int height)
  {
    std::lock_guard<std::mutex> lock(mSizeUpdateLock);

    if(!mValid) {
      LOG(WARNING) << "Tried to update invalid texture";
      return;
    }

    if(mSize < size) {
      if(mBuffer) {
        delete[] mBuffer;
      }
      mBuffer = new char[size]();
    }

    memcpy(mBuffer, buffer, size);
    mBufferWidth = width;
    mBufferHeight = height;
    mSize = size;
    mDirty = true;
  }

  void OgreTexture::setSize(int width, int height)
  {
    if(width == mWidth && height == mHeight) {
      return;
    }

    std::lock_guard<std::mutex> lock(mSizeUpdateLock);
    if(!mValid) {
      LOG(WARNING) << "Tried to update invalid texture";
      return;
    }

    mWidth = width;
    mHeight = height;
    create();

    Texture::setSize(width, height);
  }

  void OgreTexture::unloadingComplete(Ogre::Resource* res)
  {
    if(res == mTexture.get()) {
      mValid = false;
      mTexture.setNull();
    }
  }

  void OgreTexture::render()
  {
    std::lock_guard<std::mutex> lock(mSizeUpdateLock);
    if(mWidth != mBufferWidth) {
      return;
    }

    mDirty = false;
    char numBytes = Ogre::PixelUtil::getNumElemBytes(mTexture->getFormat());
    size_t textureSize = mWidth * mHeight * numBytes;

    OgreV1::HardwarePixelBufferSharedPtr texBuf = mTexture->getBuffer();
    texBuf->lock(OgreV1::HardwareBuffer::HBL_DISCARD);
    memcpy(texBuf->getCurrentLock().data, mBuffer, std::min(mSize, textureSize));
    texBuf->unlock();
    mHasData = true;
  }

  OgreTexture::~OgreTexture()
  {
    if(mTexture.isNull()) {
      return;
    }
    Ogre::TextureManager& texManager = Ogre::TextureManager::getSingleton();
    texManager.remove(mTexture->getHandle());
  }

  ManualTextureManager::ManualTextureManager()
    : mPixelFormat(Ogre::PF_R8G8B8A8)
  {
  }

  ManualTextureManager::~ManualTextureManager()
  {
  }

  TexturePtr ManualTextureManager::createTexture(RenderSystem::TextureHandle handle, DataProxy params)
  {
    if(mTextures.count(handle) > 0) {
      LOG(WARNING) << "Texture with handle " << handle << " already exists.";
      return nullptr;
    }

    unsigned int width = params.get("width", 0);
    unsigned int height = params.get("height", 0);
    if(width == 0 || height == 0) {
      LOG(ERROR) << "Failed to create texture with zero width and height";
      return nullptr;
    }

    OgreTexture* texture = new OgreTexture(handle, params, mPixelFormat);

    texture->setSize(width, height);
    mTextures[handle] = TexturePtr(texture);

    return mTextures[handle];
  }

  TexturePtr ManualTextureManager::getTexture(RenderSystem::TextureHandle handle)
  {
    if(mTextures.count(handle) == 0) {
      return nullptr;
    }

    if(!mTextures[handle]->isValid()) {
      mTextures.erase(handle);
      return nullptr;
    }

    return mTextures[handle];
  }

  bool ManualTextureManager::deleteTexture(RenderSystem::TextureHandle handle)
  {
    if(mTextures.count(handle) == 0) {
      return false;
    }

    mTextures.erase(handle);
    return true;
  }

  void ManualTextureManager::updateDirtyTexures()
  {
    for(auto pair : mTextures) {
      OgreTexture* tex = static_cast<OgreTexture*>(pair.second.get());
      if(tex->isDirty()) {
        tex->render();
      }
    }
  }

}
