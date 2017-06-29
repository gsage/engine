set(LIB_NAME "OisInput")

include_directories(
  ${OIS_INCLUDE_DIRS}
)

set(LIBS
  GsageCore
  ${OIS_LIBRARIES})

file(GLOB sources src/*.cpp)

if(APPLE)
  set(LIBS ${LIBS}
    ${IOKIT}
    ${COREFOUNDATION_LIBRARY}
    ${COCOA_LIBRARY})
endif(APPLE)

gsage_plugin(${LIB_NAME} SHARED ${sources})
target_link_libraries(${LIB_NAME} ${LIBS})
