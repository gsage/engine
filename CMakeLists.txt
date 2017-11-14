cmake_minimum_required(VERSION 3.1)

project(gsage)

include(cmake/Functions.cmake)
configure()

include(${CMAKE_SOURCE_DIR}/conanbuildinfo.cmake OPTIONAL)

if(CONAN_PACKAGE_NAME)
  set(CMAKE_INSTALL_PATH "${CMAKE_BINARY_DIR}/sdk")
  include(cmake/ConanDependencies.cmake)
else(CONAN_PACKAGE_NAME)
  set(CMAKE_INSTALL_PATH "/usr/local")
  include(cmake/Dependencies.cmake)
endif(CONAN_PACKAGE_NAME)

process_templates()

add_subdirectory(GsageCore)
add_subdirectory(GsageExe)
add_subdirectory(PlugIns)
add_subdirectory(Tests)
add_subdirectory(Vendor)
if(OGRE_FOUND)
  add_subdirectory(Editor)

  if(Qt5_FOUND)
    add_subdirectory(QEditor)
  endif(Qt5_FOUND)
endif(OGRE_FOUND)

install(DIRECTORY cmake DESTINATION "${CMAKE_INSTALL_PATH}/share/gsage")
