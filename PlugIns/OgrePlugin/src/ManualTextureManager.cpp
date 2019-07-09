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
#include <OgreHlmsUnlitDatablock.h>
#include <OgreHlmsUnlit.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsManager.h>
#else
#define _BLANKSTRING Ogre::StringUtil::BLANK
#endif

#include <OgrePixelFormat.h>
#include <OgreImage.h>
#include <OgreDataStream.h>
#include <fstream>
#include "systems/OgreRenderSystem.h"

namespace Gsage {
  OgreTexture::ScalingPolicy::ScalingPolicy(OgreTexture& texture)
    : mTexture(texture)
    , mWidth(0)
    , mHeight(0)
  {
  }

  OgreTexture::ScalingPolicy::~ScalingPolicy()
  {
  }

  void OgreTexture::ScalingPolicy::invalidate()
  {
    mDirty = true;
  }

  void OgreTexture::ScalingPolicy::update(int width, int height)
  {
    if(width != mWidth || height != mHeight) {
      mDirty = true;
      mWidth = width;
      mHeight = height;
    }
  }

  bool OgreTexture::ScalingPolicy::render()
  {
    if(mWidth == 0 || mHeight == 0) {
      return false;
    }

    if(mDirty) {
      bool created = resize();
      if(mTexture.isValid()) {
        mDirty = false;
      }
      return created;
    }

    return false;
  }

  OgreTexture::DefaultScalingPolicy::DefaultScalingPolicy(OgreTexture& texture)
    : ScalingPolicy(texture)
  {
  }

  bool OgreTexture::DefaultScalingPolicy::resize()
  {

    mTexture.create(mWidth, mHeight);
    return true;
  }

  OgreTexture::AllocateScalingPolicy::AllocateScalingPolicy(OgreTexture& texture, float scalingFactor)
    : ScalingPolicy(texture)
    , mScalingFactor(scalingFactor)
  {
    assert(scalingFactor > 1.0);
  }

  bool OgreTexture::AllocateScalingPolicy::resize()
  {
    bool created = false;
    if(!mTexture.isValid() || mTexture.mTexture->getWidth() < (Ogre::uint32)mWidth || mTexture.mTexture->getHeight() < (Ogre::uint32)mHeight) {
      mTexture.create(mWidth * mScalingFactor, mHeight * mScalingFactor);
      if(!mTexture.isValid()) {
        return false;
      }
      created = true;
    }
    float widthScale = (float)mWidth/(float)mTexture.mTexture->getWidth();
    float heightScale = (float)mHeight/(float)mTexture.mTexture->getHeight();

    Gsage::Vector2 tl(0.f, 0.f);
    Gsage::Vector2 bl(0.f, heightScale);
    Gsage::Vector2 tr(widthScale, 0.0f);
    Gsage::Vector2 br(widthScale, heightScale);
    mTexture.setUVs(tl, bl, tr, br);
    return created;
  }

  OgreTexture::OgreTexture(const std::string& name, const DataProxy& params, Ogre::PixelFormat pixelFormat, int flags)
    : Texture(name, params)
    , mHasData(false)
    , mDirty(false)
    , mCreate(false)
    , mScalingPolicy(std::move(createScalingPolicy(params)))
    , mFlags(flags)
  {
    if(mParams.count("pixelFormat") == 0) {
      mParams.put("pixelFormat", pixelFormat);
    }
    create();
  }

