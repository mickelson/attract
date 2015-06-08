// ogl.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// test program for ogl wrapper


#include <stdlib.h>
#include "base/ogl.h"
#include <SDL.h>


#undef main	// SDL weirdness under WIN32!!
extern "C" int	main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO /* | SDL_INIT_JOYSTICK | SDL_INIT_CDROM | SDL_INIT_AUDIO*/))
	{
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	int	bpp = 24;
	int	flags = SDL_OPENGL | (0 ? SDL_FULLSCREEN : 0);

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	// Set the video mode.
	if (SDL_SetVideoMode(320, 240, bpp, flags) == 0) {
		fprintf(stderr, "SDL_SetVideoMode() failed.");
		exit(1);
	}

	ogl::open();

	return 0;
}
