// ogl.h	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some OpenGL helpers; mainly to generically deal with extensions.

#ifndef OGL_H
#define OGL_H

#include "base/tu_config.h"
#include "base/tu_opengl_includes.h"
#include "base/container.h"

namespace ogl
{
	exported_module void	open();
	exported_module void	close();

	// Return true if the specified extension is available.
	bool	check_extension(const char* extension);

	// Return GL_CLAMP, or GL_CLAMP_TO_EDGE_EXT, depending on
	// which is available.  I pretty much always want the
	// GL_CLAMP_TO_EDGE_EXT behavior, but it's not in the OpenGL
	// 1.1 standard, so in those cases I must fall back to
	// GL_CLAMP.
	int	get_clamp_mode();

	// For allocating DMA or video memory, for holding vertex arrays.
	void*	allocate_vertex_memory( int size );	// @@ add a flag for selecting AGP vs. video mem?
	void	free_vertex_memory( void* buffer );

	// Fences; for synchronizing with the GPU.
	void	gen_fences(int count, unsigned int* fence_array);
	void	set_fence(unsigned int fence_id);
	void	finish_fence(unsigned int fence_id);

	// Stream operations; for pushing dynamic vertex data.
	void*	stream_get_vertex_memory(int size);
	void	stream_flush_combiners();	// do this after filling your buffer, and before calling glDrawElements()

	// Rudimentary multitexture stuff.
	void	active_texture(int stage);
	void	client_active_texture(int stage);
	void	multi_tex_coord_2f(int stage, float s, float t);
	void	multi_tex_coord_2fv(int stage, float* st);

	void create_texture(int format, int w, int h, void* data, int level = 0);
};


// Some old gl/gl.h files don't define these, e.g. default Windows includes.
// It shouldn't hurt anything to call glTexEnvf() with these values on a system
// that doesn't implement them.

#ifndef GL_TEXTURE_FILTER_CONTROL_EXT
#define GL_TEXTURE_FILTER_CONTROL_EXT 0x8500
#endif

#ifndef GL_TEXTURE_LOD_BIAS_EXT
#define GL_TEXTURE_LOD_BIAS_EXT 0x8501
#endif


#endif // OGL_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
