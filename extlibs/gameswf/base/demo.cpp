// demo.cpp  -- Thatcher Ulrich <http://tulrich.com> 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some helper code for making graphical demos.  Covers OpenGL/SDL
// initialization, and some basic viewport navigation.


#include "base/tu_config.h"
#include "base/demo.h"
#include "base/ogl.h"
#include "SDL.h"


namespace demo
{
	void init_video(int width, int height, int depth)
	{
		// Display.
		// Initialize the SDL subsystems we're using.
		if (SDL_Init(SDL_INIT_VIDEO /* | SDL_INIT_JOYSTICK | SDL_INIT_CDROM | SDL_INIT_AUDIO*/))
		{
			fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
			exit(1);
		}
		atexit(SDL_Quit);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		// Set the video mode.
		if (SDL_SetVideoMode(width, height, depth, SDL_OPENGL) == 0)
		{
			fprintf(stderr, "SDL_SetVideoMode() failed.");
			exit(1);
		}

		ogl::open();
	}


	bool update_nav2d(nav2d_state* state)
	{
		state->m_keys.resize(0);

		// Handle input.
		SDL_Event	event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
			{
				int key = event.key.keysym.sym;
				int mod = event.key.keysym.mod;

				if (key == SDLK_q || key == SDLK_ESCAPE)
				{
					return true;
				} else if (key == SDLK_EQUALS) {
					state->m_scale *= 0.5f;
				} else if (key == SDLK_MINUS) {
					state->m_scale *= 2.0f;
				} else {
					state->m_keys.resize(state->m_keys.size() + 1);
					state->m_keys.back().key = key;
					state->m_keys.back().modifier = mod;
				}
				break;
			}

			case SDL_MOUSEMOTION:
			{
				int new_x = (int) (event.motion.x);
				int new_y = (int) (event.motion.y);
				state->m_mouse_dx = new_x - state->m_mouse_x;
				state->m_mouse_dy = new_y - state->m_mouse_y;
				if (state->m_mouse_buttons & 2) {
					// Left drag: move.
					state->m_center_x -= state->m_mouse_dx * state->m_scale;
					state->m_center_y += state->m_mouse_dy * state->m_scale;
				}
				state->m_mouse_x = new_x;
				state->m_mouse_y = new_y;
				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				int	mask = 1 << (event.button.button);
				if (event.button.state == SDL_PRESSED)
				{
					state->m_mouse_buttons |= mask;
				}
				else
				{
					state->m_mouse_buttons &= ~mask;
				}
				break;
			}

			case SDL_QUIT:
				return true;
				break;

			default:
				break;
			}
		}

		return false;
	}


	void set_nav2d_viewport(const nav2d_state& state)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(state.m_center_x - 500 * state.m_scale, state.m_center_x + 500 * state.m_scale,
			state.m_center_y - 500 * state.m_scale, state.m_center_y + 500 * state.m_scale, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:


