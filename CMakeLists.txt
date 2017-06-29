cmake_minimum_required(VERSION 3.1)

project(gsage)

include(cmake/Functions.cmake)
configure()

include(locate_dependencies.cmake)
process_templates()

add_subdirectory(GsageCore)
add_subdirectory(GsageExe)
add_subdirectory(PlugIns)
add_subdirectory(Tests)
add_subdirectory(googletest)

if(OGRE_FOUND)
  add_subdirectory(Components)
  add_subdirectory(Editor)

  if(Qt5_FOUND)
    add_subdirectory(QEditor)
  endif(Qt5_FOUND)
endif(OGRE_FOUND)

install(DIRECTORY cmake DESTINATION /usr/local/share/gsage)
