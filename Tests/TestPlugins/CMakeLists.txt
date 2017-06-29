macro(test_plugin plugin_name)

set(LIBS
  GsageCore)

file(GLOB sources src/*.cpp)

if(APPLE)
  set(LIBS ${LIBS}
    ${IOKIT}
    ${COREFOUNDATION_LIBRARY}
    ${COCOA_LIBRARY})
endif(APPLE)

gsage_plugin(${plugin_name} SHARED ${sources})
target_link_libraries(${plugin_name} ${LIBS})
endmacro()

add_subdirectory(Empty)
add_subdirectory(Input)
add_subdirectory(System)
add_subdirectory(UI)
