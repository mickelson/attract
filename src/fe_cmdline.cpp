/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-15 Andrew Mickelson
 *
 *  This file is part of Attract-Mode.
 *
 *  Attract-Mode is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Attract-Mode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Attract-Mode.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "fe_settings.hpp"
#include "fe_util.hpp"
#include <iostream>
#include <cstring>
#include <SFML/Graphics/Shader.hpp>

void process_args( int argc, char *argv[],
			std::string &config_path,
			std::string &cmdln_font,
			bool &process_console )
{
	//
	// Deal with command line arguments
	//
	std::vector <FeImportTask> task_list;
	std::string output_name;
	FeFilter filter( "" );
	bool full=false;

	int next_arg=1;

	while ( next_arg < argc )
	{
		if (( strcmp( argv[next_arg], "-c" ) == 0 )
				|| ( strcmp( argv[next_arg], "--config" ) == 0 ))
		{
			next_arg++;
			if ( next_arg < argc )
			{
				config_path = argv[next_arg];
				next_arg++;
			}
			else
			{
				std::cerr << "Error, no config directory specified with --config option." << std::endl;
				exit(1);
			}
		}
		else if (( strcmp( argv[next_arg], "-f" ) == 0 )
				|| ( strcmp( argv[next_arg], "--font" ) == 0 ))
		{
			next_arg++;
			if ( next_arg < argc )
			{
				cmdln_font = argv[next_arg];
				next_arg++;
			}
			else
			{
				std::cerr << "Error, no font name specified with --font option." << std::endl;
				exit(1);
			}
		}
		else if (( strcmp( argv[next_arg], "-b" ) == 0 )
				|| ( strcmp( argv[next_arg], "--build-romlist" ) == 0 ))
		{
			next_arg++;
			int first_cmd_arg = next_arg;

			for ( ; next_arg < argc; next_arg++ )
			{
				if ( argv[next_arg][0] == '-' )
					break;

				task_list.push_back( FeImportTask( FeImportTask::BuildRomlist, argv[next_arg] ));
			}

			if ( next_arg == first_cmd_arg )
			{
				std::cerr << "Error, no target emulators specified with --build-romlist option."
							<<  std::endl;
				exit(1);
			}
		}
		else if (( strcmp( argv[next_arg], "-i" ) == 0 )
				|| ( strcmp( argv[next_arg], "--import-romlist" ) == 0 ))
		{
			std::string my_filename;
			std::string my_emulator;

			next_arg++;
			if ( next_arg < argc )
			{
				my_filename = argv[next_arg];
				next_arg++;
			}
			else
			{
				std::cerr << "Error, no filename specified with --import-romlist option." << std::endl;
				exit(1);
			}

			if ( ( next_arg < argc ) && ( argv[next_arg][0] != '-' ) )
			{
				my_emulator = argv[ next_arg ];
				next_arg++;
			}

			task_list.push_back( FeImportTask( FeImportTask::ImportRomlist, my_emulator, my_filename ));
		}
		else if (( strcmp( argv[next_arg], "-o" ) == 0 )
				|| ( strcmp( argv[next_arg], "--output" ) == 0 ))
		{
			next_arg++;
			if ( next_arg < argc )
			{
				output_name = argv[next_arg];
				next_arg++;
			}
			else
			{
				std::cerr << "Error, no output filename specified with --output option." << std::endl;
				exit(1);
			}
		}
		else if ( strcmp( argv[next_arg], "--full" ) == 0 )
		{
			full = true;
			next_arg++;
		}
		else if (( strcmp( argv[next_arg], "-F" ) == 0 )
				|| ( strcmp( argv[next_arg], "--filter" ) == 0 ))
		{
			FeRule rule;

			next_arg++;
			if ( next_arg < argc )
			{
				if ( rule.process_setting( "rule", argv[next_arg], "" ) != 0 )
				{
					// Error message already displayed
					exit(1);
				}
				next_arg++;
			}
			else
			{
				std::cerr << "Error, no rule specified with --filter option." << std::endl;
				exit(1);
			}

			filter.get_rules().push_back( rule );
		}
		else if (( strcmp( argv[next_arg], "-E" ) == 0 )
				|| ( strcmp( argv[next_arg], "--exception" ) == 0 ))
		{
			FeRule ex;

			next_arg++;
			if ( next_arg < argc )
			{
				if ( ex.process_setting( "exception", argv[next_arg], "" ) != 0 )
				{
					// Error message already displayed
					exit(1);
				}
				next_arg++;
			}
			else
			{
				std::cerr << "Error, no exception specified with --exception option." << std::endl;
				exit(1);
			}

			filter.get_rules().push_back( ex );
		}
		else if (( strcmp( argv[next_arg], "-s" ) == 0 )
				|| ( strcmp( argv[next_arg], "--scrape-art" ) == 0 ))
		{
			next_arg++;
			int first_cmd_arg = next_arg;

			for ( ; next_arg < argc; next_arg++ )
			{
				if ( argv[next_arg][0] == '-' )
					break;

				task_list.push_back( FeImportTask( FeImportTask::ScrapeArtwork, argv[next_arg] ));
			}

			if ( next_arg == first_cmd_arg )
			{
				std::cerr << "Error, no target emulators specified with --scrape-art option."
							<<  std::endl;
				exit(1);
			}
		}
		else if (( strcmp( argv[next_arg], "-v" ) == 0 )
				|| ( strcmp( argv[next_arg], "--version" ) == 0 ))
		{
			std::cout << FE_NAME << " " << FE_VERSION << " ("
				<< get_OS_string()
				<< ", SFML " << SFML_VERSION_MAJOR << '.' << SFML_VERSION_MINOR
#ifdef USE_FONTCONFIG
				<< " +FontConfig"
#endif
#ifdef USE_XINERAMA
				<< " +Xinerama"
#endif
#ifdef FE_RPI
				<< " +RPi"
#endif
#ifndef NO_SWF
				<< " +SWF"
#endif
#ifdef USE_LIBARCHIVE
				<< " +7z"
#endif
				<< ") " << std::endl << std::endl;

#ifdef NO_MOVIE
			std::cout << "No Video, using SFML for Audio." << std::endl;
#else
			print_ffmpeg_version_info();
#endif
			std::cout << std::endl;

			if ( sf::Shader::isAvailable() )
				std::cout << "Shaders are available." << std::endl;
			else
				std::cout << "Shaders are not available." << std::endl;

			exit(0);
		}
#ifndef SFML_SYSTEM_WINDOWS
		else if ( strcmp( argv[next_arg], "--console" ) == 0 )
		{
			process_console=true;
			next_arg++;
		}
#endif
		else
		{
			int retval=1;
			if (( strcmp( argv[next_arg], "-h" ) != 0 )
				&& ( strcmp( argv[next_arg], "--help" ) != 0 ))
			{
				std::cerr << "Unrecognized command line option: "
					<< argv[next_arg] <<  std::endl;
				retval=0;
			}

			std::cout << FE_COPYRIGHT << std::endl
				<< "Usage: " << argv[0] << " [option...]" << std::endl << std::endl
				<< "ROMLIST IMPORT/BUILD OPTIONS:" << std::endl
				<< "  -b, --build-romlist <emu> [emu(s)...]" << std::endl
				<< "     Builds a romlist using the configuration for the specified emulator(s)" << std::endl
				<< "  -i, --import-romlist <file> [emu]" << std::endl
				<< "     Import romlist from the specified file. Supported formats:" << std::endl
				<< "        *.lst (Mamewah/Wahcade!)" << std::endl
				<< "        *.txt (Attract-Mode)" << std::endl
				<< "        *.xml (Mame listxml format or HyperSpin)" << std::endl
				<< "     The emulator to use for list entries can be specified as well" << std::endl
				<< "  -F, --filter <rule>" << std::endl
				<< "     Apply the specified filter rule when creating romlist" << std::endl
				<< "  -E, --exception <exception>" << std::endl
				<< "     Apply the specified filter rules exception when creating romlist" << std::endl
				<< "  --full" << std::endl
				<< "     Use with --build-romlist to include all possible roms [MAME only]" << std::endl
				<< "  -o, --output <romlist>" << std::endl
				<< "     Specify the name of the romlist to create, overwriting any existing"
				<< std::endl << std::endl
				<< "ARTWORK SCRAPER OPTIONS:" << std::endl
				<< "  -s, --scrape-art <emu> [emu(s)...]" << std::endl
				<< "     Scrape missing artwork for the specified emulator(s)"
				<< std::endl << std::endl
				<< "OTHER OPTIONS:" << std::endl
				<< "  -c, --config <config_directory>" << std::endl
				<< "     Specify the configuration to use" << std::endl
				<< "  -f, --font <font_name>" << std::endl
				<< "     Specify the default font to use" << std::endl
#ifndef SFML_SYSTEM_WINDOWS
				<< "  --console" << std::endl
				<< "     Enable script console" << std::endl
#endif
				<< "  -h, --help: Show this message" << std::endl
				<< "  -v, --version: Show version information" << std::endl;
			exit( retval );
		}
	}

	if ( !task_list.empty() )
	{
		FeSettings feSettings( config_path, cmdln_font );
		feSettings.load_from_file( feSettings.get_config_dir() + FE_CFG_FILE );

		int retval = feSettings.build_romlist( task_list, output_name, filter, full );
		exit( retval );
	}
}
