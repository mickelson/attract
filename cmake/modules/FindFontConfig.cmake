# - Try to find the PANGO_CAIRO libraries
# Once done this will define
#
#  FONTCONFIG_FOUND - system has cairo-xlib
#  FONTCONFIG_INCLUDE_DIR - the glib2 include directory
#  FONTCONFIG_LIBRARIES - glib2 library

# Copyright (c) 2012 CSSlayer <wengxt@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if(FONTCONFIG_INCLUDE_DIR AND FONTCONFIG_LIBRARIES)
    # Already in cache, be silent
    set(FONTCONFIG_FIND_QUIETLY TRUE)
endif(FONTCONFIG_INCLUDE_DIR AND FONTCONFIG_LIBRARIES)

find_package(PkgConfig)
pkg_check_modules(PC_FONTCONFIG QUIET fontconfig)

find_path(FONTCONFIG_INCLUDE_DIR
          NAMES fontconfig.h
          HINTS ${PC_FONTCONFIG_INCLUDE_DIRS}
          PATH_SUFFIXES fontconfig)

find_library(FONTCONFIG_LIBRARY
             NAMES fontconfig
             HINTS ${PC_FONTCONFIG_LIBRARY_DIRS}
)

set(FONTCONFIG_LIBRARIES ${FONTCONFIG_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FontConfig DEFAULT_MSG  FONTCONFIG_LIBRARIES FONTCONFIG_INCLUDE_DIR)

mark_as_advanced(FONTCONFIG_INCLUDE_DIR FONTCONFIG_LIBRARIES FONTCONFIG_LIBRARY)
