set(LIB_NAME "CEFPlugin")

include_directories(
  ${gsage_SOURCE_DIR}/Vendor/easylogging/include
  ${gsage_SOURCE_DIR}/Vendor/cpp-channel/include
  ${gsage_SOURCE_DIR}/Vendor/gmath/src
  ${CONAN_INCLUDE_DIRS_CEF}
)

set(LIBS
  GsageCore
  easyloggingpp
  cpp-channel
  ${CONAN_LIBS_CEF}
)

file(GLOB sources src/CEFPlugin.cpp)

if(APPLE)
  set(LIBS
    "-framework \"Chromium Embedded Framework\""
    ${LIBS}
  )
endif(APPLE)


gsage_plugin(${LIB_NAME} SHARED ${sources})
target_link_libraries(${LIB_NAME} ${LIBS})

if(APPLE)
  cef_helper_app(cef.helper src/helper.cpp)
  target_link_libraries(cef.helper ${CONAN_LIBS_CEF} "-framework \"Chromium Embedded Framework\"")
else(APPLE)
  cef_helper_app(cef.helper src/helper.cpp)
  target_link_libraries(cef.helper ${CONAN_LIBS_CEF})
endif(APPLE)
