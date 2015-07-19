# - Try to find OpenGLES

set(GLES_SEARCH_PATHS
	${GLES_SEARCH_PATHS}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	/opt/graphics/OpenGL
	/opt/vc  # Raspberry pi
	/usr/openwin
	/usr/shlib
	/usr/X11R6/
)


find_path(OPENGLES_INCLUDE_DIR 
	NAMES GLES/gl.h OpenGLES/ES1/gl.h
	PATH_SUFFIXES include include/GLES GLES
	PATHS ${GLES_SEARCH_PATHS}
)

find_library(OPENGLES_LIBRARY 
	NAMES libGLESv1_CM GLESv1_CM libGLES_CM GLES_CM
	PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64 lib/x86_64-linux-gnu/mesa-egl
	PATHS ${GLES_SEARCH_PATHS}
)

if(OPENGLES_LIBRARY)
	set(OPENGLES_LIBRARIES ${OPENGLES_LIBRARY})
else()
	message(ERROR "OPENGLES Library not found! debug FindOpenGLES.cmake!")
endif()

# Handle the QUIETLY and REQUIRED arguments and set XXX_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OPENGLES DEFAULT_MSG OPENGLES_LIBRARIES OPENGLES_INCLUDE_DIR)
