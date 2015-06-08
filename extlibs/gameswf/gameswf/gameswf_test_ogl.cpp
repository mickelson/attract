// gameswf_test_ogl.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003 -*- coding: utf-8;-*-

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A minimal test player app for the gameswf library.

#include "base/tu_memdebug.h"	// must be the first in the include list

#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_opengl.h>

#include "gameswf/gameswf.h"
#include <stdlib.h>
#include <stdio.h>
#include "base/utility.h"
#include "base/container.h"
#include "base/tu_file.h"
#include "base/tu_types.h"
#include "base/tu_timer.h"
#include "gameswf/gameswf_types.h"
#include "gameswf/gameswf_impl.h"
#include "gameswf/gameswf_root.h"
#include "gameswf/gameswf_freetype.h"
#include "gameswf/gameswf_player.h"

#if TU_ENABLE_NETWORK == 1
#	include "net/tu_net_file.h"
#endif

#ifdef _WIN32
#	include <Winsock.h>
#	define stricmp _stricmp
#else
#	define stricmp strcasecmp
#endif                                  

void	print_usage()
// Brief instructions.
{
	printf(
		"gameswf_test_ogl -- a test player for the gameswf library.\n"
		"\n"
		"This program has been donated to the Public Domain.\n"
		"See http://tulrich.com/geekstuff/gameswf.html for more info.\n"
		"\n"
		"usage: gameswf_test_ogl [options] movie_file.swf\n"
		"\n"
		"Plays a SWF (Shockwave Flash) movie, using OpenGL and the\n"
		"gameswf library.\n"
		"\n"
		"options:\n"
		"\n"
		"  -h          Print this info.\n"
		"  -c          Produce a core file instead of letting SDL trap it\n"
		"  -d num      Number of milliseconds to delay in main loop\n"
		"  -a <level>  Specify the antialiasing level (0,1,2,4,8,16,...)\n"
		"  -v          Be verbose; i.e. print log messages to stdout\n"
		"  -va         Be verbose about movie Actions\n"
		"  -vp         Be verbose about parsing the movie\n"
		"  -ml <bias>  Specify the texture LOD bias (float, default is -1)\n"
		"  -p          Run full speed (no sleep) and log frame rate\n"
		"  -1          Play once; exit when/if movie reaches the last frame\n"
		"  -r <0|1|2>  0 disables rendering & sound (good for batch tests)\n"
		"              1 enables rendering & sound (default setting)\n"
		"              2 enables rendering & disables sound\n"
		"  -t <sec>    Timeout and exit after the specified number of seconds\n"
		"  -b <bits>   Bit depth of output window (16 or 32, default is 16)\n"
		"  -n          Allow use of network to try to open resource URLs\n"
		"  -u          Allow pass user variables to Flash\n"
		"  -k          Disables cursor\n"
		"  -w <w>x<h>  Specify the window size, for example 1024x768\n"
		"  -f          Force realtime framerate\n"
		"  -i          Grub bitmaps from swf file\n"
		"\n"
		"keys:\n"
		"  CTRL-Q          Quit/Exit\n"
		"  CTRL-W          Quit/Exit\n"
		"  ESC             Quit/Exit\n"
		"  CTRL-P          Toggle Pause\n"
		"  CTRL-[ or kp-   Step back one frame\n"
		"  CTRL-] or kp+   Step forward one frame\n"
		"  CTRL-A          Toggle antialiasing\n"
		"  CTRL-T          Debug.  Test the set_variable() function\n"
		"  CTRL-G          Debug.  Test the get_variable() function\n"
		"  CTRL-M          Debug.  Test the call_method() function\n"
		"  CTRL-B          Toggle background color\n"
		);
}

#define OVERSIZE	1.0f

static bool s_antialiased = true;
static int s_bit_depth = 24;
static int s_delay = 10;


// by default it's used the simplest and the fastest edge antialiasing method
// if you have modern video card you can use full screen antialiasing
// full screen antialiasing level may be 2,4,8,16, ...
static int s_aa_level = 1;

