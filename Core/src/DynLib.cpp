#include "DynLib.h"

/*
   -----------------------------------------------------------------------------
   This source file is part of OGRE
   (Object-oriented Graphics Rendering Engine)
   For the latest info, see http://www.ogre3d.org/

   Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "DynLib.h"
#include "Logger.h"

namespace Gsage {

  //-----------------------------------------------------------------------
  DynLib::DynLib( const std::string& name )
  {
    mName = name;
    mInst = NULL;
  }

  //-----------------------------------------------------------------------
  DynLib::~DynLib()
  {
  }

  //-----------------------------------------------------------------------
  bool DynLib::load()
  {
    // Log library load
    LOG(INFO) << ("Loading library " + mName);

    std::string name = mName;
#if GSAGE_PLATFORM == GSAGE_LINUX
    // dlopen() does not add .so to the filename, like windows does for .dll
    if (name.find(".so") == std::string::npos)
    {
      name += ".so";
    }
#elif GSAGE_PLATFORM == GSAGE_APPLE
    // dlopen() does not add .dylib to the filename, like windows does for .dll
    if (name.substr(name.length() - 6, 6) != ".dylib")
      name += ".dylib";
#elif GSAGE_PLATFORM == GSAGE_WIN32
    // Although LoadLibraryEx will add .dll itself when you only specify the library name,
    // if you include a relative path then it does not. So, add it to be sure.
    if (name.substr(name.length() - 4, 4) != ".dll")
      name += ".dll";
#endif
    mInst = (DYNLIB_HANDLE)DYNLIB_LOAD( name.c_str() );
    if( !mInst )
    {
      LOG(WARNING) <<
          "Could not load dynamic library " << mName <<
          ".  System Error: " << dynlibError() <<
          " DynLib::load";
    }
#if GSAGE_PLATFORM == GSAGE_APPLE
    if(!mInst)
    {
      // Try again as a framework
      mInst = (DYNLIB_HANDLE)FRAMEWORK_LOAD( mName );
    }
#endif
    if( !mInst )
    {
      LOG(ERROR) <<
          "Could not load dynamic library " << mName <<
          ".  System Error: " << dynlibError() <<
          "DynLib::load";
      return false;
    }
    return true;
  }

  //-----------------------------------------------------------------------
  bool DynLib::unload()
  {
    // Log library unload
    LOG(INFO) << ("Unloading library " + mName);

    if( DYNLIB_UNLOAD( mInst ) )
    {
      LOG(ERROR) << "Could not unload dynamic library " << mName << ".  System Error: " << dynlibError() << " DynLib::unload";
      return false;
    }
    return true;

  }

  //-----------------------------------------------------------------------
  void* DynLib::getSymbol( const std::string& strName ) const throw()
  {
    return (void*)DYNLIB_GETSYM( mInst, strName.c_str() );
  }
  //-----------------------------------------------------------------------
  std::string DynLib::dynlibError( void )
  {
#if GSAGE_PLATFORM == GSAGE_WIN32
    LPTSTR lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
        );
    std::string ret = lpMsgBuf;
    // Free the buffer.
    LocalFree( lpMsgBuf );
    return ret;
#elif GSAGE_PLATFORM == GSAGE_LINUX || GSAGE_PLATFORM == GSAGE_APPLE
    return std::string(dlerror());
#else
    return std::string("");
#endif
  }

}
