/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013 Andrew Mickelson
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
#include "fe_present.hpp"
#include "fe_overlay.hpp"
#include "fe_util.hpp"
#include "fe_image.hpp"
#include "fe_sound.hpp"
#include "fe_text.hpp"
#include "fe_window.hpp"
#include "fe_vm.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>

#ifndef NO_MOVIE
#include <Audio/AudioDevice.hpp>
#endif

void process_args( int argc, char *argv[],
			std::string &config_path,
			std::string &cmdln_font )
{
	//
	// Deal with command line arguments
	//
	std::vector <FeImportTask> task_list;
	std::string output_name;

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

				task_list.push_back( FeImportTask( "", argv[next_arg] ));
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

			task_list.push_back( FeImportTask( my_filename, my_emulator ));
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
		else if (( strcmp( argv[next_arg], "-v" ) == 0 )
				|| ( strcmp( argv[next_arg], "--version" ) == 0 ))
		{
			std::cout << FE_NAME << " " << FE_VERSION << " ("
				<< get_OS_string()
				<< ", SFML " << SFML_VERSION_MAJOR << '.' << SFML_VERSION_MINOR
#ifdef USE_FONTCONFIG
				<< " +FC"
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
				<< "        *.xml (HyperSpin)" << std::endl
				<< "     The emulator to use for list entries can be specified as well" << std::endl
				<< "  -o, --output <romlist>" << std::endl
				<< "     Specify the name of the romlist to create, overwriting any existing"
				<< std::endl << std::endl
				<< "OTHER OPTIONS:" << std::endl
				<< "  -c, --config <config_directory>" << std::endl
				<< "     Specify the configuration to use" << std::endl
				<< "  -f, --font <font_name>" << std::endl
				<< "     Specify the default font to use" << std::endl
				<< "  -h, --help: Show this message" << std::endl
				<< "  -v, --version: Show version information" << std::endl;
			exit( retval );
		}
	}

	if ( !task_list.empty() )
	{
		FeSettings feSettings( config_path, cmdln_font );
		feSettings.load();
		int retval = feSettings.build_romlist( task_list, output_name );
		exit( retval );
	}
}

