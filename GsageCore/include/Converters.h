#ifndef _Converters_H_
#define _Converters_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Gsage Authors

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

#define TYPE_CASTER(name, t, f) \
struct name {\
  typedef t Type;\
  typedef f FromType;\
  bool to(const FromType& src, Type& dst) const;\
  const FromType from(const Type& value) const;\
};\
template<> \
struct TranslatorBetween<f, t> { typedef name type; }; \

namespace Gsage {
  /**
   * NoopCaster does not do any cast.
   *
   * Implements behavior when no caster is found for the requested type.
   */
  template<class F, class T>
  struct NoopCaster
  {
    /**
     * Noop.
     */
    typedef T Type;
    typedef F FromType;
    bool to(const FromType& src, Type& dst)
    {
      return false;
    }
    /**
     * Noop.
     */
    const FromType from(const Type& value)
    {
      return FromType();
    }
  };

  /**
   * Wrapper for Casters.
   */
  template<typename F, typename T>
  struct TranslatorBetween
  {
    typedef NoopCaster<F, T> type;
  };

  TYPE_CASTER(DoubleCaster, double, std::string)
  TYPE_CASTER(IntCaster, int, std::string)
  TYPE_CASTER(UIntCaster, unsigned int, std::string)
  TYPE_CASTER(UlongCaster, unsigned long, std::string)
  TYPE_CASTER(FloatCaster, float, std::string)
  TYPE_CASTER(BoolCaster, bool, std::string)
  TYPE_CASTER(StringCaster, std::string, std::string)

  /**
   * Const string caster.
   */
  struct CStrCaster {
    typedef const char* Type;
    typedef std::string FromType;
    bool to(const FromType& src, Type& dst) const;
    const FromType from(const Type& value) const;
  };

  /**
   * Wraps CStrCaster.
   */
  template<size_t N>
  struct TranslatorBetween<std::string, char[N]>
  {
    typedef CStrCaster type;
  };
}

#endif
