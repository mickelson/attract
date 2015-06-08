// gameswf_render_handler_xbox.cpp	-- Thatcher Ulrich <http://tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A gameswf::render_handler that uses Xbox API's


#ifdef _XBOX

#include "gameswf/gameswf.h"
#include "gameswf/gameswf_log.h"
#include "gameswf/gameswf_types.h"
#include "base/image.h"
#include "base/container.h"

#include <xtl.h>
#include <d3d8.h>

#include <string.h>


namespace
{
	array<IDirect3DBaseTexture8*>	s_d3d_textures;
	DWORD	s_vshader_handle = 0;

	// Our vertex coords consist of two signed 16-bit integers, for (x,y) position only.
	DWORD	s_vshader_decl[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(0, D3DVSDT_SHORT2),
		0xFFFFFFFF
	};


	void	init_vshader()
	// Initialize the vshader we use for SWF mesh rendering.
	{
		if (s_vshader_handle == 0)
		{
			HRESULT	result;

			/* Shader source:

				xvs.1.1
				#pragma screenspace
				mov	oD0, v3
				dp4 oPos.x, v0, c[0]	// Transform position
				dp4 oPos.y, v0, c[1]
				dp4 oPos.z, v0, c[2]
				dp4 oPos.w, v0, c[3]
				dp4 oT0.x, v0, c[4]	// texgen
				dp4 oT0.y, v0, c[5]

			   Compile that with xsasm -h sometmp.vsh, and insert the contents of sometmp.h below:
			*/
			static const DWORD	s_compiled_shader[] =
			{
				0x00072078,
				0x00000000, 0x0020061b, 0x0836106c, 0x2070f818,
				0x00000000, 0x00ec001b, 0x0836186c, 0x20708800,
				0x00000000, 0x00ec201b, 0x0836186c, 0x20704800,
				0x00000000, 0x00ec401b, 0x0836186c, 0x20702800,
				0x00000000, 0x00ec601b, 0x0836186c, 0x20701800,
				0x00000000, 0x00ec801b, 0x0836186c, 0x20708848,
				0x00000000, 0x00eca01b, 0x0836186c, 0x20704849
			};

			result = IDirect3DDevice8::CreateVertexShader(
				s_vshader_decl,
				s_compiled_shader,
				&s_vshader_handle,
				D3DUSAGE_PERSISTENTDIFFUSE);
			if (result != S_OK)
			{
				gameswf::log_error("error: can't create Xbox vshader; error code = %d\n", result);
				return;
			}
		}
	}
};


// bitmap_info_xbox declaration
struct bitmap_info_xbox : public gameswf::bitmap_info
{
	bitmap_info_xbox(create_empty e);
	bitmap_info_xbox(image::rgb* im);
	bitmap_info_xbox(image::rgba* im);
	virtual void set_alpha_image(int width, int height, Uint8* data);
};


struct render_handler_xbox : public gameswf::render_handler
{
	// Some renderer state.

	gameswf::matrix	m_viewport_matrix;
	gameswf::matrix	m_current_matrix;
	gameswf::cxform	m_current_cxform;
	
