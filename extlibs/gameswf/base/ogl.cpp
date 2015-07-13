// ogl.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some OpenGL helpers; mainly to generically deal with extensions.


#include <SDL.h>
#include <SDL_opengl.h>
#include "base/ogl.h"
#include "base/utility.h"
#include <stdlib.h>
#include <string.h>


namespace ogl {

	bool	is_open = false;

	// Pointers to extension functions.
	typedef void * (APIENTRY * PFNWGLALLOCATEMEMORYNVPROC) (int size, float readfreq, float writefreq, float priority);
	typedef void (APIENTRY * PFNWGLFREEMEMORYNVPROC) (void *pointer);
	typedef void (APIENTRY * PFNGLVERTEXARRAYRANGENVPROC) (int size, void* buffer);
	typedef void (APIENTRY * PFNGLGENFENCESNVPROC) (GLsizei n, GLuint *fence_array);
	typedef void (APIENTRY * PFNGLSETFENCENVPROC) (GLuint fence_id, GLenum condition);
	typedef void (APIENTRY * PFNGLFINISHFENCENVPROC) (GLuint fence_id);

	typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
	typedef void (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
	typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
	typedef void (APIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);

	typedef void (APIENTRY * PFNGLCOMBINERINPUTNVPROC) (GLenum, GLenum, GLenum, GLenum, GLenum, GLenum);
	typedef void (APIENTRY * PFNGLCOMBINERPARAMETERINVPROC) (GLenum pname, GLint param);
	typedef void (APIENTRY * PFNGLCOMBINERPARAMETERFVNVPROC) (GLenum, const GLfloat *);

	PFNWGLALLOCATEMEMORYNVPROC	wglAllocateMemoryNV = 0;
	PFNWGLFREEMEMORYNVPROC	wglFreeMemoryNV = 0;
	PFNGLVERTEXARRAYRANGENVPROC	glVertexArrayRangeNV = 0;
	PFNGLGENFENCESNVPROC glGenFencesNV = 0;
	PFNGLSETFENCENVPROC glSetFenceNV = 0;
	PFNGLFINISHFENCENVPROC glFinishFenceNV = 0;

	PFNGLACTIVETEXTUREARBPROC	glActiveTextureARB = 0;
	PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB = 0;
	PFNGLMULTITEXCOORD2FARBPROC	glMultiTexCoord2fARB = 0;
	PFNGLMULTITEXCOORD2FVARBPROC	glMultiTexCoord2fvARB = 0;

	// GL_CLAMP or GL_CLAMP_TO_EDGE, depending on which is available.
	int	s_clamp_to_edge = GL_CLAMP;

	// Big, fast vertex-memory buffer.
	const int	VERTEX_BUFFER_SIZE = 4 << 20;
	void*	vertex_memory_buffer = 0;
	int	vertex_memory_top = 0;
	bool	vertex_memory_from_malloc = false;	// tells us whether to wglFreeMemoryNV() or free() the buffer when we're done.


	const int	STREAM_SUB_BUFFER_COUNT = 2;

	GLint s_num_compressed_format = 0;


	class vertex_stream
	{
	// Class to facilitate streaming verts to the video card.  Takes
	// care of fencing, and buffer bookkeeping.
	public:
		vertex_stream(int buffer_size);
		~vertex_stream();
	
		void*	reserve_memory(int size);
		void	flush_combiners();
	
	private:
		int	m_sub_buffer_size;
		int	m_buffer_top;
		void*	m_buffer;
		int	m_extra_bytes;	// extra bytes after last block; used to pad up to write-combiner alignment
	
		unsigned int	m_fence[4];
	};


	vertex_stream*	s_stream = NULL;


	void	open()
	// Scan for extensions.
	{
		wglAllocateMemoryNV = (PFNWGLALLOCATEMEMORYNVPROC) SDL_GL_GetProcAddress( PROC_NAME_PREFIX "AllocateMemoryNV" );
		wglFreeMemoryNV = (PFNWGLFREEMEMORYNVPROC) SDL_GL_GetProcAddress( PROC_NAME_PREFIX "FreeMemoryNV" );
		glVertexArrayRangeNV = (PFNGLVERTEXARRAYRANGENVPROC) SDL_GL_GetProcAddress( "glVertexArrayRangeNV" );

		glGenFencesNV = (PFNGLGENFENCESNVPROC) SDL_GL_GetProcAddress( "glGenFencesNV" );
		glSetFenceNV = (PFNGLSETFENCENVPROC) SDL_GL_GetProcAddress( "glSetFenceNV" );
		glFinishFenceNV = (PFNGLFINISHFENCENVPROC) SDL_GL_GetProcAddress( "glFinishFenceNV" );

		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glActiveTextureARB");
		glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glClientActiveTextureARB");
		glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
		glMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC) SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");

		if (check_extension("GL_SGIS_texture_edge_clamp")
		    || check_extension("GL_EXT_texture_edge_clamp"))
		{
			// Use CLAMP_TO_EDGE, since it's available.
			s_clamp_to_edge = GL_CLAMP_TO_EDGE;
		}

		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &s_num_compressed_format);
	}


	void	close()
	// Release anything we need to.
	{
		// @@ free that mongo vertex buffer.
	}


	int	get_clamp_mode()
	// Return a constant to pass to glTexParameteri(GL_TEXTURE_2D,
	// GL_TEXTURE_WRAP_x, ...), which is either GL_CLAMP or
	// GL_CLAMP_TO_EDGE, depending on whether GL_CLAMP_TO_EDGE is
	// available.
	{
		return s_clamp_to_edge;
	}


	bool	check_extension(const char* extension)
	// Some extension checking code snipped from glut.
	{
		static const char*	extensions = NULL;
		const char*	start;
		const char*	where;
		const char*	terminator;
		bool	supported;
	
		// Extension names should not have spaces
		where = strchr(extension, ' ');
		if (where || *extension == '\0') return false;
	
		// Grab extensions (but only once)
		if (!extensions) extensions = (const char*)glGetString(GL_EXTENSIONS);
	
		// Look for extension
		start = extensions;
		supported = false;
		while (!supported)
		{
			// Does extension SEEM to be supported?
			where = strstr((const char*)start, extension);
			if (!where) break;

			// Ok, extension SEEMS to be supported
			supported = true;

			// Check for space before extension
			supported &= (where == start) || (where[-1] == ' ');

			// Check for space after extension
			terminator = where + strlen(extension);
			supported &= (*terminator == '\0') || (*terminator == ' ');

			// Next search starts at current terminator
			start = terminator;
		}

		return supported;
	}


	void*	allocate_vertex_memory( int size )
	// Allocate a block of memory for storing vertex data.  Using this
	// allocator will hopefully give you faster glDrawElements(), if
	// you do vertex_array_range on it before rendering.
	{
		// For best results, we must allocate one big ol' chunk of
		// vertex memory on the first call to this function, via
		// wglAllocateMemoryNV, and then allocate sub-chunks out of
		// it.

		if ( vertex_memory_buffer == 0 ) {
			// Need to allocate the big chunk.
			
			// If we have NV's allocator, then use it.
			if ( wglAllocateMemoryNV ) {
				vertex_memory_buffer = wglAllocateMemoryNV( VERTEX_BUFFER_SIZE, 0.f, 0.f, 0.5f );	// @@ this gets us AGP memory.
//				wglAllocateMemoryNV( size, 0.f, 0.f, 1.0f );	// @@ this gets us video memory.
				vertex_memory_from_malloc = false;
				vertex_memory_top = 0;

				if ( vertex_memory_buffer && glVertexArrayRangeNV ) {
					glVertexArrayRangeNV( VERTEX_BUFFER_SIZE, vertex_memory_buffer );
				}

				glEnableClientState(GL_VERTEX_ARRAY_RANGE_NV);	// GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV
			}
			// else we'll fall back on malloc() for vb allocations.
		}

		// Carve a chunk out of our big buffer, or out of malloc if
		// the buffer is dry.

		if ( vertex_memory_buffer && vertex_memory_top + size <= VERTEX_BUFFER_SIZE ) {
			// Just allocate from the end of the big buffer and increment the top.
			unsigned char*	buffer = (unsigned char*) vertex_memory_buffer + vertex_memory_top;
			vertex_memory_top += size;

			return (void*) buffer;

		} else {
			// Fall back to malloc.
			printf( "avm: warning, falling back to malloc!\n" );
			return malloc( size );
		}
	}


	void	free_vertex_memory(void* buffer)
	// Frees a buffer previously allocated via allocate_vertex_memory().
	{
		// this function is not ready for prime-time.
		assert( 0 );
	}


	void	gen_fences(int count, unsigned int* fence_array)
	// Wrapper for glGenFencesNV.
	{
		if (glGenFencesNV) {
			glGenFencesNV(count, (GLuint*)fence_array);
		}
		else
		{
			// set all fences to 0.
			for (int i = 0; i < count; i++) {
				fence_array[i] = 0;
			}
		}
	}

	
	void	set_fence(unsigned int fence_id)
	// Use this to declare all previous glDrawElements() calls as
	// belonging to the specified fence.  A subsequent
	// finish_fence(id) will block until those drawing calls have
	// completed.
	{
		if (glSetFenceNV)
		{
			glSetFenceNV(fence_id, GL_ALL_COMPLETED_NV);
		}
		// else no-op.
	}


	void	finish_fence(unsigned int fence_id)
	// Block until all gl drawing calls, associated with the specified
	// fence, have completed.
	{
		if (glFinishFenceNV)
		{
			glFinishFenceNV(fence_id);
		}
		// else no-op.
	}


	void*	stream_get_vertex_memory(int size)
	// Return a buffer to contain size bytes of vertex memory.
	// Put your vertex data in it, then call
	// stream_flush_combiners(), then call glDrawElements() to
	// draw the primitives.
	{
		if (s_stream == NULL) {
			s_stream = new vertex_stream(VERTEX_BUFFER_SIZE);
		}

		return s_stream->reserve_memory(size);
	}


	void	stream_flush_combiners()
	// Make sure to flush all data written to the most recently
	// requested vertex buffer (requested by the last call to
	// stream_get_vertex_memory()).  Ensure that the data doesn't
	// get hung up in a processor write-combiner.
	{
		assert(s_stream);
		if (s_stream == NULL) return;

		s_stream->flush_combiners();
	}


	static const int	WRITE_COMBINER_ALIGNMENT = 64;	// line-size (in bytes) of the write-combiners, must be power of 2


	int	wc_align_up(int size)
	// Return given size, rounded up to the next nearest write combiner
	// alignment boundary.
	{
		assert((WRITE_COMBINER_ALIGNMENT & (WRITE_COMBINER_ALIGNMENT - 1)) == 0);	// make sure it's a power of 2

		return (size + WRITE_COMBINER_ALIGNMENT - 1) & ~(WRITE_COMBINER_ALIGNMENT - 1);
	}


	//
	// vertex_stream
	//

	vertex_stream::vertex_stream(int buffer_size)
	// Construct a streaming buffer, with vertex RAM of the specified size.
	{
		assert(buffer_size >= STREAM_SUB_BUFFER_COUNT);
	
		m_sub_buffer_size = buffer_size / STREAM_SUB_BUFFER_COUNT;
		m_buffer = ogl::allocate_vertex_memory(buffer_size);
		m_buffer_top = 0;
		m_extra_bytes = 0;
	
		// set up fences.
		ogl::gen_fences(4, &m_fence[0]);

		// Set (dummy) fences which will be finished as we reach them.
		ogl::set_fence(m_fence[1]);
		ogl::set_fence(m_fence[2]);
		ogl::set_fence(m_fence[3]);
	}

	vertex_stream::~vertex_stream()
	{
//		ogl::free_vertex_memory(m_buffer);
	}

	void*	vertex_stream::reserve_memory(int size)
	// Clients should call this to get a temporary chunk of fast
	// vertex memory.  Fill it with vertex info and call
	// glVertexPointer()/glDrawElements().  The memory won't get
	// stomped until the drawing is finished, provided you use the
	// returned buffer in a glDrawElements call before you call
	// reserve_memory() to get the next chunk.
	{
		assert(size <= m_sub_buffer_size);

		int	aligned_size = wc_align_up(size);
		m_extra_bytes = aligned_size - size;
	
		for (int sub_buffer = 1; sub_buffer <= STREAM_SUB_BUFFER_COUNT; sub_buffer++)
		{
			int	border = m_sub_buffer_size * sub_buffer;

			if (m_buffer_top <= border
			    && m_buffer_top + aligned_size > border)
			{
				// Crossing into the next sub-buffer.

				int	prev_buffer = sub_buffer - 1;
				int	next_buffer = sub_buffer % STREAM_SUB_BUFFER_COUNT;

				// Protect the previous sub-buffer.
				ogl::set_fence(m_fence[prev_buffer]);

				// Don't overwrite the next sub-buffer while it's still active.
				ogl::finish_fence(m_fence[next_buffer]);
	
				// Start the next quarter-buffer.
				m_buffer_top = m_sub_buffer_size * next_buffer;
			}
		}
	
		void*	buf = ((char*) m_buffer) + m_buffer_top;
		m_buffer_top += aligned_size;
	
		return buf;
	}
	
	
	void	vertex_stream::flush_combiners()
	// Make sure the tail of the block returned by the last call
	// to reserve_memory() gets written to the system RAM.  If the
	// block doesn't end on a write-combiner line boundary, the
	// last bit of data may still be waiting to be flushed.
	//
	// That's my theory anyway -- farfetched but I'm not sure how
	// else to explain certain bug reports of spurious triangles
	// being rendered.
	{
		if (m_extra_bytes) {
			// Fill up the rest of the last write-combiner line.
			memset(((char*) m_buffer) + m_buffer_top - m_extra_bytes, 0, m_extra_bytes);
		}
	}


	// Wrappers for multitexture extensions; no-op if the extension doesn't exist.

	void	active_texture(int stage)
	// Set the currently active texture stage; use GL_TEXTUREx_ARB.
	{
		if (glActiveTextureARB)
		{
			glActiveTextureARB(stage);
		}
	}

	void	client_active_texture(int stage)
	// Set the currently active texture stage for vertex array
	// setup; use GL_TEXTUREx_ARB.
	{
		if (glClientActiveTextureARB)
		{
			glClientActiveTextureARB(stage);
		}
	}

	void	multi_tex_coord_2f(int stage, float s, float t)
	// Texture coords for the current vertex, in the specified
	// stage.
	{
		if (glMultiTexCoord2fARB)
		{
			glMultiTexCoord2fARB(stage, s, t);
		}
	}

	void	multi_tex_coord_2fv(int stage, float* st)
	// Texture coords for the current vertex, in the specified
	// stage.
	{
		if (glMultiTexCoord2fvARB)
		{
			glMultiTexCoord2fvARB(stage, st);
		}
	}

	void create_texture(int format, int w, int h, void* data, int level)
	{
		int internal_format = format;
		if (s_num_compressed_format > 0)
		{
			switch (format)
			{
				case GL_RGB :
					internal_format = GL_COMPRESSED_RGB_ARB;
					break;
				case GL_RGBA :
					internal_format = GL_COMPRESSED_RGBA_ARB;
					break;
				case GL_ALPHA :
					internal_format = GL_COMPRESSED_ALPHA_ARB;
					break;
				case GL_LUMINANCE :
					internal_format = GL_COMPRESSED_LUMINANCE_ARB;
					break;
			}
		}
		glTexImage2D(GL_TEXTURE_2D, level, internal_format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
	}



};

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
