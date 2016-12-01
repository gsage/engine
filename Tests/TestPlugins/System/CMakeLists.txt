include_directories(${gsage_SOURCE_DIR}/Components/Sol2/include)
include_directories(${LUAJIT_INCLUDE_DIR})
test_plugin(System)
set(LIBS ${LIBS} ${LUAJIT_LIBRARIES})