	void set_antialiased(bool enable)
	{
		// not supported
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
					*(out) = (Uint8) (a >> 2);
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
		const gameswf::bitmap_info*	m_bitmap_info;
		gameswf::matrix	m_bitmap_matrix;
		gameswf::cxform	m_bitmap_color_transform;
		bool	m_has_nonzero_bitmap_additive_color;

		fill_style()
			:
			m_mode(INVALID),
			m_has_nonzero_bitmap_additive_color(false)
		{
		}

		void	apply(/*const matrix& current_matrix*/) const
		// Push our style into D3D.
		{
			assert(m_mode != INVALID);

			if (m_mode == COLOR)
			{
				apply_color(m_color);
				IDirect3DDevice8::SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
				//IDirect3DDevice8::SetRenderState(state, value);
//				glDisable(GL_TEXTURE_2D);
			}
			else if (m_mode == BITMAP_WRAP
				 || m_mode == BITMAP_CLAMP)
			{
				assert(m_bitmap_info != NULL);

				apply_color(m_color);

				if (m_bitmap_info == NULL)
				{
					IDirect3DDevice8::SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
					//glDisable(GL_TEXTURE_2D);
				}
				else
				{
					// Set up the texture for rendering.

					{
						// Do the modulate part of the color
						// transform in the first pass.  The
						// additive part, if any, needs to
						// happen in a second pass.
// 						glColor4f(m_bitmap_color_transform.m_[0][0],
// 							  m_bitmap_color_transform.m_[1][0],
// 							  m_bitmap_color_transform.m_[2][0],
// 							  m_bitmap_color_transform.m_[3][0]
// 							  );
						IDirect3DDevice8::SetVertexData4f(
							D3DVSDE_DIFFUSE, 
							m_bitmap_color_transform.m_[0][0],
							m_bitmap_color_transform.m_[1][0],
							m_bitmap_color_transform.m_[2][0],
							m_bitmap_color_transform.m_[3][0]);
					}

//					glBindTexture(GL_TEXTURE_2D, m_bitmap_info->m_texture_id);
//					glEnable(GL_TEXTURE_2D);
//					glEnable(GL_TEXTURE_GEN_S);
//					glEnable(GL_TEXTURE_GEN_T);
					IDirect3DDevice8::SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
					IDirect3DDevice8::SetTexture(0, s_d3d_textures[m_bitmap_info->m_texture_id]);
				
					if (m_mode == BITMAP_CLAMP)
					{
//						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						IDirect3DDevice8::SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
						IDirect3DDevice8::SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
					}
					else
					{
						assert(m_mode == BITMAP_WRAP);
//						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
						IDirect3DDevice8::SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
						IDirect3DDevice8::SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
					}

					// Set up the bitmap matrix for texgen.
					float	inv_width = 1.0f / m_bitmap_info->m_original_width;
					float	inv_height = 1.0f / m_bitmap_info->m_original_height;

					const gameswf::matrix&	m = m_bitmap_matrix;
//					glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
					float	p[4] = { 0, 0, 0, 0 };
					p[0] = m.m_[0][0] * inv_width;
					p[1] = m.m_[0][1] * inv_width;
					p[3] = m.m_[0][2] * inv_width;
//					glTexGenfv(GL_S, GL_OBJECT_PLANE, p);
					IDirect3DDevice8::SetVertexShaderConstant(4, p, 1);


//					glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
					p[0] = m.m_[1][0] * inv_height;
					p[1] = m.m_[1][1] * inv_height;
					p[3] = m.m_[1][2] * inv_height;
//					glTexGenfv(GL_T, GL_OBJECT_PLANE, p);
					IDirect3DDevice8::SetVertexShaderConstant(5, p, 1);
				}
			}
		}


		bool	needs_second_pass() const
		// Return true if we need to do a second pass to make
		// a valid color.  This is for cxforms with additive
		// parts.
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
		// Set D3D state for a necessary second pass.
		{
			assert(needs_second_pass());

			// Additive color.
			IDirect3DDevice8::SetVertexData4f(
				D3DVSDE_DIFFUSE,
				m_bitmap_color_transform.m_[0][1] / 255.0f,
				m_bitmap_color_transform.m_[1][1] / 255.0f,
				m_bitmap_color_transform.m_[2][1] / 255.0f,
				m_bitmap_color_transform.m_[3][1] / 255.0f
				);

			IDirect3DDevice8::SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			IDirect3DDevice8::SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

#if 0
			glDisable(GL_TEXTURE_2D);
			glColor4f(
				m_bitmap_color_transform.m_[0][1] / 255.0f,
				m_bitmap_color_transform.m_[1][1] / 255.0f,
				m_bitmap_color_transform.m_[2][1] / 255.0f,
				m_bitmap_color_transform.m_[3][1] / 255.0f
				);

			glBlendFunc(GL_ONE, GL_ONE);
#endif // 0
		}

		void	cleanup_second_pass() const
		{
			IDirect3DDevice8::SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			IDirect3DDevice8::SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

#if 0
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif // 0
		}


		void	disable() { m_mode = INVALID; }
		void	set_color(gameswf::rgba color) { m_mode = COLOR; m_color = color; }
		void	set_bitmap(const gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm, const gameswf::cxform& color_transform)
		{
			m_mode = (wm == WRAP_REPEAT) ? BITMAP_WRAP : BITMAP_CLAMP;
			m_color = gameswf::rgba();
			m_bitmap_info = bi;
			m_bitmap_matrix = m;
			m_bitmap_color_transform = color_transform;

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


	render_handler_xbox()
	// Constructor.
	{
		init_vshader();
	}


	// Style state.
	enum style_index
	{
		LEFT_STYLE = 0,
		RIGHT_STYLE,
		LINE_STYLE,

		STYLE_COUNT
	};
	fill_style	m_current_styles[STYLE_COUNT];


	gameswf::bitmap_info*	create_bitmap_info(image::rgb* im)
	// Given an image, returns a pointer to a bitmap_info struct
	// that can later be passed to fill_styleX_bitmap(), to set a
	// bitmap fill style.
	{
		return new bitmap_info_xbox(im);
	}


	gameswf::bitmap_info*	create_bitmap_info(image::rgba* im)
	// Given an image, returns a pointer to a bitmap_info struct
	// that can later be passed to fill_style_bitmap(), to set a
	// bitmap fill style.
	//
	// This version takes an image with an alpha channel.
	{
		return new bitmap_info_xbox(im);
	}


	gameswf::bitmap_info*	create_bitmap_info_blank()
	// Creates and returns an empty bitmap_info structure.	Image data
	// can be bound to this info later, via set_alpha_image().
	{
		return new bitmap_info_xbox(gameswf::bitmap_info::empty);
	}


	void	set_alpha_image(gameswf::bitmap_info* bi, int w, int h, Uint8* data)
	// Set the specified bitmap_info so that it contains an alpha
	// texture with the given data (1 byte per texel).
	//
	// Munges *data (in order to make mipmaps)!!
	{
		assert(bi);

		bi->set_alpha_image(w, h, data);
	}


	void	delete_bitmap_info(gameswf::bitmap_info* bi)
	// Delete the given bitmap info struct.
	{
		delete bi;
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
// 		// Matrix setup
// 		D3DXMATRIX	ortho;
// 		D3DXMatrixOrthoOffCenterRH(&ortho, x0, x1, y0, y1, 0.0f, 1.0f);
// 		IDirect3DDevice8::SetTransform(D3DTS_PROJECTION, &ortho);

// 		D3DXMATRIX	ident;
// 		D3DXMatrixIdentity(&ident);
// 		IDirect3DDevice8::SetTransform(D3DTS_VIEW, &ident);
// 		// IDirect3DDevice8::SetTransform(D3DTS_WORLD, &ident);
// 		// etc.?

		// Viewport.
		D3DVIEWPORT8	vp;
		vp.X = viewport_x0;
		vp.Y = viewport_y0;
		vp.Width = viewport_width;
		vp.Height = viewport_height;
		vp.MinZ = 0.0f;
		vp.MaxZ = 0.0f;
		IDirect3DDevice8::SetViewport(&vp);

		// Matrix to map from SWF movie (TWIPs) coords to
		// viewport coordinates.
		float	dx = x1 - x0;
		float	dy = y1 - y0;
		if (dx < 1) { dx = 1; }
		if (dy < 1) { dy = 1; }
		m_viewport_matrix.set_identity();
		m_viewport_matrix.m_[0][0] = viewport_width / dx;
		m_viewport_matrix.m_[1][1] = viewport_height / dy;
		m_viewport_matrix.m_[0][2] = viewport_x0 - m_viewport_matrix.m_[0][0] * x0;
		m_viewport_matrix.m_[1][2] = viewport_y0 - m_viewport_matrix.m_[1][1] * y0;

		// Blending renderstates
		IDirect3DDevice8::SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		IDirect3DDevice8::SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		IDirect3DDevice8::SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		// Textures off by default.
		IDirect3DDevice8::SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);

		// @@ for sanity's sake, let's turn of backface culling...
		IDirect3DDevice8::SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);//xxxxx
		IDirect3DDevice8::SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);	//xxxxx