static bool s_background = true;
static bool s_measure_performance = false;
// Controls whether we will try to load things over the net or not.
static bool s_allow_http = false;

static void	message_log(const char* message)
// Process a log message.
{
	if (gameswf::get_verbose_parse())
	{
		fputs(message, stdout);
		fflush(stdout);
	}
}


static void	log_callback(bool error, const char* message)
// Error callback for handling gameswf messages.
{
	if (error)
	{
		// Log, and also print to stderr.
		message_log(message);
		fputs(message, stderr);
		fflush(stderr);
	}
	else
	{
		message_log(message);
	}
}


static tu_file*	file_opener(const char* url)
// Callback function.  This opens files for the gameswf library.
{
	if (s_allow_http) 
	{
#if TU_ENABLE_NETWORK == 1
		return new_tu_net_file(url, "rb");
#else
		return NULL;
#endif
	}
	else
	{
		return new tu_file(url, "rb");
	}
}

static gameswf::key::code	translate_key(SDLKey key)
// For forwarding SDL key events to gameswf.
{
	gameswf::key::code	c(gameswf::key::INVALID);

	if (key >= SDLK_0 && key <= SDLK_9)
	{
		c = (gameswf::key::code) ((key - SDLK_0) + gameswf::key::_0);
	}
	else if (key >= SDLK_a && key <= SDLK_z)
	{
		c = (gameswf::key::code) ((key - SDLK_a) + gameswf::key::A);
	}
	else if (key >= SDLK_F1 && key <= SDLK_F15)
	{
		c = (gameswf::key::code) ((key - SDLK_F1) + gameswf::key::F1);
	}
	else if (key >= SDLK_KP0 && key <= SDLK_KP9)
	{
		c = (gameswf::key::code) ((key - SDLK_KP0) + gameswf::key::KP_0);
	}
	else
	{
		// many keys don't correlate, so just use a look-up table.
		struct
		{
			SDLKey	sdlk;
			gameswf::key::code	gs;
		} table[] =
			{
                { SDLK_RETURN, gameswf::key::ENTER },
                { SDLK_ESCAPE, gameswf::key::ESCAPE },
                { SDLK_LEFT, gameswf::key::LEFT },
                { SDLK_UP, gameswf::key::UP },
                { SDLK_RIGHT, gameswf::key::RIGHT },
                { SDLK_DOWN, gameswf::key::DOWN },
                { SDLK_SPACE, gameswf::key::SPACE },
                { SDLK_PAGEDOWN, gameswf::key::PGDN },
                { SDLK_PAGEUP, gameswf::key::PGUP },
                { SDLK_HOME, gameswf::key::HOME },
                { SDLK_END, gameswf::key::END },
                { SDLK_INSERT, gameswf::key::INSERT },
                { SDLK_DELETE, gameswf::key::DELETEKEY },
                { SDLK_BACKSPACE, gameswf::key::BACKSPACE },
                { SDLK_TAB, gameswf::key::TAB },
                { SDLK_RSHIFT, gameswf::key::SHIFT },
                { SDLK_LSHIFT, gameswf::key::SHIFT },
                { SDLK_PERIOD, gameswf::key::PERIOD },
                { SDLK_SLASH, gameswf::key::SLASH },
                { SDLK_BACKSLASH, gameswf::key::BACKSLASH },
                { SDLK_SEMICOLON, gameswf::key::SEMICOLON },
                { SDLK_QUOTE, gameswf::key::QUOTE },
                { SDLK_LEFTBRACKET, gameswf::key::LEFT_BRACKET },
                { SDLK_RIGHTBRACKET, gameswf::key::RIGHT_BRACKET },
                { SDLK_COMMA, gameswf::key::COMMA },

				// @@ TODO fill this out some more
				{ SDLK_UNKNOWN, gameswf::key::INVALID }
			};

		for (int i = 0; table[i].sdlk != SDLK_UNKNOWN; i++)
		{
			if (key == table[i].sdlk)
			{
				c = table[i].gs;
				break;
			}
		}
	}

	return c;
}

