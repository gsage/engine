cmake_minimum_required(VERSION 2.8)

if(APPLE)
	# This has to be before most other options so CMake properly handles the 
	# compiler variables, it MUST bebefore the project() definition

  #option(BUILD_UNIVERSAL_BINARIES "Build universal binaries for all architectures supported" ON)
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

project(cosmos)

if(WIN32)
  add_definitions( -DWIN32 )
endif(WIN32)

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++14 -fPIC ${CMAKE_CXX_FLAGS}")
endif(UNIX)

include(locate_dependencies.cmake)

add_subdirectory(Components)
add_subdirectory(Core)
add_subdirectory(GsageFacade)
add_subdirectory(RocketWrapper)
add_subdirectory(PlugIns)

if(Qt5_FOUND)
  add_subdirectory(Editor)
endif(Qt5_FOUND)

if(gtest_FOUND)
  add_subdirectory(Tests)
endif(gtest_FOUND)
