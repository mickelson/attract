cmake_minimum_required(VERSION 2.9 FATAL_ERROR)

set(FE_VERSION "v2.0.0")

project(attract)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# cmake module search path, prefer ours
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

include(GNUInstallDirs)
include(InstallRequiredSystemLibraries)
include(GetGitRevisionDescription)
include(CheckIncludeFile)
include(TargetArch)

target_architecture(CMAKE_TARGET_ARCHITECTURES)
string(TOUPPER ${CMAKE_SYSTEM_NAME} TARGET_SYSTEM)
string(CONCAT TARGET_SYSTEM TARGET_ ${TARGET_SYSTEM})
string(TOUPPER ${CMAKE_TARGET_ARCHITECTURES} TARGET_ARCH)
string(CONCAT TARGET_ARCH TARGET_ ${TARGET_ARCH})

message(STATUS "Target: ${CMAKE_SYSTEM_NAME} ${CMAKE_TARGET_ARCHITECTURES}")

#### BEGIN: Options ####
# All options mapped to config.h definitions

option(NO_MOVIE       "Disable movie support (ffmpeg)" OFF)
option(NO_SWF         "Disable SWF support (gameswf)" OFF)
option(USE_FONTCONFIG "FontConfig support" ON)
option(USE_LIBARCHIVE "Archive library support" ON)
option(USE_XINERAMA   "Xinerama support" ON)
option(USE_GLES       "GLES instead of GL" OFF)
option(USE_LIBCURL    "Libcurl support" ON)
option(FE_DEBUG       "Debug" OFF)
option(FE_RPI         "Build for Raspberry PI" OFF)

if(WIN32)
    option(WINDOWS_CONSOLE "Enable console on Windows" OFF)
endif()

if(FE_RPI)
    set(USE_GLES ON)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(FE_DEBUG ON)
endif()

if(USE_XINERAMA AND NOT WIN32 AND NOT APPLE)
    find_package(Xinerama)
endif()
set(USE_XINERAMA ${XINERAMA_FOUND})

if(USE_LIBARCHIVE)
    find_package(LibArchive)
endif()
set(USE_LIBARCHIVE ${LibArchive_Found})

if(USE_FONTCONFIG AND NOT WIN32)
    find_package(FontConfig)
endif()
set(USE_FONTCONFIG ${FONTCONFIG_FOUND})

if(USE_LIBCURL)
    find_package(CURL)
endif()
set(USE_LIBCURL ${CURL_FOUND})
#### END: Options ####

#### BEGIN: Version ####
# parse VER_* definitions for config.h, prefer git, fallback to FE_VERSION

git_describe(VER_TAG --tags --abbrev=0)
if("${VER_TAG}" STREQUAL "GIT-NOTFOUND")
    set(VER_TAG ${FE_VERSION})
    set(VER_COUNT "0")
else()
    git_revlist(VER_COUNT --count ${VER_TAG}..HEAD)
endif()
string(REGEX REPLACE "^v" "" RESULT ${VER_TAG})
string(REGEX REPLACE "[-\\.]" ";" RESULT ${RESULT})
list(GET RESULT 0 VER_MAJOR)
list(GET RESULT 1 VER_MINOR)
list(GET RESULT 2 VER_PATCH)
git_describe(VER_DIRTY --dirty)
if(NOT "${VER_COUNT}" EQUAL "0")
    set(VER_TAG "${VER_TAG}-${VER_COUNT}")
endif()
if("${VER_DIRTY}" MATCHES "-dirty")
    set(VER_TAG "${VER_TAG}-dirty")
endif()
message(STATUS "Version Tag: ${VER_TAG}")

#### END: Version ####

#### BEGIN: Dependencies ####

if(USE_FONTCONFIG)
    find_package(EXPAT REQUIRED)
endif()

# FFmpeg
if(NOT NO_MOVIE)
    find_package(FFmpeg COMPONENTS avformat avcodec avutil swscale swresample)
    if(NOT FFMPEG_SWRESAMPLE_FOUND)
        find_package(FFmpeg COMPONENTS avformat avcodec avutil swscale avresample REQUIRED)
    endif()
endif()

#### END: Dependencies ####

#### BEGIN: Misc ####

# If ARM, assume GLES
if("${CMAKE_TARGET_ARCHITECTURES}" MATCHES "arm")
    set(USE_GLES ON)
    set(VIDEOCORE_PATH /opt/vc)
    find_file(HAVE_VIDEOCORE bcm_host.h PATHS ${VIDEOCORE_PATH}/include NO_DEFAULT_PATH)
    if(HAVE_VIDEOCORE)
        set(FE_RPI ON)
        include_directories(${VIDEOCORE_PATH}/include)
        link_directories(${VIDEOCORE_PATH}/lib)
    endif()
    message(STATUS "ARM detected: USE_GLES:${USE_GLES} FE_RPI:${FE_RPI}")
endif()

if(NOT WIN32)
    set(DATA_PATH "${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/")
endif()

#### END: Misc ####

configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_BINARY_DIR}/config.h")

# process CMakeLists.txt in these folders
add_subdirectory(extlibs)
add_subdirectory(src)

install(DIRECTORY config/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME})

include(cmake/PackageDeb.cmake)
add_custom_target(deb DEPENDS attract
                      COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=/usr .
                      COMMAND ${CMAKE_CPACK_COMMAND} -G DEB
                      WORKING_DIRECTORY ${BUILD_DIR})
