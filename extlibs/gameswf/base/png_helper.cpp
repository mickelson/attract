// png_helper.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2004

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Wrapper for png file operations.  The actual work is done by the
// libpng lib.


#include "base/utility.h"
#include "base/png_helper.h"
#include "base/tu_file.h"
#include <stdio.h>

#if TU_CONFIG_LINK_TO_LIBPNG

#include <png.h>

namespace png_helper
{
	void	write_grayscale(FILE* out, uint8* data, int width, int height)
	// Writes an 8-bit grayscale image in .png format, to the
	// given output stream.
	{
		png_structp	png_ptr;
		png_infop	info_ptr;

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL)
		{
			// @@ log error here!
			return;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL)
		{
			// @@ log error here!
			png_destroy_write_struct(&png_ptr, NULL);
			return;
		}

		if (setjmp(png_jmpbuf(png_ptr)))
		{
			// Error.
			png_destroy_write_struct(&png_ptr, &info_ptr);
			return;
		}

		png_init_io(png_ptr, out);

		png_set_IHDR(
			png_ptr,
			info_ptr,
			width,
			height,
			8,
			PNG_COLOR_TYPE_GRAY,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);

		png_write_info(png_ptr, info_ptr);
		
		for (int y = 0; y < height; y++)
		{
			png_write_row(png_ptr, data + width * y);
		}

		png_write_end(png_ptr, info_ptr);

		png_destroy_write_struct(&png_ptr, &info_ptr);
	}

	void write_data(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		fwrite(data, length, 1, (FILE*) png_ptr->io_ptr);
	}

	void	write_rgba(FILE* out, uint8* data, int width, int height, int bpp)
	// Writes a 24 or 32-bit color image in .png format, to the
	// given output stream.  Data should be in [RGB or RGBA...] byte order.
	{
		if (bpp != 3 && bpp != 4)
		{
			return;
		}

		png_structp	png_ptr;
		png_infop	info_ptr;

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL)
		{
			// @@ log error here!
			return;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL)
		{
			// @@ log error here!
			png_destroy_write_struct(&png_ptr, NULL);
			return;
		}

		if (setjmp(png_jmpbuf(png_ptr)))
		{
			// Error.
			png_destroy_write_struct(&png_ptr, &info_ptr);
			return;
		}

		png_init_io(png_ptr, out);
		png_set_write_fn(png_ptr, (png_voidp) out, write_data, NULL);

		png_set_IHDR(
			png_ptr,
			info_ptr,
			width,
			height,
			8,
			bpp == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);

		// png_set_swap_alpha(png_ptr);
		// png_set_bgr(png_ptr);

		png_write_info(png_ptr, info_ptr);
		
		for (int y = 0; y < height; y++)
		{
			png_write_row(png_ptr, data + (width * bpp) * y);
		}

		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
}


#else // not TU_CONFIG_LINK_TO_LIBPNG


namespace jpeg
{
	void	write_grayscale(FILE* out, uint8* data, int width, int height)
	{
		// no-op
	}
}


#endif // not TU_CONFIG_LINK_TO_LIBPNG


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
