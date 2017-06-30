/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

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

#ifndef _ObjectPool_H_
#define _ObjectPool_H_
#include <vector>
#include <iostream>
#include <stdexcept>

namespace Gsage {
  template<typename T>
  class DefaultMemoryAllocator
  {
    public:
      static inline void *allocate(size_t size)
      {
        return ::operator new(size, ::std::nothrow);
      }
      static inline void deallocate(void *pointer, size_t size)
      {
        ::operator delete(pointer);
      }
  };

  /**
   * Class that allocates objects in continuous block of memory
   */
  template<typename T, typename TMemoryAllocator=DefaultMemoryAllocator<T>>
  class ObjectPool
  {
    public:
      explicit ObjectPool(size_t initialCapacity=32, size_t maxBlockLength=1000000):
        mFirstDeleted(NULL),
        mCountInNode(0),
        mNodeCapacity(initialCapacity),
        mFirstNode(initialCapacity),
        mMaxBlockLength(maxBlockLength)
      {
        if (mMaxBlockLength < 1)
          throw std::invalid_argument("mMaxBlockLength must be at least 1.");

        mNodeMemory = mFirstNode.memory;
        mLastNode = &mFirstNode;
      }

      ~ObjectPool()
      {
        Node *node = mFirstNode.nextNode;
        while(node)
        {
          Node *nextNode = node->nextNode;
          delete node;
          node = nextNode;
        }
      }

      /**
       * Create element in pool. Note that element should have default constructor to do it.
       */
      T* create()
      {
        T* address = getAddress();
        return new (address) T();
      }

      template<class ... Types>
      T* create(Types ... args)
      {
        T* address = getAddress();
        return new (address) T(args...);
      }

      /**
       * This method is useful if you want to call a non-default constructor.
       * It should be used like this:
       *  new (pool.getAddress()) ObjectType(... parameters ...);
       */
      T *getAddress()
      {
        T *result;
        if (mFirstDeleted)
        {
          result = (T *)mFirstDeleted;
          mFirstDeleted = *((T **)mFirstDeleted);
        }else{
          if (mCountInNode >= mNodeCapacity)
            allocateNewNode();
          char *address = (char *)mNodeMemory;
          address += mCountInNode * itemSize;
          mCountInNode++;
          result = (T *)address;
        }

        mElements.push_back(result);
        return result;
      }

      /**
       * Remove element by pointer and call it's destructor
       * @param content Element pointer
       */
      void erase(T *content)
      {
        content->~T();
        remove(content);
      }

      /**
       * Remove element by pointer
       * @param content Element pointer
       */
      void remove(T *content)
      {
        *((T **)content) = mFirstDeleted;
        mFirstDeleted = content;
        for(unsigned int i = 0; i < mElements.size(); i++)
        {
          if(mElements[i] == content)
          {
            mElements.erase(mElements.begin() + i);
            break;
          }
        }
      }

      typedef std::vector<T*> PointerVector;

      /**
       * Get list of all elements as a vector
       */
      PointerVector& getElements()
      {
        return mElements;
      }

      /**
       * Resets data pointer. Clears pointer vector
       */
      void clear()
      {
        mFirstDeleted = NULL;
        mCountInNode = 0;
        mElements.clear();
      }

      int size() {
        return mElements.size();
      }
    private:
      struct Node
      {
        void *memory;
        size_t capacity;
        Node *nextNode;

        Node(size_t c)
        {
          if (c < 1)
            throw std::invalid_argument("capacity must be at least 1.");

          memory = TMemoryAllocator::allocate(itemSize * c);
          if (memory == NULL)
            throw std::bad_alloc();

          capacity = c;
          nextNode = NULL;
        }
        ~Node()
        {
          TMemoryAllocator::deallocate(memory, itemSize * capacity);
        }
      };


      ObjectPool(const ObjectPool<T, TMemoryAllocator> &source);
      void operator = (const ObjectPool<T, TMemoryAllocator> &source);

      void allocateNewNode()
      {
        size_t size = mCountInNode;
        if (size >= mMaxBlockLength)
          size = mMaxBlockLength;
        else
        {
          size *= 2;

          if (size < mCountInNode)
            throw std::overflow_error("size became too big.");

          if (size >= mMaxBlockLength)
            size = mMaxBlockLength;
        }

        Node *newNode = new Node(size);
        mLastNode->nextNode = newNode;
        mLastNode = newNode;
        mNodeMemory = newNode->memory;
        mCountInNode = 0;
        mNodeCapacity = size;
      }

      void *mNodeMemory;
      T *mFirstDeleted;
      size_t mCountInNode;
      size_t mNodeCapacity;
      Node mFirstNode;
      Node *mLastNode;
      size_t mMaxBlockLength;
      PointerVector mElements;

      static const size_t itemSize;

  };

  template<typename T, class TMemoryAllocator>
  const size_t ObjectPool<T,TMemoryAllocator>::itemSize = ((sizeof(T) + sizeof(void *)-1) / sizeof(void *)) * sizeof(void *);
}

#endif
