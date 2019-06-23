#ifndef _MeshTools_H_
#define _MeshTools_H_

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

#include <Ogre.h>
#include <OgreSingleton.h>
#include "Definitions.h"
#include "Logger.h"

namespace Ogre {

  /**
   * A single mesh information
   */
  class MeshInformation
  {
    public:
      enum Flags {
        Vertices  = 0x01,
        Indices   = 1 << 1,
        Normals   = 1 << 2,

        Default   = Vertices | Indices,
        All       = Vertices | Indices | Normals
      };
      MeshInformation();
      ~MeshInformation();

      /**
       * Allocates MeshInformation data arrays
       */
      void allocate(size_t vertexCount, size_t indexCount, MeshInformation::Flags flags);

      Ogre::Vector3* vertices;
      Ogre::Vector3* normals;
      Ogre::uint32* indices;

      size_t vertexCount;
      size_t indexCount;
  };
  /**
   * Provides cached access to raw mesh data
   */
  class MeshTools : public Ogre::Singleton<MeshTools>
  {
    public:
      MeshTools();
      virtual ~MeshTools();

      static MeshTools& getSingleton(void);
      static MeshTools* getSingletonPtr(void);

      /**
       * Gets movable object mesh information, if eligible
       * @return false if not eligible to get this information
       */
      bool getMeshInformation(Ogre::MovableObject* object,
        MeshInformation& dest,
        Ogre::Matrix4 transform,
        MeshInformation::Flags flags = MeshInformation::Default
      );

      /**
       * Gets movable object mesh information, if eligible
       * @return false if not eligible to get this information
       */
      bool getMeshInformation(Ogre::MovableObject* object,
        MeshInformation& dest,
        const Ogre::Vector3 &position,
        const Ogre::Quaternion &orient,
        const Ogre::Vector3 &scale,
        MeshInformation::Flags flags = MeshInformation::Default
      );

      /**
       * Gets v1 mesh information
       */
      void getMeshInformation(OgreV1::MeshPtr mesh,
        MeshInformation& dest,
        const Ogre::Matrix4& transform,
        MeshInformation::Flags flags = MeshInformation::Default
      );
#if OGRE_VERSION >= 0x020100
      /**
       * Gets v2 mesh information
       */
      void getMeshInformation(Ogre::MeshPtr mesh,
        MeshInformation& dest,
        const Ogre::Matrix4& transform,
        MeshInformation::Flags flags = MeshInformation::Default
      );
#endif
  };
}

#endif
