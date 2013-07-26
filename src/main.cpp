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
#include "fe_text.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>
#include "media.hpp"

#ifdef SFML_SYSTEM_WINDOWS
#include <windows.h>
#endif // SFML_SYSTEM_WINDOWS

template <class MyPlayer> void sound_event(
		FeInputMap::Command c,
		FeSettings &fes,
		MyPlayer &player )
{
	int volume = fes.get_play_volume(
			( c == FeInputMap::AmbientSound ) ? FeSoundInfo::Ambient : FeSoundInfo::Sound );

	if ( volume > 0 )
	{
		std::string sound;
		if ( fes.get_sound_file( c, sound ) )
		{
			if ( player.openFromFile( sound ) )
			{
				player.setVolume( volume );
				player.play();
			}
		}
	}
}

void process_args( int argc, char *argv[],
			std::string &config_path,
			std::string &cmdln_font,
			bool &show_mouse )
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
		else if ( strcmp( argv[next_arg], "--show-mouse" ) == 0 )
		{
			next_arg++;
			show_mouse = true;
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

			std::cout << FE_IDENTITY << std::endl
				<< "Usage: " << argv[0] << " [option...]" << std::endl
				<< "OPTIONS:" << std::endl
				<< "\t--config [config_directory]: specify config directory" << std::endl
				<< "\t--font [font_name]: specify default font name" << std::endl
				<< "\t--show-mouse: show mouse cursor in front-end" << std::endl
				<< "\t--build-rom-list [emulator...]: build rom list for specified emulators" << std::endl
				<< "\t--help: show this message" << std::endl;
			exit( retval );
		}
	}

	if ( !romlist_emulators.empty() )
	{
		FeSettings feSettings( config_path, cmdln_font, show_mouse );
		int retval = feSettings.build_romlist( romlist_emulators );
		exit( retval );
	}
}