		// Vertex format.
		IDirect3DDevice8::SetVertexShader(s_vshader_handle);

		// No pixel shader.
		IDirect3DDevice8::SetPixelShaderProgram(NULL);

#if 0
		glViewport(viewport_x0, viewport_y0, viewport_width, viewport_height);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glOrtho(x0, x1, y0, y1, -1, 1);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// GL_MODULATE

		glDisable(GL_TEXTURE_2D);
#endif // 0

		// Clear the background, if background color has alpha > 0.
		if (background_color.m_a > 0)
		{
			// @@ for testing
			static int	bobo = 0;
			IDirect3DDevice8::Clear(
				0,
				NULL,
				D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
				bobo += 5,
				0.0f,
				0);

			// Draw a big quad.
			apply_color(background_color);
			set_matrix(gameswf::matrix::identity);
			apply_matrix(m_current_matrix);

			IDirect3DDevice8::Begin(D3DPT_TRIANGLESTRIP);
			IDirect3DDevice8::SetVertexData2f(D3DVSDE_POSITION, x0, y0);
			IDirect3DDevice8::SetVertexData2f(D3DVSDE_POSITION, x1, y0);
			IDirect3DDevice8::SetVertexData2f(D3DVSDE_POSITION, x1, y1);
			IDirect3DDevice8::SetVertexData2f(D3DVSDE_POSITION, x0, y1);
			IDirect3DDevice8::End();

#if 0
			glBegin(GL_QUADS);
			glVertex2f(x0, y0);
			glVertex2f(x1, y0);
			glVertex2f(x1, y1);
			glVertex2f(x0, y1);
			glEnd();
#endif // 0
		}
	}


	void	end_display()
	// Clean up after rendering a frame.  Client program is still
	// responsible for calling glSwapBuffers() or whatever.
	{
#if 0
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
#endif // 0
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
	
	void	apply_matrix(const gameswf::matrix& mat_in)
	// Set the given transformation matrix.
	{
		gameswf::matrix	m(m_viewport_matrix);
		m.concatenate(mat_in);

		float	row0[4];
		float	row1[4];
		row0[0] = m.m_[0][0];
		row0[1] = m.m_[0][1];
		row0[2] = 0;
		row0[3] = m.m_[0][2];

		row1[0] = m.m_[1][0];
		row1[1] = m.m_[1][1];
		row1[2] = 0;
		row1[3] = m.m_[1][2];

//		glMultMatrixf(mat);
//		IDirect3DDevice8::SetTransform(D3DTS_VIEW, (D3DMATRIX*) &mat[0]);
		IDirect3DDevice8::SetVertexShaderConstant(0, row0, 1);
		IDirect3DDevice8::SetVertexShaderConstant(1, row1, 1);
	}

	static void	apply_color(const gameswf::rgba& c)
	// Set the given color.
	{
//		glColor4ub(c.m_r, c.m_g, c.m_b, c.m_a);
		IDirect3DDevice8::SetVertexData4ub(D3DVSDE_DIFFUSE, c.m_r, c.m_g, c.m_b, c.m_a);
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


	void	fill_style_color(int fill_side, gameswf::rgba color)
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


	void	fill_style_bitmap(int fill_side, const gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm)
	{
		assert(fill_side >= 0 && fill_side < 2);
		m_current_styles[fill_side].set_bitmap(bi, m, wm, m_current_cxform);
	}
	
	void	line_style_width(float width)
	{
		// WK: what to do here???
	}


	void	draw_mesh_strip(const void* coords, int vertex_count)
	{
		// Set up current style.
		m_current_styles[LEFT_STYLE].apply();

		apply_matrix(m_current_matrix);

		// @@ we'd like to use a VB instead, and use DrawPrimitive().

		// Draw the mesh.
		IDirect3DDevice8::DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vertex_count - 2, coords, sizeof(Sint16) * 2);

		if (m_current_styles[LEFT_STYLE].needs_second_pass())
		{
			// 2nd pass, if necessary.
			m_current_styles[LEFT_STYLE].apply_second_pass();
			IDirect3DDevice8::DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vertex_count - 2, coords, sizeof(Sint16) * 2);
			m_current_styles[LEFT_STYLE].cleanup_second_pass();
		}

#if 0
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		apply_matrix(m_current_matrix);

		// Send the tris to OpenGL
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_SHORT, sizeof(Sint16) * 2, coords);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);

		if (m_current_styles[LEFT_STYLE].needs_second_pass())
		{
			m_current_styles[LEFT_STYLE].apply_second_pass();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
			m_current_styles[LEFT_STYLE].cleanup_second_pass();
		}

		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();
