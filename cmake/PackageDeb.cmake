set(CPACK_SET_DESTDIR "on")
set(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
set(CPACK_GENERATOR "DEB")
set(CPACK_STRIP_FILES ON)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Attract-Mode emulator frontend")
set(CPACK_PACKAGE_VENDOR "Andrew Mickelson")
set(CPACK_PACKAGE_CONTACT "andrew@attractmode.org")

set(CPACK_PACKAGE_VERSION_MAJOR "${VER_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VER_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VER_PATCH}")

set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")

set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${VER_MAJOR}.${VER_MINOR}.${VER_PATCH}-${VER_COUNT}_${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)
