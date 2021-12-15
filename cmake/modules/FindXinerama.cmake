# - Try to find the Xinerama libraries
# Once done this will define
#
#  XINERAMA_FOUND
#  XINERAMA_INCLUDE_DIRS
#  XINERAMA_LIBRARIES
#
#  XINERAMA_INCLUDE_DIR
#  XINERAMA_LIBRARY

# Copyright (c) 2016 Jeffrey Clark <dude@zaplabs.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if(XINERAMA_INCLUDE_DIR AND XINERAMA_LIBRARY)
    # Already in cache, be silent
    set(XINERAMA_FIND_QUIETLY TRUE)
endif(XINERAMA_INCLUDE_DIR AND XINERAMA_LIBRARY)

find_package(PkgConfig)
pkg_check_modules(PC_XINERAMA QUIET xinerama)

find_path(XINERAMA_INCLUDE_DIR
          NAMES Xinerama.h
          HINTS ${PC_XINERAMA_INCLUDE_DIRS}
          PATH_SUFFIXES X11/extensions
          DOC "Xinerama include directory")

find_library(XINERAMA_LIBRARY
             NAMES Xinerama xinerama
             HINTS ${PC_XINERAMA_LIBRARY_DIRS}
             DOC "Xinerama library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Xinerama DEFAULT_MSG XINERAMA_LIBRARY XINERAMA_INCLUDE_DIR)

if(XINERAMA_FOUND)
     set(XINERAMA_LIBRARIES ${XINERAMA_LIBRARY})
     set(XINERAMA_INCLUDE_DIRS ${XINERAMA_INCLUDE_DIR})
endif()

mark_as_advanced(XINERAMA_INCLUDE_DIR XINERAMA_LIBRARY)
