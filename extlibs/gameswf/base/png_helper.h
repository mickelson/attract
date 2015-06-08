// png_helper.h	-- Thatcher Ulrich <tu@tulrich.com> 2004

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Wrapper for png file operations.  The actual work is done by the
// libpng lib.

#ifndef PNG_HELPER_H
#define PNG_HELPER_H


#include "base/tu_config.h"
#include <stdio.h>


namespace png_helper
{
	void	write_grayscale(FILE* out, uint8* data, int width, int height);

	// Data should be in [RGBA...] byte order.  width and height
	// in pixels (not bytes).
	void	write_rgba(FILE* out, uint8* data, int width, int height, int bpp);

	// TODO: add more helpers as needed...
}


#endif // PNG_HELPER_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