static void	fs_callback(gameswf::character* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
	assert(movie);
	gameswf::gc_ptr<gameswf::player> player = movie->get_player();

	if (stricmp(command, "fullscreen") == 0)
	{
		// TODO
	}
	else
	if (stricmp(command, "set_max_volume") == 0)
	{
		// set max sound volume in percent, [0..100]
		// usefull for embedded games
		gameswf::sound_handler* s = gameswf::get_sound_handler();
		if (s)
		{
			int vol = atoi(args);
			s->set_max_volume(vol);
		}
	}
	else
	if (stricmp(command, "notify_keypress") == 0)
	{
		// simulate keypress event
		for (int i = 0, n = strlen(args); i < n; i++)
		{
			// SDL has no uppercase key codes
			SDLKey key = static_cast<SDLKey>(tolower(args[i]));
//			gameswf::key::code c = translate_key(key);
//			if (c != gameswf::key::INVALID)
//			{
//				player->notify_key_event(c, true);
//			}
			SDL_Event ev;
			memset(&ev, 0, sizeof(ev));
			ev.type = SDL_KEYDOWN;
			ev.key.keysym.sym = key;
			SDL_PushEvent(&ev);
		}
	}
	else
	if (stricmp(command, "set_delay") == 0)
	{
		// set the number of milli-seconds to delay in main loop
		int delay = atoi(args);

		// sanity check
		if (delay >= 0 && delay <= 1000)
		{
			s_delay = delay;
		}
	}
	else
	if (stricmp(command, "clear_events") == 0)
	{
		// clear queue of system events (mouse events, keyboard events, etc)
		SDL_Event	event;
		if (SDL_PollEvent(&event) == 0)
		{
			return;
		}
	}

}

