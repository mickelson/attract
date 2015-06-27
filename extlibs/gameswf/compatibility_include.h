// Dummy header; should get included first into tu-testbed headers.
// This is for manual project-specific configuration.

//
// Some optional general configuration.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// Choose STL containers, or tu-testbed's simplified work-alikes.
// #define _TU_USE_STL 1

// Memory allocation functions.
// #define tu_malloc(size) ...
// #define tu_realloc(old_ptr, new_size, old_size) ...
// #define tu_free(old_ptr, old_size) ...

// @@ TODO operator new stub

// Fatal error handler.
// #define tu_error_exit(error_code, error_message) ...


//
// Some optional gameswf configuration.
//

// #define GAMESWF_FONT_NOMINAL_GLYPH_SIZE_DEFAULT 32

// Define this to avoid using mesh shapes to render large text
// #define GAMESWF_ALWAYS_USE_TEXTURES_FOR_TEXT_WHEN_POSSIBLE 1

// For disabling zlib and jpeg functionality; you may be able to use
// this in final builds of gameswf, if you preprocess all your SWF
// content to make unpacked formats.
// #define TU_CONFIG_LINK_TO_JPEGLIB 0
// #define TU_CONFIG_LINK_TO_ZLIB 0

// For enabling XML/XMLSocket functionality in gameswf, using GNOME
// libxml2
// #define HAVE_LIBXML 1

#define TU_CONFIG_LINK_TO_LIBPNG 0

#define _TU_USE_STL 1
#define TU_CONFIG_LINK_TO_THREAD 0
#define TU_CONFIG_LINK_STATIC 1
#define TU_USE_SDL 0
#define TU_USE_OPENAL 1

#ifndef __APPLE__
#define TU_CONFIG_LINK_TO_FREETYPE 1
#endif