int main(int argc, char *argv[])
{
	std::string config_path, cmdln_font;
	bool launch_game = false;

	process_args( argc, argv, config_path, cmdln_font );

	//
	// Run the front-end
	//
	FeSettings feSettings( config_path, cmdln_font );
	feSettings.load();

	if ( feSettings.autolaunch_last_game() )
	{
		feSettings.select_last_launch();
		launch_game=true;
	}

	feSettings.init_list();

	std::string defaultFontFile;
	if ( feSettings.get_font_file( defaultFontFile ) == false )
	{
		std::cerr << "Error, could not find default font."  << std::endl;
		return 1;
	}

	FeFontContainer defaultFont;
	defaultFont.set_font( defaultFontFile );

	//
	// Set up music/sound playing objects
	//
#ifndef NO_MOVIE
	sf::AudioDevice audio_device;
#endif
	FeSoundSystem soundsys( &feSettings );
	soundsys.play_ambient();

	FeWindow window( feSettings );
	window.initial_create();

	FePresent fePresent( &feSettings, defaultFont );
	FeOverlay feOverlay( window, feSettings, fePresent );
	FeVM feVM( feSettings, fePresent, window, feOverlay );

	fePresent.load_layout( true );

	bool exit_selected=false;

	if ( feSettings.get_language().empty() )
	{
		// If our language isn't set at this point, we want to prompt the user for the language
		// they wish to use
		//
		if ( feOverlay.languages_dialog() < 0 )
			exit_selected = true;
	}

	soundsys.sound_event( FeInputMap::EventStartup );

	bool redraw=true;
	int guard_joyid=-1, guard_axis=-1;

	// go straight into config mode if there are no lists configured for
	// display
	//
	bool config_mode = ( feSettings.lists_count() < 1 );

	while (window.isOpen() && (!exit_selected))
	{
		if ( config_mode )
		{
			//
			// Enter config mode
			//
			int old_mode = feSettings.get_window_mode();
			if ( feOverlay.config_dialog() )
			{
				// Settings changed, reload
				//
				soundsys.update_volumes();

				if ( feSettings.get_font_file( defaultFontFile ) )
					defaultFont.set_font( defaultFontFile );

				feSettings.init_list();
				fePresent.load_layout();

				soundsys.stop();
				soundsys.play_ambient();

				// Recreate window if the window mode changed
				if ( feSettings.get_window_mode() != old_mode )
				{
					window.on_exit();
					window.initial_create();
				}
			}
			fePresent.reset_screen_saver();
			config_mode=false;
			redraw=true;
		}
		else if ( launch_game )
		{
			soundsys.stop();

			fePresent.pre_run();
			window.run();
			fePresent.post_run();

			soundsys.sound_event( FeInputMap::EventGameReturn );
			soundsys.play_ambient();

			launch_game=false;
			redraw=true;
		}

		FeInputMap::Command c;
		sf::Event ev;
		while ( feVM.poll_command( c, ev ) )
		{
			//
			// Special case handling based on event type
			//
			switch ( ev.type )
			{
				case sf::Event::Closed:
					exit_selected = true;
					break;

				case sf::Event::MouseMoved:
					if ( feSettings.test_mouse_reset( ev.mouseMove.x, ev.mouseMove.y ))
					{
						// We reset the mouse if we are capturing it and it has moved
						// outside of its bounding box
						//
						sf::Vector2u s = window.getSize();
						sf::Mouse::setPosition( sf::Vector2i( s.x / 2, s.y / 2 ), window );
					}
					break;

				case sf::Event::KeyPressed:
				case sf::Event::MouseButtonPressed:
				case sf::Event::JoystickButtonPressed:
					//
					// We always want to reset the screen saver on these events,
					// even if they aren't mapped otherwise (mapped events cause
					// a reset too)
					//
					if (( c == FeInputMap::LAST_COMMAND )
							&& ( fePresent.reset_screen_saver() ))
						redraw = true;
					break;

				case sf::Event::GainedFocus:
				case sf::Event::Resized:
					redraw = true;
					break;


				case sf::Event::JoystickMoved:
					if ( c == FeInputMap::LAST_COMMAND )
					{
						if (( (int)ev.joystickMove.joystickId == guard_joyid )
							&& ( ev.joystickMove.axis == guard_axis ))
						{
							// Reset the joystick guard because the axis we are guarding has moved
							// below the joystick threshold
							guard_joyid = -1;
							guard_axis = -1;
						}
					}
					else
					{
						// Only allow one mapped "Joystick Moved" input through at a time
						//
						if ( guard_joyid != -1 )
							continue;

						guard_joyid = ev.joystickMove.joystickId;
						guard_axis = ev.joystickMove.axis;
					}
					break;

				case sf::Event::Count:
				default:
					break;
			}

			if ( c == FeInputMap::LAST_COMMAND )
				continue;

			//
			// Now handle the command appropriately.
			//
			if ( feVM.handle_event( c ) )
				continue;

			soundsys.sound_event( c );
			if ( fePresent.handle_event( c, ev ) )
				redraw = true;
			else
			{
				// handle the things that fePresent doesn't do
				switch ( c )
				{
				case FeInputMap::ExitMenu:
					{
						int retval = feOverlay.confirm_dialog( "Exit Attract-Mode?" );

						//
						// retval is 0 if the user confirmed exit.
						// it is <0 if we are being forced to close
						//
						if ( retval < 1 )
						{
							exit_selected = true;
							if ( retval == 0 )
								feSettings.exit_command();
						}
						else
							redraw=true;
					}
					break;

				case FeInputMap::ExitNoMenu:
					exit_selected = true;
					break;

				case FeInputMap::ReplayLastGame:
					if ( feSettings.select_last_launch() )
						fePresent.load_layout();
					else
						fePresent.update_to_new_list();

					launch_game=true;
					redraw=true;
					break;

				case FeInputMap::Select:
					launch_game=true;
					break;

				case FeInputMap::ToggleMute:
					feSettings.set_mute( !feSettings.get_mute() );
					soundsys.update_volumes();
					fePresent.toggle_mute();
					break;

				case FeInputMap::ScreenShot:
					{
						std::string filename;
						get_available_filename( feSettings.get_config_dir(),
										"screen", ".png", filename );

						sf::Image sshot_img = window.capture();
						sshot_img.saveToFile( filename );
					}
					break;

				case FeInputMap::Configure:
					config_mode = true;
					break;

				case FeInputMap::ListsMenu:
					{
						std::vector<std::string> names_list;
						feSettings.get_list_names( names_list );

						std::string title;
						feSettings.get_resource( "Lists", title );

						int last_list = names_list.size() - 1;
						if ( feSettings.get_lists_menu_exit() )
						{
							//
							// Add an exit option at the end of the lists menu
							//
							std::string exit_str;
							feSettings.get_resource( "Exit Attract-Mode", exit_str );
							names_list.push_back( exit_str );
						}

						int list_index = feOverlay.common_list_dialog(
										title,
										names_list,
										feSettings.get_current_list_index(),
										names_list.size() - 1 );

						// test if the exit option selected, return -2 if it has been
						if ( list_index > last_list )
							list_index = -2;

						if (( list_index < 0 ) || ( list_index > last_list ))
						{
							// list index is -1 if user pressed the "exit no dialog"
							// button, and is > last_list if they selected the "exit"
							// menu option.  We only want to run the exit command if
							// the menu option was selected
							//
							exit_selected = true;
							if ( list_index > last_list )
								feSettings.exit_command();
						}
						else
						{
							if ( feSettings.set_list( list_index ) )
								fePresent.load_layout();
							else
								fePresent.update_to_new_list();
						}
						redraw=true;
					}
					break;

				case FeInputMap::FiltersMenu:
					{
						std::vector<std::string> names_list;
						feSettings.get_current_list_filter_names( names_list );

						std::string title;
						feSettings.get_resource( "Filters", title );

						int list_index = feOverlay.common_list_dialog(
										title,
										names_list,
										feSettings.get_current_filter_index(),
										names_list.size() - 1 );

						if ( list_index < 0 )
							exit_selected = true;
						else
						{
							feSettings.set_filter( list_index );
							fePresent.update_to_new_list();
						}

						redraw=true;
					}
					break;

				case FeInputMap::ToggleFavourite:
					{
						bool new_state = !feSettings.get_current_fav();

						if ( feSettings.confirm_favs() )
						{
							std::string msg = ( new_state )
								? "Add '$1' to Favourites?"
								: "Remove '$1' from Favourites?";

							// returns 0 if user confirmed toggle
							if ( feOverlay.confirm_dialog(
									msg,
									feSettings.get_rom_info( 0, FeRomInfo::Title ) ) == 0 )
							{
								if ( feSettings.set_current_fav( new_state ) )
									fePresent.update_to_new_list(); // changing fav status altered the current list
							}
						}
						else
						{
							if ( feSettings.set_current_fav( new_state ) )
								fePresent.update_to_new_list(); // changing fav status altered the current list
						}
						redraw = true;
					}
					break;

				case FeInputMap::ToggleTags:
					if ( feOverlay.tags_dialog() < 0 )
						exit_selected = true;

					redraw = true;
					break;

				default:
					break;
				}
			}
		}

		if ( fePresent.tick() )
			redraw=true;

		if ( fePresent.saver_activation_check() )
			soundsys.sound_event( FeInputMap::ScreenSaver );

		if ( redraw )
		{
			// begin drawing
			window.clear();
			window.draw( fePresent );
			window.display();
			redraw=false;
		}
		else
			sf::sleep( sf::milliseconds( 30 ) );

		soundsys.tick();
	}

	window.on_exit();
	fePresent.on_stop_frontend();

	if ( window.isOpen() )
		window.close();

	soundsys.stop();
	feSettings.save_state();

	return 0;
}