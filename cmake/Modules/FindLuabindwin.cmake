# - Find Luabind includes and library
#
# This module defines
#  Luabind_INCLUDE_DIR
#  LUABIND_LIBRARIES, the libraries to link against to use Luabind.
#  Luabind_LIB_DIR, the location of the libraries
#  Luabind_FOUND, If false, do not try to use Luabind
#
# Copyright © 2007, Matt Williams
# Changes for Luabind detection by Garvek, 2008
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (LUABIND_LIBRARIES AND Luabind_INCLUDE_DIR)
   SET(Luabind_FIND_QUIETLY TRUE) # Already in cache, be silent
ENDIF (LUABIND_LIBRARIES AND Luabind_INCLUDE_DIR)

IF (WIN32) #Windows
   MESSAGE(STATUS "Looking for Luabind")
   SET(Luabind_SRC $ENV{LUABIND_SRC})
   SET(Luabind_SDK $ENV{LUABIND_DIR})
   IF (Luabind_SRC)
      MESSAGE(STATUS "Using Luabind source")
      STRING(REGEX REPLACE "[\\]" "/" Luabind_SRC "${Luabind_SRC}")
      SET(Luabind_INCLUDE_DIR ${Luabind_SRC})
      SET(Luabind_LIB_DIR ${Luabind_SRC}/src)
      SET(LUABIND_LIBRARIES debug Debug/luabind_d optimized Release/luabind)
   ENDIF (Luabind_SRC)
   IF (Luabind_SDK)
      MESSAGE(STATUS "Using Luabind SDK")
      STRING(REGEX REPLACE "[\\]" "/" Luabind_SDK "${Luabind_SDK}")
      SET(Luabind_INCLUDE_DIR ${Luabind_SDK})
      SET(Luabind_LIB_DIR ${Luabind_SDK})
      SET(LUABIND_LIBRARIES debug luabind_d optimized luabind)
   ENDIF (Luabind_SDK)
ELSE (WIN32) #Unix
   CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
   FIND_PACKAGE(PkgConfig REQUIRED)
   PKG_SEARCH_MODULE(Luabind Luabind)
   SET(Luabind_INCLUDE_DIR ${Luabind_INCLUDE_DIRS})
   SET(Luabind_LIB_DIR ${Luabind_LIBDIR})
   SET(LUABIND_LIBRARIES ${LUABIND_LIBRARIES} CACHE STRING "")
ENDIF (WIN32)

#Do some preparation
SEPARATE_ARGUMENTS(Luabind_INCLUDE_DIR)
SEPARATE_ARGUMENTS(LUABIND_LIBRARIES)

SET(Luabind_INCLUDE_DIR ${Luabind_INCLUDE_DIR} CACHE PATH "")
SET(LUABIND_LIBRARIES ${LUABIND_LIBRARIES} CACHE STRING "")
SET(Luabind_LIB_DIR ${Luabind_LIB_DIR} CACHE PATH "")

IF (Luabind_INCLUDE_DIR AND LUABIND_LIBRARIES)
   SET(Luabind_FOUND TRUE)
ENDIF (Luabind_INCLUDE_DIR AND LUABIND_LIBRARIES)

IF (Luabind_FOUND)
   IF (NOT Luabind_FIND_QUIETLY)
      MESSAGE(STATUS "  libraries : ${LUABIND_LIBRARIES} from ${Luabind_LIB_DIR}")
      MESSAGE(STATUS "  includes  : ${Luabind_INCLUDE_DIR}")
   ENDIF (NOT Luabind_FIND_QUIETLY)
ELSE (Luabind_FOUND)
   IF (Luabind_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Luabind")
   ENDIF (Luabind_FIND_REQUIRED)
ENDIF (Luabind_FOUND)
