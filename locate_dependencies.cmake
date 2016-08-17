if(WIN32)
  set(CMAKE_MODULE_PATH "$ENV{OGRE_HOME}/CMake/;${CMAKE_MODULE_PATH}")
  set(OGRE_SAMPLES_INCLUDEPATH
    $ENV{OGRE_HOME}/Samples/include
    )
endif(WIN32)

if(UNIX)
  if(EXISTS "/usr/local/lib/OGRE/cmake")

    set(CMAKE_MODULE_PATH "/usr/local/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
    set(OGRE_SAMPLES_INCLUDEPATH "/usr/local/share/OGRE/samples/Common/include/") # We could just *assume* that developers uses this basepath : /usr/local

  elseif(EXISTS "/usr/lib/OGRE/cmake")

    set(CMAKE_MODULE_PATH "/usr/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
    set(OGRE_SAMPLES_INCLUDEPATH "/usr/share/OGRE/samples/Common/include/") # Otherwise, this one
  elseif(DEFINED ENV{OGRE_HOME})
    set(CMAKE_MODULE_PATH "$ENV{OGRE_HOME}/CMake/;${CMAKE_MODULE_PATH}")
    set(OGRE_SAMPLES_INCLUDEPATH
      $ENV{OGRE_HOME}/Samples/include
      )
  else ()
    message(SEND_ERROR "Failed to find module path.")
  endif(EXISTS "/usr/local/lib/OGRE/cmake")
endif(UNIX)

if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

set(CMAKE_DEBUG_POSTFIX "_d")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(APPLE)
set(OGRE_STATIC true)
endif(APPLE)
find_package(OGRE REQUIRED)
ogre_find_plugin(Plugin_ParticleUniverse ParticleUniverseSystemManager.h)

find_package(OIS REQUIRED)
find_package(Luabind REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(LibRocket REQUIRED)
find_package(gtest)

find_package(Qt5 COMPONENTS Quick Core QUIET)

if(NOT OIS_FOUND)
  message(SEND_ERROR "Failed to find OIS.")
endif()

# Find Boost
if (NOT OGRE_BUILD_PLATFORM_IPHONE)
  if (WIN32 OR APPLE)
    set(Boost_USE_STATIC_LIBS TRUE)
  else ()
    # Statically linking boost to a dynamic Ogre build doesn't work on Linux 64bit
    set(Boost_USE_STATIC_LIBS ${OGRE_STATIC})
  endif ()
  if (MINGW)
    # this is probably a bug in CMake: the boost find module tries to look for
    # boost libraries with name libboost_*, but CMake already prefixes library
    # search names with "lib". This is the workaround.
    set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} "")
  endif ()
  set(Boost_ADDITIONAL_VERSIONS "1.55" "1.44.0" "1.42" "1.42.0" "1.41.0" "1.41" "1.40.0" "1.40" "1.39.0" "1.39" "1.38.0" "1.38" "1.37.0" "1.37" )
  # Components that need linking (NB does not include header-only components like bind)
  set(OGRE_BOOST_COMPONENTS system filesystem thread date_time python signals regex)
  find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)

  if (NOT Boost_FOUND)
    # Try again with the other type of libs
    set(Boost_USE_STATIC_LIBS NOT ${Boost_USE_STATIC_LIBS})
    find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)
  endif(NOT Boost_FOUND)

  # Set up referencing of Boost
  include_directories(${Boost_INCLUDE_DIR})
  add_definitions(-DBOOST_ALL_NO_LIB)
  set(OGRE_LIBRARIES ${OGRE_LIBRARIES} ${Boost_LIBRARIES})
endif()
