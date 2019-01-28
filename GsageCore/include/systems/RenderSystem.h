#ifndef _RenderSystem_H_
#define _RenderSystem_H_

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

#include <tuple>
#include <vector>
#include <memory>
#include "Logger.h"
#include "GeometryPrimitives.h"
#include "DataProxy.h"
#include "EventDispatcher.h"

namespace Gsage {
  class RenderSystem;
  class Geom
  {
    public:
      typedef std::tuple<int*, int> Tris;
      typedef std::tuple<float*, int> Verts;
      typedef std::tuple<float*, int> Normals;

      Geom();
      virtual ~Geom();

    /**
      * Retrieves the vertices stored within this Geom. The verts are an array of floats in which each
      * subsequent three floats are in order the x, y and z coordinates of a vert. The size of this array is
      * always a multiple of three and is exactly 3*getVertCount().
      **/
      Verts getVerts();

      /**
       * Retrieves the tris stored in this Geom.
       * A tri is defined by a sequence of three indexes which refer to an index position in the getVerts() array.
       * Similar to getVerts, the size of this array is a multitude of 3 and is exactly 3*getTriCount().
       **/
      Tris getTris();

      /**
       * Retrieve the normals calculated for this inputGeom. Note that the normals are not exact and are not meant for rendering,
       * but they are good enough for navmesh calculation. Each normal corresponds to one vertex from getVerts() with the same index.
       * The size of the normals array is 3*getVertCount().
       **/
      Normals getNormals();

      /**
       * The axis aligned bounding box minimum of this input Geom.
       **/
      float* getMeshBoundsMin(void);

      /**
       * The axis aligned bounding box maximum of this input Geom.
       **/
      float* getMeshBoundsMax(void);

      /**
       * Check if geom is actually empty
       */
      inline bool empty() { return ntris == 0 && nverts == 0; }

      int* tris;
      int ntris;
      float* verts;
      int nverts;
      float* normals;
      int nnormals;
      float* bmax;
      float* bmin;
  };

  typedef std::unique_ptr<Geom> GeomPtr;

  /**
   * Abstract representation of rendered texture
   */
  class GSAGE_API Texture : public EventDispatcher
  {
    public:
      static const Event::Type RESIZE;

      Texture();
      virtual ~Texture();

      /**
       * Update texture data
       *
       * @param buffer buffer to use
       * @param size provided buffer size
       * @param width buffer width
       * @param height buffer height
       */
      virtual void update(const void* buffer, size_t size, int width, int height) = 0;

      /**
       * Set texture size
       *
       * @param width texture width
       * @param height texture height
       */
      virtual void setSize(int width, int height);

      /**
       * Get texture size
       */
      inline std::tuple<int, int> getSize() const { return std::make_tuple(mWidth, mHeight); };

      /**
       * Check if the texture is valid
       *
       * @returns false if render system is not ready for example
       */
      inline bool isValid() const { return mValid; }

      virtual bool hasData() const = 0;
    protected:
      bool mValid;
      int mWidth;
      int mHeight;

      int mBufferWidth;
      int mBufferHeight;

      char* mBuffer;
      size_t mSize;
  };

  typedef std::shared_ptr<Texture> TexturePtr;

  class RenderSystem
  {
    public:
      typedef std::string TextureHandle;
      RenderSystem();
      virtual ~RenderSystem();

      /**
       * Get implementation independent Geometry information
       *
       * @param bounds get entities in bounds
       * @param flags filter entities by flags (default is all)
       *
       * @returns GeomPtr
       */
      virtual GeomPtr getGeometry(const BoundingBox& bounds, int flags = 0xFF) = 0;

      /**
       * Get implementation independent Geometry information
       *
       * @param entities filter entities by names
       * @returns GeomPtr
       */
      virtual GeomPtr getGeometry(std::vector<std::string> entities) = 0;

      /**
       * Create texture manually
       *
       * @param handle texture id
       * @param parameters variable parameters that can be used for texture creation
       */
      virtual TexturePtr createTexture(RenderSystem::TextureHandle handle, const DataProxy& parameters) = 0;

      /**
       * Get texture by name
       *
       * @param handle texture id
       */
      virtual TexturePtr getTexture(RenderSystem::TextureHandle handle) = 0;

      /**
       * Delete texture by handle
       *
       * @param handle texture id
       * @returns true if succeed
       */
      virtual bool deleteTexture(RenderSystem::TextureHandle handle) = 0;
  };
}

#endif
