/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-17 Andrew Mickelson
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
#include <cmath>
#include <cstdlib>

#ifndef NO_MOVIE
#include <Audio/AudioDevice.hpp>
#endif

#ifdef SFML_SYSTEM_ANDROID
#include "fe_util_android.hpp"
#endif

#ifdef SFML_SYSTEM_WINDOWS
#include <windows.h>

extern "C"
{
// Request more powerful GPU (Nvidea,AMD)
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

void process_args( int argc, char *argv[],
			std::string &config_path,
			std::string &cmdln_font,
			bool &process_console,
			std::string &log_file,
			FeLogLevel &log_level );

int main(int argc, char *argv[])
{
	std::string config_path, cmdln_font, log_file;
	bool launch_game = false;
	bool process_console = false;
	FeLogLevel log_level = FeLog_Info;

	process_args( argc, argv, config_path, cmdln_font, process_console, log_file, log_level );

	FeSettings feSettings( config_path, cmdln_font );

	//
	// Setup logging
	//
#if defined(SFML_SYSTEM_WINDOWS) && !defined(WINDOWS_CONSOLE)
	if ( log_file.empty() ) // on windows non-console version, write log to "last_run.log" by default
	{
		log_file = feSettings.get_config_dir();
		log_file += "last_run.log";
	}
#endif
	//
	// If a log file was supplied at the command line, write to that log file
	// If no file is supplied, logging is to stdout
	//
	if ( !log_file.empty() )
		fe_set_log_file( clean_path( log_file ) );

	// The following call also initializes the log callback for ffmpeg and gameswf
	//
	fe_set_log_level( log_level );

	//
	// Run the front-end
	//
	fe_print_version();
	FeLog() << std::endl;

	feSettings.load();

	std::string def_font_path, def_font_file;
	if ( feSettings.get_font_file( def_font_path, def_font_file ) == false )
	{
		FeLog() << "Error, could not find default font."  << std::endl;
		return 1;
	}

	FeFontContainer def_font;
	def_font.set_font( def_font_path, def_font_file );

	//
	// Set up music/sound playing objects
	//
#ifndef NO_MOVIE
	sf::AudioDevice audio_device;
#endif
	FeSoundSystem soundsys( &feSettings );

	soundsys.update_volumes();
	soundsys.play_ambient();

	FeWindow window( feSettings );
	window.initial_create();

#ifdef WINDOWS_CONSOLE
	if ( feSettings.get_hide_console() )
		hide_console();
#endif

	FeVM feVM( feSettings, def_font, window, soundsys.get_ambient_sound(), process_console );
	FeOverlay feOverlay( window, feSettings, feVM );
	feVM.set_overlay( &feOverlay );

	bool exit_selected=false;

	if ( feSettings.get_language().empty() )
	{
#ifdef SFML_SYSTEM_ANDROID
		android_copy_assets();
#endif
		// If our language isn't set at this point, we want to prompt the user for the language
		// they wish to use
		//
		if ( feOverlay.languages_dialog() < 0 )
			exit_selected = true;

		// Font may change depending on the language selected
		feSettings.get_font_file( def_font_path, def_font_file );
		def_font.set_font( def_font_path, def_font_file );

		if ( !exit_selected )
		{
			FeLog() << "Performing some initial configuration" << std::endl;
			feVM.setup_wizard();
			FeLog() << "Done initial configuration" << std::endl;
		}
	}

	soundsys.sound_event( FeInputMap::EventStartup );

	bool redraw=true;
	int guard_joyid=-1, guard_axis=-1;

	// variables used to track movement when a key is held down
	FeInputMap::Command move_state( FeInputMap::LAST_COMMAND );

	sf::Clock move_timer;
	sf::Event move_event;
	int move_last_triggered( 0 );

	// go straight into config mode if there are no lists configured for
	// display
	//
	bool config_mode = ( feSettings.displays_count() < 1 );

	if ( !config_mode )
	{
		// start the intro now
		if ( !feVM.load_intro() )
		{
			// ... or start the layout if there is no intro
			feVM.load_layout( true );

			switch ( feSettings.get_startup_mode() )
			{
			case FeSettings::LaunchLastGame:
				feSettings.select_last_launch();
				launch_game=true;
				break;

			case FeSettings::ShowDisplaysMenu:
				FeVM::cb_signal( "displays_menu" );
				break;

			default:
				break;
			}

		}
	}

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
				if ( feSettings.get_font_file( def_font_path, def_font_file ) )
					def_font.set_font( def_font_path, def_font_file );

				feSettings.set_display(
					feSettings.get_current_display_index() );

				feSettings.on_joystick_connect(); // update joystick mappings

				soundsys.stop();
				soundsys.update_volumes();
				soundsys.play_ambient();

				// Recreate window if the window mode changed
				if ( feSettings.get_window_mode() != old_mode )
				{
					window.on_exit();
					window.initial_create();
					feVM.init_monitors();
				}

				feVM.load_layout();
			}
			feVM.reset_screen_saver();
			config_mode=false;
			redraw=true;
		}
		else if (( launch_game )
			&& ( !soundsys.is_sound_event_playing( FeInputMap::Select ) ))
		{
			const std::string &emu_name = feSettings.get_rom_info( 0, 0, FeRomInfo::Emulator );
			if ( emu_name.compare( 0, 1, "@" ) == 0 )
			{
				if ( emu_name.size() > 1 )
				{
					// If emu name size >1 and  starts with a "@", then we treat the rest of the
					// string as the signal we have to send
					//
					FeVM::cb_signal( emu_name.substr( 1 ).c_str() );
				}
				else
				{
					// If emu name is just "@", then this is a shortcut to another display, so instead
					// of running a game we switch to the display specified in the rom_info's Romname field
					//
					std::string name = feSettings.get_rom_info( 0, 0, FeRomInfo::Romname );
					int index = feSettings.get_display_index_from_name( name );

					// if index not found or if we are already in the specified display, then
					// jump to the altromname display instead
					//
					if (( index < 0 ) || ( index == feSettings.get_current_display_index() ))
					{
						name = feSettings.get_rom_info( 0, 0, FeRomInfo::AltRomname );
						if ( !name.empty() )
							index =  feSettings.get_display_index_from_name( name );
					}

					if ( index < 0 )
					{
						FeLog() << "Error resolving shortcut, Display `" << name << "' not found." << std::endl;
					}
					else
					{
						if ( feSettings.set_display( index, true ) )
							feVM.load_layout();
						else
							feVM.update_to_new_list( 0, true );
					}
				}
			}
			else
			{
				soundsys.stop();

				soundsys.release_audio( true );
				feVM.pre_run();

				// window.run() returns true if our window has been closed while
				// the other program was running
				if ( !window.run() )
					exit_selected = true;

				feVM.post_run();
				soundsys.release_audio( false );
				soundsys.update_volumes();

				soundsys.sound_event( FeInputMap::EventGameReturn );
				soundsys.play_ambient();
			}

			launch_game=false;
			redraw=true;
		}

		FeInputMap::Command c;
		sf::Event ev;
		bool from_ui;
		while ( feVM.poll_command( c, ev, from_ui ) )
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

				case sf::Event::KeyReleased:
				case sf::Event::MouseButtonReleased:
				case sf::Event::JoystickButtonReleased:
					//
					// We always want to reset the screen saver on these events,
					// even if they aren't mapped otherwise (mapped events cause
					// a reset too)
					//
					if (( c == FeInputMap::LAST_COMMAND )
							&& ( feVM.reset_screen_saver() ))
						redraw = true;
					break;

				case sf::Event::GainedFocus:
				case sf::Event::Resized:
					redraw = true;
					break;


				case sf::Event::JoystickMoved:
					if ( c != FeInputMap::LAST_COMMAND )
					{
						// Only allow one mapped "Joystick Moved" input through at a time
						//
						if ( guard_joyid != -1 )
							continue;

						guard_joyid = ev.joystickMove.joystickId;
						guard_axis = ev.joystickMove.axis;
					}
					break;

				case sf::Event::JoystickConnected:
				case sf::Event::JoystickDisconnected:
					feSettings.on_joystick_connect();
					break;

				case sf::Event::Count:
				default:
					break;
			}

			// Test if we need to keep the joystick axis guard up
			//
			if (( guard_joyid >= 0 )
				&& ( std::abs( sf::Joystick::getAxisPosition(
					guard_joyid, (sf::Joystick::Axis)guard_axis )
						< feSettings.get_joy_thresh() )))
			{
				guard_joyid = -1;
				guard_axis = -1;
			}

			// Nothing further to do if there is no command or if we are in the process
			// of launching a game already
			//
			if (( c == FeInputMap::LAST_COMMAND ) || ( launch_game ))
				continue;

			//
			// Special case: handle the reload signal now
			//
			if ( c == FeInputMap::Reload )
			{
				feVM.load_layout();
				continue;
			}

			//
			// If we are responding to user input (as opposed to a signal raised from a
			// script) then manage keyrepeats now.
			//
			if ( from_ui )
			{
				if ( move_state != FeInputMap::LAST_COMMAND )
					continue;

				move_state=FeInputMap::LAST_COMMAND;

				if ( ( FeInputMap::is_ui_command( c ) && ( c != FeInputMap::Back ) )
					|| FeInputMap::is_repeatable_command( c ) )
				{
					// setup variables to test for when the navigation keys are held down
					move_state = c;
					move_timer.restart();
					move_event = ev;
				}
			}

			//
			// Map Up/Down/Left/Right/Back to their default action now
			//
			if ( FeInputMap::is_ui_command( c ) )
			{
				//
				// Give the script the option to handle the (pre-map) action.
				//
				if ( feVM.script_handle_event( c ) )
				{
					FeDebug() << "Command intercepted by script handler: "
						<< FeInputMap::commandStrings[c] << std::endl;

					redraw=true;
					continue;
				}

				//
				// Handle special case Back UI button behaviour
				//
				if ( c == FeInputMap::Back )
				{
					//
					// If a display shortcut was used previously, then the "Back" UI
					// button goes back to the previous display accordingly...
					//
					if ( feSettings.back_displays_available() )
					{
						if ( feSettings.set_display( -1, true ) )
							feVM.load_layout();
						else
							feVM.update_to_new_list( 0, true );

						redraw=true;
						continue;
					}

					//
					// If FE is configured to show displays menu on startup, then the "Back" UI
					// button goes back to that menu accordingly...
					//
					if (( feSettings.get_startup_mode() == FeSettings::ShowDisplaysMenu )
						&& ( feSettings.get_present_state() == FeSettings::Layout_Showing )
						&& ( feSettings.get_current_display_index() >= 0 ))
					{
						FeVM::cb_signal( "displays_menu" );
						redraw=true;
						continue;
					}
				}

				c = feSettings.get_default_command( c );
				if ( c == FeInputMap::LAST_COMMAND )
				{
					redraw=true;
					continue;
				}
			}

			//
			// Give the script the option to handle the command.
			//
			if ( feVM.script_handle_event( c ) )
			{
				FeDebug() << "Command intercepted by script handler: "
					<< FeInputMap::commandStrings[c] << std::endl;

				redraw=true;
				continue;
			}

			//
			// Check if we need to get out of intro mode
			//
			if ( feSettings.get_present_state() == FeSettings::Intro_Showing )
			{
				move_state = FeInputMap::LAST_COMMAND;
				move_last_triggered = 0;

				feVM.load_layout( true );

				switch ( feSettings.get_startup_mode() )
				{
				case FeSettings::LaunchLastGame:
					feSettings.select_last_launch();
					launch_game=true;
					break;

				case FeSettings::ShowDisplaysMenu:
					FeVM::cb_signal( "displays_menu" );
					break;

				default:
					break;
				}

				redraw=true;
				continue;
			}

			//
			// Default command handling
			//
			FeDebug() << "Handling command: " << FeInputMap::commandStrings[c] << std::endl;

			soundsys.sound_event( c );
			if ( feVM.handle_event( c ) )
				redraw = true;
			else
			{
				// handle the things that fePresent doesn't do
				switch ( c )
				{
				case FeInputMap::Exit:
					{
						if ( feOverlay.common_exit() )
							exit_selected=true;

						redraw=true;
					}
					break;

				case FeInputMap::ExitToDesktop:
					exit_selected = true;
					break;

				case FeInputMap::ReplayLastGame:
					if ( feSettings.select_last_launch() )
						feVM.load_layout();
					else
						feVM.update_to_new_list();

					launch_game=true;
					redraw=true;
					break;

				case FeInputMap::Select:
					launch_game=true;
					break;

				case FeInputMap::ToggleMute:
					feSettings.set_mute( !feSettings.get_mute() );
					soundsys.update_volumes();
					feVM.toggle_mute();
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

				case FeInputMap::InsertGame:
					{
						std::vector< std::string > options;
						std::string temp;

						// 0
						feSettings.get_resource(  "Insert Game Entry", temp );
						options.push_back( temp );

						// 1
						feSettings.get_resource(  "Insert Display Shortcut", temp );
						options.push_back( temp );

						// 2
						feSettings.get_resource(  "Insert Command Shortcut", temp );
						options.push_back( temp );

						feSettings.get_resource(  "Insert Menu Entry", temp );

						int sel = feOverlay.common_list_dialog(
							temp, options, 0, 0 );

						std::string emu_name;
						std::string def_name;

						switch ( sel )
						{
						case 0:
							{
								std::vector<std::string> tl;
								feSettings.get_list_of_emulators( tl );
								if ( !tl.empty() )
									emu_name = tl[0];

								feSettings.get_resource( "Blank Game", def_name );
							}
							break;

						case 1:
							{
								emu_name = "@";
								feSettings.get_resource( "Display Shortcut", def_name );

								if ( feSettings.displays_count() > 0 )
									def_name = feSettings.get_display( 0 )
										->get_info( FeDisplayInfo::Name );
							}
							break;

						case 2:
							emu_name = "@exit";
							feSettings.get_resource( "Exit", def_name );
							break;

						default:
							break;
						};

						FeRomInfo new_entry( def_name );
						new_entry.set_info( FeRomInfo::Title, def_name );
						new_entry.set_info( FeRomInfo::Emulator, emu_name );

						int f_idx = feSettings.get_current_filter_index();

						FeRomInfo dummy;
						FeRomInfo *r = feSettings.get_rom_absolute(
							f_idx, feSettings.get_rom_index( f_idx, 0 ) );

						if ( !r )
							r = &dummy;

						feSettings.update_romlist_after_edit( *r,
							new_entry,
							FeSettings::InsertEntry );

						// initial update shows new entry behind config
						// dialog
						feVM.update_to_new_list();

						if ( feOverlay.edit_game_dialog() )
							feVM.update_to_new_list();

						redraw=true;
					}
					break;

				case FeInputMap::EditGame:
					if ( feOverlay.edit_game_dialog() )
						feVM.update_to_new_list();

					redraw=true;
					break;

				case FeInputMap::DisplaysMenu:
					{
						if ( !feSettings.get_info( FeSettings::MenuLayout ).empty() )
						{
							//
							// If user has configured a custom layout for the display
							// menu, then setting display to -1 and loading will
							// bring up the displays menu using that layout
							//
							feSettings.set_display(-1);
							feVM.load_layout( true );

							redraw=true;
							break;
						}
						//
						// If no custom layout is configured, then we simply show the
						// displays menu the same way as any other popup menu...
						//

						std::vector<std::string> disp_names;
						std::vector<int> disp_indices;
						int current_idx;

						std::string title;
						feSettings.get_displays_menu( title, disp_names, disp_indices, current_idx );

						int exit_opt=-999;
						if ( feSettings.get_info_bool( FeSettings::DisplaysMenuExit ) )
						{
							//
							// Add an exit option at the end of the lists menu
							//
							std::string exit_str;
							feSettings.get_resource( "Exit Attract-Mode", exit_str );
							disp_names.push_back( exit_str );
							exit_opt = disp_names.size() - 1;
						}

						if ( !disp_names.empty() )
						{
							int sel_idx = feOverlay.common_list_dialog(
								title,
								disp_names,
								current_idx,
								-1,
								FeInputMap::DisplaysMenu );

							if ( sel_idx == exit_opt )
							{
								if ( feOverlay.common_exit() )
									exit_selected = true;
							}
							else if ( sel_idx >= 0 )
							{
								if ( feSettings.set_display( disp_indices[sel_idx] ) )
									feVM.load_layout();
								else
									feVM.update_to_new_list( 0, true );
							}

							redraw=true;
						}
					}
					break;

				case FeInputMap::FiltersMenu:
					{
						std::vector<std::string> names_list;
						feSettings.get_current_display_filter_names( names_list );

						std::string title;
						feSettings.get_resource( "Filters", title );

						int filter_index = feOverlay.common_list_dialog(
										title,
										names_list,
										feSettings.get_current_filter_index(),
										-1,
										FeInputMap::FiltersMenu );

						if ( filter_index >= 0 )
						{
							feSettings.set_current_selection( filter_index, -1 );
							feVM.update_to_new_list();
						}

						redraw=true;
					}
					break;

				case FeInputMap::ToggleFavourite:
					{
						bool new_state = !feSettings.get_current_fav();

						if ( feSettings.get_info_bool( FeSettings::ConfirmFavourites ) )
						{
							std::string msg = ( new_state )
								? "Add '$1' to Favourites?"
								: "Remove '$1' from Favourites?";

							// returns 0 if user confirmed toggle
							if ( feOverlay.confirm_dialog(
									msg,
									feSettings.get_rom_info( 0, 0, FeRomInfo::Title ) ) == 0 )
							{
								if ( feSettings.set_current_fav( new_state ) )
									feVM.update_to_new_list( 0, true ); // our current display might have changed, so update
							}
						}
						else
						{
							if ( feSettings.set_current_fav( new_state ) )
								feVM.update_to_new_list( 0, true ); // our current display might have changed, so update
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

		//
		// Determine if we have to do anything because a key is being held down
		//
		if ( move_state != FeInputMap::LAST_COMMAND )
		{
			bool cont=false;

			switch ( move_event.type )
			{
			case sf::Event::KeyPressed:
				if ( sf::Keyboard::isKeyPressed( move_event.key.code ) )
					cont=true;
				break;

			case sf::Event::MouseButtonPressed:
				if ( sf::Mouse::isButtonPressed( move_event.mouseButton.button ) )
					cont=true;
				break;

			case sf::Event::JoystickButtonPressed:
				if ( sf::Joystick::isButtonPressed(
						move_event.joystickButton.joystickId,
						move_event.joystickButton.button ) )
					cont=true;
				break;

			case sf::Event::JoystickMoved:
				{
					sf::Joystick::update();

					float pos = sf::Joystick::getAxisPosition(
							move_event.joystickMove.joystickId,
							move_event.joystickMove.axis );
					if ( std::abs( pos ) > feSettings.get_joy_thresh() )
						cont=true;
				}
				break;

			default:
				break;
			}

			if ( cont )
			{
				const int TRIG_CHANGE_MS = 400;
				int t = move_timer.getElapsedTime().asMilliseconds();
				if (( t > TRIG_CHANGE_MS ) && ( t - move_last_triggered > feSettings.selection_speed() ))
				{
					FeInputMap::Command ms = move_state;
					if ( FeInputMap::is_ui_command( ms ) )
					{
						//
						// Give the script the option to handle the (pre-map) action.
						//
						if ( feVM.script_handle_event( ms ) )
						{
							redraw=true;
							ms = FeInputMap::LAST_COMMAND;
						}
						else
						{
							ms = feSettings.get_default_command( ms );
							if ( !FeInputMap::is_repeatable_command( ms ) )
								ms = FeInputMap::LAST_COMMAND;
						}
					}

					if ( ms != FeInputMap::LAST_COMMAND )
					{
						move_last_triggered = t;
						int step = 1;

						if ( feSettings.get_info_bool( FeSettings::AccelerateSelection ) )
						{
							// As the button is held down, the advancement accelerates
							int shift = ( t / TRIG_CHANGE_MS ) - 3;
							if ( shift < 0 )
								shift = 0;
							else if ( shift > 7 ) // don't go above a maximum advance of 2^7 (128)
								shift = 7;

							step = 1 << ( shift );
						}

						switch ( ms )
						{
						case FeInputMap::PrevGame: step = -step; break;
						case FeInputMap::NextGame: break; // do nothing
						case FeInputMap::PrevPage: step *= -feVM.get_page_size(); break;
						case FeInputMap::NextPage: step *= feVM.get_page_size(); break;
						case FeInputMap::PrevFavourite:
							{
								int temp = feSettings.get_prev_fav_offset();
								step = ( temp < 0 ) ? temp : 0;
							}
							break;
						case FeInputMap::NextFavourite:
							{
								int temp = feSettings.get_next_fav_offset();
								step = ( temp > 0 ) ? temp : 0;
							}
							break;
						case FeInputMap::PrevLetter:
							{
								int temp = feSettings.get_next_letter_offset( -1 );
								step = ( temp < 0 ) ? temp : 0;
							}
							break;
						case FeInputMap::NextLetter:
							{
								int temp = feSettings.get_next_letter_offset( 1 );
								step = ( temp > 0 ) ? temp : 0;
							}
							break;
						default: break;
						}

						//
						// Limit the size of our step so that there is no wrapping around at the end of the list
						//
						int curr_sel = feSettings.get_rom_index( feSettings.get_current_filter_index(), 0 );
						if ( ( curr_sel + step ) < 0 )
							step = -curr_sel;
						else
						{
							int list_size = feSettings.get_filter_size( feSettings.get_current_filter_index() );
							if ( ( curr_sel + step ) >= list_size )
								step = list_size - curr_sel - 1;
						}

						if ( step != 0 )
						{
							if ( feVM.script_handle_event( ms ) == false )
								feVM.change_selection( step, false );

							redraw=true;
						}
					}
				}
			}
			else
			{
				//
				// If we are ending a "repeatable" input (prev/next game etc) or a UI input
				// that maps to a repeatable input (i.e. "Up"->"prev game") then trigger the
				// "End Navigation" stuff now
				//
				FeInputMap::Command ms = move_state;
				if ( FeInputMap::is_ui_command( ms ) )
					ms = feSettings.get_default_command( ms );

				if ( FeInputMap::is_repeatable_command( ms ) )
					feVM.on_end_navigation();

				move_state = FeInputMap::LAST_COMMAND;
				move_last_triggered = 0;

				redraw=true;
			}
		}

		if ( feVM.on_tick() )
			redraw=true;

		if ( feVM.video_tick() )
			redraw=true;

		if ( feVM.saver_activation_check() )
			soundsys.sound_event( FeInputMap::ScreenSaver );

		if ( redraw )
		{
			feVM.redraw_surfaces();

			// begin drawing
			window.clear();
			window.draw( feVM );
			window.display();
			redraw=false;
		}
		else
			sf::sleep( sf::milliseconds( 15 ) );

		soundsys.tick();
	}

	window.on_exit();
	feVM.on_stop_frontend();

	if ( window.isOpen() )
		window.close();

	FeRomListSorter::clear_title_rex();

	soundsys.stop();
	feSettings.save_state();

	FeDebug() << "Attract-Mode ended normally" << std::endl;
	return 0;
}
