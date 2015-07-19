cmake_minimum_required(VERSION 2.9)
project(attract)

if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Version Number
set(attract_VERSION_MAJOR 1)
set(attract_VERSION_MINOR 5)
set(attract_VERSION_PATCH 3)

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
set(EXTLIBS_PATH "${CMAKE_SOURCE_DIR}/extlibs")

find_package(SFML 2 COMPONENTS system window graphics REQUIRED)
find_package(SFML 2 COMPONENTS system window graphics network audio)
find_package(FFmpeg COMPONENTS avformat avcodec avutil swscale swresample avresample)

include(InstallRequiredSystemLibraries)

set(EXECUTABLE_NAME "attract")

add_subdirectory(extlibs)
add_subdirectory(src)