int	main(int argc, char *argv[])
{

	tu_memdebug::open();

	{	// for testing memory leaks

		assert(tu_types_validate());

		const char* infile = NULL;

		float	exit_timeout = 0;
		bool	do_render = true;
		bool	do_sound = true;
		bool	do_loop = true;
		bool	sdl_abort = true;
		bool	sdl_cursor = true;
		float	tex_lod_bias;
		bool	force_realtime_framerate = false;

	#ifdef _WIN32

		WSADATA wsaData;

		int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
		if ( iResult != NO_ERROR )
			printf("Error at WSAStartup()\n");
	#endif


		// -1.0 tends to look good.
		tex_lod_bias = -1.2f;
		tu_string flash_vars;

		int	width = 0;
		int	height = 0;

		gameswf::gc_ptr<gameswf::player> player = new gameswf::player();

		for (int arg = 1; arg < argc; arg++)
		{
			if (argv[arg][0] == '-')
			{
				// Looks like an option.

				if (argv[arg][1] == 'h')
				{
					// Help.
					print_usage();
					exit(1);
				}
				if (argv[arg][1] == 'u')
				{
					arg++;
					if (arg < argc)
					{
						flash_vars = argv[arg];
					}
					else
					{
						fprintf(stderr, "-u arg must be followed string like myvar=x&myvar2=y and so on\n");
						print_usage();
						exit(1);
					}

				}
				else if (argv[arg][1] == 'w')
				{
					arg++;
					if (arg < argc)
					{
						width = atoi(argv[arg]);
						const char* x = strstr(argv[arg], "x");
						if (x)
						{
							height = atoi(x + 1);
						}
					}

					if (width <=0 || height <= 0)
					{
						fprintf(stderr, "-w arg must be followed by the window size\n");
						print_usage();
						exit(1);
					}
				}
				else if (argv[arg][1] == 'c')
				{
					sdl_abort = false;
				}
                else if (argv[arg][1] == 'f')
                {
                    force_realtime_framerate = true;
                }
				else if (argv[arg][1] == 'k')
				{
					sdl_cursor = false;
				}
				else if (argv[arg][1] == 'a')
				{
					// Set antialiasing on or off.
					arg++;
					if (arg < argc)
					{
						s_aa_level = atoi(argv[arg]);
						s_antialiased = s_aa_level > 0 ? true : false;
					}
					else
					{
						fprintf(stderr, "-a arg must be followed by the antialiasing level\n");
						print_usage();
						exit(1);
					}
				}
				else if (argv[arg][1] == 'b')
				{
					// Set default bit depth.
					arg++;
					if (arg < argc)
					{
						s_bit_depth = atoi(argv[arg]);
						if (s_bit_depth != 16 && s_bit_depth != 24 && s_bit_depth != 32)
						{
							fprintf(stderr, "Command-line supplied bit depth %d, but it must be 16, 24 or 32", s_bit_depth);
							print_usage();
							exit(1);
						}
					}
					else
					{
						fprintf(stderr, "-b arg must be followed by 16 or 32 to set bit depth\n");
						print_usage();
						exit(1);
					}
				}
				else if (argv[arg][1] == 'd')
				{
					// Set a delay
					arg++;
					if (arg < argc)
					{
						s_delay = atoi(argv[arg]);
					}
					else
					{
						fprintf(stderr, "-d arg must be followed by number of milli-seconds to del in the main loop\n");
						print_usage();
						exit(1);
					}
				}
				else if (argv[arg][1] == 'p')
				{
					// Enable frame-rate/performance logging.
					s_measure_performance = true;
				}
				else if (argv[arg][1] == '1')
				{
					// Play once; don't loop.
					do_loop = false;
				}
				else if (argv[arg][1] == 'r')
				{
					// Set rendering on/off.
					arg++;
					if (arg < argc)
					{
						const int render_arg = atoi(argv[arg]);
						switch (render_arg) {
						case 0:
							// Disable both
							do_render = false;
							do_sound = false;
							break;
						case 1:
							// Enable both
							do_render = true;
							do_sound = true;
							break;
						case 2:
							// Disable just sound
							do_render = true;
							do_sound = false;
							break;
						default:
							fprintf(stderr, "-r must be followed by 0, 1 or 2 (%d is invalid)\n",
								render_arg);
							print_usage();
							exit(1);
							break;
						}
					} else {
						fprintf(stderr, "-r must be followed by 0 an argument to disable/enable rendering\n");
						print_usage();
						exit(1);
					}
				}
				else if (argv[arg][1] == 't')
				{
					// Set timeout.
					arg++;
					if (arg < argc)
					{
						exit_timeout = (float) atof(argv[arg]);
					}
					else
					{
						fprintf(stderr, "-t must be followed by an exit timeout, in seconds\n");
						print_usage();
						exit(1);
					}
				}
				else if (argv[arg][1] == 'v')
				{
					// Be verbose; i.e. print log messages to stdout.
					if (argv[arg][2] == 'a')
					{
						// Enable spew re: action.
						player->verbose_action(true);
					}
					else if (argv[arg][2] == 'p')
					{
						// Enable parse spew.
						player->verbose_parse(true);
					}
					// ...
				}
				else if (argv[arg][1] == 'm')
				{
					if (argv[arg][2] == 'l') {
						arg++;
						tex_lod_bias = (float) atof(argv[arg]);
						//printf("Texture LOD Bais is no %f\n", tex_lod_bias);
					}
					else
					{
						fprintf(stderr, "unknown variant of -m arg\n");
						print_usage();
						exit(1);
					}
				}
				else if (argv[arg][1] == 'n')
				{
					s_allow_http = true;
				}
				else if (argv[arg][1] == 'i')
				{
					player->set_separate_thread(false);
					player->set_log_bitmap_info(true);
				}
			}
			else
			{
				infile = argv[arg];
			}
		}

		if (infile == NULL)
		{
			printf("no input file\n");
			print_usage();
			exit(1);
		}

		player->set_force_realtime_framerate(force_realtime_framerate);

		// use this for multifile games
		// workdir is used when LoadMovie("myfile.swf", _root) is called
		{
			tu_string workdir;
			// Find last slash or backslash.
 			const char* ptr = infile + strlen(infile);
			for (; ptr >= infile && *ptr != '/' && *ptr != '\\'; ptr--) {}
			// Use everything up to last slash as the "workdir".
			int len = ptr - infile + 1;
			if (len > 0)
			{
				tu_string workdir(infile, len);
				player->set_workdir(workdir.c_str());
			}
		}

		gameswf::register_file_opener_callback(file_opener);
		gameswf::register_fscommand_callback(fs_callback);
		if (gameswf::get_verbose_parse())
		{
			gameswf::register_log_callback(log_callback);
		}
		
		gameswf::sound_handler*	sound = NULL;
		gameswf::render_handler*	render = NULL;
		if (do_render)
		{
			if (do_sound)
			{

#if TU_USE_SDL == 1
				sound = gameswf::create_sound_handler_sdl();
#endif

#if TU_USE_OPENAL == 1
				sound = gameswf::create_sound_handler_openal();
#endif

				gameswf::set_sound_handler(sound);
			}

#if TU_USE_SDL == 1
			render = gameswf::create_render_handler_ogl();
			gameswf::set_render_handler(render);
#endif

#if TU_USE_OGLES == 1
			render = gameswf::create_render_handler_ogles();
			gameswf::set_render_handler(render);
#endif

#if TU_CONFIG_LINK_TO_FREETYPE == 1
			gameswf::set_glyph_provider(gameswf::create_glyph_provider_freetype());
#else
			gameswf::set_glyph_provider(gameswf::create_glyph_provider_tu());
#endif
		}

		//
		//	set_proxy("192.168.1.201", 8080);
		//

		// gameswf::set_use_cache_files(true);

		player->set_flash_vars(flash_vars);
		{
			gameswf::gc_ptr<gameswf::root>	m = player->load_file(infile);
			if (m == NULL)
			{
				exit(1);
			}

			if (width == 0 || height == 0)
			{
				width = m->get_movie_width();
				height = m->get_movie_height();
			}
			float scale_x = (float) width / m->get_movie_width();
			float scale_y = (float) height / m->get_movie_height();

			float	movie_fps = m->get_movie_fps();

			if (do_render)
			{
				// Initialize the SDL subsystems we're using. Linux
				// and Darwin use Pthreads for SDL threads, Win32
				// doesn't. Otherwise the SDL event loop just polls.
				if (sdl_abort)
				{
					//  Other flags are SDL_INIT_JOYSTICK | SDL_INIT_CDROM
	#ifdef _WIN32
					if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	#else
					if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTTHREAD ))
	#endif
					{
						fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
						exit(1);
					}
				}
				else
				{
					fprintf(stderr, "warning: SDL won't trap core dumps \n");
	#ifdef _WIN32
					if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE  | SDL_INIT_EVENTTHREAD))
	#else
					if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE))
	#endif
					{
						fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
						exit(1);
					}
				}

				atexit(SDL_Quit);

				SDL_EnableKeyRepeat(250, 33);
				SDL_ShowCursor(sdl_cursor ? SDL_ENABLE : SDL_DISABLE);

				switch (s_bit_depth)
				{
					case 16:
						// 16-bit color, surface creation is likely to succeed.
						SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
						SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
						SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
						SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
						SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
						SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 5);
						break;

					case 24:
						// 24-bit color
						//	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
						//	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
						//	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
						//	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
						//	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
						SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
						SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
						break;

					case 32:
						// 32-bit color etc, for getting dest alpha,
						// for MULTIPASS_ANTIALIASING (see gameswf_render_handler_ogl.cpp).
						SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
						SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
						SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
						SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
						SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
						SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
						SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
						break;

					default:
						assert(0);
				}

				// try to enable FSAA
				if (s_aa_level > 1)
				{
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, s_aa_level);
				}

				// Change the LOD BIAS values to tweak blurriness.
				if (tex_lod_bias != 0.0f) {
	#ifdef FIX_I810_LOD_BIAS	
					// If 2D textures weren't previously enabled, enable
					// them now and force the driver to notice the update,
					// then disable them again.
					if (!glIsEnabled(GL_TEXTURE_2D)) {
						// Clearing a mask of zero *should* have no
						// side effects, but coupled with enbling
						// GL_TEXTURE_2D it works around a segmentation
						// fault in the driver for the Intel 810 chip.
						glEnable(GL_TEXTURE_2D);
						glClear(0);
						glDisable(GL_TEXTURE_2D);
					}
	#endif // FIX_I810_LOD_BIAS
					glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, tex_lod_bias);
				}

				// Set the video mode.
