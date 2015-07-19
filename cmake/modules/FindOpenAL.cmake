# - Try to find the OpenAL libraries
#   Cmake included module does not include static deps
#
# Once done this will define
#
#  OPENAL_FOUND
#  OPENAL_INCLUDE_DIRS
#  OPENAL_LIBRARIES
#  OPENAL_CFLAGS
#
#  OPENAL_INCLUDE_DIR
#  OPENAL_LIBRARY

# Copyright (c) 2016 Jeffrey Clark <dude@zaplabs.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig)
pkg_check_modules(PC_OPENAL openal)

find_path(OPENAL_INCLUDE_DIR
          NAMES al.h
          HINTS ${PC_OPENAL_LIBRARY_DIRS}
          PATH_SUFFIXES include/AL include/OpenAL include
          DOC "OpenAL include directory")

find_library(OPENAL_LIBRARY
             NAMES OpenAL al openal OpenAL32
             HINTS ${PC_OPENAL_LIBRARY_DIRS}
             DOC "OpenAL library")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenAL DEFAULT_MSG OPENAL_LIBRARY OPENAL_INCLUDE_DIR)

if(OPENAL_FOUND)
    set(OPENAL_LIBRARIES ${PC_OPENAL_LIBRARIES})
    set(OPENAL_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR})
    set(OPENAL_CFLAGS ${PC_OPENAL_CFLAGS_OTHER})
endif()

mark_as_advanced(OPENAL_INCLUDE_DIR OPENAL_LIBRARY OPENAL_CFLAGS)
