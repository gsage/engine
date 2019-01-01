#ifndef _AsyncObject_H_
#define _AsyncObject_H_

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

#include <string>
#include <map>
#include <memory>

namespace Gsage {
  class AsyncObject
  {
    public:
      class PropertyBase {
        public:
          PropertyBase(int name, AsyncObject* container)
            : mName(name)
            , mContainer(container)
          {
          }

          int getName() {
            return mName;
          }
        protected:
          int mName;
          AsyncObject* mContainer;
      };

      typedef std::unique_ptr<PropertyBase> PropertyBasePtr;

      template<class C>
      class Property : public PropertyBase {
        public:
          Property(int name, AsyncObject* container)
            : PropertyBase(name, container)
          {
          }
          void assign(C v)
          {
            if(mValue == v) {
              return;
            }

            mValue = v;
            mContainer->onChange(this);
          }

          C getValue() {
            return mValue;
          }
        private:
          C mValue;
      };

      AsyncObject();
      virtual ~AsyncObject();

      template<class C>
      bool getProperty(int name, C& dest) {
        if(mProperties.count(name) == 0) {
          return false;
        }

        dest = static_cast<Property<C>*>(mProperties[name].get())->getValue();
        return true;
      }

      template<class C>
      bool getProperty(int name, C& dest) const {
        return const_cast<AsyncObject*>(this)->getProperty(name, dest);
      }

      template<class C>
      void setProperty(int name, C value)
      {
        if(mProperties.count(name) == 0) {
          mProperties.emplace(std::make_pair(name, PropertyBasePtr(new Property<C>(name, this))));
        }

        static_cast<Property<C>*>(mProperties[name].get())->assign(value);
      }

      bool isSet(int name) const {
        return mProperties.count(name) > 0;
      }

      virtual void onChange(PropertyBase* property) {
      }
    private:
      typedef std::map<int, PropertyBasePtr> Properties;
      Properties mProperties;
  };
}

#endif
