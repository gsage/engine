cmake_minimum_required(VERSION 3.1)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS true)

project(gsage)

include(cmake/Functions.cmake)
configure()

include(${CMAKE_SOURCE_DIR}/conanbuildinfo.cmake OPTIONAL)

set(CMAKE_INSTALL_PATH "./")
include(cmake/ConanDependencies.cmake)

process_templates()

add_subdirectory(Core)
add_subdirectory(PlugIns)
add_subdirectory(Tests)
add_subdirectory(Vendor)
if(OGRE_FOUND)
  add_subdirectory(Editor)
endif(OGRE_FOUND)

set(CPACK_PACKAGE_NAME "GSAGE Engine")
set(CPACK_PACKAGE_VERSION_MAJOR ${GSAGE_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${GSAGE_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${GSAGE_VERSION_PATCH})

set(CPACK_COMPONENT_resources_REQUIRED true)

set(CPACK_CREATE_DESKTOP_LINKS gsage)
set(CPACK_PACKAGE_EXECUTABLES gsage "GSAGE Editor")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

install_resources()

if(WIN32)
  # pack
  set(A_PACK_DESCRIPTION_SUMMARY "GSAGE engine")
  set(CPACK_COMPONENTS_ALL Unspecified gsage game headers packager resources)

  set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/resources/editor/gsage.ico")
  set(CPACK_WIX_PRODUCT_GUID "1A33E980-D505-41A4-B4E1-B70D96E2DC4E")
  set(CPACK_WIX_UPGRADE_GUID "1A33E980-D505-41A4-B4E1-B70D96E2DC4F")
  set(CPACK_WIX_LICENSE_RTF "${CMAKE_SOURCE_DIR}/resources/editor/license.rtf")

  set(CPACK_GENERATOR "WIX")
endif(WIN32)

if(APPLE)
  set(CPACK_COMPONENTS_ALL gsage)
  set(CPACK_DMG_BACKGROUND_IMAGE ${gsage_SOURCE_DIR}/resources/editor/dmgbg.png)
  set(CPACK_DMG_DS_STORE_SETUP_SCRIPT ${gsage_SOURCE_DIR}/CMakeDSStore.oas)
  set(CPACK_DMG_VOLUME_NAME "GSAGE engine")

  set(CPACK_GENERATOR "DragNDrop")
elseif(UNIX)
  set(CPACK_COMPONENTS_ALL Unspecified gsage game headers packager resources)
  set(CPACK_DEBIAN_PACKAGE_NAME "gsage")
  set(CPACK_DEBIAN_PACKAGE_VERSION "${GSAGE_VERSION_MAJOR}.${GSAGE_VERSION_MINOR}.${GSAGE_VERSION_PATCH}")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Artem Chernyshev")
  # additional dependencies for cef
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcups2, libgconf-2-4, libgtk2.0-dev, libgdk-pixbuf2.0-dev, freeglut3-dev")

  # other deps will be generated automatically
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Open source game engine")

  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${gsage_SOURCE_DIR}/tools/postinst;${gsage_SOURCE_DIR}/tools/prerm")

  set(CPACK_GENERATOR "DEB")
endif(APPLE)


include(CPack)