#endif // 0
	}


	void	draw_line_strip(const void* coords, int vertex_count)
	// Draw the line strip formed by the sequence of points.
	{
		// Set up current style.
		m_current_styles[LINE_STYLE].apply();

		apply_matrix(m_current_matrix);

		IDirect3DDevice8::DrawPrimitiveUP(D3DPT_LINESTRIP, vertex_count - 1, coords, sizeof(Sint16) * 2);
	}


	void	draw_bitmap(
		const gameswf::matrix& m,
		const gameswf::bitmap_info* bi,
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

		// Set texture.
		IDirect3DDevice8::SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		IDirect3DDevice8::SetTexture(0, s_d3d_textures[bi->m_texture_id]);

		// @@ TODO this is wrong; needs fixing!  Options:
		//
		// * compute texgen parameters for the bitmap
		//
		// * change to a vshader which passes the texcoords through

		// No texgen; just pass through.
		float	row0[4] = { 1, 0, 0, 0 };
		float	row1[4] = { 0, 1, 0, 0 };
		IDirect3DDevice8::SetVertexShaderConstant(4, row0, 1);
		IDirect3DDevice8::SetVertexShaderConstant(5, row1, 1);

		// Draw the quad.
		IDirect3DDevice8::Begin(D3DPT_TRIANGLESTRIP);
		
		IDirect3DDevice8::SetVertexData2f(D3DVSDE_TEXCOORD0, uv_coords.m_x_min, uv_coords.m_y_min);
		IDirect3DDevice8::SetVertexData4f(D3DVSDE_VERTEX, a.m_x, a.m_y, 0, 1);

		IDirect3DDevice8::SetVertexData2f(D3DVSDE_TEXCOORD0, uv_coords.m_x_max, uv_coords.m_y_min);
		IDirect3DDevice8::SetVertexData4f(D3DVSDE_VERTEX, b.m_x, b.m_y, 0, 1);

		IDirect3DDevice8::SetVertexData2f(D3DVSDE_TEXCOORD0, uv_coords.m_x_min, uv_coords.m_y_max);
		IDirect3DDevice8::SetVertexData4f(D3DVSDE_VERTEX, c.m_x, c.m_y, 0, 1);

		IDirect3DDevice8::SetVertexData2f(D3DVSDE_TEXCOORD0, uv_coords.m_x_max, uv_coords.m_y_max);
		IDirect3DDevice8::SetVertexData4f(D3DVSDE_VERTEX, d.m_x, d.m_y, 0, 1);

		IDirect3DDevice8::End();

#if 0
		glBindTexture(GL_TEXTURE_2D, bi->m_texture_id);
		glEnable(GL_TEXTURE_2D);
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
#endif // 0
	}
	
	void begin_submit_mask()
	{
#if 0
		glEnable(GL_STENCIL_TEST); 
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glColorMask(0,0,0,0);	// disable framebuffer writes
		glEnable(GL_STENCIL_TEST);	// enable stencil buffer for "marking" the mask
		glStencilFunc(GL_ALWAYS, 1, 1);	// always passes, 1 bit plane, 1 as mask
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	// we set the stencil buffer to 1 where we draw any polygon
							// keep if test fails, keep if test passes but buffer test fails
							// replace if test passes 
#endif // 0
	}
	
	void end_submit_mask()
	{
#if 0
		glColorMask(1,1,1,1);	// enable framebuffer writes
		glStencilFunc(GL_EQUAL, 1, 1);	// we draw only where the stencil is 1 (where the mask was drawn)
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	// don't change the stencil buffer	  
#endif // 0
	}
	
	void disable_mask()
	{
#if 0
	    glDisable(GL_STENCIL_TEST); 
#endif // 0
	}
	
};	// end struct render_handler_xbox