int main(int argc, char *argv[])
{
	std::string config_path, cmdln_font;
	bool show_mouse( false );
	process_args( argc, argv, config_path, cmdln_font, show_mouse );

	//
	// Run the front-end
	//
	FeSettings feSettings( config_path, cmdln_font, show_mouse );
	if ( feSettings.load() == false )
		return 1;

	feSettings.init_list();

	std::string defaultFontFile;
	if ( feSettings.get_font_file( defaultFontFile ) == false )
		return 1;

	sf::Font defaultFont;
	defaultFont.loadFromFile( defaultFontFile );

	FePresent fePresent( &feSettings, defaultFont );
	fePresent.load_layout();

	//
	// Set up music/sound playing objects
	//
#ifdef NO_MOVIE
	sf::Music musicPlayer;
#else
	FeMedia musicPlayer( FeMedia::Audio );
#endif
	musicPlayer.setVolume( feSettings.get_play_volume( FeSoundInfo::Ambient ));
	musicPlayer.setLoop( true );

	sf::Music soundPlayer;
	soundPlayer.setVolume( feSettings.get_play_volume( FeSoundInfo::Sound ));
	soundPlayer.setLoop( false );

	sound_event( FeInputMap::AmbientSound, feSettings, musicPlayer );

	sf::VideoMode mode = sf::VideoMode::getDesktopMode();

	// Create window
	sf::RenderWindow window(
			mode,
			"Attract-Mode",
			sf::Style::None );

#ifdef SFML_SYSTEM_WINDOWS
	// The "WS_POPUP" style creates grief switching to MAME.  Use the "WS_BORDER" style to fix this...
	//
	sf::WindowHandle hw = window.getSystemHandle();
	if ( ( GetWindowLong( hw, GWL_STYLE ) & WS_POPUP ) != 0 )
	{
		SetWindowLong( hw, GWL_STYLE, WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
		SetWindowPos(hw, HWND_TOP, 0, 0, mode.width, mode.height, SWP_FRAMECHANGED);
		ShowWindow(hw, SW_SHOW);
	}
#endif

	window.setIcon( fe_icon.width, fe_icon.height, fe_icon.pixel_data );
	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false);
	window.setMouseCursorVisible(show_mouse);

	FeOverlay feOverlay( window, feSettings, fePresent );

	sound_event( FeInputMap::EventStartup, feSettings, soundPlayer );

	FeScreenSaver feSSave( &feSettings );
	sf::Clock sSave_clock;

	sf::Event ev;
	bool redraw=true;

	while (window.isOpen())
	{
		while (window.pollEvent(ev))
		{
			FeInputMap::Command c = feSettings.map( ev );
			sSave_clock.restart();

			if ( feSSave.is_enabled() )
			{
				feSSave.enable( false );
				fePresent.play( true );
				redraw=true;
			}
			else if ( c != FeInputMap::LAST_COMMAND )
			{
				sound_event( c, feSettings, soundPlayer );

				redraw = fePresent.handle_event( c, ev );

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
					fePresent.play( false );
					musicPlayer.stop();
					feOverlay.splash_message( "Loading..." );

					feSettings.run();
					fePresent.perform_autorotate();

					fePresent.play( true );
					sound_event( FeInputMap::EventGameReturn,
									feSettings, soundPlayer );
					sound_event( FeInputMap::AmbientSound,
									feSettings, musicPlayer );
					redraw=true;
					break;

				case FeInputMap::NextList:
      			if ( feSettings.next_list() )
						fePresent.load_layout();
					else
						fePresent.update( true );
					redraw=true;
					break;

				case FeInputMap::PrevList:
      			if ( feSettings.prev_list() )
						fePresent.load_layout();
					else
						fePresent.update( true );
					redraw=true;
					break;

				case FeInputMap::ToggleLayout:
					feSettings.toggle_layout();
					fePresent.load_layout();
					redraw=true;
					break;

				case FeInputMap::ToggleMute:
					feSettings.set_mute( !feSettings.get_mute() );
					musicPlayer.setVolume( feSettings.get_play_volume(
															FeSoundInfo::Ambient ));
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
					if ( feOverlay.config_dialog() )
					{
						// Settings changed, reload
						//
						musicPlayer.setVolume( feSettings.get_play_volume(
												FeSoundInfo::Ambient ));
						soundPlayer.setVolume( feSettings.get_play_volume(
												FeSoundInfo::Sound ));

						if ( feSettings.get_font_file( defaultFontFile ) )
						{
							defaultFont.loadFromFile( defaultFontFile );
							fePresent.set_default_font( defaultFont );

							feSettings.init_list();
							fePresent.load_layout();
						}

#ifdef NO_MOVIE
						musicPlayer.stop();
#else
						musicPlayer.close();
#endif
						sound_event( FeInputMap::AmbientSound,
									feSettings, musicPlayer );
					}
					redraw=true;
					break;

				default:
					break;
				}
			}
		}

		int screen_to = feSettings.get_screen_saver_timeout();
		if ((!feSSave.is_enabled()) && ( screen_to > 0 )
				&& ( sSave_clock.getElapsedTime().asSeconds() > screen_to ))
		{
			fePresent.play( false );
			feSSave.enable( true );
		}

		if ( feSSave.is_enabled() )
		{
			feSSave.tick();

			window.clear();
			window.draw( feSSave );
			window.display();
		}
		else
		{
			if ( fePresent.tick() )
				redraw=true;

			if ( redraw || show_mouse )
			{
				// begin drawing
				window.clear();
				window.draw( fePresent );

				if ( show_mouse )
				{
				   sf::VideoMode vm = sf::VideoMode::getDesktopMode();
					sf::Vector2i pos =  sf::Mouse::getPosition();

					std::string pos_str = as_str( pos.x );
					pos_str += ", ";
					pos_str += as_str( pos.y );

					FeTextPrimative disp_pos( &defaultFont,
							sf::Color::White,
							sf::Color::Transparent,
							16,
							FeTextPrimative::Left );

					disp_pos.setSize( sf::Vector2f( vm.width, 24 ));
					disp_pos.setPosition( sf::Vector2f( 0, vm.height - 24 ));
					disp_pos.setString( pos_str );
					window.draw( disp_pos );
				}

				window.display();
				redraw=false;
			}
			else
				sf::sleep( sf::milliseconds( 30 ) );
		}

#ifndef NO_MOVIE
		// tick() in case we need to loop the music in the player
		musicPlayer.tick();
#endif
	}

	fePresent.play( false );
	musicPlayer.stop();
	feSettings.save_state();
	return 0;
}
