set(LIB_NAME "OisInputPlugin")

include_directories(
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${OIS_INCLUDE_DIRS}
)

set(LIBS
  GsageCore
  easyloggingpp
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