// bitmap_info_xbox implementation


bitmap_info_xbox::bitmap_info_xbox(create_empty e)
{
	// A null texture.  Needs to be initialized later.
}


bitmap_info_xbox::bitmap_info_xbox(image::rgb* im)
// Image with no alpha.
{
	assert(im);

	// Rescale.
	m_original_width = im->m_width;
	m_original_height = im->m_height;

	int	w = 1; while (w < im->m_width) { w <<= 1; }
	int	h = 1; while (h < im->m_height) { h <<= 1; }

	image::rgb*	rescaled = image::create_rgb(w, h);
	image::resample(rescaled, 0, 0, w - 1, h - 1,
			im, 0, 0, (float) im->m_width, (float) im->m_height);

	// Need to insert a dummy alpha byte in the image data, for
	// D3DXLoadSurfaceFromMemory.
	// @@ this sucks :(
	int	pixel_count = w * h;
	Uint8*	expanded_data = new Uint8[pixel_count * 4];
	for (int y = 0; y < h; y++)
	{
		Uint8*	scanline = image::scanline(rescaled, y);
		for (int x = 0; x < w; x++)
		{
			expanded_data[((y * w) + x) * 4 + 0] = scanline[x * 3 + 0];	// red
			expanded_data[((y * w) + x) * 4 + 1] = scanline[x * 3 + 1];	// green
			expanded_data[((y * w) + x) * 4 + 2] = scanline[x * 3 + 2];	// blue
			expanded_data[((y * w) + x) * 4 + 3] = 255;	// alpha
		}
	}

	// Create the texture.
	s_d3d_textures.push_back(NULL);
	m_texture_id = s_d3d_textures.size() - 1;

	IDirect3DTexture8*	tex;
	HRESULT	result = IDirect3DDevice8::CreateTexture(
		w,
		h,
		0,
		D3DUSAGE_BORDERSOURCE_TEXTURE,
		D3DFMT_DXT1,
		NULL,
		&tex);
	if (result != S_OK)
	{
		gameswf::log_error("error: can't create texture\n");
		return;
	}
	s_d3d_textures.back() = tex;

	IDirect3DSurface8*	surf = NULL;
	result = tex->GetSurfaceLevel(0, &surf);
	if (result != S_OK)
	{
		gameswf::log_error("error: can't get surface\n");
		return;
	}
	assert(surf);

	RECT	source_rect;
	source_rect.left = 0;
	source_rect.top = 0;
	source_rect.right = w;
	source_rect.bottom = h;
	result = D3DXLoadSurfaceFromMemory(
		surf,
		NULL,
		NULL,
		expanded_data,
		D3DFMT_LIN_A8B8G8R8,
		w * 4,
		NULL,
		&source_rect,
		D3DX_FILTER_POINT,
		0);
	delete [] expanded_data;
	if (result != S_OK)
	{
		gameswf::log_error("error: can't load surface from memory, result = %d\n", result);
		return;
	}

	if (surf) { surf->Release(); }
#if 0
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, (GLuint*)&m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST /* LINEAR_MIPMAP_LINEAR */);

	m_original_width = im->m_width;
	m_original_height = im->m_height;

	int	w = 1; while (w < im->m_width) { w <<= 1; }
	int	h = 1; while (h < im->m_height) { h <<= 1; }

	image::rgb*	rescaled = image::create_rgb(w, h);
	image::resample(rescaled, 0, 0, w - 1, h - 1,
			im, 0, 0, (float) im->m_width, (float) im->m_height);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rescaled->m_data);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rescaled->m_width, rescaled->m_height, GL_RGB, GL_UNSIGNED_BYTE, rescaled->m_data);

	delete [] rescaled;
