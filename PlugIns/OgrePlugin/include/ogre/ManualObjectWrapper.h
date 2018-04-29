#ifndef _ManualObjectWrapper_H_
#define _ManualObjectWrapper_H_

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

#include "ogre/MovableObjectWrapper.h"
#include "DataProxy.h"
#include <OgreManualObject.h>

namespace Gsage {

  class ManualObjectWrapper : public MovableObjectWrapper<Ogre::ManualObject>
  {
    public:
      static const std::string TYPE;
      ManualObjectWrapper();
      virtual ~ManualObjectWrapper();

      /**
       * Set manual object data
       *
       * @param data to create manual object from
       */
      void setData(const DataProxy& data);

      /**
       * Get manual object data
       *
       * @return data
       */
      const DataProxy& getData() const;

    private:
      DataProxy mData;
  };
}


#endif
