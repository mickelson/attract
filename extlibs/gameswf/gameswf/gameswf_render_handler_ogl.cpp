// gameswf_render_handler_ogl.cpp	-- Willem Kokke <willem@mindparity.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A gameswf::render_handler that uses SDL & OpenGL

#include "base/tu_config.h"

#ifdef TU_USE_SDL
#include <SDL.h>  // for cursor handling & the scanning for extensions.
#include <SDL_opengl.h>	// for opengl const
#else
#include "base/tu_opengl_includes.h"
#include <gl/glext.h>
#endif //TU_USE_SDL

#include "gameswf/gameswf.h"
#include "gameswf/gameswf_types.h"
#include "base/image.h"
#include "base/utility.h"

#include <string.h>	// for memset()

// Pointers to opengl extension functions.
typedef char GLchar;

#if TU_USE_SDL == 1

typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC) (GLenum texture);
PFNGLACTIVETEXTUREPROC _glActiveTexture = 0;

typedef void (APIENTRY* PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
PFNGLACTIVETEXTUREARBPROC	_glActiveTextureARB = 0;

typedef void (APIENTRY* PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
PFNGLCLIENTACTIVETEXTUREARBPROC	_glClientActiveTextureARB = 0;

typedef void (APIENTRY* PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
PFNGLMULTITEXCOORD2FARBPROC	_glMultiTexCoord2fARB = 0;

typedef void (APIENTRY* PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
PFNGLMULTITEXCOORD2FVARBPROC	_glMultiTexCoord2fvARB = 0;

typedef void (APIENTRY* PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = 0;

typedef void (APIENTRY* PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = 0;

typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = 0;

typedef GLenum (APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = 0;

typedef void (APIENTRY* PFNGLDELETEPROGRAMPROC) (GLuint program);
PFNGLDELETEPROGRAMPROC glDeleteProgram = 0;

typedef void (APIENTRY* PFNGLDELETESHADERPROC) (GLuint shader);
PFNGLDELETESHADERPROC glDeleteShader = 0;

typedef GLuint (APIENTRY* PFNGLCREATESHADERPROC) (GLenum type);
PFNGLCREATESHADERPROC glCreateShader = 0;

typedef void (APIENTRY* PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
PFNGLSHADERSOURCEPROC glShaderSource = 0;

typedef void (APIENTRY* PFNGLCOMPILESHADERPROC) (GLuint shader);
PFNGLCOMPILESHADERPROC glCompileShader = 0;

typedef void (APIENTRY* PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
PFNGLGETSHADERIVPROC glGetShaderiv = 0;

typedef void (APIENTRY* PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = 0;

typedef GLuint (APIENTRY* PFNGLCREATEPROGRAMPROC) (void);
PFNGLCREATEPROGRAMPROC glCreateProgram = 0;

typedef void (APIENTRY* PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
PFNGLATTACHSHADERPROC glAttachShader = 0;

typedef void (APIENTRY* PFNGLLINKPROGRAMPROC) (GLuint program);
PFNGLLINKPROGRAMPROC glLinkProgram = 0;

typedef void (APIENTRY* PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
PFNGLGETPROGRAMIVPROC glGetProgramiv = 0;

typedef void (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = 0;

typedef void (APIENTRY* PFNGLVALIDATEPROGRAMPROC) (GLuint program);
PFNGLVALIDATEPROGRAMPROC glValidateProgram = 0;

typedef GLint (APIENTRY* PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = 0;

typedef void (APIENTRY* PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
PFNGLUNIFORM1FPROC glUniform1f = 0;

typedef void (APIENTRY* PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
PFNGLUNIFORM1IPROC glUniform1i = 0;

typedef void (APIENTRY* PFNGLUSEPROGRAMPROC) (GLuint program);
PFNGLUSEPROGRAMPROC glUseProgram = 0;
#endif  // TU_USE_SDL

static GLint s_num_compressed_format = 0;
void create_texture(int format, int w, int h, void* data, int level)
{
	int internal_format = format;
	if (s_num_compressed_format > 0)
	{
		switch (format)
		{
			default:
				break;
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

// We do not want to use GLU library
// gluPerspective ==> glFrustum
void gluPerspective(float fov, float aspect, float znear, float zfar)
{
	//	gluPerspective(fov, aspect, nnear, ffar) ==> glFrustum(left, right, bottom, top, nnear, ffar);
	//	fov * 0.5 = arctan ((top-bottom)*0.5 / near)
	//	Since bottom == -top for the symmetrical projection that gluPerspective() produces, then:
	//	top = tan(fov * 0.5) * near
	//	bottom = -top
	//	Note: fov must be in radians for the above formulae to work with the C math library. 
	//	If you have comnputer your fov in degrees (as in the call to gluPerspective()), 
	//	then calculate top as follows:
	//	top = tan(fov*3.14159/360.0) * near
	//	The left and right parameters are simply functions of the top, bottom, and aspect:
	//	left = aspect * bottom
	//	right = aspect * top

	float top = tan(fov * 3.141592f / 360.0f) * znear;
	float bottom = - top;
	float left = aspect* bottom;
	float right = aspect * top;
	glFrustum(left, right, bottom, top, znear, zfar);
}

// used by gluLookAt
void normalize(float v[3])
{
	float r = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
	if (r == 0.0f) return;
	v[0] /= r; v[1] /= r; v[2] /= r;
}

// used by gluLookAt
void cross(float v1[3], float v2[3], float result[3])
{
	result[0] = v1[1] * v2[2] - v1[2] * v2[1];
	result[1] = v1[2] * v2[0] - v1[0] * v2[2];
	result[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

// We do not want to use GLU library
void gluLookAt(float eyex, float eyey, float eyez, float centerx, float centery, 
							 float centerz, float upx, float upy, float upz)
{
    float forward[3];
    forward[0] = centerx - eyex;
    forward[1] = centery - eyey;
    forward[2] = centerz - eyez;

    float up[3];
    up[0] = upx;
    up[1] = upy;
    up[2] = upz;

    normalize(forward);

    // Side = forward x up
    float side[3];
    cross(forward, up, side);
    normalize(side);

    // Recompute up as: up = side x forward
    cross(side, forward, up);

    GLfloat m[4][4];

		// make identity
		memset(&m[0], 0, sizeof(m));
		for (int i = 0; i < 4; i++)
		{
			m[i][i] = 1;
		}

    m[0][0] = side[0];
    m[1][0] = side[1];
    m[2][0] = side[2];

    m[0][1] = up[0];
    m[1][1] = up[1];
    m[2][1] = up[2];

    m[0][2] = -forward[0];
    m[1][2] = -forward[1];
    m[2][2] = -forward[2];

    glMultMatrixf(&m[0][0]);
    glTranslated(-eyex, -eyey, -eyez);
}

#if TU_USE_SDL == 1
#define SDL_CURSOR_HANDLING
#else
#undef SDL_CURSOR_HANDLING
#endif

#ifdef SDL_CURSOR_HANDLING

// XPM
static const char *s_hand_image[] = {
	// width height num_colors chars_per_pixel
	"    32    32        3            1",
	// colors
	"X c #000000",
	". c #ffffff",
	"  c None",
	// pixels
	"   XX                           ",
	"  X..X                          ",
	"  X..X                          ",
	"  X..X                          ",
	"  X..X                          ",
	"  X..XXX                        ",
	"  X..X..XXX                     ",
	"XXX..X..X..XXX                  ",
	"X.X..X..X..X..X                 ",
	"X.X..X..X..X..X                 ",
	"X....X..X..X..X                 ",
	"X..........X..X                 ",
	" X............X                 ",
	" X...........X                  ",
	" X...........X                  ",
	" X...........X                  ",
	" XXXXXXXXXXXXX                  ",
	" XXXXXXXXXXXXX                  ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"3,0"
};

struct sdl_cursor_handler {
	bool m_inited;
	SDL_Cursor* m_system_cursor;
	SDL_Cursor* m_active_cursor;

	sdl_cursor_handler() :
		m_inited(false),
		m_system_cursor(NULL),
		m_active_cursor(NULL)
	{
	}

	void init()
	{
		assert(!m_inited);
		m_inited = true;
		
		// store system cursor
		m_system_cursor = SDL_GetCursor();

		// Init active cursor.
		int i, row, col;
		Uint8 data[4 * 32];
		Uint8 mask[4 * 32];
		int hot_x, hot_y;

		i = -1;
		for (row=0; row<32; ++row) {
			for (col=0; col<32; ++col)
			{
				if (col % 8)
				{
					data[i] <<= 1;
					mask[i] <<= 1;
				} 
				else
				{
					++i;
					data[i] = mask[i] = 0;
				}

				switch (s_hand_image[4 + row][col])
				{
					default:
						break;

					case 'X':
						// black
						data[i] |= 0x01;
						mask[i] |= 0x01;
						break;

					case '.':
						// white
						mask[i] |= 0x01;
						break;

					case ' ':
						// transparent
						break;
				}
			}
		}
		sscanf(s_hand_image[4 + row], "%d,%d", &hot_x, &hot_y);
		
		m_active_cursor = SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
	}

	~sdl_cursor_handler()
	{
		m_inited = false;
		if (m_system_cursor)
		{
			SDL_SetCursor(m_system_cursor);
			m_system_cursor = NULL;
		}
		if (m_active_cursor)
		{
			SDL_FreeCursor(m_active_cursor);
			m_active_cursor = NULL;
		}
	}


	void set_cursor(gameswf::render_handler::cursor_type cursor)
	{
		if (m_inited == false)
		{
			init();
		}
		
		switch (cursor)
		{
			case gameswf::render_handler::SYSTEM_CURSOR:
				if (m_system_cursor)
				{
					SDL_SetCursor(m_system_cursor);
				}
				break;

			case gameswf::render_handler::ACTIVE_CURSOR:
				if (m_active_cursor)
				{
					SDL_SetCursor(m_active_cursor);
				}
				break;

			case gameswf::render_handler::INVISIBLE_CURSOR:
				SDL_ShowCursor(SDL_DISABLE);
				break;

			case gameswf::render_handler::VISIBLE_CURSOR:
				SDL_ShowCursor(SDL_ENABLE);
				break;

			default:
				assert(0);
		}
	}
};

#endif  // SDL_CURSOR_HANDLING

// choose the resampling method:
// 1 = hardware (experimental, should be fast, somewhat buggy)
// 2 = fast software bilinear (default)
// 3 = use image::resample(), slow software resampling
#define RESAMPLE_METHOD 2


// Determines whether to generate mipmaps for smoother rendering of
// minified textures.  It actually tends to look OK to not use
// mipmaps, though it is potentially a performance hit if you use big
// textures.
//
// TODO: need to code mipmap LOD bias adjustment, to keep mipmaps from
// looking too blurry.  (Also applies to text rendering.)
#define GENERATE_MIPMAPS 0


//inline bool opengl_accessible()
//{
//#ifdef _WIN32
//	return wglGetCurrentContext() != 0;
//#else
//	return glXGetCurrentContext() != 0;
//#endif
//}

// bitmap_info_ogl declaration
struct bitmap_info_ogl : public gameswf::bitmap_info
{
	unsigned int	m_texture_id;
	int m_width;
	int m_height;
	image::image_base* m_suspended_image;

	bitmap_info_ogl();
	bitmap_info_ogl(int width, int height, Uint8* data);
	bitmap_info_ogl(image::rgb* im);
	bitmap_info_ogl(image::rgba* im);

	virtual void layout();

	// get byte per pixel
	virtual int get_bpp() const
	{
		if (m_suspended_image)
		{
			switch (m_suspended_image->m_type)
			{
				default: return 0;
				case image::image_base::RGB: return 3;
				case image::image_base::RGBA: return 4;
				case image::image_base::ALPHA: return 1;
			};
		}
		return 0;
	}

	virtual unsigned char* get_data() const
	{
		if (m_suspended_image)
		{
			return m_suspended_image->m_data;
		}
		return NULL;
	}

	virtual void activate()
	{
		assert(m_texture_id > 0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
	}

	~bitmap_info_ogl()
	{
		if (m_texture_id > 0)
		{
			glDeleteTextures(1, (GLuint*) &m_texture_id);
			m_texture_id = 0;	// for debuging
		}
		delete m_suspended_image;
	}
		
	virtual int get_width() const { return m_width; }
	virtual int get_height() const { return m_height; }

};

struct video_handler_ogl : public gameswf::video_handler
{
	GLuint m_texture;
	float m_scoord;
	float m_tcoord;
	gameswf::rgba m_background_color;

	video_handler_ogl():
		m_texture(0),
		m_scoord(0),
		m_tcoord(0),
		m_background_color(0,0,0,0)	// current background color
	{
	}

	~video_handler_ogl()
	{
		glDeleteTextures(1, &m_texture);
	}

	void display(Uint8* data, int width, int height, 
		const gameswf::matrix* m, const gameswf::rect* bounds, const gameswf::rgba& color)
	{

		// this can't be placed in constructor becuase opengl may not be accessible yet
		if (m_texture == 0)
		{
			glEnable(GL_TEXTURE_2D);
			glGenTextures(1, &m_texture);
			glBindTexture(GL_TEXTURE_2D, m_texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// GL_NEAREST ?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		glBindTexture(GL_TEXTURE_2D, m_texture);
		glEnable(GL_TEXTURE_2D);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);

		// update texture from video frame
		if (data)
		{
			int w2p = p2(width);
			int h2p = p2(height);
			m_scoord = (float) width / w2p;
			m_tcoord = (float) height / h2p;

			if (m_clear_background)
			{
				// set video background color
				// assume left-top pixel of the first frame as background color
				if (m_background_color.m_a == 0)
				{
					m_background_color.m_a = 255;
					m_background_color.m_r = data[2];
					m_background_color.m_g = data[1];
					m_background_color.m_b = data[0];
				}

				// clear video background, input data has BGRA format
				Uint8* p = data;
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width; x++)
					{
						// calculate color distance, dist is in [0..195075]
						int r = m_background_color.m_r - p[2];
						int g = m_background_color.m_g - p[1];
						int b = m_background_color.m_b - p[0];
						float dist = (float) (r * r + g * g + b * b);

						static int s_min_dist = 3 * 64 * 64;	// hack
						Uint8 a = (dist < s_min_dist) ? (Uint8) (255 * (dist / s_min_dist)) : 255;

						p[3] = a;		// set alpha
						p += 4;
					}
				}
			}

			// don't use compressed texture for video, it slows down video
			//			ogl::create_texture(GL_RGBA, m_width2p, m_height2p, NULL);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2p, h2p, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data);
		}

		if (m_scoord == 0.0f && m_scoord == 0.0f)
		{
			// no data
			return;
		}

		gameswf::point a, b, c, d;
		m->transform(&a, gameswf::point(bounds->m_x_min, bounds->m_y_min));
		m->transform(&b, gameswf::point(bounds->m_x_max, bounds->m_y_min));
		m->transform(&c, gameswf::point(bounds->m_x_min, bounds->m_y_max));
		d.m_x = b.m_x + c.m_x - a.m_x;
		d.m_y = b.m_y + c.m_y - a.m_y;

		glColor4ub(color.m_r, color.m_g, color.m_b, color.m_a);
		glBegin(GL_TRIANGLE_STRIP);
		{
			glTexCoord2f(0, 0);
			glVertex2f(a.m_x, a.m_y);
			glTexCoord2f(m_scoord, 0);
			glVertex2f(b.m_x, b.m_y);
			glTexCoord2f(0, m_tcoord);
			glVertex2f(c.m_x, c.m_y);
			glTexCoord2f(m_scoord, m_tcoord);
			glVertex2f(d.m_x, d.m_y);
		}
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}

};


struct render_handler_ogl : public gameswf::render_handler
{
	// Some renderer state.

	// Enable/disable antialiasing.
	bool	m_enable_antialias;
	
	// Output size.
	float	m_display_width;
	float	m_display_height;
	
	gameswf::matrix	m_current_matrix;
	gameswf::cxform	m_current_cxform;

	int m_mask_level;	// nested mask level


	render_handler_ogl() :
		m_enable_antialias(false),
		m_display_width(0),
		m_display_height(0),
		m_mask_level(0)
	{
	}

	~render_handler_ogl()
	{
	}

	void open()
	{
#if TU_USE_SDL == 1
		// Scan for extensions used by gameswf
		_glActiveTexture =  (PFNGLACTIVETEXTUREPROC) SDL_GL_GetProcAddress("glActiveTexture");
		_glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glActiveTextureARB");
		_glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glClientActiveTextureARB");
		_glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
		_glMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC) SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");
		glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) SDL_GL_GetProcAddress("glGenFramebuffersEXT");
		glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) SDL_GL_GetProcAddress("glBindFramebufferEXT");
		glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
		glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
		glDeleteProgram = (PFNGLDELETEPROGRAMPROC) SDL_GL_GetProcAddress("glDeleteProgram");
		glDeleteShader = (PFNGLDELETESHADERPROC) SDL_GL_GetProcAddress("glDeleteShader");
		glCreateShader = (PFNGLCREATESHADERPROC) SDL_GL_GetProcAddress("glCreateShader");
		glShaderSource = (PFNGLSHADERSOURCEPROC) SDL_GL_GetProcAddress("glShaderSource");
		glCompileShader = (PFNGLCOMPILESHADERPROC) SDL_GL_GetProcAddress("glCompileShader");
		glGetShaderiv = (PFNGLGETSHADERIVPROC) SDL_GL_GetProcAddress("glGetShaderiv");
		glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) SDL_GL_GetProcAddress("glGetShaderInfoLog");
		glCreateProgram = (PFNGLCREATEPROGRAMPROC) SDL_GL_GetProcAddress("glCreateProgram");
		glAttachShader = (PFNGLATTACHSHADERPROC) SDL_GL_GetProcAddress("glAttachShader");
		glLinkProgram = (PFNGLLINKPROGRAMPROC) SDL_GL_GetProcAddress("glLinkProgram");
		glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SDL_GL_GetProcAddress("glGetProgramiv");
		glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SDL_GL_GetProcAddress("glGetProgramInfoLog");
		glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) SDL_GL_GetProcAddress("glValidateProgram");
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SDL_GL_GetProcAddress("glGetUniformLocation");
		glUniform1f = (PFNGLUNIFORM1FPROC) SDL_GL_GetProcAddress("glUniform1f");
		glUniform1i = (PFNGLUNIFORM1IPROC) SDL_GL_GetProcAddress("glUniform1i");
		glUseProgram = (PFNGLUSEPROGRAMPROC) SDL_GL_GetProcAddress("glUseProgram");
#endif
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &s_num_compressed_format);
	}

	void set_antialiased(bool enable)
	{
		// first try hardware FSAA (full screen antialiasing)
		int aa_samples;

#if TU_USE_SDL == 1
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &aa_samples);
#else
		// Here is where you have to inset the alternative to SDL
		aa_samples=0;
#endif

		if (aa_samples > 0)
		{
			if (enable)
			{
				glEnable(GL_MULTISAMPLE_ARB);
			}
			else
			{
				glDisable(GL_MULTISAMPLE_ARB);
			}
			return;
		}

		// there are no hardware antialiasing
		// use edge antialiasing
		m_enable_antialias = enable;
	}

	static void make_next_miplevel(int* width, int* height, Uint8* data)
	// Utility.  Mutates *width, *height and *data to create the
	// next mip level.
	{
		assert(width);
		assert(height);
		assert(data);

		int	new_w = *width >> 1;
		int	new_h = *height >> 1;
		if (new_w < 1) new_w = 1;
		if (new_h < 1) new_h = 1;
		
		if (new_w * 2 != *width	 || new_h * 2 != *height)
		{
			// Image can't be shrunk along (at least) one
			// of its dimensions, so don't bother
			// resampling.	Technically we should, but
			// it's pretty useless at this point.  Just
			// change the image dimensions and leave the
			// existing pixels.
		}
		else
		{
			// Resample.  Simple average 2x2 --> 1, in-place.
			for (int j = 0; j < new_h; j++) {
				Uint8*	out = ((Uint8*) data) + j * new_w;
				Uint8*	in = ((Uint8*) data) + (j << 1) * *width;
				for (int i = 0; i < new_w; i++) {
					int	a;
					a = (*(in + 0) + *(in + 1) + *(in + 0 + *width) + *(in + 1 + *width));
					*(out) = a >> 2;
					out++;
					in += 2;
				}
			}
		}

		// Munge parameters to reflect the shrunken image.
		*width = new_w;
		*height = new_h;
	}
	
	struct fill_style
	{
		enum mode
		{
			INVALID,
			COLOR,
			BITMAP_WRAP,
			BITMAP_CLAMP,
			LINEAR_GRADIENT,
			RADIAL_GRADIENT,
		};
		mode	m_mode;
		gameswf::rgba	m_color;
		gameswf::bitmap_info*	m_bitmap_info;
		gameswf::matrix	m_bitmap_matrix;
		gameswf::cxform	m_bitmap_color_transform;
		bool	m_has_nonzero_bitmap_additive_color;
		float m_width;	// for line style
		
		fill_style() :
			m_mode(INVALID),
			m_has_nonzero_bitmap_additive_color(false)
		{
		}

		void	apply(/*const matrix& current_matrix*/) const
		// Push our style into OpenGL.
		{
			assert(m_mode != INVALID);
			
			if (m_mode == COLOR)
			{
				apply_color(m_color);
				glDisable(GL_TEXTURE_2D);
			}
			else
			if (m_mode == BITMAP_WRAP || m_mode == BITMAP_CLAMP)
			{
				assert(m_bitmap_info != NULL);

				apply_color(m_color);

				if (m_bitmap_info == NULL)
				{
					glDisable(GL_TEXTURE_2D);
				}
				else
				{
					// Set up the texture for rendering.
					{
						// Do the modulate part of the color
						// transform in the first pass.  The
						// additive part, if any, needs to
						// happen in a second pass.
						glColor4f(m_bitmap_color_transform.m_[0][0],
							  m_bitmap_color_transform.m_[1][0],
							  m_bitmap_color_transform.m_[2][0],
							  m_bitmap_color_transform.m_[3][0]
							  );
					}

					m_bitmap_info->layout();
//					glBindTexture(GL_TEXTURE_2D, m_bitmap_info->m_texture_id);
//					glEnable(GL_TEXTURE_2D);
					glEnable(GL_TEXTURE_GEN_S);
					glEnable(GL_TEXTURE_GEN_T);
				
					if (m_mode == BITMAP_CLAMP)
					{	
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					}
					else
					{
						assert(m_mode == BITMAP_WRAP);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					}

					// Set up the bitmap matrix for texgen.

					float	inv_width = 1.0f / m_bitmap_info->get_width();
					float	inv_height = 1.0f / m_bitmap_info->get_height();

					const gameswf::matrix&	m = m_bitmap_matrix;
					glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
					float	p[4] = { 0, 0, 0, 0 };
					p[0] = m.m_[0][0] * inv_width;
					p[1] = m.m_[0][1] * inv_width;
					p[3] = m.m_[0][2] * inv_width;
					glTexGenfv(GL_S, GL_OBJECT_PLANE, p);

					glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
					p[0] = m.m_[1][0] * inv_height;
					p[1] = m.m_[1][1] * inv_height;
					p[3] = m.m_[1][2] * inv_height;
					glTexGenfv(GL_T, GL_OBJECT_PLANE, p);
				}
			}
		}


		bool	needs_second_pass() const
		// Return true if we need to do a second pass to make
		// a valid color.  This is for cxforms with additive
		// parts; this is the simplest way (that we know of)
		// to implement an additive color with stock OpenGL.
		{
			if (m_mode == BITMAP_WRAP
			    || m_mode == BITMAP_CLAMP)
			{
				return m_has_nonzero_bitmap_additive_color;
			}
			else
			{
				return false;
			}
		}

		void	apply_second_pass() const
		// Set OpenGL state for a necessary second pass.
		{
			assert(needs_second_pass());

			// The additive color also seems to be modulated by the texture. So,
			// maybe we can fake this in one pass using using the mean value of 
			// the colors: c0*t+c1*t = ((c0+c1)/2) * t*2
			// I don't know what the alpha component of the color is for.
			//glDisable(GL_TEXTURE_2D);

			glColor4f(
				m_bitmap_color_transform.m_[0][1] / 255.0f,
				m_bitmap_color_transform.m_[1][1] / 255.0f,
				m_bitmap_color_transform.m_[2][1] / 255.0f,
				m_bitmap_color_transform.m_[3][1] / 255.0f
				);

			glBlendFunc(GL_ONE, GL_ONE);
		}

		void	cleanup_second_pass() const
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}


		void	disable() { m_mode = INVALID; }
		void	set_color(gameswf::rgba color) { m_mode = COLOR; m_color = color; }
		void	set_bitmap(gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm, const gameswf::cxform& color_transform)
		{
			m_mode = (wm == WRAP_REPEAT) ? BITMAP_WRAP : BITMAP_CLAMP;
			m_bitmap_info = bi;
			m_bitmap_matrix = m;
			m_bitmap_color_transform = color_transform;
			m_bitmap_color_transform.clamp();

			m_color = gameswf::rgba(
				Uint8(m_bitmap_color_transform.m_[0][0] * 255.0f), 
				Uint8(m_bitmap_color_transform.m_[1][0] * 255.0f), 
				Uint8(m_bitmap_color_transform.m_[2][0] * 255.0f), 
				Uint8(m_bitmap_color_transform.m_[3][0] * 255.0f));

			if (m_bitmap_color_transform.m_[0][1] > 1.0f
			    || m_bitmap_color_transform.m_[1][1] > 1.0f
			    || m_bitmap_color_transform.m_[2][1] > 1.0f
			    || m_bitmap_color_transform.m_[3][1] > 1.0f)
			{
				m_has_nonzero_bitmap_additive_color = true;
			}
			else
			{
				m_has_nonzero_bitmap_additive_color = false;
			}
		}
		bool	is_valid() const { return m_mode != INVALID; }
	};


	// Style state.
	enum style_index
	{
		LEFT_STYLE = 0,
		RIGHT_STYLE,
		LINE_STYLE,

		STYLE_COUNT
	};
	fill_style	m_current_styles[STYLE_COUNT];


	gameswf::bitmap_info*	create_bitmap_info_rgb(image::rgb* im)
	// Given an image, returns a pointer to a bitmap_info struct
	// that can later be passed to fill_styleX_bitmap(), to set a
	// bitmap fill style.
	{
		return new bitmap_info_ogl(im);
	}


	gameswf::bitmap_info*	create_bitmap_info_rgba(image::rgba* im)
	// Given an image, returns a pointer to a bitmap_info struct
	// that can later be passed to fill_style_bitmap(), to set a
	// bitmap fill style.
	//
	// This version takes an image with an alpha channel.
	{
		return new bitmap_info_ogl(im);
	}


	gameswf::bitmap_info*	create_bitmap_info_empty()
	// Create a placeholder bitmap_info.  Used when
	// DO_NOT_LOAD_BITMAPS is set; then later on the host program
	// can use movie_definition::get_bitmap_info_count() and
	// movie_definition::get_bitmap_info() to stuff precomputed
	// textures into these bitmap infos.
	{
		return new bitmap_info_ogl;
	}

	gameswf::bitmap_info*	create_bitmap_info_alpha(int w, int h, Uint8* data)
	// Create a bitmap_info so that it contains an alpha texture
	// with the given data (1 byte per texel).
	//
	// Munges *data (in order to make mipmaps)!!
	{
		return new bitmap_info_ogl(w, h, data);
	}

	gameswf::video_handler*	create_video_handler()
	{
		return new video_handler_ogl();
	}

	void	begin_display(
		gameswf::rgba background_color,
		int viewport_x0, int viewport_y0,
		int viewport_width, int viewport_height,
		float x0, float x1, float y0, float y1)
	// Set up to render a full frame from a movie and fills the
	// background.	Sets up necessary transforms, to scale the
	// movie to fit within the given dimensions.  Call
	// end_display() when you're done.
	//
	// The rectangle (viewport_x0, viewport_y0, viewport_x0 +
	// viewport_width, viewport_y0 + viewport_height) defines the
	// window coordinates taken up by the movie.
	//
	// The rectangle (x0, y0, x1, y1) defines the pixel
	// coordinates of the movie that correspond to the viewport
	// bounds.
	{
		m_display_width = fabsf(x1 - x0);
		m_display_height = fabsf(y1 - y0);

		glViewport(viewport_x0, viewport_y0, viewport_width, viewport_height);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glOrtho(x0, x1, y0, y1, -1, 1);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// GL_MODULATE

		glDisable(GL_TEXTURE_2D);

		// Clear the background, if background color has alpha > 0.
		if (background_color.m_a > 0)
		{
			// Draw a big quad.
			apply_color(background_color);
			glBegin(GL_QUADS);
			glVertex2f(x0, y0);
			glVertex2f(x1, y0);
			glVertex2f(x1, y1);
			glVertex2f(x0, y1);
			glEnd();
		}

		// Old unused code.  Might get revived someday.
		#if 0
				// See if we want to, and can, use multitexture
				// antialiasing.
				s_multitexture_antialias = false;
				if (m_enable_antialias)
				{
					int	tex_units = 0;
					glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &tex_units);
					if (tex_units >= 2)
					{
						s_multitexture_antialias = true;
					}

					// Make sure we have an edge texture available.
					if (s_multitexture_antialias == true
						&& s_edge_texture_id == 0)
					{
						// Very simple texture: 2 texels wide, 1 texel high.
						// Both texels are white; left texel is all clear, right texel is all opaque.
						unsigned char	edge_data[8] = { 255, 255, 255, 0, 255, 255, 255, 255 };

						ogl::active_texture(GL_TEXTURE1_ARB);
						glEnable(GL_TEXTURE_2D);
						glGenTextures(1, &s_edge_texture_id);
						glBindTexture(GL_TEXTURE_2D, s_edge_texture_id);

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, edge_data);

						glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// @@ should we use a 1D texture???

						glDisable(GL_TEXTURE_2D);
						ogl::active_texture(GL_TEXTURE0_ARB);
						glDisable(GL_TEXTURE_2D);
					}
				}
		#endif // 0
	}


	void	end_display()
	// Clean up after rendering a frame.  Client program is still
	// responsible for calling glSwapBuffers() or whatever.
	{
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}


	void	set_matrix(const gameswf::matrix& m)
	// Set the current transform for mesh & line-strip rendering.
	{
		m_current_matrix = m;
	}


	void	set_cxform(const gameswf::cxform& cx)
	// Set the current color transform for mesh & line-strip rendering.
	{
		m_current_cxform = cx;
	}
	
	static void	apply_matrix(const gameswf::matrix& m)
	// multiply current matrix with opengl matrix
	{
		float	mat[16];
		memset(&mat[0], 0, sizeof(mat));
		mat[0] = m.m_[0][0];
		mat[1] = m.m_[1][0];
		mat[4] = m.m_[0][1];
		mat[5] = m.m_[1][1];
		mat[10] = 1;
		mat[12] = m.m_[0][2];
		mat[13] = m.m_[1][2];
		mat[15] = 1;
		glMultMatrixf(mat);
	}

	static void	apply_color(const gameswf::rgba& c)
	// Set the given color.
	{
		glColor4ub(c.m_r, c.m_g, c.m_b, c.m_a);
	}

	void	fill_style_disable(int fill_side)
	// Don't fill on the {0 == left, 1 == right} side of a path.
	{
		assert(fill_side >= 0 && fill_side < 2);

		m_current_styles[fill_side].disable();
	}


	void	line_style_disable()
	// Don't draw a line on this path.
	{
		m_current_styles[LINE_STYLE].disable();
	}


	void	fill_style_color(int fill_side, const gameswf::rgba& color)
	// Set fill style for the left interior of the shape.  If
	// enable is false, turn off fill for the left interior.
	{
		assert(fill_side >= 0 && fill_side < 2);

		m_current_styles[fill_side].set_color(m_current_cxform.transform(color));
	}


	void	line_style_color(gameswf::rgba color)
	// Set the line style of the shape.  If enable is false, turn
	// off lines for following curve segments.
	{
		m_current_styles[LINE_STYLE].set_color(m_current_cxform.transform(color));
	}


	void	fill_style_bitmap(int fill_side, gameswf::bitmap_info* bi, const gameswf::matrix& m,
		bitmap_wrap_mode wm, bitmap_blend_mode bm)
	{
		assert(fill_side >= 0 && fill_side < 2);
		m_current_styles[fill_side].set_bitmap(bi, m, wm, m_current_cxform);
	}
	
	void	line_style_width(float width)
	{
		m_current_styles[LINE_STYLE].m_width = width;
	}


	void	draw_mesh_primitive(int primitive_type, const void* coords, int vertex_count)
	// Helper for draw_mesh_strip and draw_triangle_list.
	{
#define NORMAL_RENDERING
//#define MULTIPASS_ANTIALIASING

#ifdef NORMAL_RENDERING
		// Set up current style.
		m_current_styles[LEFT_STYLE].apply();

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		apply_matrix(m_current_matrix);

		// Send the tris to OpenGL
		glEnableClientState(GL_VERTEX_ARRAY);

		#if TU_USES_FLOAT_AS_COORDINATE_COMPONENT
			glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, coords);
		#else
			glVertexPointer(2, GL_SHORT, sizeof(Sint16) * 2, coords);
		#endif

		glDrawArrays(primitive_type, 0, vertex_count);

		if (m_current_styles[LEFT_STYLE].needs_second_pass())
		{
			m_current_styles[LEFT_STYLE].apply_second_pass();
			glDrawArrays(primitive_type, 0, vertex_count);
			m_current_styles[LEFT_STYLE].cleanup_second_pass();
		}

		// the antialiasing of polygon edges
		if (m_enable_antialias)
		{
			glEnable(GL_POLYGON_SMOOTH);
			glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);	// GL_NICEST, GL_FASTEST, GL_DONT_CARE
			glDrawArrays(primitive_type, 0, vertex_count);
			glDisable(GL_POLYGON_SMOOTH);
		}

		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();
#endif // NORMAL_RENDERING

#ifdef MULTIPASS_ANTIALIASING
		// So this approach basically works.  This
		// implementation is not totally finished; two pass
		// materials (i.e. w/ additive color) aren't correct,
		// and there are some texture etc issues because I'm
		// just hosing state uncarefully here.  It needs the
		// optimization of only filling the bounding box of
		// the shape.  You must have destination alpha.
		//
		// It doesn't look quite perfect on my GF4.  For one
		// thing, you kinda want to crank down the max curve
		// subdivision error, because suddenly you can see
		// sub-pixel shape much better.  For another thing,
		// the antialiasing isn't quite perfect, to my eye.
		// It could be limited alpha precision, imperfections
		// GL_POLYGON_SMOOTH, and/or my imagination.

		glDisable(GL_TEXTURE_2D);

		glEnable(GL_POLYGON_SMOOTH);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);	// GL_NICEST, GL_FASTEST, GL_DONT_CARE

		// Clear destination alpha.
		//
		// @@ TODO Instead of drawing this huge screen-filling
		// quad, we should take a bounding-box param from the
		// caller, and draw the box (after apply_matrix;
		// i.e. the box is in object space).  The point being,
		// to only fill the part of the screen that the shape
		// is in.
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glColor4f(1, 1, 1, 0);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(100000, 0);
		glVertex2f(100000, 100000);
		glVertex2f(0, 100000);
		glEnd();

		// Set mode for drawing alpha mask.
		glBlendFunc(GL_ONE, GL_ONE);	// additive blending
		glColor4f(0, 0, 0, m_current_styles[LEFT_STYLE].m_color.m_a / 255.0f);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		apply_matrix(m_current_matrix);

		// Send the tris to OpenGL.  This produces an
		// antialiased alpha mask of the mesh shape, in the
		// destination alpha channel.
		glEnableClientState(GL_VERTEX_ARRAY);
		#if TU_USES_FLOAT_AS_COORDINATE_COMPONENT
			glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, coords);
		#else
			glVertexPointer(2, GL_SHORT, sizeof(Sint16) * 2, coords);
		#endif
		glDrawArrays(primitive_type, 0, vertex_count);
		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();
		
		// Set up desired fill style.
		m_current_styles[LEFT_STYLE].apply();

		// Apply fill, modulated with alpha mask.
		//
		// @@ TODO see note above about filling bounding box only.
		glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(100000, 0);
		glVertex2f(100000, 100000);
		glVertex2f(0, 100000);
		glEnd();

// xxxxx ??? Hm, is our mask still intact, or did we just erase it?
// 		if (m_current_styles[LEFT_STYLE].needs_second_pass())
// 		{
// 			m_current_styles[LEFT_STYLE].apply_second_pass();
// 			glDrawArrays(primitive_type, 0, vertex_count);
// 			m_current_styles[LEFT_STYLE].cleanup_second_pass();
// 		}

		// @@ hm, there is perhaps more state that needs
		// fixing here, or setting elsewhere.
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif // MULTIPASS_ANTIALIASING
	}

	void draw_mesh_strip(const void* coords, int vertex_count)
	{
		draw_mesh_primitive(GL_TRIANGLE_STRIP, coords, vertex_count);
	}
			
	void	draw_triangle_list(const void* coords, int vertex_count)
	{
		draw_mesh_primitive(GL_TRIANGLES, coords, vertex_count);
	}


	void	draw_line_strip(const void* coords, int vertex_count)
	// Draw the line strip formed by the sequence of points.
	{
		// Set up current style.
		m_current_styles[LINE_STYLE].apply();

		// apply line width

		float scale = fabsf(m_current_matrix.get_x_scale()) + fabsf(m_current_matrix.get_y_scale());
		float w = m_current_styles[LINE_STYLE].m_width * scale / 2.0f;
    w = TWIPS_TO_PIXELS(w);

		GLfloat width_info[2];
		glGetFloatv(GL_LINE_WIDTH_RANGE, width_info); 
//		if (w > width_info[1])
//		{
//			printf("Your OpenGL implementation does not support the line width"
//				" requested. Lines will be drawn with reduced width.");
//		}

		glLineWidth(w <= 1.0f ? 1.0f : w);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		apply_matrix(m_current_matrix);

		// Send the line-strip to OpenGL
		glEnableClientState(GL_VERTEX_ARRAY);
		#if TU_USES_FLOAT_AS_COORDINATE_COMPONENT
			glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, coords);
		#else
			glVertexPointer(2, GL_SHORT, sizeof(Sint16) * 2, coords);
		#endif
		glDrawArrays(GL_LINE_STRIP, 0, vertex_count);

    // Draw a round dot on the beginning and end coordinates to lines.
    glPointSize(w);
		glEnable(GL_POINT_SMOOTH);
    glDrawArrays(GL_POINTS, 0, vertex_count);
    glDisable(GL_POINT_SMOOTH);
    glPointSize(1);

		glDisableClientState(GL_VERTEX_ARRAY);

		// restore defaults
		glPointSize(1);
    glLineWidth(1);

		glPopMatrix();
	}


	void	draw_bitmap(
		const gameswf::matrix& m,
		gameswf::bitmap_info* bi,
		const gameswf::rect& coords,
		const gameswf::rect& uv_coords,
		gameswf::rgba color)
	// Draw a rectangle textured with the given bitmap, with the
	// given color.	 Apply given transform; ignore any currently
	// set transforms.
	//
	// Intended for textured glyph rendering.
	{
		assert(bi);

		apply_color(color);

		gameswf::point a, b, c, d;
		m.transform(&a, gameswf::point(coords.m_x_min, coords.m_y_min));
		m.transform(&b, gameswf::point(coords.m_x_max, coords.m_y_min));
		m.transform(&c, gameswf::point(coords.m_x_min, coords.m_y_max));
		d.m_x = b.m_x + c.m_x - a.m_x;
		d.m_y = b.m_y + c.m_y - a.m_y;

		bi->layout();
//		glBindTexture(GL_TEXTURE_2D, bi->m_texture_id);
//		glEnable(GL_TEXTURE_2D);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);

		glBegin(GL_TRIANGLE_STRIP);

		glTexCoord2f(uv_coords.m_x_min, uv_coords.m_y_min);
		glVertex2f(a.m_x, a.m_y);

		glTexCoord2f(uv_coords.m_x_max, uv_coords.m_y_min);
		glVertex2f(b.m_x, b.m_y);

		glTexCoord2f(uv_coords.m_x_min, uv_coords.m_y_max);
		glVertex2f(c.m_x, c.m_y);

		glTexCoord2f(uv_coords.m_x_max, uv_coords.m_y_max);
		glVertex2f(d.m_x, d.m_y);

		glEnd();
	}
	
	bool test_stencil_buffer(const gameswf::rect& bound, Uint8 pattern)
	{
		// get viewport size
		GLint vp[4]; 
		glGetIntegerv(GL_VIEWPORT, vp); 
		int vp_width = vp[2];
		int vp_height = vp[3];

		bool ret = false;

		int x0 = (int) bound.m_x_min;
		int y0 = (int) bound.m_y_min;
		int width = (int) bound.m_x_max - x0;
		int height = (int) bound.m_y_max - y0;

		if (width > 0 && height > 0 &&
			x0 >= 0 && x0 + width <= vp_width &&
			y0 >= 0 && y0 + height <= vp_height)
		{
			int bufsize = width * height;
			Uint8* buf = (Uint8*) malloc(4 * bufsize);

			glReadPixels(x0, vp[3] - y0 - height, width, height, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buf);

			for (int i = 0; i < bufsize; i++)
			{
				if (buf[i] == pattern)
				{
					ret = true;
					break;
				}
			}

			free(buf);
		}

		return ret;
	}

	void begin_submit_mask()
	{
		if (m_mask_level == 0)
		{
			assert(glIsEnabled(GL_STENCIL_TEST) == false);
			glEnable(GL_STENCIL_TEST);
			glClearStencil(0);
			glClear(GL_STENCIL_BUFFER_BIT);
		}

		// disable framebuffer writes
		glColorMask(0, 0, 0, 0);

		// we set the stencil buffer to 'm_mask_level+1' 
		// where we draw any polygon and stencil buffer is 'm_mask_level'
		glStencilFunc(GL_EQUAL, m_mask_level++, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); 
	}

	// called after begin_submit_mask and the drawing of mask polygons
	void end_submit_mask()
	{	     
		// enable framebuffer writes
		glColorMask(1, 1, 1, 1);

		// we draw only where the stencil is m_mask_level (where the current mask was drawn)
		glStencilFunc(GL_EQUAL, m_mask_level, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	
	}

	void disable_mask()
	{	     
		assert(m_mask_level > 0);
		if (--m_mask_level == 0)
		{
			glDisable(GL_STENCIL_TEST); 
			return;
		}

		// begin submit previous mask

		glColorMask(0, 0, 0, 0);

		// we set the stencil buffer to 'm_mask_level' 
		// where the stencil buffer m_mask_level + 1
		glStencilFunc(GL_EQUAL, m_mask_level + 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR); 

		// draw the quad to fill stencil buffer
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(m_display_width, 0);
		glVertex2f(m_display_width, m_display_height);
		glVertex2f(0, m_display_height);
		glEnd();

		end_submit_mask();
	}

	bool is_visible(const gameswf::rect& bound)
	{
		gameswf::rect viewport;
		viewport.m_x_min = 0;
		viewport.m_y_min = 0;
		viewport.m_x_max = m_display_width;
		viewport.m_y_max = m_display_height;
		return viewport.bound_test(bound);
	}

#ifdef SDL_CURSOR_HANDLING
	// SDL cursor handling.
	sdl_cursor_handler m_cursor_handler;
	
	void set_cursor(cursor_type cursor)
	{
		m_cursor_handler.set_cursor(cursor);
	}
#endif  // SDL_CURSOR_HANDLING

};	// end struct render_handler_ogl


// bitmap_info_ogl implementation


#if (RESAMPLE_METHOD == 1)

void	hardware_resample(int bytes_per_pixel, int src_width, int src_height, uint8* src_data, int dst_width, int dst_height)
// Code from Alex Streit
//
// Sets the current texture to a resampled/expanded version of the
// given image data.
{
	assert(bytes_per_pixel == 3 || bytes_per_pixel == 4);

	unsigned int	in_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;
	unsigned int	out_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;

	// alex: use the hardware to resample the image
	// issue: does not work when image > allocated window size!
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT);
	{
		char* temp = new char[dst_width * dst_height * bytes_per_pixel];
		//memset(temp,255,w*h*3);
		glTexImage2D(GL_TEXTURE_2D, 0, in_format, dst_width, dst_height, 0, out_format, GL_UNSIGNED_BYTE, temp);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, src_width, src_height, out_format, GL_UNSIGNED_BYTE, src_data);

		glLoadIdentity();
		glViewport(0, 0, dst_width, dst_height);
		glOrtho(0, dst_width, 0, dst_height, 0.9, 1.1);
		glColor3f(1, 1, 1);
		glNormal3f(0, 0, 1);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0, (float) src_height / dst_height);
			glVertex3f(0, 0, -1);
			glTexCoord2f( (float) src_width / dst_width, (float) src_height / dst_height);
			glVertex3f((float) dst_width, 0, -1);
			glTexCoord2f( (float) src_width / dst_width, 0);
			glVertex3f((float) dst_width, (float) dst_height, -1);
			glTexCoord2f(0, 0);
			glVertex3f(0, (float) dst_height, -1);
		}
		glEnd();
		glCopyTexImage2D(GL_TEXTURE_2D, 0, out_format, 0,0, dst_width, dst_height, 0);
		delete temp;
	}
	glPopAttrib();
	glPopMatrix();
	glPopMatrix();
}

#endif

void	generate_mipmaps(unsigned int internal_format, unsigned int input_format, int bytes_per_pixel, image::image_base* im)
// DESTRUCTIVELY generate mipmaps of the given image.  The image data
// and width/height of im are munged in this process.
{
	int	level = 1;
	while (im->m_width > 1 || im->m_height > 1)
	{
		if (bytes_per_pixel == 3)
		{
			image::make_next_miplevel((image::rgb*) im);
		}
		else
		{
			image::make_next_miplevel((image::rgba*) im);
		}

//		glTexImage2D(GL_TEXTURE_2D, level, internal_format, im->m_width, im->m_height, 0,
//			     input_format, GL_UNSIGNED_BYTE, im->m_data);
		create_texture(input_format, im->m_width, im->m_height, im->m_data, level);
		level++;
	}
}


void	software_resample(
	int bytes_per_pixel,
	int src_width,
	int src_height,
	int src_pitch,
	uint8* src_data,
	int dst_width,
	int dst_height)
// Code from Alex Streit
//
// Creates an OpenGL texture of the specified dst dimensions, from a
// resampled version of the given src image.  Does a bilinear
// resampling to create the dst image.
{
//	printf("original bitmap %dx%d, resampled bitmap %dx%d\n",
//		src_width, src_height, dst_width, dst_height);

	assert(bytes_per_pixel == 3 || bytes_per_pixel == 4);

//	assert(dst_width >= src_width);
//	assert(dst_height >= src_height);

//	unsigned int	internal_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;
	unsigned int	input_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;

	// FAST bi-linear filtering
	// the code here is designed to be fast, not readable
	Uint8* rescaled = new Uint8[dst_width * dst_height * bytes_per_pixel];
	float Uf, Vf;		// fractional parts
	float Ui, Vi;		// integral parts
	float w1, w2, w3, w4;	// weighting
	Uint8* psrc;
	Uint8* pdst = rescaled;

	// i1,i2,i3,i4 are the offsets of the surrounding 4 pixels
	const int i1 = 0;
	const int i2 = bytes_per_pixel;
	int i3 = src_pitch;
	int i4 = src_pitch + bytes_per_pixel;

	// change in source u and v
	float dv = (float)(src_height - 2) / dst_height;
	float du = (float)(src_width - 2) / dst_width;

	// source u and source v
	float U;
	float V = 0;

#define BYTE_SAMPLE(offset)	\
	(Uint8) (w1 * psrc[i1 + (offset)] + w2 * psrc[i2 + (offset)] + w3 * psrc[i3 + (offset)] + w4 * psrc[i4 + (offset)])

	if (bytes_per_pixel == 3)
	{
		for (int v = 0; v < dst_height; ++v)
		{
			Vf = modff(V, &Vi);
			V += dv;
			U = 0;

			for (int u = 0; u < dst_width; ++u)
			{
				Uf = modff(U, &Ui);
				U += du;

				w1 = (1 - Uf) * (1 - Vf);
				w2 = Uf * (1 - Vf);
				w3 = (1 - Uf) * Vf;
				w4 = Uf * Vf;
				psrc = &src_data[(int) (Vi * src_pitch) + (int) (Ui * bytes_per_pixel)];

				*pdst++ = BYTE_SAMPLE(0);	// red
				*pdst++ = BYTE_SAMPLE(1);	// green
				*pdst++ = BYTE_SAMPLE(2);	// blue

				psrc += 3;
			}
		}

#ifdef DEBUG_WRITE_TEXTURES_TO_PPM
		static int s_image_sequence = 0;
		char temp[256];
		sprintf(temp, "image%d.ppm", s_image_sequence++);
		FILE* f = fopen(temp, "wb");
		if (f)
		{
			fprintf(f, "P6\n# test code\n%d %d\n255\n", dst_width, dst_height);
			fwrite(rescaled, dst_width * dst_height * 3, 1, f);
			fclose(f);
		}
#endif
	}
	else
	{
		assert(bytes_per_pixel == 4);

		for (int v = 0; v < dst_height; ++v)
		{
			Vf = modff(V, &Vi);
			V += dv;
			U = 0;

			for (int u = 0; u < dst_width; ++u)
			{
				Uf = modff(U, &Ui);
				U += du;

				w1 = (1 - Uf) * (1 - Vf);
				w2 = Uf * (1 - Vf);
				w3 = (1 - Uf) * Vf;
				w4 = Uf * Vf;
				psrc = &src_data[(int) (Vi * src_pitch) + (int) (Ui * bytes_per_pixel)];

				*pdst++ = BYTE_SAMPLE(0);	// red
				*pdst++ = BYTE_SAMPLE(1);	// green
				*pdst++ = BYTE_SAMPLE(2);	// blue
				*pdst++ = BYTE_SAMPLE(3);	// alpha

				psrc += 4;
			}
		}
	}

//	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, dst_width, dst_height, 0, input_format, GL_UNSIGNED_BYTE, rescaled);
	create_texture(input_format, dst_width, dst_height, rescaled, 0);

#if GENERATE_MIPMAPS
	// Build mipmaps.
	image::image_base	im(rescaled, dst_width, dst_height, dst_width * bytes_per_pixel);
	generate_mipmaps(internal_format, input_format, bytes_per_pixel, &im);
#endif // GENERATE_MIPMAPS

	delete [] rescaled;
}

bitmap_info_ogl::bitmap_info_ogl() :
	m_texture_id(0),
	m_width(0),
	m_height(0),
	m_suspended_image(0)
{
}

bitmap_info_ogl::bitmap_info_ogl(image::rgba* im) :
	m_texture_id(0),
	m_width(im->m_width),
	m_height(im->m_height)
{
	assert(im);
	m_suspended_image = image::create_rgba(im->m_width, im->m_height);
	memcpy(m_suspended_image->m_data, im->m_data, im->m_pitch * im->m_height);
}

bitmap_info_ogl::bitmap_info_ogl(int width, int height, Uint8* data) :
	m_texture_id(0),
	m_width(width),
	m_height(height)
{
	assert(width > 0 && height > 0 && data);
	m_suspended_image = image::create_alpha(width, height);
	memcpy(m_suspended_image->m_data, data, m_suspended_image->m_pitch * m_suspended_image->m_height);
}

bitmap_info_ogl::bitmap_info_ogl(image::rgb* im) :
	m_texture_id(0),
	m_width(im->m_width),
	m_height(im->m_height)
{
	assert(im);
	m_suspended_image = image::create_rgb(im->m_width, im->m_height);
	memcpy(m_suspended_image->m_data, im->m_data, im->m_pitch * im->m_height);
}

// layout image to opengl texture memory
void bitmap_info_ogl::layout()
{
	if (m_texture_id == 0)
	{
		assert(m_suspended_image);

		// Create the texture.
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, (GLuint*) &m_texture_id);
		glBindTexture(GL_TEXTURE_2D, m_texture_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// GL_NEAREST ?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		m_width = m_suspended_image->m_width;
		m_height = m_suspended_image->m_height;

		int bpp = 4;
		int format = GL_RGBA;

		switch (m_suspended_image->m_type)
		{
			case image::image_base::RGB:
			{
				bpp = 3;
				format = GL_RGB;
			}

			case image::image_base::RGBA:
			{
				int	w = p2(m_suspended_image->m_width);
				int	h = p2(m_suspended_image->m_height);
				if (w != m_suspended_image->m_width || h != m_suspended_image->m_height)
				{
					// Faster/simpler software bilinear rescale.
					software_resample(bpp, m_suspended_image->m_width, m_suspended_image->m_height,
						m_suspended_image->m_pitch, m_suspended_image->m_data, w, h);
				}
				else
				{
					// Use original image directly.
					create_texture(format, w, h, m_suspended_image->m_data, 0);
				}
				break;
			}

			case image::image_base::ALPHA:
			{
#if GENERATE_MIPMAPS
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#endif
				int	w = m_suspended_image->m_width;
				int	h = m_suspended_image->m_height;
				create_texture(GL_ALPHA, w, h, m_suspended_image->m_data, 0);

#if GENERATE_MIPMAPS
				// Build mips.
				int	level = 1;
				while (w > 1 || h > 1)
				{
					render_handler_ogl::make_next_miplevel(&w, &h, m_suspended_image->m_data);
					create_texture(GL_ALPHA, w, h, m_suspended_image->m_data, level);
					level++;
				}
#endif

				break;
			}

			default:
				assert(0);
		}

		delete m_suspended_image;
		m_suspended_image = NULL;
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
		glEnable(GL_TEXTURE_2D);
	}
}

gameswf::render_handler*	gameswf::create_render_handler_ogl()
// Factory.
{
	return new render_handler_ogl;
}

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:

