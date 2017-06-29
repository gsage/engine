set(APP_NAME "GsageEditor")

include_directories(include)
include_directories(${gsage_SOURCE_DIR}/GsageCore/include)
include_directories(${gsage_SOURCE_DIR}/Components/OgreCrowd/include)
include_directories(${gsage_SOURCE_DIR}/Components/Sol2/include)

include_directories(
  ${LUAJIT_INCLUDE_DIR}
  ${JSONCPP_INCLUDE_DIR}
  ${MSGPACK_INCLUDE_DIR}
  ${PYBIND11_INCLUDE_DIRS})

FILE(GLOB sources src/*.cpp)

set(LIBS
  GsageCore
  ${LUAJIT_LIBRARIES}
)

#set(PYTHON_MODULE_PREFIX ${PYTHON_MODULE_PREFIX} CACHE INTERNAL "")
#set(PYTHON_MODULE_EXTENSION ${PYTHON_MODULE_EXTENSION} CACHE INTERNAL "so")
gsage_executable(${APP_NAME} ${sources})

if(APPLE)
  set(LIBS ${LIBS}
    ${COREFOUNDATION_LIBRARY}
    ${CARBON}
    ${IOKIT}
    ${COCOA_LIBRARY}
  )
endif(APPLE)

target_link_libraries(${APP_NAME} ${LIBS})

#add_library(${LIB_NAME} MODULE ${sources})
#target_link_libraries(${LIB_NAME} PRIVATE ${LIBS})
#target_link_libraries(editorv2 PRIVATE ${PYTHON_LIBRARIES})
#set_target_properties(${LIB_NAME} PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}")

#if(UNIX)
#  target_link_libraries(${LIB_NAME} PRIVATE "-undefined dynamic_lookup")
#endif(UNIX)