//				if (SDL_SetVideoMode(width, height, s_bit_depth, SDL_OPENGL | SDL_RESIZABLE) == 0)
				if (SDL_SetVideoMode(width, height, s_bit_depth, SDL_OPENGL) == 0)
				{
					fprintf(stderr, "SDL_SetVideoMode() failed.");
					exit(1);
				}

				render->open();
				render->set_antialiased(s_antialiased);

				// Turn on alpha blending.
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				// Turn on line smoothing.  Antialiased lines can be used to
				// smooth the outsides of shapes.
				glEnable(GL_LINE_SMOOTH);
				glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);	// GL_NICEST, GL_FASTEST, GL_DONT_CARE

				glMatrixMode(GL_PROJECTION);
				glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				// We don't need lighting effects
				glDisable(GL_LIGHTING);
				// glColorPointer(4, GL_UNSIGNED_BYTE, 0, *);
				// glInterleavedArrays(GL_T2F_N3F_V3F, 0, *)
				glPushAttrib (GL_ALL_ATTRIB_BITS);		
			}

			// Mouse state.
			int	mouse_x = 0;
			int	mouse_y = 0;
			int	mouse_buttons = 0;

			float	speed_scale = 1.0f;
			Uint32	start_ticks = 0;
			if (do_render)
			{
				start_ticks = tu_timer::get_ticks();
			}
			Uint32	last_ticks = start_ticks;
			int	frame_counter = 0;
			int	last_logged_fps = last_ticks;
			int fps = 0;

			//TODO
	//		gameswf::player* p = gameswf::create_player();
	//		p.run();


			for (;;)
			{
				Uint32	ticks;
				if (do_render)
				{
					ticks = tu_timer::get_ticks();
				}
				else
				{
					// Simulate time.
					ticks = last_ticks + (Uint32) (1000.0f / movie_fps);
				}
				int	delta_ticks = ticks - last_ticks;
				float	delta_t = delta_ticks / 1000.f;
				last_ticks = ticks;

				// Check auto timeout counter.
				if (exit_timeout > 0
					&& ticks - start_ticks > (Uint32) (exit_timeout * 1000))
				{
					// Auto exit now.
					break;
				}

				bool ret = true;
				if (do_render)
				{
					SDL_Event	event;
					// Handle input.
					while (ret)
					{
						if (SDL_PollEvent(&event) == 0)
						{
							break;
						}

						//printf("EVENT Type is %d\n", event.type);
						switch (event.type)
						{
						case SDL_VIDEORESIZE:
							//	TODO
							//					s_scale = (float) event.resize.w / (float) width;
							//					width = event.resize.w;
							//					height = event.resize.h;
							//					if (SDL_SetVideoMode(event.resize.w, event.resize.h, 0, SDL_OPENGL | SDL_RESIZABLE) == 0)
							//					{
							//						fprintf(stderr, "SDL_SetVideoMode() failed.");
							//						exit(1);
							//					}
							//					glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);
							break;

						case SDL_USEREVENT:
							//printf("SDL_USER_EVENT at %s, code %d%d\n", __FUNCTION__, __LINE__, event.user.code);
							ret = false;
							break;
						case SDL_KEYDOWN:
							{
								SDLKey	key = event.key.keysym.sym;
								bool	ctrl = (event.key.keysym.mod & KMOD_CTRL) != 0;

								if (key == SDLK_ESCAPE
									|| (ctrl && key == SDLK_q)
									|| (ctrl && key == SDLK_w))
								{
									goto done;
								}
								else if (ctrl && key == SDLK_p)
								{
									// Toggle paused state.
									if (m->get_play_state() == gameswf::character::STOP)
									{
										m->set_play_state(gameswf::character::PLAY);
									}
									else
									{
										m->set_play_state(gameswf::character::STOP);
									}
								}
								else if (ctrl && key == SDLK_i)
								{
									// Init library, for detection of memory leaks (for testing purposes)
	/*
									// Clean up gameswf as much as possible, so valgrind will help find actual leaks.

									gameswf::set_sound_handler(NULL);
									delete sound;

									gameswf::set_render_handler(NULL);
									delete render;

									if (do_render)
									{
										if (do_sound)
										{
											sound = gameswf::create_sound_handler_sdl();
											gameswf::set_sound_handler(sound);
										}
										render = gameswf::create_render_handler_ogl();
										gameswf::set_render_handler(render);
									}
	*/
									// Load the actual movie.
									m = player->load_file(infile);
									if (m == NULL)
									{
										exit(1);
									}
								}
								else if (ctrl && (key == SDLK_LEFTBRACKET || key == SDLK_KP_MINUS))
								{
									m->goto_frame(m->get_current_frame()-1);
								}
								else if (ctrl && (key == SDLK_RIGHTBRACKET || key == SDLK_KP_PLUS))
								{
									m->goto_frame(m->get_current_frame()+1);
								}
								else if (ctrl && key == SDLK_a)
								{
									// Toggle antialiasing.
									s_antialiased = !s_antialiased;
									if (render)
									{
										render->set_antialiased(s_antialiased);
									}
								}
								else if (ctrl && key == SDLK_t)
								{
									// test text replacement / variable setting:
									m->set_variable("test.text", "set_edit_text was here...\nanother line of text for you to see in the text box");
								}
								else if (ctrl && key == SDLK_g)
								{
									// test get_variable.
									message_log("testing get_variable: '");
									message_log(m->get_variable("test.text"));
									message_log("'\n");
								}
								else if (ctrl && key == SDLK_m)
								{
									// Test call_method.
									const char* result = m->call_method(
										"test_call",
										"%d, %f, %s, %ls",
										200,
										1.0f,
										"Test string",
										L"Test long string");

									if (result)
									{
										message_log("call_method: result = ");
										message_log(result);
										message_log("\n");
									}
									else
									{
										message_log("call_method: null result\n");
									}
								}
								else if (ctrl && key == SDLK_b)
								{
									// toggle background color.
									s_background = !s_background;
								}
	//							else if (ctrl && key == SDLK_f)	//xxxxxx
	//							{
	//								extern bool gameswf_debug_show_paths;
	//								gameswf_debug_show_paths = !gameswf_debug_show_paths;
	//							}
								else if (ctrl && key == SDLK_EQUALS)
								{
									float	f = gameswf::get_curve_max_pixel_error();
									f *= 1.1f;
									gameswf::set_curve_max_pixel_error(f);
									printf("curve error tolerance = %f\n", f);
								}
								else if (ctrl && key == SDLK_MINUS)
								{
									float	f = gameswf::get_curve_max_pixel_error();
									f *= 0.9f;
									gameswf::set_curve_max_pixel_error(f);
									printf("curve error tolerance = %f\n", f);
								} else if (ctrl && key == SDLK_F2) {
									// Toggle wireframe.
									static bool wireframe_mode = false;
									wireframe_mode = !wireframe_mode;
									if (wireframe_mode) {
										glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
									} else {
										glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
									}
									// TODO: clean up this interafce and re-enable.
									// 					} else if (ctrl && key == SDLK_d) {
									// 						// Flip a special debug flag.
									// 						gameswf_tesselate_dump_shape = true;
								}

								gameswf::key::code c = translate_key(key);
								if (c != gameswf::key::INVALID)
								{
									player->notify_key_event(c, true);
								}

								break;
							}

						case SDL_KEYUP:
							{
								SDLKey	key = event.key.keysym.sym;

								gameswf::key::code c = translate_key(key);
								if (c != gameswf::key::INVALID)
								{
									player->notify_key_event(c, false);
								}

								break;
							}

						case SDL_MOUSEMOTION:
							mouse_x = (int) (event.motion.x / scale_x);
							mouse_y = (int) (event.motion.y / scale_y);
							break;

						case SDL_MOUSEBUTTONDOWN:
						case SDL_MOUSEBUTTONUP:
							{
								int	mask = 1 << (event.button.button - 1);
								if (event.button.state == SDL_PRESSED)
								{
									mouse_buttons |= mask;
								}
								else
								{
									mouse_buttons &= ~mask;
								}
								break;
							}

						case SDL_QUIT:
							goto done;
							break;

						default:
							break;
						}
					}
				}

				if (do_render)
				{
					glDisable(GL_DEPTH_TEST);	// Disable depth testing.
					glDrawBuffer(GL_BACK);
				}

				m = player->get_root();
				m->set_display_viewport(0, 0, width, height);
				m->set_background_alpha(s_background ? 1.0f : 0.05f);

				m->notify_mouse_state(mouse_x, mouse_y, mouse_buttons);

				Uint32 t_advance = tu_timer::get_ticks();
				m->advance(delta_t * speed_scale);
				t_advance = tu_timer::get_ticks() - t_advance;

				if (do_sound && sound)
				{
					sound->advance(delta_t * speed_scale);
				}

				Uint32 t_display = tu_timer::get_ticks();
				m->display();
				t_display = tu_timer::get_ticks() - t_display;

				if (do_render)
				{
					Uint32 t_swap = tu_timer::get_ticks();
					SDL_GL_SwapBuffers();
					t_swap = tu_timer::get_ticks() - t_swap;
					//glPopAttrib ();


					frame_counter++;

					// Log the frame rate every second or so.
					if (last_ticks - last_logged_fps > 1000)
					{
						float	delta = (last_ticks - last_logged_fps) / 1000.f;
						fps = (int) ((float) frame_counter / delta);
						last_logged_fps = last_ticks;
						frame_counter = 0;
					}

					if (s_measure_performance == false)
					{
						// Don't hog the CPU.
						SDL_Delay(s_delay);
					}
					else
					{
						printf("fps = %d\n", fps);
					}

					// for perfomance testing
//					printf("advance time: %d, display time %d, swap buffers time = %d\n",
//						t_advance, t_display, t_swap);

#ifdef HAVE_PERFOMANCE_INFO
					char buffer[8];
					snprintf(buffer, 8, "%03d", t_advance);
					m->set_variable("t_Advance", buffer);
					snprintf(buffer, 8, "%03d", t_display);
					m->set_variable("t_Display", buffer);
					snprintf(buffer, 8, "%03d", t_swap);
					m->set_variable("t_SwapBuffers", buffer);
					snprintf(buffer, 8, "%d", fps);
					m->set_variable("FPS", buffer);
#endif
				}

				// TODO: clean up this interface and re-enable.
				//		gameswf_tesselate_dump_shape = false;  ///xxxxx

				// See if we should exit.
				if (do_loop == false && m->get_current_frame() + 1 == m->get_frame_count())
				{
					// We're reached the end of the movie; exit.
					break;
				}
			}

	done:


			gameswf::set_sound_handler(NULL);
			delete sound;

			gameswf::set_render_handler(NULL);
			delete render;

			SDL_Quit();
		}

	}	// for testing memory leaks

	tu_memdebug::close();

	return 0;
}

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
