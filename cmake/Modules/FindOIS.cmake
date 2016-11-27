#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find OIS
# Once done, this will define
#
#  OIS_FOUND - system has OIS
#  OIS_INCLUDE_DIRS - the OIS include directories 
#  OIS_LIBRARIES - link these to use OIS
#  OIS_BINARY_REL / OIS_BINARY_DBG - DLL names (windows only)

if (OIS_HOME)
  # OIS uses the 'includes' path for its headers in the source release, not 'include'
  set(OIS_INC_SEARCH_PATH ${OIS_INC_SEARCH_PATH} ${OIS_HOME}/includes)
endif()
if (APPLE AND OIS_HOME)
  # OIS source build on Mac stores libs in a different location
  # Also this is for static build
  set(OIS_LIB_SEARCH_PATH ${OIS_LIB_SEARCH_PATH} ${OIS_HOME}/Mac/XCode-2.2/build)
endif()

set(OIS_LIBRARY_NAMES OIS)

# construct search paths
set(OIS_PREFIX_PATH
  ${OIS_HOME} $ENV{OIS_HOME}
  ${OGRE_DEPENDENCIES_DIR} $ENV{OGRE_DEPENDENCIES_DIR}
  ${OGRE_SOURCE}/iOSDependencies $ENV{OGRE_SOURCE}/iOSDependencies
  ${OGRE_SOURCE}/Dependencies $ENV{OGRE_SOURCE}/Dependencies
  ${OGRE_SDK} $ENV{OGRE_SDK}
  ${OGRE_HOME} $ENV{OGRE_HOME})

find_path(OIS_INCLUDE_DIR
  NAMES
  OIS.h
  HINTS
  ${OIS_INC_SEARCH_PATH}
  ${OIS_PREFIX_PATH}
  PATH_SUFFIXES
  include/OIS
  OIS)

find_library(OIS_LIBRARY_REL
  NAMES
  ${OIS_LIBRARY_NAMES}
  HINTS
  ${OIS_LIB_SEARCH_PATH}
  ${OIS_PKGC_LIBRARY_DIRS}
  PATH_SUFFIXES
  ""
  Release
  RelWithDebInfo
  MinSizeRel)

find_library(
  OIS_LIBRARY_DBG
  NAMES
  ${OIS_LIBRARY_NAMES_DBG}
  HINTS
  ${OIS_LIB_SEARCH_PATH}
  ${OIS_PKGC_LIBRARY_DIRS}
  PATH_SUFFIXES
  ""
  Debug)

# For OIS, prefer static library over framework (important when referencing OIS source build)
set(CMAKE_FIND_FRAMEWORK "LAST")
set(OIS_INCLUDE_DIRS ${OIS_INCLUDE_DIR})

if (OIS_INCLUDE_DIRS AND (OIS_LIBRARY_DBG OR OIS_LIBRARY_REL))
  set(OIS_FOUND TRUE)
endif (OIS_INCLUDE_DIRS AND (OIS_LIBRARY_DBG OR OIS_LIBRARY_REL))

if (OIS_FOUND)
  if (NOT OIS_FIND_QUIETLY)
    set(OIS_LIBRARIES ${OIS_LIBRARY_DBG} ${OIS_LIBRARY_REL})
    message(STATUS "Found OIS: ${OIS_LIBRARIES}")
  endif (NOT OIS_FIND_QUIETLY)
else (OIS_FOUND)
  if (OIS_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find OIS")
  endif (OIS_FIND_REQUIRED)
endif (OIS_FOUND)

# show the GSAGE_INCLUDE_DIRS and GSAGE_LIBRARIES variables only in the advanced view
mark_as_advanced(OIS_INCLUDE_DIRS OIS_LIBRARIES)

if (WIN32)
	set(OIS_BIN_SEARCH_PATH ${OGRE_DEPENDENCIES_DIR}/bin ${CMAKE_SOURCE_DIR}/Dependencies/bin ${OIS_HOME}/dll
		$ENV{OIS_HOME}/dll $ENV{OGRE_DEPENDENCIES_DIR}/bin
		${OGRE_SOURCE}/Dependencies/bin $ENV{OGRE_SOURCE}/Dependencies/bin
    ${OGRE_SDK}/bin $ENV{OGRE_SDK}/bin
    ${OGRE_HOME}/bin $ENV{OGRE_HOME}/bin)
	find_file(OIS_BINARY_REL NAMES "OIS.dll" HINTS ${OIS_BIN_SEARCH_PATH}
	  PATH_SUFFIXES "" Release RelWithDebInfo MinSizeRel)
	find_file(OIS_BINARY_DBG NAMES "OIS_d.dll" HINTS ${OIS_BIN_SEARCH_PATH}
	  PATH_SUFFIXES "" Debug )
endif()
mark_as_advanced(OIS_BINARY_REL OIS_BINARY_DBG)

# Reset framework finding
set(CMAKE_FIND_FRAMEWORK "FIRST")