#endif // 0
}


bitmap_info_xbox::bitmap_info_xbox(image::rgba* im)
// Version of the constructor that takes an image with alpha.
{
	assert(im);

	m_original_width = im->m_width;
	m_original_height = im->m_height;

	int	w = 1; while (w < im->m_width) { w <<= 1; }
	int	h = 1; while (h < im->m_height) { h <<= 1; }

	// Create the texture.
	s_d3d_textures.push_back(NULL);
	m_texture_id = s_d3d_textures.size() - 1;

	IDirect3DTexture8*	tex;
	HRESULT	result = IDirect3DDevice8::CreateTexture(
		w,
		h,
		0,
		D3DUSAGE_BORDERSOURCE_TEXTURE,
		D3DFMT_DXT1,
		NULL,
		&tex);
	if (result != S_OK)
	{
		gameswf::log_error("error: can't create texture\n");
		return;
	}
	s_d3d_textures.back() = tex;

	IDirect3DSurface8*	surf = NULL;
	result = tex->GetSurfaceLevel(0, &surf);
	if (result != S_OK)
	{
		gameswf::log_error("error: can't get surface\n");
		return;
	}
	assert(surf);

	RECT	source_rect;
	source_rect.left = 0;
	source_rect.top = 0;
	source_rect.right = w;
	source_rect.bottom = h;

	// Set the actual data.
	if (w != im->m_width
	    || h != im->m_height)
	{
		image::rgba*	rescaled = image::create_rgba(w, h);
		image::resample(rescaled, 0, 0, w - 1, h - 1,
				im, 0, 0, (float) im->m_width, (float) im->m_height);

		result = D3DXLoadSurfaceFromMemory(
			surf,
			NULL,
			NULL,
			rescaled->m_data,
			D3DFMT_LIN_A8B8G8R8,
			rescaled->m_pitch,
			NULL,
			&source_rect,
			D3DX_FILTER_POINT,
			0);
		if (result != S_OK)
		{
			gameswf::log_error("error: can't load surface from memory, result = %d\n", result);
			return;
		}
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rescaled->m_data);

		delete [] rescaled;
	}
	else
	{
		// Use original image directly.
		result = D3DXLoadSurfaceFromMemory(
			surf,
			NULL,
			NULL,
			im->m_data,
			D3DFMT_LIN_A8B8G8R8,
			im->m_pitch,
			NULL,
			&source_rect,
			D3DX_FILTER_POINT,
			0);
		if (result != S_OK)
		{
			gameswf::log_error("error: can't load surface from memory, result = %d\n", result);
			return;
		}
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, im->m_data);
	}

	if (surf) { surf->Release(); }

