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

#include <boost/property_tree/ptree.hpp>
namespace Gsage {
  typedef boost::property_tree::ptree DataNode;
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
