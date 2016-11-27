cmake_minimum_required(VERSION 3.1)

project(gsage)

if(APPLE)
	# This has to be before most other options so CMake properly handles the
	# compiler variables, it MUST bebefore the project() definition
  if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
    set ( CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 -image_base 7fff04c4a000" )
    option ( LUA_USE_POSIX "Use POSIX functionality." ON )
    option ( LUA_USE_DLOPEN "Use dynamic linker to load modules." ON )
    set ( LJVM_MODE machasm )
    message("-- Using fix for luajit")
  endif()

  option(BUILD_UNIVERSAL_BINARIES "Build universal binaries for all architectures supported" ON)
	if (NOT CMAKE_OSX_ARCHITECTURES AND BUILD_UNIVERSAL_BINARIES)
		if(IOS)
			# set the architecture for iOS
			if (${IOS_PLATFORM} STREQUAL "OS")
				set (IOS_ARCH armv6 armv7 armv7s arm64)
				set (CMAKE_OSX_ARCHITECTURES ${IOS_ARCH} CACHE string  "Build architecture for iOS")
			else (${IOS_PLATFORM} STREQUAL "OS")
				set (IOS_ARCH i386 x86_64)
				set (CMAKE_OSX_ARCHITECTURES ${IOS_ARCH} CACHE string  "Build architecture for iOS Simulator")
			endif (${IOS_PLATFORM} STREQUAL "OS")

		else(IOS)
			# set the architectures for OS X
			set (OSXI_ARCH i386 x86_64)
			set (CMAKE_OSX_ARCHITECTURES ${OSXI_ARCH} CACHE string  "Build architecture for OS X universal binaries")
		endif(IOS)
	endif (NOT CMAKE_OSX_ARCHITECTURES AND BUILD_UNIVERSAL_BINARIES)
endif(APPLE)

if(WIN32)
  add_definitions( -DWIN32 )
endif(WIN32)

if(UNIX)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_FLAGS "-fPIC ${CMAKE_CXX_FLAGS}")
endif(UNIX)

include(locate_dependencies.cmake)

add_subdirectory(GsageCore)
add_subdirectory(PlugIns)
add_subdirectory(Tests)
add_subdirectory(googletest)

if(OGRE_FOUND)
  add_subdirectory(Components)
  add_subdirectory(RocketWrapper)
  add_subdirectory(Editor)

  if(Qt5_FOUND)
    add_subdirectory(QEditor)
  endif(Qt5_FOUND)
endif(OGRE_FOUND)