#if 0
	// Create the texture.

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, (GLuint*)&m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// GL_NEAREST ?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST /* LINEAR_MIPMAP_LINEAR */);

	m_original_width = im->m_width;
	m_original_height = im->m_height;

	int	w = 1; while (w < im->m_width) { w <<= 1; }
	int	h = 1; while (h < im->m_height) { h <<= 1; }

	if (w != im->m_width
	    || h != im->m_height)
	{
		image::rgba*	rescaled = image::create_rgba(w, h);
		image::resample(rescaled, 0, 0, w - 1, h - 1,
				im, 0, 0, (float) im->m_width, (float) im->m_height);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rescaled->m_data);

		delete [] rescaled;
	}
	else
	{
		// Use original image directly.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, im->m_data);
	}
#endif // 0
}


void bitmap_info_xbox::set_alpha_image(int width, int height, Uint8* data)
// Initialize this bitmap_info to an alpha image
// containing the specified data (1 byte per texel).
//
// !! Munges *data in order to create mipmaps !!
{
	assert(m_texture_id == 0);	// only call this on an empty bitmap_info
	assert(data);
	
	// Create the texture.
	
	m_original_width = width;
	m_original_height = height;

	#ifndef NDEBUG
	// You must use power-of-two dimensions!!
	int	w = 1; while (w < width) { w <<= 1; }
	int	h = 1; while (h < height) { h <<= 1; }
	assert(w == width);
	assert(h == height);
	#endif // not NDEBUG

	s_d3d_textures.push_back(NULL);
	m_texture_id = s_d3d_textures.size() - 1;

	IDirect3DTexture8*	tex;
	HRESULT	result = IDirect3DDevice8::CreateTexture(
		width,
		height,
		0,
		D3DUSAGE_BORDERSOURCE_TEXTURE,
		D3DFMT_A8,
		NULL,
		&tex);
	if (result != S_OK)
	{
		gameswf::log_error("error: can't create texture\n");
		return;
	}
	s_d3d_textures.back() = tex;

	IDirect3DSurface8*	surf = NULL;
	result = tex->GetSurfaceLevel(0, &surf);
	if (result != S_OK)
	{
		gameswf::log_error("error: can't get surface\n");
		return;
	}
	assert(surf);

	RECT	source_rect;
	source_rect.left = 0;
	source_rect.top = 0;
	source_rect.right = width;
	source_rect.bottom = height;
	result = D3DXLoadSurfaceFromMemory(
		surf,
		NULL,
		NULL,
		data,
		D3DFMT_LIN_A8,
		width,
		NULL,
		&source_rect,
		D3DX_FILTER_POINT,
		0);
	if (result != S_OK)
	{
		gameswf::log_error("error: can't load surface from memory, result = %d\n", result);
		return;
	}

	if (surf) { surf->Release(); }

//	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);

// 	// Build mips.
// 	int	level = 1;
// 	while (width > 1 || height > 1)
// 	{
// 		render_handler_xbox::make_next_miplevel(&width, &height, data);
// 		glTexImage2D(GL_TEXTURE_2D, level, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
// 		level++;
// 	}

#if 0
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, (GLuint*)&m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// GL_NEAREST ?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	m_original_width = width;
	m_original_height = height;

	#ifndef NDEBUG
	// You must use power-of-two dimensions!!
	int	w = 1; while (w < width) { w <<= 1; }
	int	h = 1; while (h < height) { h <<= 1; }
	assert(w == width);
	assert(h == height);
	#endif // not NDEBUG

	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);

	// Build mips.
	int	level = 1;
	while (width > 1 || height > 1)
	{
		render_handler_xbox::make_next_miplevel(&width, &height, data);
		glTexImage2D(GL_TEXTURE_2D, level, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
		level++;
	}
#endif // 0
}


gameswf::render_handler*	gameswf::create_render_handler_xbox()
// Factory.
{
	return new render_handler_xbox;
}


#endif // _XBOX


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
