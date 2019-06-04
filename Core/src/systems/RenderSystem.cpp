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

#include "systems/RenderSystem.h"
#include "Logger.h"

namespace Gsage {
  Geom::Geom()
    : tris(nullptr)
    , verts(nullptr)
    , normals(nullptr)
    , ntris(0)
    , nverts(0)
    , bmin(nullptr)
    , bmax(nullptr)
  {
  }

  Geom::~Geom()
  {
    if(tris != nullptr) {
      delete tris;
    }

    if(verts != nullptr) {
      delete verts;
    }

    if(normals != nullptr) {
      delete normals;
    }

    if(bmin != nullptr) {
      delete bmin;
    }

    if(bmax != nullptr) {
      delete bmax;
    }
  }

  Geom::Verts Geom::getVerts()
  {
    return std::make_tuple(verts, nverts);
  }

  Geom::Tris Geom::getTris()
  {
    return std::make_tuple(tris, ntris);
  }

  Geom::Normals Geom::getNormals()
  {
    return std::make_tuple(normals, nverts);
  }

  float* Geom::getMeshBoundsMin(void)
  {
    return bmin;
  }

  float* Geom::getMeshBoundsMax(void)
  {
    return bmax;
  }

  const Event::Type Texture::RESIZE = "Texture::RESIZE";

  const Event::Type Texture::RECREATE = "Texture::RECREATE";

  const Event::Type Texture::UV_UPDATE = "Texture::UV_UPDATE";

  const Event::Type Texture::DESTROY = "Texture::DESTROY";

  Texture::Texture(const std::string& name, const DataProxy& params)
    : mValid(false)
    , mWidth(0)
    , mHeight(0)
    , mBuffer(0)
    , mSize(0)
    , mBufferWidth(0)
    , mBufferHeight(0)
    , mHandle(name)
    , mUVTL(0.0, 0.0)
    , mUVBL(0.0, 1.0)
    , mUVTR(1.0, 0.0)
    , mUVBR(1.0, 1.0)
  {
    params.dump(mParams, DataProxy::ForceCopy);
    params.read("width", mWidth);
    params.read("height", mHeight);
  }

  void Texture::setSize(int width, int height)
  {
    fireEvent(Event(Texture::RESIZE));
  }

  Texture::~Texture()
  {
  }

  Texture::UVs Texture::getUVs() const
  {
    return std::make_tuple(mUVTL, mUVBL, mUVTR, mUVBR);
  }

  void Texture::setUVs(const Gsage::Vector2& tl, const Gsage::Vector2& bl, const Gsage::Vector2& tr, const Gsage::Vector2& br)
  {
    mUVTL = tl;
    mUVBL = bl;
    mUVTR = tr;
    mUVBR = br;

    fireEvent(Event(Texture::UV_UPDATE));
  }

  void Texture::updateConfig(const DataProxy& config)
  {
    mLock.lock();
    mParams = merge(mParams, config);
    mLock.unlock();
  }

  RenderSystem::RenderSystem()
  {
  }

  RenderSystem::~RenderSystem()
  {
  }

}
