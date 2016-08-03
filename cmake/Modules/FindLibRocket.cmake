# - Try to find libRocket
# Once done this will define
#  LIBROCKET_FOUND - System has libRocket
#  LIBROCKET_INCLUDE_DIRS - The LibRocket include directories
#  LIBROCKET_LIBRARIES - The libraries needed to use LibRocket
#  LIBROCKET_DEFINITIONS - Compiler switches required for using LibRocket

if(DEFINED ENV{LIBROCKET_DIR})
  set(LIBROCKET_DIR $ENV{LIBROCKET_DIR})
endif(DEFINED ENV{LIBROCKET_DIR})

find_path(LIBROCKET_INCLUDE_DIR NAMES 
    Rocket/Core.h
    PATH_SUFFIXES include/
    PATHS ${LIBROCKET_DIR})

find_library(LIBROCKET_CORE NAMES RocketCore RocketCore_d HINTS ${LIBROCKET_DIR} PATH_SUFFIXES lib)
find_library(LIBROCKET_CONTROLS NAMES RocketControls RocketControls_d HINTS ${LIBROCKET_DIR} PATH_SUFFIXES lib)
find_library(LIBROCKET_DEBUGGER NAMES RocketDebugger RocketDebugger_d HINTS ${LIBROCKET_DIR} PATH_SUFFIXES lib)
find_library(LIBROCKET_CONTROLS_LUA NAMES RocketControlsLua RocketControlsLua_d HINTS ${LIBROCKET_DIR} PATH_SUFFIXES lib)
find_library(LIBROCKET_LUA NAMES RocketCoreLua RocketCoreLua_d HINTS ${LIBROCKET_DIR} PATH_SUFFIXES lib)

set(LIBROCKET_LIBRARIES ${LIBROCKET_CORE} ${LIBROCKET_CONTROLS} ${LIBROCKET_DEBUGGER} ${LIBROCKET_CONTROLS_LUA} ${LIBROCKET_LUA})
set(LIBROCKET_INCLUDE_DIRS ${LIBROCKET_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
#TODO: investigate why this does not work

# handle the QUIETLY and REQUIRED arguments and set LIBROCKET_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibRocket 
  DEFAULT_MSG
  LIBROCKET_LIBRARIES
  LIBROCKET_INCLUDE_DIR)

mark_as_advanced(LIBROCKET_INCLUDE_DIR LIBROCKET_LIBRARY )
