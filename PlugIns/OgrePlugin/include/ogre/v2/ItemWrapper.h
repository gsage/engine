/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev

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

#ifndef _ItemWrapper_H_
#define _ItemWrapper_H_

#include "Definitions.h"
#include "ogre/MovableObjectWrapper.h"
#include "DataProxy.h"
#include <OgreSkeleton.h>
#include <OgreItem.h>

namespace Ogre
{
  class Item;
}

namespace Gsage {

  class ItemWrapper : public MovableObjectWrapper<Ogre::Item>
  {
    public:
      static const std::string TYPE;

      enum Query { STATIC = 0x01, DYNAMIC = 0x02, UNKNOWN = 0x04 };

      ItemWrapper();
      virtual ~ItemWrapper();

      /**
       * Set model mesh (this function creates entity)
       * @param mesh Mesh file name
       */
      void setMesh(const std::string& mesh);

      /**
       * Get mesh file name
       */
      const std::string& getMesh() const;

      /**
       * Set model entity flags, used for raycasting and querying entities
       * @param type static means that entity is a part of location, dynamic means that entity is some actor
       */
      void setQueryFlags(const std::string& type);

      /**
       * Get query flags of the model
       */
      const std::string& getQueryFlags() const;

      /**
       * Get underlying entity
       */
      Ogre::Item* getItem() {
        return mObject;
      }

      /**
       * Set item datablock
       * @param name datablock name (Hlms)
       */
      void setDatablock(const std::string& name);

      /**
       * Get item datablock name
       */
      const std::string& getDatablock() const;
    private:
      std::string mMeshName;
      std::string mDatablock;
      std::string mQueryString;

      Query mQuery;
  };
}
#endif
