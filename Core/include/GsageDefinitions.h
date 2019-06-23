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

#ifndef __GsageDefinitions_H__
#define __GsageDefinitions_H__

#define ENTITY_POOL_SIZE 1024
#define COMPONENT_POOL_SIZE 1024
#define NOMINMAX

#include <sstream>
#include <istream>
#include <iterator>
#include <ostream>
#include <vector>
#include <channel>
#include <map>

static inline std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    str.erase(0, str.find_first_not_of(chars));
    return str;
}

static inline std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}

static inline std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    return ltrim(rtrim(str, chars), chars);
}

static inline std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


static inline std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

static inline std::string join(const std::vector<std::string>& vector, const char delim)
{
  std::stringstream s;
  for(unsigned int i = 0; i < vector.size(); i++)
  {
    s << vector[i];
    if(i < vector.size() - 1)
    {
      s << delim;
    }
  }
  return s.str();
}

/**
 * Short definition of std::map key search
 */
template<class K, class T>
static bool contains(const std::map<K, T>& m, K value)
{
  return m.find(value) != m.end();
}

/**
 * Short definition of std::map key search
 */
template<class K, class T, class CompStr>
static bool contains(const std::map<K, T, CompStr>& m, K value)
{
  return m.find(value) != m.end();
}

/**
 * Short definition of std::vector key search
 */
template<class T>
static bool contains(const std::vector<T>& v, T value)
{
  return std::find(v.start(), v.end(), value) != v.end();
}

namespace Gsage {
  enum ChannelSignal {
    NONE = 0,
    DONE = 1,
    SHUTDOWN = 2,
    FAILURE = 3
  };

  typedef cpp::channel<ChannelSignal, 32> SignalChannel;
}

#define GSAGE_UNSUPPORTED 0
#define GSAGE_WIN32 1
#define GSAGE_LINUX 2
#define GSAGE_APPLE 3
#define GSAGE_IOS 4
#define GSAGE_ANDROID 5
#define GSAGE_UNIX 6
#define GSAGE_POSIX 7

#ifdef _WIN32
  #define GSAGE_PLATFORM GSAGE_WIN32
#elif __APPLE__
  #include "TargetConditionals.h"
  #if TARGET_IPHONE_SIMULATOR
    #define GSAGE_PLATFORM GSAGE_IOS
  #elif TARGET_OS_IPHONE
    #define GSAGE_PLATFORM GSAGE_IOS
  #elif TARGET_OS_MAC
    #define GSAGE_PLATFORM GSAGE_APPLE
  #else
    #define GSAGE_PLATFORM GSAGE_UNSUPPORTED
  #endif
#elif __ANDROID__
  #define GSAGE_PLATFORM GSAGE_ANDROID
#elif __linux
  #define GSAGE_PLATFORM GSAGE_LINUX
#elif __unix // all unices not caught above
  #define GSAGE_PLATFORM GSAGE_UNIX
#elif __posix
  #define GSAGE_PLATFORM GSAGE_POSIX
#endif

#endif

#if GSAGE_PLATFORM == GSAGE_WIN32
#define GSAGE_PATH_SEPARATOR '\\'
#define ELPP_AS_DLL
#ifdef GSAGE_DLL_EXPORT
#define GSAGE_API __declspec(dllexport)
#else
#define GSAGE_API __declspec(dllimport)
#endif
#else
#define GSAGE_PATH_SEPARATOR '/'
#define GSAGE_API
#endif