  void OgreTexture::create(int width, int height)
  {
    mValid = false;
    Ogre::TextureManager* texManager = Ogre::TextureManager::getSingletonPtr();
    if(!texManager)
      return;

    Ogre::String group = mParams.get("group", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::TextureType textureType = mParams.get("textureType", Ogre::TEX_TYPE_2D);

    unsigned int depth = mParams.get("depth", 1);
    int numMipmaps = mParams.get("numMipmaps", 0);
    Ogre::PixelFormat pixelFormat = (Ogre::PixelFormat)mParams.get("pixelFormat", (int)Ogre::PF_R8G8B8A8);
    Ogre::TextureUsage usage = (Ogre::TextureUsage)mParams.get("usage", (unsigned int)Ogre::TU_DEFAULT);
    bool hwGammaCorrection = mParams.get("hwGammaCorrection", true);
    unsigned int fsaa = mParams.get("fsaa", 0);
    std::string fsaaHint = mParams.get("fsaaHint", _BLANKSTRING);
#if OGRE_VERSION >= 0x020100
    bool explicitResolve = mParams.get("explicitResolve", false);
    bool shareableDepthBuffer = mParams.get("shareableDepthBuffer", true);
#endif

    Ogre::ResourcePtr resource = texManager->getResourceByName(mHandle, group);
    if(!resource.isNull()) {
      if(!mTexture.isNull()) {
        mTexture->removeListener(this);
      }
      texManager->remove(resource);
    }

    width = width < 0 ? mWidth : width;
    height = height < 0 ? mHeight : height;

    mHasData = false;
    mTexture = texManager->createManual(
        mHandle,
        group,
        textureType,
        width,
        height,
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
#if GSAGE_PLATFORM != GSAGE_APPLE
    if((usage & Ogre::TU_RENDERTARGET) == 0) {
      OgreV1::HardwarePixelBufferSharedPtr texBuf = mTexture->getBuffer();
      texBuf->lock(OgreV1::HardwareBuffer::HBL_DISCARD);
      memset(texBuf->getCurrentLock().data, 0, width * height * Ogre::PixelUtil::getNumElemBytes(mTexture->getFormat()));
      texBuf->unlock();
    }
#endif

#if OGRE_VERSION >= 0x020100
    // additionally set up Hlms for 2.1
    Ogre::HlmsManager *hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
    Ogre::HlmsUnlit *hlmsUnlit = static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));
    Ogre::HlmsUnlitDatablock *datablock = static_cast<Ogre::HlmsUnlitDatablock*>(hlmsUnlit->getDatablock(mHandle));
    if(!datablock) {
      datablock = static_cast<Ogre::HlmsUnlitDatablock*>(
          hlmsUnlit->createDatablock(mHandle,
            mHandle,
            Ogre::HlmsMacroblock(),
            Ogre::HlmsBlendblock(),
            Ogre::HlmsParamVec()));
    }

    datablock->setTexture(Ogre::PBSM_DIFFUSE, 0, mTexture);
#endif
    mTexture->addListener(this);
    LOG(TRACE) << "Created texture " << mHandle << " with size " << width << "x" << height;
    mValid = true;
    if(usage & Ogre::TU_RENDERTARGET) {
      mHasData = true;
    }
  }

  void OgreTexture::update(const void* buffer, size_t size, int width, int height)
  {
    update(
      buffer, size, width, height,
      Rect<int>(0, 0, width, height)
    );
  }

  void OgreTexture::update(const void* buffer, size_t size, int width, int height, const Rect<int>& area)
  {
    std::lock_guard<std::mutex> lock(mLock);
    int pixelSize = size / (width * height);

    if(mSize < size) {
      allocateBuffer(size);
    }

    mBufferWidth = width;
    mBufferHeight = height;
    mSize = size;
    mScalingPolicy->update(width, height);

    size_t textureRowWidth = width * pixelSize;

    size_t start = area.x * pixelSize + textureRowWidth * area.y;
    size_t end = std::min(mSize, start + textureRowWidth * area.height);
    size_t areaRowSize = area.width * pixelSize;

    char* src = (char*)buffer;

    if (start == 0 && end == mSize) {
      memcpy(mBuffer, src, mSize);
    } else {
      while (start < end) {
        memcpy(&mBuffer[start], &src[start], areaRowSize);
        start += textureRowWidth;
      }
    }

    mDirty = true;
    // no need to update dirty regions if the texture does not support partial write
    if(mFlags & OgreTexture::BlitDirty) {
      mDirtyRegions.push_back(area);
    }
  }

  void OgreTexture::setSize(int width, int height)
  {
    if(width == mWidth && height == mHeight) {
      return;
    }

    std::lock_guard<std::mutex> lock(mLock);

    mWidth = width;
    mHeight = height;
    Texture::setSize(width, height);
  }

  void OgreTexture::unloadingComplete(Ogre::Resource* res)
  {
    if(res == mTexture.get()) {
      mValid = false;
      mTexture.setNull();
    }
  }

