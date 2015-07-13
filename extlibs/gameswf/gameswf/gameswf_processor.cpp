// gameswf_processor.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A SWF preprocessor for the gameswf library.  Loads a set of SWF
// files and generates precomputed data such as font bitmaps and shape
// tesselation meshes.  The precomputed data can be appended to the
// original SWF files, so that gameswf can more rapidly load those SWF
// files later.


#include "base/tu_file.h"
#include "base/container.h"
#include "gameswf/gameswf.h"
#include "gameswf/gameswf_impl.h"


static bool	s_verbose = false;


static void	message_log(const char* message)
// Process a log message.
{
	if (s_verbose)
	{
		fputs(message, stdout);
		//flush(stdout); // needed on osx for some reason
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
	}
	else
	{
		message_log(message);
	}
}


static tu_file*	file_opener(const char* url)
// Callback function.  This opens files for the gameswf library.
{
	return new tu_file(url, "rb");
}


static void	print_usage()
{
	printf(
		"gameswf_processor -- a SWF preprocessor for gameswf.\n"
		"\n"
		"This program has been donated to the Public Domain.\n"
		"See http://tulrich.com/geekstuff/gameswf.html for more info.\n"
		"\n"
		"usage: gameswf_processor [options] [swf files to process...]\n"
		"\n"
		"Preprocesses the given SWF movie files.  Optionally write preprocessed shape\n"
		"and font data to cache files, so the associated SWF files can be loaded\n"
		"faster by gameswf.\n"
		"\n"
		"options:\n"
		"\n"
		"  -h          Print this info.\n"
		"  -w          Write a .gsc file with preprocessed info, for each input file.\n"
		"  -v          Be verbose; i.e. print log messages to stdout\n"
		"  -vp         Be verbose about movie parsing\n"
		"  -va         Be verbose about ActionScript\n"
		);
}


struct movie_data
{
	gameswf::movie_definition*	m_movie;
	tu_string	m_filename;
};


static gameswf::movie_definition*	play_movie(gameswf::player* player, const char* filename);
static int	write_cache_file(const movie_data& md);


static bool	s_do_output = false;
static bool	s_stop_on_errors = true;


int	main(int argc, char *argv[])
{
	assert(tu_types_validate());

	array<const char*> infiles;

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
			else if (argv[arg][1] == 'w')
			{
				// Write cache files.
				s_do_output = true;
			}
			else if (argv[arg][1] == 'v')
			{
				// Be verbose; i.e. print log messages to stdout.
				s_verbose = true;

				if (argv[arg][2] == 'a')
				{
					// Enable spew re: action.
					gameswf::set_verbose_action(true);
				}
				else if (argv[arg][2] == 'p')
				{
					// Enable parse spew.
					gameswf::set_verbose_parse(true);
				}
				// ...
			}
		}
		else
		{
			infiles.push_back(argv[arg]);
		}
	}

	if (infiles.size() == 0)
	{
		printf("no input files\n");
		print_usage();
		exit(1);
	}
	gameswf::gc_ptr<gameswf::player> player = new gameswf::player();
	gameswf::register_file_opener_callback(file_opener);
	gameswf::register_log_callback(log_callback);
	gameswf::set_use_cache_files(false);	// don't load old cache files!

	array<movie_data>	data;

	// Play through all the movies.
	for (int i = 0, n = infiles.size(); i < n; i++)
	{
		gameswf::movie_definition*	m = play_movie(player.get_ptr(), infiles[i]);
		if (m == NULL)
		{
			if (s_stop_on_errors)
			{
				// Fail.
				fprintf(stderr, "error playing through movie '%s', quitting\n", infiles[i]);
				exit(1);
			}
		}
		
		movie_data	md;
		md.m_movie = m;
		md.m_filename = infiles[i];
		data.push_back(md);
	}

	// Now append processed data.
	if (s_do_output)
	{
		for (int i = 0, n = data.size(); i < n; i++)
		{
			int	error = write_cache_file(data[i]);
			if (error)
			{
				if (s_stop_on_errors)
				{
					// Fail.
					fprintf(stderr, "error processing movie '%s', quitting\n", data[i].m_filename.c_str());
					exit(1);
				}
			}
		}
	}

	return 0;
}


gameswf::movie_definition*	play_movie(gameswf::player* player, const char* filename)
// Load the named movie, make an instance, and play it, virtually.
// I.e. run through and render all the frames, even though we are not
// actually doing any output (our output handlers are disabled).
//
// What this does is warm up all the cached data in the movie, so that
// if we save that data for later, we won't have to tesselate shapes
// or build font textures again.
//
// Return the movie definition.
{
	gameswf::movie_definition*	md = player->create_movie(filename);
	if (md == NULL)
	{
		fprintf(stderr, "error: can't play movie '%s'\n", filename);
		exit(1);
	}
	gameswf::root*	m = md->create_instance();
	if (m == NULL)
	{
		fprintf(stderr, "error: can't create instance of movie '%s'\n", filename);
		exit(1);
	}

	int	kick_count = 0;

	// Run through the movie.
	player->set_root(m);
	for (;;)
	{
		// @@ do we also have to run through all sprite frames
		// as well?
		//
		// @@ also, ActionScript can rescale things
		// dynamically -- we can't really do much about that I
		// guess?
		//
		// @@ Maybe we should allow the user to specify some
		// safety margin on scaled shapes.

		int	last_frame = m->get_current_frame();
		m->advance(0.010f);
		m->display();

		if (m->get_current_frame() == md->get_frame_count() - 1)
		{
			// Done.
			break;
		}

		if (m->get_play_state() == gameswf::character::STOP)
		{
			// Kick the movie.
			printf("kicking movie, kick ct = %d\n", kick_count);
			m->goto_frame(last_frame + 1);
			m->set_play_state(gameswf::character::PLAY);
			kick_count++;

			if (kick_count > 10)
			{
				printf("movie is stalled; giving up on playing it through.\n");
				break;
			}
		}
		else if (m->get_current_frame() < last_frame)
		{
			// Hm, apparently we looped back.  Skip ahead...
			printf("loop back; jumping to frame %d\n", last_frame);
			m->goto_frame(last_frame + 1);
		}
		else
		{
			kick_count = 0;
		}
	}

	return md;
}


int	write_cache_file(const movie_data& md)
// Write a cache file for the given movie.
{
	// Open cache file.
	tu_string	cache_filename(md.m_filename);
	cache_filename += ".gsc";
	tu_file	out(cache_filename.c_str(), "wb");	// "gsc" == "gameswf cache"
	if (out.get_error() == TU_FILE_NO_ERROR)
	{
		// Write out the data.
		gameswf::cache_options	opt;
		md.m_movie->output_cached_data(&out, opt);
		if (out.get_error() == TU_FILE_NO_ERROR)
		{
			printf(
				"wrote '%s'\n",
				cache_filename.c_str());
		}
		else
		{
			fprintf(stderr, "error: write failure to '%s'\n", cache_filename.c_str());
		}
	}
	else
	{
		fprintf(stderr, "error: can't open '%s' for cache file output\n", cache_filename.c_str());
		return 1;
	}

	// // xxx temp debug code: dump cached data to stdout
	// tu_file	tu_stdout(stdout, false);
	// tu_stdout.copy_from(&cached_data);

	return 0;
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
