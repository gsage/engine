#ifndef _DynLib_H_
#define _DynLib_H_

#include "GsageDefinitions.h"

#if GSAGE_PLATFORM == GSAGE_WIN32
#  define WIN32_LEAN_AND_MEAN
#  if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // required to stop windows.h messing up std::min
#  endif
#  include <windows.h>
#    define DYNLIB_HANDLE hInstance
#    define DYNLIB_LOAD( a ) LoadLibraryEx( a, NULL, 0 ) // we can not use LOAD_WITH_ALTERED_SEARCH_PATH with relative paths
#    define DYNLIB_GETSYM( a, b ) GetProcAddress( a, b )
#    define DYNLIB_UNLOAD( a ) !FreeLibrary( a )

struct HINSTANCE__;
typedef struct HINSTANCE__* hInstance;

#elif GSAGE_PLATFORM == GSAGE_LINUX || GSAGE_PLATFORM == GSAGE_ANDROID
#   include <dlfcn.h>
#    define DYNLIB_HANDLE void*
#    define DYNLIB_LOAD( a ) dlopen( a, RTLD_LAZY | RTLD_GLOBAL)
#    define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#    define DYNLIB_UNLOAD( a ) dlclose( a )

#elif GSAGE_PLATFORM == GSAGE_APPLE || GSAGE_PLATFORM == GSAGE_IOS
extern "C" {
#   include <dlfcn.h>
#   include "macUtils.h"
}
#    define DYNLIB_HANDLE void*
#    define DYNLIB_LOAD( a ) mac_loadDylib( a )
#    define FRAMEWORK_LOAD( a ) mac_loadFramework( a )
#    define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#    define DYNLIB_UNLOAD( a ) dlclose( a )

#endif

namespace Gsage
{
  /** Resource holding data about a dynamic library.
    @remarks
    This class holds the data required to get symbols from
    libraries loaded at run-time (i.e. from DLL's for so's)
    @author
    Adrian Cearn„u (cearny@cearny.ro)
    @since
    27 January 2002
    */
  class DynLib
  {
    public:
      /** Default constructor - used by DynLibManager.
        @warning
        Do not call directly
        */
      DynLib( const std::string& name );

      /** Default destructor.
      */
      ~DynLib();

      /** Load the library
      */
      bool load();
      /** Unload the library
      */
      bool unload();
      /// Get the name of the library
      const std::string& getName(void) const { return mName; }

      /**
        Returns the address of the given symbol from the loaded library.
        @param
        strName The name of the symbol to search for
        @return
        If the function succeeds, the returned value is a handle to
        the symbol.
        @par
        If the function fails, the returned value is <b>NULL</b>.
      */
      void* getSymbol( const std::string& strName ) const throw();

    protected:
      /// Handle to the loaded library.
      DYNLIB_HANDLE mInst;

      std::string mName;
      /// Gets the last loading error
      std::string dynlibError(void);
  };

}

#endif