  bool OgreTexture::blitAll()
  {
    if(mValid && mSize > 0) {
      OgreV1::HardwarePixelBufferSharedPtr texBuf = mTexture->getBuffer();
      texBuf->lock(OgreV1::HardwareBuffer::HBL_DISCARD);

      size_t pixelSize = Ogre::PixelUtil::getNumElemBytes(mTexture->getFormat());
      int textureRowWidth = mTexture->getWidth() * pixelSize;
      size_t textureSize = textureRowWidth * mTexture->getHeight();
      char* dest = static_cast<char*>(texBuf->getCurrentLock().data);
      if(mSize != textureSize) {
        int bufferRowWidth = mBufferWidth * pixelSize;

        int bufferOffset = 0;
        int destOffset = 0;
        while(bufferOffset < mSize && destOffset < textureSize) {
          memcpy(&dest[destOffset], &mBuffer[bufferOffset], bufferRowWidth);
          destOffset += textureRowWidth;
          bufferOffset += bufferRowWidth;
        }
      } else {
        memcpy(dest, mBuffer, mSize);
      }

      texBuf->unlock();
      mDirtyRegions.clear();

      return true;
    }

    return false;
  }

  bool OgreTexture::blitDirty()
  {

    size_t pixelSize = Ogre::PixelUtil::getNumElemBytes(mTexture->getFormat());
    size_t textureSize = mTexture->getWidth() * mTexture->getHeight() * pixelSize;

    if(mValid && mSize > 0 && textureSize >= mSize) {
      OgreV1::HardwarePixelBufferSharedPtr texBuf = mTexture->getBuffer();

      while(mDirtyRegions.size() > 0) {
        Rect<int> area = *(mDirtyRegions.end() - 1);

        const Ogre::PixelBox& pb = texBuf->lock(Ogre::Image::Box(area.x, area.y, area.x + area.width, area.y + area.height), OgreV1::HardwareBuffer::HBL_DISCARD);
        char* dest = static_cast<char*>(pb.data);
        size_t textureRowWidth = mBufferWidth * pixelSize;

        size_t start = area.x * pixelSize + textureRowWidth * area.y;
        size_t end = start + textureRowWidth * area.height;

        size_t imageStart = 0;
        size_t rowSize = pb.rowPitch * pixelSize;

        while (start < end) {
          memcpy(&dest[imageStart], &mBuffer[start], rowSize);
          start += textureRowWidth;
          imageStart += rowSize;
        }

        texBuf->unlock();

        mDirtyRegions.pop_back();
      }

      return true;
    }

    return false;
  }

  void OgreTexture::render()
  {
    std::lock_guard<std::mutex> lock(mLock);
    bool wasCreated = mScalingPolicy->render();

    if(mTexture->getUsage() & Ogre::TU_RENDERTARGET) {
      mHasData = true;
    } else {
      if(mFlags & OgreTexture::BlitDirty) {
        mHasData = blitDirty();
      } else {
        mHasData = blitAll();
      }
    }

    if(mHasData)
      mDirty = false;

    if(wasCreated && mValid) {
      fireEvent(Event(Texture::RECREATE));
    }
  }

  std::unique_ptr<OgreTexture::ScalingPolicy> OgreTexture::createScalingPolicy(const DataProxy& params)
  {
    std::string policyType = params.get("scalingPolicy.type", "default");
    if(policyType == "default") {
      return std::make_unique<OgreTexture::DefaultScalingPolicy>(*this);
    } else if(policyType == "allocate") {
      return std::make_unique<OgreTexture::AllocateScalingPolicy>(*this, params.get("scalingPolicy.scalingFactor", 2.0f));
    }

    LOG(WARNING) << "Unknown scaling policy type " << policyType << ", falling back to default policy";
    return std::make_unique<OgreTexture::DefaultScalingPolicy>(*this);
  }

  OgreTexture::~OgreTexture()
  {
    destroy();
  }

  void OgreTexture::destroy()
  {
    if(!mTexture.isNull()) {
      Ogre::TextureManager* texManager = Ogre::TextureManager::getSingletonPtr();
      if(texManager) {
        texManager->remove(mHandle);
      }
      mTexture.setNull();
    }
    mScalingPolicy->invalidate();
    mValid = false;
    mDirty = true;
    fireEvent(Event(Texture::DESTROY));
  }

