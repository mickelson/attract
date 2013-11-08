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
#include "fe_icon.hpp"
#include "fe_image.hpp"
#include "fe_sound.hpp"
#include "fe_text.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>

#ifdef SFML_SYSTEM_WINDOWS
#include <windows.h>
#endif // SFML_SYSTEM_WINDOWS

void process_args( int argc, char *argv[],
			std::string &config_path,
			std::string &cmdln_font )
{
	//
	// Deal with command line arguments
	//
	std::vector<std::string> romlist_emulators;
	int next_arg=1;

	while ( next_arg < argc )
	{
		if ( strcmp( argv[next_arg], "--config" ) == 0 )
		{
			next_arg++;
			if ( next_arg < argc )
			{
				config_path = argv[next_arg];
				next_arg++;
			}
			else
			{
				std::cout << "Error, no config directory specified with --config option." << std::endl;
				exit(1);
			}
		}
		else if ( strcmp( argv[next_arg], "--font" ) == 0 )
		{
			next_arg++;
			if ( next_arg < argc )
			{
				cmdln_font = argv[next_arg];
				next_arg++;
			}
			else
			{
				std::cout << "Error, no font name specified with --font optoin." << std::endl;
				exit(1);
			}
		}
		else if ( strcmp( argv[next_arg], "--build-rom-list" ) == 0 )
		{
			next_arg++;

			for ( ; next_arg < argc; next_arg++ )
			{
				if ( argv[next_arg][0] == '-' )
					break;
				romlist_emulators.push_back( argv[next_arg] );
			}

			if ( romlist_emulators.empty() )
			{
				std::cout << "Error, no target emulators specified with --build-rom-list option."
							<<  std::endl;
				exit(1);
			}

		}
		else
		{
			int retval=1;
			if ( strcmp( argv[next_arg], "--help" ) != 0 )
			{
				std::cout << "Unrecognized command line option: "
					<< argv[next_arg] <<  std::endl;
				retval=0;
			}

			std::cout << FE_COPYRIGHT << std::endl
				<< "Usage: " << argv[0] << " [option...]" << std::endl
				<< "OPTIONS:" << std::endl
				<< "\t--config [config_directory]: specify config directory" << std::endl
				<< "\t--font [font_name]: specify default font name" << std::endl
				<< "\t--build-rom-list [emulator...]: build rom list for specified emulators" << std::endl
				<< "\t--help: show this message" << std::endl;
			exit( retval );
		}
	}

	if ( !romlist_emulators.empty() )
	{
		FeSettings feSettings( config_path, cmdln_font, false );
		int retval = feSettings.build_romlist( romlist_emulators );
		exit( retval );
	}
}

int main(int argc, char *argv[])
{
	std::string config_path, cmdln_font;
	process_args( argc, argv, config_path, cmdln_font );

	//
	// Run the front-end
	//
	FeSettings feSettings( config_path, cmdln_font, false );
	if ( feSettings.load() == false )
		return 1;

	feSettings.init_list();

	std::string defaultFontFile;
	if ( feSettings.get_font_file( defaultFontFile ) == false )
		return 1;

	sf::Font defaultFont;
	defaultFont.loadFromFile( defaultFontFile );

	//
	// Set up music/sound playing objects
	//
	FeSoundSystem soundsys( &feSettings );
	soundsys.play_ambient();
	
	sf::VideoMode mode = sf::VideoMode::getDesktopMode();

	// Create window
	sf::RenderWindow window(
			mode,
			"Attract-Mode",
			sf::Style::None );

#ifdef SFML_SYSTEM_WINDOWS
	// In Windows, the "WS_POPUP" style creates grief switching to MAME.  
	// Use the "WS_BORDER" style to fix this...
	//
	sf::WindowHandle hw = window.getSystemHandle();
	if ( ( GetWindowLong( hw, GWL_STYLE ) & WS_POPUP ) != 0 )
	{
		SetWindowLong( hw, GWL_STYLE, 
			WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
		SetWindowPos(hw, HWND_TOP, 0, 0, 
			mode.width, mode.height, SWP_FRAMECHANGED);

		ShowWindow(hw, SW_SHOW);
	}
#endif

	window.setIcon( fe_icon.width, fe_icon.height, fe_icon.pixel_data );
	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false);
	window.setMouseCursorVisible(false);

	FePresent fePresent( &feSettings, defaultFont );
	fePresent.load_layout( &window );

	FeOverlay feOverlay( window, feSettings, fePresent );
	soundsys.sound_event( FeInputMap::EventStartup );

	sf::Event ev;
	bool redraw=true;

	// go straight into config mode if there are no lists configured for
	// display
	//
	bool config_mode = ( feSettings.lists_count() < 1 ) ? true : false;

	while (window.isOpen())
	{
		if ( config_mode )
		{
			//
			// Enter config mode
			//
			if ( feOverlay.config_dialog() )
			{
				// Settings changed, reload
				//
				soundsys.update_volumes();

				if ( feSettings.get_font_file( defaultFontFile ) )
				{
					defaultFont.loadFromFile( defaultFontFile );
					fePresent.set_default_font( defaultFont );

					feSettings.init_list();
					fePresent.load_layout( &window );
				}

				soundsys.stop();
				soundsys.play_ambient();
			}
			redraw=true;
			config_mode=false;
		}

		while (window.pollEvent(ev))
		{
			FeInputMap::Command c = feSettings.map( ev );

			soundsys.sound_event( c );
			if ( fePresent.handle_event( c, ev, &window ) )
				redraw = true;
			else
			{
				// handle the things that fePresent doesn't do
				switch ( c )
				{
				case FeInputMap::ExitMenu:
					{
						int retval = feOverlay.exit_dialog();

						//
						// retval is 0 if the user confirmed exit.
						// it is <0 if we are being forced to close
						//
						if ( retval < 1 )
						{
							window.close();
							if ( retval == 0 )
								feSettings.exit_command();
						}
						else
							redraw=true;
					}
					break;

				case FeInputMap::ExitNoMenu:
					window.close();
					break;

				case FeInputMap::Select:
				case FeInputMap::RandomGame:
					soundsys.stop();

					if ( c == FeInputMap::RandomGame )
					{
						feSettings.change_rom( rand() );
						fePresent.update( false );
					}

					fePresent.pre_run( &window );
					feSettings.run();
					fePresent.post_run( &window );

					soundsys.sound_event( FeInputMap::EventGameReturn );
					soundsys.play_ambient();

					redraw=true;
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
						int list_index = feOverlay.lists_dialog();
						if ( list_index < 0 )
						{
							// list index is -1 if user pressed the "exit no dialog"
							// button, and -2 if they selected the "exit" menu
							// option.  We only want to run the exit command if the
							// menu option was selected
							//
							window.close();
							if ( list_index < -1 ) 
								feSettings.exit_command();
						}
						else
						{
							if ( feSettings.set_list( list_index ) )
								fePresent.load_layout( &window );
							else
								fePresent.update( true );

							redraw=true;
						}
					}
					break;

				default:
					break;
				}
			}
		}

		if ( fePresent.tick( &window ) )
			redraw=true;

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

	fePresent.stop( &window );
	soundsys.stop();
	feSettings.save_state();
	return 0;
}