  ManualTextureManager::ManualTextureManager(OgreRenderSystem* renderSystem)
    : mPixelFormat(Ogre::PF_R8G8B8A8)
    , mRenderSystem(renderSystem)
  {
    mRenderSystemCapabilities["OpenGL 3+ Rendering Subsystem"] = OgreTexture::BlitDirty;
    mRenderSystemCapabilities["OpenGL Rendering Subsystem"] = OgreTexture::BlitDirty;
  }

  ManualTextureManager::~ManualTextureManager()
  {
  }

  void ManualTextureManager::reset() {
    for(auto& pair : mTextures) {
      static_cast<OgreTexture*>(pair.second.get())->destroy();
    }
  }

  TexturePtr ManualTextureManager::createTexture(RenderSystem::TextureHandle handle, DataProxy params)
  {
    if(mTextures.count(handle) > 0) {
      LOG(WARNING) << "Texture with handle " << handle << " already exists.";
      return nullptr;
    }

    unsigned int width = params.get("width", 0);
    unsigned int height = params.get("height", 0);
    if((width == 0 || height == 0)) {
      LOG(ERROR) << "Failed to create texture with zero width and height";
      return nullptr;
    }

    int flags = 0;
    Ogre::String rsName = mRenderSystem->getRenderSystem()->getName();
    if(mRenderSystemCapabilities.find(rsName) != mRenderSystemCapabilities.end()) {
      flags |= mRenderSystemCapabilities[rsName];
    }

    OgreTexture* texture = new OgreTexture(handle, params, mPixelFormat, flags);
    TexturePtr tex = TexturePtr(texture);
    std::string scalemode = params.get("scalemode", "keepAspect");

    Ogre::PixelFormat fmt = texture->getOgreTexture()->getFormat();

    std::string path = params.get("path", "");
    if(!path.empty()) {
      mRenderSystem->asyncTask([path, tex, fmt, scalemode]() {
        std::ifstream ifs(path, std::ios::binary|std::ios::in);
        if(ifs.is_open()) {
          size_t indexOfExtension = path.find_last_of('.');
          if (indexOfExtension != std::string::npos) {
            std::string ext = path.substr(indexOfExtension + 1);
            Ogre::DataStreamPtr dataStream(new Ogre::FileStreamDataStream(path, &ifs, false));
            Ogre::Image img;
            img.load(dataStream, ext);
            Ogre::MemoryDataStreamPtr buf;
            buf.bind(OGRE_NEW Ogre::MemoryDataStream(
                  Ogre::PixelUtil::getMemorySize(
                    img.getWidth(), img.getHeight(), img.getDepth(), fmt)));

            Ogre::PixelBox corrected(img.getWidth(), img.getHeight(), img.getDepth(), fmt, buf->getPtr());
            Ogre::PixelUtil::bulkPixelConversion(img.getPixelBox(0, 0), corrected);

            size_t size = corrected.getWidth() * corrected.getHeight() * Ogre::PixelUtil::getNumElemBytes(fmt);
            int width;
            int height;

            if(scalemode == "keepSize") {
              width = img.getWidth();
              height = img.getHeight();
            } else if(scalemode == "keepAspect" && img.getHeight() > 0) {
              std::tie(width, height) = tex->getSize();
              float aspect = (float)img.getWidth() / (float)img.getHeight();

              if(aspect > 1) {
                height = (int)(width / aspect);
              } else {
                width = (int)(height * aspect);
              }
            } else {
              std::tie(width, height) = tex->getSize();
            }

            tex->setSize(width, height);
            tex->update(corrected.getTopLeftFrontPixelPtr(), size, corrected.getWidth(), corrected.getHeight());
            LOG(TRACE) << "Loaded image " << path << " " << size << " " << img.getWidth() << " " << img.getHeight();
          } else {
            LOG(ERROR) << "Failed to load image " << path << ", unknown image extension";
          }
          ifs.close();
        } else {
          LOG(ERROR) << "Failed to load image " << path;
        }
      });
    }

    texture->setSize(width, height);
    mTextures[handle] = tex;

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
