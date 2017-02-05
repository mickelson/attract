/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-16 Andrew Mickelson
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

#include "fe_util.hpp"
#include "fe_settings.hpp"
#include "zip.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include <stdlib.h>
#include <cctype>

#include <SFML/System/Clock.hpp>
#include <SFML/Config.hpp>

#ifdef USE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

#ifndef NO_MOVIE
#include "media.hpp" // for FeMedia::is_supported_media(), get/set_current_decoder()
#endif

#ifdef SFML_SYSTEM_WINDOWS

const char *FE_DEFAULT_CFG_PATH		= "./";
const char *FE_DEFAULT_FONT			= "arial";
const char *FE_DEFAULT_FONT_PATHS[]	= { "%SYSTEMROOT%/Fonts/", NULL };

#else
#ifdef SFML_SYSTEM_MACOS

const char *FE_DEFAULT_CFG_PATH		= "$HOME/.attract/";
const char *FE_DEFAULT_FONT			= "Arial";
const char *FE_DEFAULT_FONT_PATHS[]	=
{
	"/Library/Fonts/",
	"$HOME/Library/Fonts/",
	NULL
};

#else

const char *FE_DEFAULT_CFG_PATH		= "$HOME/.attract/";
const char *FE_DEFAULT_FONT			= "FreeSans";
const char *FE_DEFAULT_FONT_PATHS[]	=
{
	"/usr/share/fonts/",
	"$HOME/.fonts/",
	NULL
};

#endif
#endif

const char *FE_ART_EXTENSIONS[]		=
{
	".png",
	".jpg",
	".jpeg",
	".gif",
	".bmp",
	".tga",
	NULL
};

const char *FE_FONT_EXTENSIONS[]		=
{
	".ttf",
	".ttc",
	".otf",
	".fnt",
	".pcf",
	".bdf",
	NULL
};

#ifdef DATA_PATH
const char *FE_DATA_PATH = DATA_PATH;
#else
const char *FE_DATA_PATH = NULL;
#endif

const char *FE_CFG_FILE					= "attract.cfg";
const char *FE_STATE_FILE				= "attract.am";
const char *FE_SCREENSAVER_FILE		= "screensaver.nut";
const char *FE_PLUGIN_FILE				= "plugin.nut";
const char *FE_LOADER_FILE				= "loader.nut";
const char *FE_INTRO_FILE				= "intro.nut";
const char *FE_LAYOUT_FILE_BASE		= "layout";
const char *FE_LAYOUT_FILE_EXTENSION	= ".nut";
const char *FE_LANGUAGE_FILE_EXTENSION = ".msg";
const char *FE_SWF_EXT					= ".swf";
const char *FE_PLUGIN_FILE_EXTENSION	= FE_LAYOUT_FILE_EXTENSION;
const char *FE_GAME_EXTRA_FILE_EXTENSION = ".cfg";
const char *FE_LAYOUT_SUBDIR			= "layouts/";
const char *FE_ROMLIST_SUBDIR			= "romlists/";
const char *FE_SOUND_SUBDIR			= "sounds/";
const char *FE_SCREENSAVER_SUBDIR		= "screensaver/";
const char *FE_PLUGIN_SUBDIR 			= "plugins/";
const char *FE_LANGUAGE_SUBDIR		= "language/";
const char *FE_MODULES_SUBDIR			= "modules/";
const char *FE_STATS_SUBDIR			= "stats/";
const char *FE_LOADER_SUBDIR			= "loader/";
const char *FE_INTRO_SUBDIR			= "intro/";
const char *FE_SCRAPER_SUBDIR			= "scraper/";
const char *FE_MENU_ART_SUBDIR		= "menu-art/";
const char *FE_LIST_DEFAULT			= "default-display.cfg";
const char *FE_FILTER_DEFAULT			= "default-filter.cfg";
const char *FE_CFG_YES_STR				= "yes";
const char *FE_CFG_NO_STR				= "no";

const std::string FE_EMPTY_STRING;

bool internal_resolve_config_file(
	const std::string &config_path,
	std::string &result,
	const char *subdir,
	const std::string &name  )
{
	std::string path;
	path = config_path;
	if ( subdir ) path += subdir;
	path += name;

	if ( file_exists( path ) )
	{
		result = path;
		return true;
	}

	if ( FE_DATA_PATH != NULL )
	{
		path = FE_DATA_PATH;
		if ( subdir ) path += subdir;
		path += name;

		if ( file_exists( path ) )
		{
			result = path;
			return true;
		}
	}

	return false;
}

int find_idx_in_vec( int idx, const std::vector<int> &vec )
{
	int i=0;
	for ( i=0; i < (int)vec.size() - 1; i++ )
	{
		if ( idx <= vec[i] )
			break;
	}

	return i;
}

FeLanguage::FeLanguage( const std::string &l )
	: language( l )
{
}

const char *FeSettings::windowModeTokens[] =
{
	"default",
	"fullscreen",
	"window",
	"window_no_border",
	NULL
};

const char *FeSettings::windowModeDispTokens[] =
{
	"Fill Screen (Default)",
	"Fullscreen Mode",
	"Window",
	"Window (No Border)",
	NULL
};

const char *FeSettings::filterWrapTokens[] =
{
	"default",
	"jump_to_next_display",
	"no_wrap",
	NULL
};

const char *FeSettings::filterWrapDispTokens[] =
{
	"Wrap within Display (Default)",
	"Jump to Next Display",
	"No Wrap",
	NULL
};

const char *FeSettings::startupTokens[] =
{
	"default",
	"launch_last_game",
	"displays_menu",
	NULL
};

const char *FeSettings::startupDispTokens[] =
{
	"Show Last Selection (Default)",
	"Launch Last Game",
	"Show Displays Menu",
	NULL
};

FeSettings::FeSettings( const std::string &config_path,
				const std::string &cmdln_font )
	:  m_rl( m_config_path ),
	m_inputmap(),
	m_saver_params( FeLayoutInfo::ScreenSaver ),
	m_intro_params( FeLayoutInfo::Intro ),
	m_current_display( -1 ),
	m_current_config_object( NULL ),
	m_ssaver_time( 600 ),
	m_last_launch_display( 0 ),
	m_last_launch_filter( 0 ),
	m_last_launch_rom( 0 ),
	m_joy_thresh( 75 ),
	m_mouse_thresh( 10 ),
	m_current_search_index( 0 ),
	m_displays_menu_exit( true ),
	m_hide_brackets( false ),
	m_startup_mode( ShowLastSelection ),
	m_confirm_favs( true ),
	m_confirm_exit( true ),
	m_track_usage( true ),
	m_multimon( true ),
	m_window_mode( Default ),
	m_smooth_images( true ),
	m_filter_wrap_mode( WrapWithinDisplay ),
	m_accel_selection( true ),
	m_selection_speed( 40 ),
	m_scrape_snaps( true ),
	m_scrape_marquees( true ),
	m_scrape_flyers( true ),
	m_scrape_wheels( true ),
	m_scrape_fanart( false ),
	m_scrape_vids( false ),
	m_scrape_mamedb( true ),
	m_scrape_overview( true ),
#ifdef SFML_SYSTEM_WINDOWS
	m_hide_console( false ),
#endif
	m_loaded_game_extras( false ),
	m_present_state( Layout_Showing )
{
	int i=0;
	while ( FE_DEFAULT_FONT_PATHS[i] != NULL )
		m_font_paths.push_back( FE_DEFAULT_FONT_PATHS[i++] );

	if ( config_path.empty() )
		m_config_path = absolute_path( clean_path(FE_DEFAULT_CFG_PATH) );
	else
		m_config_path = absolute_path( clean_path( config_path, true ) );

	// absolute_path can drop the trailing slash
	if (
#ifdef SFML_SYSTEM_WINDOWS
			(m_config_path[m_config_path.size()-1] != '\\') &&
#endif
			(m_config_path[m_config_path.size()-1] != '/') )
		m_config_path += '/';

	m_default_font = cmdln_font;
}

void FeSettings::clear()
{
	m_current_config_object=NULL;
	m_current_display = -1;

	m_displays.clear();
	m_rl.clear_emulators();
	m_plugins.clear();
}

void FeSettings::load()
{
	clear();

	std::string load_language( "en" );
	std::string filename = m_config_path + FE_CFG_FILE;

	if (( FE_DATA_PATH != NULL ) && ( !directory_exists( FE_DATA_PATH ) ))
	{
		std::cerr << "Warning: Attract-Mode was compiled to look for its default configuration files in: "
			<< FE_DATA_PATH << ", which is not available." << std::endl;
	}

	if ( load_from_file( filename ) == false )
	{
		std::cout << "Config file not found: " << filename << ", performing initial setup." << std::endl;

		//
		// If there is no config file, then we do some initial setting up of the FE here, prompt
		// the user to select a language and then launch straight into configuration mode.
		//
		// Setup step: if there is a Data directory, then copy the default emulator configurations provided
		// in that directory over to the user's configuration directory.
		//
		if (( FE_DATA_PATH != NULL ) && ( directory_exists( FE_DATA_PATH ) ))
		{
			confirm_directory( m_config_path, FE_EMULATOR_SUBDIR );

			std::string from_path( FE_DATA_PATH ), to_path( m_config_path );
			from_path += FE_EMULATOR_SUBDIR;
			to_path += FE_EMULATOR_SUBDIR;

			std::vector<std::string> ll;
			get_basename_from_extension( ll, from_path, FE_EMULATOR_FILE_EXTENSION, false );

			for( std::vector<std::string>::iterator itr=ll.begin(); itr != ll.end(); ++itr )
			{
				std::string from = from_path + (*itr);
				std::string to = to_path + (*itr);

				// Only copy if the destination file does not exist already
				//
				if ( !file_exists( to ) )
				{
					std::cout << "Copying: '" << from << "' to '" << to_path << "'" << std::endl;

					std::ifstream src( from.c_str() );
					std::ofstream dst( to.c_str() );
					dst << src.rdbuf();
				}
			}
		}
	}
	else
	{
		std::cout << "Config: " << filename << std::endl;

		if ( m_language.empty() )
			m_language = "en";

		load_language = m_language;
	}

	load_state();
	init_display();

	// Make sure we have some keyboard mappings
	//
	m_inputmap.initialize_mappings();

	// If we haven't got our font yet from the config file
	// or command line then set to the default value now
	//
	if ( m_default_font.empty() )
		m_default_font = FE_DEFAULT_FONT;

	// Load language strings now.
	//
	// If we didn't find a config file, then we leave m_language empty but load the english language strings
	// now.  The user will be prompted (using the english strings) to select a language and then launched
	// straight into configuration mode.
	//
	// If a config file was found but it didn't specify a language, then we use the english language (this
	// preserves the previous behaviour for config files created in an earlier version)
	//
	internal_load_language( load_language );

	//
	// Initialize the regular expression used when sorting by title now...
	//
	std::string rex_str;
	get_resource( "_sort_regexp", rex_str );
	FeRomListSorter::init_title_rex( rex_str );

	// If no menu prompt is configured, default to calling it "Displays Menu" (in current language)
	//
	if ( m_menu_prompt.empty() )
		get_resource( "Displays Menu", m_menu_prompt );
}

const char *FeSettings::configSettingStrings[] =
{
	"language",
	"exit_command",
	"default_font",
	"font_path",
	"screen_saver_timeout",
	"displays_menu_exit",
	"hide_brackets",
	"startup_mode",
	"confirm_favourites",
	"confirm_exit",
	"mouse_threshold",
	"joystick_threshold",
	"window_mode",
	"filter_wrap_mode",
	"track_usage",
	"multiple_monitors",
	"smooth_images",
	"accelerate_selection",
	"selection_speed_ms",
	"scrape_snaps",
	"scrape_marquees",
	"scrape_flyers",
	"scrape_wheels",
	"scrape_fanart",
	"scrape_videos",
	"scrape_mamedb",
	"scrape_overview",
#ifdef SFML_SYSTEM_WINDOWS
	"hide_console",
#endif
	"video_decoder",
	"menu_prompt",
	"menu_layout",
	NULL
};

const char *FeSettings::otherSettingStrings[] =
{
	"display",
	"sound",
	"input_map",
	"general",
	"plugin",
	FeLayoutInfo::indexStrings[0], // "saver_config"
	FeLayoutInfo::indexStrings[1], // "layout_config"
	FeLayoutInfo::indexStrings[2], // "intro_config"
	NULL
};


int FeSettings::process_setting( const std::string &setting,
					const std::string &value,
					const std::string &fn )
{
	if (( setting.compare( otherSettingStrings[0] ) == 0 ) // list
		|| ( setting.compare( "list" ) == 0 )) // for backwards compatability.  As of 1.5, "list" became "display"
	{
		m_displays.push_back( FeDisplayInfo( value ) );
		m_current_config_object = &m_displays.back();
	}
	else if ( setting.compare( otherSettingStrings[1] ) == 0 ) // sound
		m_current_config_object = &m_sounds;
	else if ( setting.compare( otherSettingStrings[2] ) == 0 ) // input_map
		m_current_config_object = &m_inputmap;
	else if ( setting.compare( otherSettingStrings[3] ) == 0 ) // general
		m_current_config_object = NULL;
	else if ( setting.compare( otherSettingStrings[4] ) == 0 ) // plugin
	{
		FePlugInfo new_plug( value );
		m_plugins.push_back( new_plug );
		m_current_config_object = &m_plugins.back();
	}
	else if ( setting.compare( otherSettingStrings[5] ) == 0 ) // saver_config
		m_current_config_object = &m_saver_params;
	else if ( setting.compare( otherSettingStrings[6] ) == 0 ) // layout_config
	{
		for ( std::vector< FeLayoutInfo >::iterator itr=m_layout_params.begin();
			itr != m_layout_params.end(); ++itr )
		{
			if ( value.compare( (*itr).get_name() ) == 0 )
			{
				// If a duplicate, replace existing
				(*itr).clear_params();
				m_current_config_object = &(*itr);
				return 0;
			}
		}

		// Add new layout info
		FeLayoutInfo new_entry( value );
		m_layout_params.push_back( new_entry );
		m_current_config_object = &m_layout_params.back();
	}
	else if ( setting.compare( otherSettingStrings[7] ) == 0 ) // intro_config
		m_current_config_object = &m_intro_params;
	else if ( setting.compare( configSettingStrings[DefaultFont] ) == 0 ) // default_font
	{
		// Special case for the default font, we don't want to set it here
		// if it was already specified at the command line
		//
		if ( m_default_font.empty() ) // don't overwrite command line font
			m_default_font = value;
	}
	else
	{
		int i=0;
		while ( configSettingStrings[i] != NULL )
		{
			if ( setting.compare( configSettingStrings[i] ) == 0 )
			{
				if ( set_info( i, value ) == false )
				{
					invalid_setting( fn,
						configSettingStrings[i],
						value, NULL, NULL, "value" );
					return 1;
				}
				return 0;
			}
			i++;
		}

		// if we get this far, then none of the settings associated with
		// this object were found, pass to the current child object
		if ( m_current_config_object != NULL )
			return m_current_config_object->process_setting( setting, value, fn );
		else
		{
			// For backwards compatability, as of 1.5 "lists_menu_exit" became "displays_menu_exit"
			if ( setting.compare( "lists_menu_exit" ) == 0 )
			{
				set_info( DisplaysMenuExit, value );
				return 0;
			}
			// After 1.6.2, "autolaunch_last_game" became an option for "startup_mode"
			else if ( setting.compare( "autolaunch_last_game" ) == 0 )
			{
				if ( config_str_to_bool( value ) )
					m_startup_mode = LaunchLastGame;
				return 0;
			}

			invalid_setting( fn,
				"general", setting, otherSettingStrings, configSettingStrings );
			return 1;
		}
	}

	return 0;
}

void FeSettings::init_display()
{
	m_loaded_game_extras = false;

	//
	// Setting new_index to negative causes us to do the 'Displays Menu' w/ custom layout
	//
	if ( m_current_display < 0 )
	{
		m_rl.init_as_empty_list();
		FeRomInfoListType &l = m_rl.get_list();

		for ( unsigned int i=0; i<m_display_menu.size(); i++ )
		{
			FeDisplayInfo *p = &(m_displays[m_display_menu[i]]);

			FeRomInfo rom( p->get_info( FeDisplayInfo::Name ) );
			rom.set_info( FeRomInfo::Title, p->get_info( FeDisplayInfo::Name ) );
			rom.set_info( FeRomInfo::Emulator, "@" );
			rom.set_info( FeRomInfo::AltRomname, p->get_info( FeDisplayInfo::Romlist ) );

			l.push_back( rom );
		}

		if ( m_displays_menu_exit )
		{
			std::string exit_str;
			get_resource( "Exit Attract-Mode", exit_str );

			FeRomInfo rom( exit_str );
			rom.set_info( FeRomInfo::Title, exit_str );
			rom.set_info( FeRomInfo::Emulator, "@exit" );
			rom.set_info( FeRomInfo::AltRomname, "exit" );

			l.push_back( rom );
		}

		FeDisplayInfo empty( "" );
		m_rl.create_filters( empty );

		// we want to keep m_current_search_index through the set_search_value() call
		int temp_idx = m_current_search_index;
		set_search_rule( "" );
		m_current_search_index = temp_idx;

		construct_display_maps();
		return;
	}

	set_search_rule( "" );

	const std::string &romlist_name = m_displays[m_current_display].get_info(FeDisplayInfo::Romlist);
	if ( romlist_name.empty() )
	{
		m_rl.init_as_empty_list();

		construct_display_maps();
		return;
	}

	std::cout << std::endl << "*** Initializing display: '" << get_current_display_title() << "'" << std::endl;

	std::string stat_path;
	if ( m_track_usage )
		stat_path = m_config_path + FE_STATS_SUBDIR + romlist_name + "/";

	std::string list_path( m_config_path );
	list_path += FE_ROMLIST_SUBDIR;
	std::string user_path( list_path );

	// Check for a romlist in the data path if there isn't one that matches in the
	// config directory
	//
	if (( !file_exists( list_path + romlist_name + FE_ROMLIST_FILE_EXTENSION ) )
		&& ( FE_DATA_PATH != NULL ))
	{
		std::string temp = FE_DATA_PATH;
		temp += FE_ROMLIST_SUBDIR;

		if ( file_exists( temp + romlist_name + FE_ROMLIST_FILE_EXTENSION ) )
			list_path = temp;
	}

	if ( m_rl.load_romlist( list_path,
				romlist_name,
				user_path,
				stat_path,
				m_displays[m_current_display] ) == false )
		std::cerr << "Error opening romlist: " << romlist_name << std::endl;

	//
	// Construct our display index views here, for lack of a better spot
	//
	// Do this here so that index views are rebuilt on a forced reset of
	// the display (which happens when settings get changed for example)
	//
	construct_display_maps();
}

void FeSettings::construct_display_maps()
{
	m_display_cycle.clear();
	m_display_menu.clear();

	for ( unsigned int i=0; i<m_displays.size(); i++ )
	{
		if ( m_displays[i].show_in_cycle() )
			m_display_cycle.push_back( i );

		if ( m_displays[i].show_in_menu() )
			m_display_menu.push_back( i );
	}
}

void FeSettings::save_state()
{
	int display_idx = m_current_display;
	if ( display_idx < 0 )
		display_idx = m_current_search_index;

	m_rl.save_state();

	std::string filename( m_config_path );
	confirm_directory( m_config_path, FE_EMPTY_STRING );

	filename += FE_STATE_FILE;

	std::ofstream outfile( filename.c_str() );
	if ( outfile.is_open() )
	{
		outfile << display_idx << ";"
			<< m_last_launch_display << "," << m_last_launch_filter
			<< "," << m_last_launch_rom << ";" << m_menu_layout_file
			<< std::endl;

		for ( std::vector<FeDisplayInfo>::const_iterator itl=m_displays.begin();
					itl != m_displays.end(); ++itl )
			outfile << (*itl).state_as_output() << std::endl;

		outfile.close();
	}
}

void FeSettings::load_state()
{
	if ( m_displays.empty() )
	{
		m_current_display = -1;
		return;
	}

	std::string filename( m_config_path );
	filename += FE_STATE_FILE;

	std::ifstream myfile( filename.c_str() );
	std::string line;

	if ( myfile.is_open() && myfile.good() )
	{
		getline( myfile, line );
		size_t pos=0;
		std::string tok;
		token_helper( line, pos, tok, ";" );

		m_current_display = as_int( tok );

		token_helper( line, pos, tok, ";" );

		int i=0;
		size_t pos2=0;
		while (( pos2 < tok.size() ) && ( i < 3 ))
		{
			std::string tok2;
			token_helper( tok, pos2, tok2, "," );
			int temp = as_int( tok2 );

			switch (i)
			{
				case 0: m_last_launch_display = temp; break;
				case 1: m_last_launch_filter = temp; break;
				case 2: m_last_launch_rom = temp; break;
			}
			i++;
		}

		token_helper( line, pos, tok, ";" );
		m_menu_layout_file = tok;

		for ( std::vector<FeDisplayInfo>::iterator itl=m_displays.begin();
					itl != m_displays.end(); ++itl )
		{
			if ( myfile.good() )
			{
				getline( myfile, line );
				(*itl).process_state( line );
			}
		}
	}

	// bound checking on the current list state
	if ( m_current_display >= (int)m_displays.size() )
		m_current_display = m_displays.size() - 1;
	if ( m_current_display < 0 )
		m_current_display = 0;

	// bound checking on the last launch state
	if ( m_last_launch_display >= (int)m_displays.size() )
		m_last_launch_display = m_displays.size() - 1;
	if ( m_last_launch_display < 0 )
		m_last_launch_display = 0;

	if ( m_last_launch_filter >= m_displays[ m_last_launch_display ].get_filter_count() )
		m_last_launch_filter = m_displays[ m_last_launch_display ].get_filter_count() - 1;
	if ( m_last_launch_filter < 0 )
		m_last_launch_filter = 0;
}

FeInputMap::Command FeSettings::map_input( const sf::Event &e )
{
	return m_inputmap.map_input( e, m_mousecap_rect, m_joy_thresh );
}

FeInputMap::Command FeSettings::input_conflict_check( const FeInputMapEntry &e )
{
	return m_inputmap.input_conflict_check( e );
}

void FeSettings::get_input_config_metrics( sf::IntRect &mousecap_rect, int &joy_thresh )
{
	mousecap_rect = m_mousecap_rect;
	joy_thresh = m_joy_thresh;
}

FeInputMap::Command FeSettings::get_default_command( FeInputMap::Command c )
{
	return m_inputmap.get_default_command( c );
}

void FeSettings::set_default_command( FeInputMap::Command c, FeInputMap::Command v )
{
	m_inputmap.set_default_command( c, v );
}

bool FeSettings::get_current_state( FeInputMap::Command c )
{
	return m_inputmap.get_current_state( c, m_joy_thresh );
}

void FeSettings::init_mouse_capture( int window_x, int window_y )
{
	int radius = window_x * m_mouse_thresh / 400;
	int centre_x = window_x / 2;
	int centre_y = window_y / 2;

	m_mousecap_rect.left = centre_x - radius;
	m_mousecap_rect.top = centre_y - radius;
	m_mousecap_rect.width = radius * 2;
	m_mousecap_rect.height = radius * 2;
}

bool FeSettings::test_mouse_reset( int mouse_x, int mouse_y ) const
{
	return (( m_inputmap.has_mouse_moves() ) && ( !m_mousecap_rect.contains( mouse_x, mouse_y ) ));
}

int FeSettings::get_filter_index_from_offset( int offset ) const
{
	if ( m_current_display < 0 )
		return 0;

	int f_count = m_displays[m_current_display].get_filter_count();

	if ( f_count == 0 )
		return 0;

	int off = abs( offset ) % f_count;

	int retval = get_current_filter_index();
	if ( offset < 0 )
		retval -= off;
	else
		retval += off;

	if ( retval < 0 )
		retval += f_count;
	if ( retval >= f_count )
		retval -= f_count;

	return retval;
}

int FeSettings::get_filter_count() const
{
	if ( m_current_display < 0 )
		return 1;

	return m_displays[m_current_display].get_filter_count();
}

int FeSettings::get_filter_size( int filter_index ) const
{
	if ( !m_current_search.empty()
			&& ( get_current_filter_index() == filter_index ))
		return m_current_search.size();

	return m_rl.filter_size( filter_index );
}

int FeSettings::get_rom_index( int filter_index, int offset ) const
{
	int retval, rl_size;
	if ( m_current_display < 0 )
	{
		retval = m_current_search_index;
		rl_size = m_rl.filter_size( filter_index );
	}
	else if ( !m_current_search.empty()
			&& ( get_current_filter_index() == filter_index ))
	{
		retval = m_current_search_index;
		rl_size = m_current_search.size();
	}
	else
	{
		retval = m_displays[m_current_display].get_rom_index( filter_index );
		rl_size = m_rl.filter_size( filter_index );
	}

	// keep in bounds
	if ( retval >= rl_size )
		retval = rl_size - 1;

	if ( retval < 0 )
		retval = 0;

	// apply the offset
	if ( rl_size > 0 )
	{
		int off = abs( offset ) % rl_size;
		if ( offset < 0 )
			retval -= off;
		else
			retval += off;

		if ( retval < 0 )
			retval = retval + rl_size;
		if ( retval >= rl_size )
			retval = retval - rl_size;
	}

	return retval;
}

void FeSettings::set_search_rule( const std::string &rule_str )
{
	m_current_search.clear();
	m_current_search_index=0;
	m_current_search_str.clear();

	if ( rule_str.empty() )
		return;

	FeRule rule;
	if ( rule.process_setting( "", rule_str, "" ) )
		return;

	rule.init();

	int filter_index = get_current_filter_index();
	for ( int i=0; i<m_rl.filter_size( filter_index ); i++ )
	{
		FeRomInfo &r = m_rl.lookup( filter_index, i );
		if ( rule.apply_rule( r ) )
			m_current_search.push_back( &r );
	}

	if ( !m_current_search.empty() )
		m_current_search_str = rule_str;
}

const std::string &FeSettings::get_search_rule() const
{
	return m_current_search_str;
}

const std::string &FeSettings::get_current_display_title() const
{
	if ( m_current_display < 0 )
		return m_menu_prompt; // When showing the 'Displays Menu', title=configured menu prompt

	return m_displays[m_current_display].get_info( FeDisplayInfo::Name );
}

const std::string &FeSettings::get_rom_info( int filter_offset, int rom_offset, FeRomInfo::Index index )
{
	int filter_index = get_filter_index_from_offset( filter_offset );
	return get_rom_info_absolute(
		filter_index,
		get_rom_index( filter_index, rom_offset ),
		index );
}

const std::string &FeSettings::get_rom_info_absolute( int filter_index, int rom_index, FeRomInfo::Index index )
{
	if ( get_filter_size( filter_index ) < 1 )
		return FE_EMPTY_STRING;

	// Make sure we have file availability information if user is requesting it.
	if ( index == FeRomInfo::FileIsAvailable )
		m_rl.get_file_availability();

	// handle situation where we are currently showing a search result
	//
	if ( !m_current_search.empty()
			&& ( get_current_filter_index() == filter_index ))
		return m_current_search[ rom_index ]->get_info( index );

	return m_rl.lookup( filter_index, rom_index ).get_info( index );
}

FeRomInfo *FeSettings::get_rom_absolute( int filter_index, int rom_index )
{
	if ( get_filter_size( filter_index ) < 1 )
		return NULL;

	// handle situation where we are currently showing a search result
	//
	if ( !m_current_search.empty()
			&& ( get_current_filter_index() == filter_index ))
		return m_current_search[ rom_index ];

	return &(m_rl.lookup( filter_index, rom_index ));
}

bool FeSettings::get_path(
	FePathType t,
	std::string &path,
	std::string &file ) const
{
	std::string temp;
	FePathType tt;
	if ( t == Current )
	{
		switch ( m_present_state )
		{
		case ScreenSaver_Showing: tt = ScreenSaver; break;
		case Intro_Showing: tt = Intro; break;
		case Layout_Showing:
		default:
			tt = Layout;
			break;
		}
	}
	else
		tt = t;

	switch ( tt )
	{
	case Intro:
		if ( internal_resolve_config_file( m_config_path,
				temp, FE_INTRO_SUBDIR, FE_INTRO_FILE ) )
		{
			size_t len = temp.find_last_of( "/\\" );
			ASSERT( len != std::string::npos );

			path = temp.substr( 0, len + 1 );
			file = FE_INTRO_FILE;
			return true;
		}
		return false;

	case Loader:
		if ( internal_resolve_config_file( m_config_path,
				temp, FE_LOADER_SUBDIR, FE_LOADER_FILE ) )
		{
			size_t len = temp.find_last_of( "/\\" );
			ASSERT( len != std::string::npos );

			path = temp.substr( 0, len + 1 );
			file = FE_LOADER_FILE;
			return true;
		}
		return false;

	case ScreenSaver:
		if ( internal_resolve_config_file( m_config_path,
				temp, FE_SCREENSAVER_SUBDIR, FE_SCREENSAVER_FILE ) )
		{
			size_t len = temp.find_last_of( "/\\" );
			ASSERT( len != std::string::npos );

			path = temp.substr( 0, len + 1 );
			file = FE_SCREENSAVER_FILE;

			return true;
		}
		else
		{
			std::cerr << "Error loading screensaver: " << FE_SCREENSAVER_FILE
					<< std::endl;
			return false;
		}
		break;

	case Layout:
	default:
		if ( m_current_display < 0 )
		{
			// We are showing the "Displays Menu"
			if ( !get_layout_dir( m_menu_layout, path ) )
				return false;

			file = m_menu_layout_file;
		}
		else
		{
			if ( m_current_display < 0 )
				return false;

			if ( !get_layout_dir(
					m_displays[ m_current_display ].get_info( FeDisplayInfo::Layout ),
					path ) )
				return false;

			file = m_displays[m_current_display].get_current_layout_file();
		}

		if ( file.empty() )
		{
			if ( is_supported_archive( path ) )
			{
				std::string temp = FE_LAYOUT_FILE_BASE;
				temp += FE_LAYOUT_FILE_EXTENSION;

				get_archive_filename_with_base(
				        file,
				        path,
				        temp );
			}
			else
			{
				std::string temp = path + FE_LAYOUT_FILE_BASE
					+ FE_LAYOUT_FILE_EXTENSION;
				if ( file_exists( temp ) )
				{
					file = FE_LAYOUT_FILE_BASE;
					file += FE_LAYOUT_FILE_EXTENSION;
				}
			}
		}
		else
			file += FE_LAYOUT_FILE_EXTENSION;
		break;
	}
	return true;
}

bool FeSettings::get_path(
	FePathType t,
	std::string &path ) const
{
	std::string temp;
	return get_path( t, path, temp );
}

void FeSettings::get_layout_file_basenames_from_path(
	const std::string &path,
	std::vector<std::string> &names_list )
{
	std::vector<std::string> temp_list;

	// check if we are dealing with an archive (.zip, .7z, etc.)
	int i=0;
	int a_index=-1;
	while ( FE_ARCHIVE_EXT[i] != NULL )
	{
		if ( tail_compare( path, FE_ARCHIVE_EXT[i] ) )
		{
			a_index = i;
			break;
		}

		i++;
	}

	if ( a_index >= 0 )
	{
		// Archive

		std::vector<std::string> t2;
		fe_zip_get_dir( path.c_str(), t2 );

		size_t zlen = strlen( FE_ARCHIVE_EXT[a_index] );

		for ( std::vector<std::string>::iterator itr=t2.begin();
			itr != t2.end(); ++itr )
		{
			if ( tail_compare( *itr, FE_LAYOUT_FILE_EXTENSION ) )
				temp_list.push_back(
					(*itr).substr( 0, (*itr).size()-zlen ) );
		}
	}
	else
	{
		get_basename_from_extension( temp_list, path,
			FE_LAYOUT_FILE_EXTENSION );
	}

	names_list.clear();

	int test_len = strlen( FE_LAYOUT_FILE_BASE );
	for ( std::vector< std::string >::iterator itr=temp_list.begin(); itr != temp_list.end(); ++itr )
	{
		// Archives may contain layout*.nut files in a subfolder of the archive,
		// so we want to exclude any preceding path info from our comparison here
		//
		std::size_t pos = (*itr).find_last_of( "/\\" );
		if ( pos != std::string::npos )
			pos++;
		else
			pos = 0;

		if ( (*itr).compare( pos, test_len, FE_LAYOUT_FILE_BASE ) == 0 )
			names_list.push_back( *itr );
	}
}

bool FeSettings::get_layout_dir(
	const std::string &layout_name,
	std::string &layout_dir ) const
{
	if ( layout_name.empty() )
		return false;

	std::string temp;
	if ( internal_resolve_config_file( m_config_path, layout_dir,
			FE_LAYOUT_SUBDIR, layout_name + "/" ) )
		return true;

	int i=0;
	while ( FE_ARCHIVE_EXT[i] != NULL )
	{
		if ( internal_resolve_config_file( m_config_path,
				layout_dir,
				FE_LAYOUT_SUBDIR,
				layout_name + FE_ARCHIVE_EXT[i] ) )
			return true;

		i++;
	}

	return false;
}

FeLayoutInfo &FeSettings::get_layout_config( const std::string &layout_name )
{
	for ( std::vector<FeLayoutInfo>::iterator itr=m_layout_params.begin(); itr != m_layout_params.end(); ++itr )
	{
		if ( layout_name.compare( (*itr).get_name() ) == 0 )
			return (*itr);
	}

	// Add a new config entry if one doesn't exist
	m_layout_params.push_back( FeLayoutInfo( layout_name ) );
	return m_layout_params.back();
}

FeLayoutInfo &FeSettings::get_current_config( FePathType t )
{
	FePathType tt;
	if ( t == Current )
	{
		switch ( m_present_state )
		{
		case ScreenSaver_Showing: tt = ScreenSaver; break;
		case Intro_Showing: tt = Intro; break;
		case Layout_Showing:
		default:
			tt = Layout;
			break;
		}
	}
	else
		tt = t;

	switch ( tt )
	{
	case Intro:
			return m_intro_params;
	case ScreenSaver:
			return m_saver_params;
	case Loader:
	case Layout:
	default:
		if ( m_current_display < 0 )
		{
			// showing the "Displays Menu"
			return get_layout_config( m_menu_layout );
		}
		else
		{
			return get_layout_config(
				m_displays[ m_current_display ].get_info(
					FeDisplayInfo::Layout ) );
		}
	}
}

const std::string &FeSettings::get_config_dir() const
{
	return m_config_path;
}

bool FeSettings::config_file_exists() const
{
	std::string config_file = m_config_path;
	config_file += FE_CFG_FILE;

	return file_exists( config_file );
}

// return true if layout needs to be reloaded as a result
bool FeSettings::set_display( int index )
{
	std::string old_path, old_file;
	get_path( Layout, old_path, old_file );

	if ( index < 0 )
		m_current_search_index = find_idx_in_vec( m_current_display, m_display_menu );

	m_current_display = index;

	m_rl.save_state();
	init_display();

	std::string new_path, new_file;
	get_path( Layout, new_path, new_file );

	return (( old_path.compare( new_path ) != 0 )
		|| ( old_file.compare( new_file ) != 0 ));
}

// return true if layout needs to be reloaded as a result
bool FeSettings::navigate_display( int step, bool wrap_mode )
{
	int i = find_idx_in_vec( m_current_display, m_display_cycle );
	i += step;

	int idx=0;
	if ( !m_display_cycle.empty() )
	{
		if ( i >= (int)m_display_cycle.size() )
			idx = m_display_cycle[0];
		else if ( i < 0 )
			idx = m_display_cycle[m_display_cycle.size()-1];
		else
			idx = m_display_cycle[i];
	}

	if ( idx >= (int)m_displays.size() )
		return false;

	bool retval = set_display( idx );

	if ( wrap_mode )
	{
		if ( step > 0 )
			set_current_selection( 0, -1 );
		else if ( m_current_display >= 0 )
			set_current_selection( m_displays[m_current_display].get_filter_count() - 1, -1 );
	}

	return retval;
}

// return true if layout needs to be reloaded as a result
bool FeSettings::navigate_filter( int step )
{
	if ( m_current_display < 0 )
		return false;

	int filter_count = m_displays[m_current_display].get_filter_count();
	int new_filter = m_displays[m_current_display].get_current_filter_index() + step;

	if ( new_filter >= filter_count )
	{
		if ( m_filter_wrap_mode == JumpToNextDisplay )
			return navigate_display( 1, true );

		new_filter = ( m_filter_wrap_mode == NoWrap ) ? filter_count - 1 : 0;
	}
	if ( new_filter < 0 )
	{
		if ( m_filter_wrap_mode == JumpToNextDisplay )
			return navigate_display( -1, true );

		new_filter = ( m_filter_wrap_mode == NoWrap ) ? 0 : filter_count - 1;
	}

	set_search_rule( "" );
	set_current_selection( new_filter, -1 );
	return false;
}

int FeSettings::get_current_display_index() const
{
	return m_current_display;
}

int FeSettings::get_display_index_from_name( const std::string &n ) const
{
	for ( unsigned int i=0; i<m_displays.size(); i++ )
	{
		if ( n.compare( m_displays[i].get_info( FeDisplayInfo::Name ) ) == 0 )
			return i;
	}
	return -1;
}

// if rom_index < 0, then the rom index is left unchanged and only the filter index is changed
void FeSettings::set_current_selection( int filter_index, int rom_index )
{
	// handle situation where we are currently showing a search result
	//
	if (( m_current_display < 0 )
		|| ( !m_current_search.empty()
			&& ( get_current_filter_index() == filter_index )))
	{
		m_current_search_index = rom_index;
	}
	else
	{
		m_displays[m_current_display].set_current_filter_index( filter_index );
		if ( rom_index >= 0 )
			m_displays[m_current_display].set_rom_index( filter_index, rom_index );
	}

	m_loaded_game_extras = false;
}

namespace {
	const char *game_extra_strings[] = {
		"executable",
		"args",
		"overview",
		NULL
	};
};

const std::string &FeSettings::get_game_extra( GameExtra id )
{
	if ( !m_loaded_game_extras )
	{
		m_game_extras.clear();
		m_loaded_game_extras = true;

		//
		// Load extra rom settings now (if available)
		//
		if ( m_current_display < 0 )
			return FE_EMPTY_STRING;

		const std::string &romlist_name = m_displays[m_current_display].get_info(FeDisplayInfo::Romlist);
		if ( romlist_name.empty() )
			return FE_EMPTY_STRING;

		if ( !load_game_extras(
				romlist_name,
				get_rom_info( 0, 0, FeRomInfo::Romname ),
				m_game_extras ) )
			return FE_EMPTY_STRING;
	}

	std::map<GameExtra,std::string>::iterator it = m_game_extras.find( id );
	if ( it != m_game_extras.end() )
		return (*it).second;

	return FE_EMPTY_STRING;
}

bool FeSettings::load_game_extras(
		const std::string &romlist_name,
		const std::string &romname,
		std::map<GameExtra,std::string> &extras )
{
	std::string path( m_config_path );
	path += FE_ROMLIST_SUBDIR;
	path += romlist_name;
	path += "/";

	if (( !directory_exists( path ) ) || romname.empty() )
		return false;

	path += romname;
	path += FE_GAME_EXTRA_FILE_EXTENSION;
	if ( !file_exists( path ) )
		return false;

	std::ifstream in_file( path.c_str() );
	if ( !in_file.is_open() )
		return false;

	while ( in_file.good() )
	{
		std::string line, s, v;
		getline( in_file, line );
		if ( line_to_setting_and_value( line, s, v ) )
		{
			int i=0;
			while ( game_extra_strings[i] != NULL )
			{
				if ( s.compare( game_extra_strings[i] ) == 0 )
				{
					extras[ (GameExtra)i ] = v;
					break;
				}
				i++;
			}

			if ( game_extra_strings[i] == NULL )
				std::cerr << " ! Unrecognized game setting: " << s << " " << v << std::endl;
		}
	}

	in_file.close();
	return true;
}

void FeSettings::set_game_extra( GameExtra id, const std::string &value )
{
	if ( value.empty() )
	{
		std::map<GameExtra,std::string>::iterator it=m_game_extras.find( id );
		if ( it != m_game_extras.end() )
			m_game_extras.erase( it );
	}
	else
		m_game_extras[ id ] = value;
}

void FeSettings::save_game_extras()
{
	if ( m_current_display < 0 )
		return;

	const std::string &romlist_name = m_displays[m_current_display].get_info(FeDisplayInfo::Romlist);
	if ( romlist_name.empty() )
		return;

	save_game_extras(
		romlist_name,
		get_rom_info( 0, 0, FeRomInfo::Romname ),
		m_game_extras );
}

void FeSettings::save_game_extras( const std::string &romlist_name,
		const std::string &romname,
		const std::map<GameExtra,std::string> &extras )
{
	std::string path( m_config_path );
	path += FE_ROMLIST_SUBDIR;

	confirm_directory( path, romlist_name );

	path += romlist_name;
	path += "/";

	if ( romname.empty() )
		return;

	path += romname;
	path += FE_GAME_EXTRA_FILE_EXTENSION;

	if ( extras.empty() )
	{
		if ( file_exists( path ) )
			delete_file( path );

		return;
	}

	std::ofstream out_file( path.c_str() );
	if ( !out_file.is_open() )
		return;

	std::map<GameExtra,std::string>::const_iterator it;
	for ( it=extras.begin(); it!=extras.end(); ++it )
		out_file << game_extra_strings[(int)(*it).first] << " " << (*it).second << std::endl;

	out_file.close();
}

int FeSettings::get_current_filter_index() const
{
	if ( m_current_display < 0 )
		return 0;

	return m_displays[m_current_display].get_current_filter_index();
}

const std::string &FeSettings::get_filter_name( int filter_index )
{
	if ( m_current_display < 0 )
		return FE_EMPTY_STRING;

	FeFilter *f = m_displays[m_current_display].get_filter( filter_index );

	if ( !f )
		return FE_EMPTY_STRING;

	return f->get_name();
}

void FeSettings::get_current_sort( FeRomInfo::Index &idx, bool &rev, int &limit )
{
	idx = FeRomInfo::LAST_INDEX;
	rev = false;
	limit = 0;

	if ( m_current_display < 0 )
		return;

	FeFilter *f = m_displays[m_current_display].get_filter(
			m_displays[m_current_display].get_current_filter_index() );

	if ( f )
	{
		idx = f->get_sort_by();
		rev = f->get_reverse_order();
		limit = f->get_list_limit();
	}
}

void FeSettings::step_current_selection( int step )
{
	int filter_index = get_current_filter_index();
	set_current_selection( filter_index, get_rom_index( filter_index, step )  );
}

bool FeSettings::select_last_launch()
{
	bool retval = false;
	if (( m_current_display != m_last_launch_display )
		&& ( m_last_launch_display < (int)m_displays.size() ))
	{
		set_display( m_last_launch_display );
		retval = true;
	}

	set_search_rule( "" );
	set_current_selection( m_last_launch_filter, m_last_launch_rom );
	return retval;
}

bool FeSettings::get_current_fav()
{
	int filter_index = get_current_filter_index();

	const std::string &s = get_rom_info_absolute( filter_index,
		get_rom_index( filter_index, 0 ),
		FeRomInfo::Favourite );

	if ( s.empty() || ( s.compare("1") != 0 ))
		return false;
	else
		return true;
}

bool FeSettings::set_current_fav( bool status )
{
	if ( m_current_display < 0 )
		return false;

	int filter_index = get_current_filter_index();

	FeRomInfo *r = get_rom_absolute( filter_index,
			get_rom_index( filter_index, 0 ) );

	if ( !r )
		return false;

	return m_rl.set_fav( *r, m_displays[m_current_display], status );
}

int FeSettings::get_prev_fav_offset()
{
	int filter_index = get_current_filter_index();
	int idx = get_rom_index( filter_index, 0 );

	for ( int i=1; i < get_filter_size( filter_index ); i++ )
	{
		int t_idx = ( i <= idx ) ? ( idx - i )
			: ( get_filter_size( filter_index ) - ( i - idx ) );

		if ( get_rom_info_absolute( filter_index,
				t_idx,
				FeRomInfo::Favourite ).compare( "1" ) == 0 )
			return ( t_idx - idx );
	}

	return 0;
}

int FeSettings::get_next_fav_offset()
{
	int filter_index = get_current_filter_index();
	int idx = get_rom_index( filter_index, 0 );

	for ( int i=1; i < get_filter_size( filter_index ); i++ )
	{
		int t_idx = ( idx + i ) % get_filter_size( filter_index );
		if ( get_rom_info_absolute( filter_index,
				t_idx,
				FeRomInfo::Favourite ).compare( "1" ) == 0 )
			return ( t_idx - idx );
	}

	return 0;
}

int FeSettings::get_next_letter_offset( int step )
{
	int filter_index = get_current_filter_index();
	int idx = get_rom_index( filter_index, 0 );

	FeRomListSorter s;
	const char curr_l = s.get_first_letter( get_rom_absolute( filter_index, idx ) );
	bool is_alpha = std::isalpha( curr_l );
	int retval = 0;

	for ( int i=1; i < get_filter_size( filter_index ); i++ )
	{
		int t_idx;
		if ( step > 0 )
			t_idx = ( idx + i ) % get_filter_size( filter_index );
		else
			t_idx = ( i <= idx ) ? ( idx - i ) : ( get_filter_size( filter_index ) - ( i - idx ) );

		const char test_l = s.get_first_letter( get_rom_absolute( filter_index, t_idx ) );

		if ((( is_alpha ) && ( test_l != curr_l ))
				|| ((!is_alpha) && ( std::isalpha( test_l ) )))
		{
			retval = t_idx - idx;
			break;
		}
	}

	return retval;
}

void FeSettings::get_current_tags_list(
	std::vector< std::pair<std::string, bool> > &tags_list )
{
	int filter_index = get_current_filter_index();

	FeRomInfo *r = get_rom_absolute( filter_index,
			get_rom_index( filter_index, 0 ) );

	if ( !r )
		return;

	m_rl.get_tags_list( *r, tags_list );
}

bool FeSettings::set_current_tag(
		const std::string &tag, bool flag )
{
	if ( m_current_display < 0 )
		return false;

	int filter_index = get_current_filter_index();

	FeRomInfo *r = get_rom_absolute( filter_index,
			get_rom_index( filter_index, 0 ) );

	if ( !r )
		return false;

	return m_rl.set_tag( *r, m_displays[m_current_display], tag, flag );
}

void FeSettings::toggle_layout()
{
	std::vector<std::string> list;

	std::string layout_path, layout_file;
	get_path( Layout, layout_path, layout_file );

	// strip extension off layout_file
	layout_file = layout_file.substr( 0,
		layout_file.size() - strlen( FE_LAYOUT_FILE_EXTENSION ) );

	get_layout_file_basenames_from_path(
			layout_path,
			list );

	if ( list.size() <= 1 ) // nothing to do if there isn't more than one file
		return;

	unsigned int index=0;
	for ( unsigned int i=0; i< list.size(); i++ )
	{
		if ( layout_file.compare( list[i] ) == 0 )
		{
			index = i;
			break;
		}
	}

	layout_file = list[ ( index + 1 ) % list.size() ];

	if ( m_current_display < 0 )
		m_menu_layout_file = layout_file;
	else
		m_displays[ m_current_display ].set_current_layout_file( layout_file );
}

void FeSettings::set_volume( FeSoundInfo::SoundType t, const std::string &v )
{
	m_sounds.set_volume( t, v );
}

int FeSettings::get_set_volume( FeSoundInfo::SoundType t ) const
{
	return m_sounds.get_set_volume( t );
}

int FeSettings::get_play_volume( FeSoundInfo::SoundType t ) const
{
	return m_sounds.get_play_volume( t );
}

bool FeSettings::get_mute() const
{
	return m_sounds.get_mute();
}

void FeSettings::set_mute( bool m )
{
	m_sounds.set_mute( m );
}

bool FeSettings::get_sound_file( FeInputMap::Command c, std::string &s, bool full_path ) const
{
	std::string filename;
	if ( m_sounds.get_sound( c, filename ) )
	{
		if ( full_path )
		{
			if ( !internal_resolve_config_file( m_config_path, s, FE_SOUND_SUBDIR, filename ) )
			{
				std::cerr << "Sound file not found: " << filename << std::endl;
				return false;
			}
		}
		else
			s = filename;

		return true;
	}
	return false;
}

void FeSettings::set_sound_file( FeInputMap::Command c, const std::string &s )
{
	m_sounds.set_sound( c, s );
}

void FeSettings::get_sounds_list( std::vector < std::string > &ll ) const
{
	ll.clear();
	internal_gather_config_files( ll, "", FE_SOUND_SUBDIR );
}

void FeSettings::run( int &minimum_run_seconds, launch_callback_fn launch_cb, void *launch_opaque )
{
	minimum_run_seconds=0;
	int filter_index = get_current_filter_index();

	if ( get_filter_size( filter_index ) < 1 )
		return;

	int rom_index = get_rom_index( filter_index, 0 );

	FeRomInfo *rom = get_rom_absolute( filter_index, rom_index );
	if ( !rom )
		return;

	const FeEmulatorInfo *emu = get_emulator(
			rom->get_info( FeRomInfo::Emulator ) );
	if ( !emu )
		return;

	const std::string &rom_name = rom->get_info( FeRomInfo::Romname );
	minimum_run_seconds = as_int( emu->get_info( FeEmulatorInfo::Minimum_run_time ) );

	m_last_launch_display = get_current_display_index();
	m_last_launch_filter = filter_index;

	if ( !m_current_search.empty()
			&& ( get_current_filter_index() == filter_index ))
	{
		//
		// Search results are temporary, so if we are currently
		// showing search results we need to find the selected game
		// in the filter itself (without the search applied)
		//
		m_last_launch_rom = 0;
		for ( int i=0; i<m_rl.filter_size( filter_index ); i++ )
		{
			if ( m_rl.lookup( filter_index, i ) == *rom )
			{
				m_last_launch_rom = i;
				break;
			}
		}
	}
	else
		m_last_launch_rom = rom_index;

	std::string command, args, rom_path, extension, romfilename;

	std::vector<std::string>::const_iterator itr;

	const std::vector<std::string> &exts = emu->get_extensions();
	const char *my_filter[ exts.size() + 1 ];
	bool check_subdirs = false;

	unsigned int i=0;
	for ( std::vector<std::string>::const_iterator itr= exts.begin();
			itr != exts.end(); ++itr )
	{
		if ( exts[i].compare( FE_DIR_TOKEN ) == 0 )
			check_subdirs = true;
		else
		{
			my_filter[i] = (*itr).c_str();
			i++;
		}
	}
	my_filter[ i ] = NULL;

	const std::vector<std::string> &paths = emu->get_paths();

	//
	// Search for the rom
	//
	bool found=false;
	for ( itr = paths.begin(); itr != paths.end(); ++itr )
	{
		std::string path = emu->clean_path_with_wd( *itr, true );

		std::vector < std::string > in_list;
		std::vector < std::string > out_list;

		get_filename_from_base( in_list, out_list, path, rom_name, my_filter );

		if (( check_subdirs ) && ( directory_exists( path + rom_name ) ))
			in_list.push_back( path + rom_name );

		if ( !in_list.empty() )
		{
			//
			// Found the rom to run
			//
			rom_path = path;
			for ( std::vector<std::string>::const_iterator i = in_list.begin(); i != in_list.end(); ++i )
				if ( romfilename.empty() || romfilename.length() > i->length() )
					romfilename = *i;
			found = true;
			break;
		}
	}

	if ( found )
	{
		//
		// figure out the extension
		//
		for ( itr = exts.begin(); itr != exts.end(); ++itr )
		{
			if ( ( (*itr).compare( FE_DIR_TOKEN ) == 0 )
					? directory_exists( romfilename )
					: tail_compare( romfilename, (*itr) ) )
			{
				extension = (*itr);
				break;
			}
		}
	}
	else
	{
		if ( !exts.empty() )
			extension = exts.front();

		if ( !paths.empty() )
			rom_path = clean_path( paths.front(), true );

		romfilename = rom_path + rom_name + extension;

		std::cerr << "Warning: could not locate rom.  Best guess: "
				<< romfilename << std::endl;
	}

	args = get_game_extra( Arguments );
	if ( args.empty() )
		args = emu->get_info( FeEmulatorInfo::Command );

	perform_substitution( args, "[nothing]", "" );
	perform_substitution( args, "[name]", rom_name );
	perform_substitution( args, "[rompath]", rom_path );
	perform_substitution( args, "[romext]", extension );
	perform_substitution( args, "[romfilename]", romfilename );
	perform_substitution( args, "[emulator]",
				emu->get_info( FeEmulatorInfo::Name ) );
	perform_substitution( args, "[workdir]",
				clean_path( emu->get_info( FeEmulatorInfo::Working_dir ) ) );

	const std::vector<std::string> &systems = emu->get_systems();
	if ( !systems.empty() )
	{
		perform_substitution( args, "[system]", systems.front() );
		perform_substitution( args, "[systemn]", systems.back() );
	}

	std::string temp = get_game_extra( Executable );
	if ( temp.empty() )
		temp = emu->get_info( FeEmulatorInfo::Executable );

	command = clean_path( temp );

	std::string work_dir = clean_path( emu->get_info( FeEmulatorInfo::Working_dir ), true );
	if ( work_dir.empty() )
	{
		size_t pos = command.find_last_of( "/\\" );
		if ( pos != std::string::npos )
			work_dir = command.substr( 0, pos );
	}

	if ( !work_dir.empty() )
		std::cout << " - Working directory: " << work_dir << std::endl;

	std::cout << "*** Running: " << command << " " << args << std::endl;

	std::string exit_hotkey = emu->get_info( FeEmulatorInfo::Exit_hotkey );

#if defined(SFML_SYSTEM_LINUX) && !defined(FE_RPI)
	if ( !exit_hotkey.empty() && ( m_window_mode == Fullscreen ))
	{
		std::cout << " ! NOTE: The 'Exit Hotkey' setting is not supported when running Attract-Mode in 'Fullscreen Mode' on Linux."
			<< "  Configured exit hotkey of '" << exit_hotkey << "' is being ignored." << std::endl;

		exit_hotkey.clear();
	}
#endif

	sf::Clock play_timer;
	run_program(
		command,
		args,
		work_dir,
		NULL,
		NULL,
		true,
		exit_hotkey,
		m_joy_thresh,
		launch_cb,
		launch_opaque );

	if ( m_track_usage )
		update_stats( 1, play_timer.getElapsedTime().asSeconds() );
}

void FeSettings::update_stats( int play_count, int play_time )
{
	if ( m_current_display < 0 )
		return;

	int filter_index = get_current_filter_index();

	if ( get_filter_size( filter_index ) < 1 )
		return;

	int rom_index = get_rom_index( filter_index, 0 );

	FeRomInfo *rom = get_rom_absolute( filter_index, rom_index );
	if ( !rom )
		return;

	const std::string rl_name = m_displays[m_current_display].get_info( FeDisplayInfo::Romlist );

	std::string path = m_config_path + FE_STATS_SUBDIR;
	confirm_directory( path, rl_name );

	path += rl_name + "/";
	rom->update_stats( path, play_count, play_time );
}

int FeSettings::exit_command() const
{
	int r( -1 );
	if ( !m_exit_command.empty() )
		r = system( m_exit_command.c_str() );

	return r;
}

void FeSettings::do_text_substitutions( std::string &str, int filter_offset, int index_offset )
{
	int filter_index = get_filter_index_from_offset( filter_offset );
	do_text_substitutions_absolute(
				str,
				filter_index,
				get_rom_index( filter_index, index_offset ) );
}

void FeSettings::do_text_substitutions_absolute( std::string &str, int filter_index, int rom_index )
{
	//
	// Perform substitutions of the [XXX] sequences occurring in str
	//
	size_t n = std::count( str.begin(), str.end(), '[' );

	size_t open = 0;
	for ( size_t x=0; x<n; x++ )
	{
		open = str.find_first_of( '[', open );
		size_t close = str.find_first_of( ']', open );

		if ( close == std::string::npos )
			break; // done, no more enclosed tokens

		std::string token = str.substr( open+1, close-open-1 );
		bool matched=false;

		//
		// First check for rom info attribute matches
		//
		for ( int i=0; i<FeRomInfo::LAST_INDEX; i++ )
		{
			// these are special cases dealt with elsewhere
			if (( i == FeRomInfo::Title )
					|| ( i == FeRomInfo::PlayedTime ))
				continue;

			if ( token.compare( FeRomInfo::indexStrings[i] ) == 0 )
			{
				const std::string &rep = get_rom_info_absolute(
						filter_index,
						rom_index,
						(FeRomInfo::Index)i );

				str.replace( open, close-open+1, rep );
				open += rep.size();
				matched=true;
			}
		}

		if ( matched )
			continue;

		//
		// Next check for various special case attributes
		//
		const char *tokenStrings[] =
		{
			"DisplayName",
			"ListTitle", // deprecated as of 1.5
			"FilterName",
			"ListFilterName", // deprecated as of 1.5
			"ListSize",
			"ListEntry",
			"Search",
			FeRomInfo::indexStrings[FeRomInfo::Title],
			"TitleFull",
			FeRomInfo::indexStrings[FeRomInfo::PlayedTime],
			"SortName",
			"SortValue",
			"System",
			"SystemN",
			"Overview",
			NULL
		};

		int i=0;
		while ( tokenStrings[i] != NULL )
		{
			if ( token.compare( tokenStrings[i] ) == 0 )
			{
				matched = true;
				break;
			}

			i++;
		}

		if ( !matched )
		{
			open++;
			continue;
		}

		std::string rep;
		switch ( i )
		{
		case 0: // "DisplayName"
		case 1: // "ListTitle" // deprecated as of 1.5
			rep = get_current_display_title();
			break;

		case 2:	// "FilterName"
		case 3: // "ListFilterName" // deprecated as of 1.5
			rep = get_filter_name( filter_index );
			break;

		case 4: // "ListSize"
			rep = as_str( get_filter_size( filter_index ) );
			break;

		case 5: // "ListEntry"
			rep = as_str( rom_index + 1 );
			break;

		case 6: // "Search"
			rep = m_current_search_str;
			break;

		case 7: // "Title"
		case 8: // "TitleFull"
			rep = get_rom_info_absolute( filter_index,
				rom_index, FeRomInfo::Title );
			if (( m_hide_brackets ) && ( i == 7 )) // 7 == Title
				rep = name_with_brackets_stripped( rep );
			break;

		case 9: // "PlayedTime"
			rep = get_played_display_string( filter_index, rom_index );
			break;

		case 10: // "SortName"
		case 11: // "SortValue"
			{
				FeRomInfo::Index sort_by;
				bool reverse_sort;
				int list_limit;
				std::string sort_name;

				get_current_sort( sort_by, reverse_sort, list_limit );

				if ( sort_by == FeRomInfo::LAST_INDEX )
				{
					get_resource( "None", sort_name );
					sort_by = FeRomInfo::Title;
				}
				else
					get_resource( FeRomInfo::indexStrings[sort_by], sort_name );


				if ( i == 10 ) // SortName
				{
					rep = sort_name;
				}
				else
				{
					if ( sort_by == FeRomInfo::PlayedTime )
						rep = get_played_display_string(
							filter_index,
							rom_index );
					else
						rep = get_rom_info_absolute(
							filter_index,
							rom_index, sort_by );
				}
			}
			break;

		case 12: // "System"
		case 13: // "SystemN"
			{
				const std::string &en
					= get_rom_info_absolute(
						filter_index,
						rom_index,
						FeRomInfo::Emulator );

				const FeEmulatorInfo *emu = get_emulator( en );
				if ( emu )
				{
					const std::vector< std::string > &ss =
						emu->get_systems();

					if ( !ss.empty() )
					{
						rep = ( i == 12 )
							? ss.front()
							: ss.back();
					}
				}
			}
			break;

		case 14: // "Overview"
			if (( filter_index == 0 ) && ( rom_index == 0 ))
				rep = get_game_extra( Overview );
			else if ( m_current_display >= 0 )
			{
				std::map<GameExtra,std::string> extras;

				load_game_extras(
					m_displays[m_current_display].get_info( FeDisplayInfo::Romlist ),
					get_rom_info_absolute( filter_index, rom_index, FeRomInfo::Romname ),
					extras );

				rep = extras[ Overview ];
			}

			perform_substitution( rep, "\\n", "\n" );
			break;

		default:
			ASSERT( 0 ); // unhandled token
			break;
		}

		str.replace( open, close-open+1, rep );
		open += rep.size();
	}
}

std::string FeSettings::get_played_display_string( int filter_index, int rom_index )
{
	std::string label;

	int raw = as_int(
		get_rom_info_absolute( filter_index,
			rom_index,
			FeRomInfo::PlayedTime ) );

	float num;

	if ( raw < 3600 )
	{
		num = raw / 60.f;
		label = "Minutes";
	}
	else if ( raw < 86400 )
	{
		num = raw / 3600.f;
		label = "Hours";
	}
	else
	{
		num = raw / 86400.f;
		label = "Days";
	}

	std::string op_label;
	get_resource( label, op_label );

	return as_str( num, 1 ) + " " + op_label;
}


FeEmulatorInfo *FeSettings::get_emulator( const std::string &n )
{
	return m_rl.get_emulator( n );
}

FeEmulatorInfo *FeSettings::create_emulator( const std::string &n )
{
	return m_rl.create_emulator( n );
}

void FeSettings::delete_emulator( const std::string &n )
{
	m_rl.delete_emulator( n );
}

void FeSettings::get_list_of_emulators( std::vector<std::string> &emu_list )
{
	std::string path = get_config_dir();
	path += FE_EMULATOR_SUBDIR;

	get_basename_from_extension(
		emu_list,
		path,
		FE_EMULATOR_FILE_EXTENSION );

	std::sort( emu_list.begin(), emu_list.end() );
}

bool FeSettings::get_font_file( std::string &fpath,
	std::string &ffile,
	const std::string &fontname ) const
{
	if ( fontname.empty() )
	{
		if ( m_default_font.empty() )
			return false;
		else
			return get_font_file( fpath, ffile, m_default_font );
	}

	//
	// First check if there is a matching font file in the
	// layout/plugin directory
	//
	std::string test;
	std::string layout_dir;
	get_path( Current, layout_dir );

	if ( is_supported_archive( layout_dir ) )
	{
		std::string temp;
		if ( get_archive_filename_with_base( temp,
			layout_dir,
			fontname,
			FE_FONT_EXTENSIONS ) )
		{
			fpath = layout_dir;
			ffile = temp;
			return true;
		}
	}
	else if ( !layout_dir.empty() && search_for_file( layout_dir,
		fontname, FE_FONT_EXTENSIONS, test ) )
	{
		ffile = test;
		return true;
	}

#ifdef USE_FONTCONFIG
	bool fc_found = false;
	FcConfig *config = FcInitLoadConfigAndFonts();
	if ( config )
	{
		FcPattern *pat = FcNameParse( (const FcChar8 *)(fontname.c_str()) );
		if ( pat )
		{
			FcConfigSubstitute( config, pat, FcMatchPattern );
			FcDefaultSubstitute( pat );

			FcResult res = FcResultNoMatch;
			FcPattern *font = FcFontMatch( config, pat, &res );
			if ( font )
			{
				FcChar8 *file = NULL;
				if ( FcPatternGetString( font, FC_FILE, 0, &file ) == FcResultMatch )
				{
					ffile = (char *)file;
					fc_found = true;
				}
				FcPatternDestroy( font );
			}
			FcPatternDestroy( pat );
		}
		FcConfigDestroy( config );
	}

	if ( fc_found )
		return true;
#endif

	std::vector<std::string> path_list;
	std::vector<std::string>::const_iterator its;

	//
	// m_font_paths contains the configured paths (which may need further
	// processing ($HOME substitution etc)
	//
	for ( its=m_font_paths.begin(); its!=m_font_paths.end(); ++its )
		path_list.push_back( clean_path( *its, true ) );

	for ( its=path_list.begin(); its!= path_list.end(); ++its )
	{
		if ( search_for_file( (*its), fontname, FE_FONT_EXTENSIONS, test ) )
		{
			ffile = test;
			return true;
		}
	}

	return false;
}

FeSettings::WindowType FeSettings::get_window_mode() const
{
	return m_window_mode;
}

FeSettings::FilterWrapModeType FeSettings::get_filter_wrap_mode() const
{
	return m_filter_wrap_mode;
}

FeSettings::StartupModeType FeSettings::get_startup_mode() const
{
	return m_startup_mode;
}

int FeSettings::get_screen_saver_timeout() const
{
	return m_ssaver_time;
}

void FeSettings::get_displays_menu(
	std::string &title,
	std::vector<std::string> &names,
	std::vector<int> &indices,
	int &current_index ) const
{
	title = m_menu_prompt;

	names.clear();
	indices = m_display_menu;

	for ( unsigned int i=0; i<indices.size(); i++ )
		names.push_back(
			m_displays[indices[i]].get_info( FeDisplayInfo::Name ) );

	int temp = find_idx_in_vec( m_current_display, m_display_menu );
	current_index = ( temp < 0 ) ? 0 : temp;
}

const std::string FeSettings::get_info( int index ) const
{
	switch ( index )
	{
	case Language:
		return m_language;
	case ExitCommand:
		return m_exit_command;
	case DefaultFont:
		return m_default_font;
	case FontPath:
		if ( !m_font_paths.empty() )
		{
			std::string ret = m_font_paths.front();
			for ( unsigned int i=1; i < m_font_paths.size(); i++ )
			{
				ret += ";";
				ret += m_font_paths[i];
			}
			return ret;
		}
		break;
	case ScreenSaverTimeout:
		return as_str( m_ssaver_time);
	case MouseThreshold:
		return as_str( m_mouse_thresh );
	case JoystickThreshold:
		return as_str( m_joy_thresh );
	case WindowMode:
		return windowModeTokens[ m_window_mode ];
	case FilterWrapMode:
		return filterWrapTokens[ m_filter_wrap_mode ];
	case SelectionSpeed:
		return as_str( m_selection_speed );
	case StartupMode:
		return startupTokens[ m_startup_mode ];

	case DisplaysMenuExit:
	case HideBrackets:
	case ConfirmFavourites:
	case ConfirmExit:
	case TrackUsage:
	case MultiMon:
	case SmoothImages:
	case AccelerateSelection:
	case ScrapeSnaps:
	case ScrapeMarquees:
	case ScrapeFlyers:
	case ScrapeWheels:
	case ScrapeFanArt:
	case ScrapeVids:
	case ScrapeMameDB:
	case ScrapeOverview:
#ifdef SFML_SYSTEM_WINDOWS
	case HideConsole:
#endif
		return ( get_info_bool( index ) ? FE_CFG_YES_STR : FE_CFG_NO_STR );
	case VideoDecoder:
#ifdef NO_MOVIE
		return "software";
#else
		return FeMedia::get_decoder_label( FeMedia::get_current_decoder() );
#endif

	case MenuPrompt:
		return m_menu_prompt;

	case MenuLayout:
		return m_menu_layout;

	default:
		break;
	}
	return FE_EMPTY_STRING;
}

bool FeSettings::get_info_bool( int index ) const
{
	switch ( index )
	{
	case DisplaysMenuExit:
		return m_displays_menu_exit;
	case HideBrackets:
		return m_hide_brackets;
	case ConfirmFavourites:
		return m_confirm_favs;
	case ConfirmExit:
		return m_confirm_exit;
	case TrackUsage:
		return m_track_usage;
	case MultiMon:
		return m_multimon;
	case SmoothImages:
		return m_smooth_images;
	case AccelerateSelection:
		return m_accel_selection;
	case ScrapeSnaps:
		return m_scrape_snaps;
	case ScrapeMarquees:
		return m_scrape_marquees;
	case ScrapeFlyers:
		return m_scrape_flyers;
	case ScrapeWheels:
		return m_scrape_wheels;
	case ScrapeFanArt:
		return m_scrape_fanart;
	case ScrapeVids:
		return m_scrape_vids;
	case ScrapeMameDB:
		return m_scrape_mamedb;
	case ScrapeOverview:
		return m_scrape_overview;
#ifdef SFML_SYSTEM_WINDOWS
	case HideConsole:
		return m_hide_console;
#endif
	default:
		break;
	}
	return false;
}

bool FeSettings::set_info( int index, const std::string &value )
{
	switch ( index )
	{
	case Language:
		m_language = value;
		break;

	case ExitCommand:
		m_exit_command = value;
		break;

	case DefaultFont:
		m_default_font = value;
		break;

	case FontPath:
		{
			size_t pos=0;
			m_font_paths.clear();
			do
			{
				std::string path;
				token_helper( value, pos, path );
				m_font_paths.push_back( path );
			} while ( pos < value.size() );
		}
		break;

	case ScreenSaverTimeout:
		m_ssaver_time = as_int( value );
		break;

	case DisplaysMenuExit:
		m_displays_menu_exit = config_str_to_bool( value );
		break;

	case HideBrackets:
		m_hide_brackets = config_str_to_bool( value );
		break;

	case StartupMode:
		{
			int i=0;
			while ( startupTokens[i] != NULL )
			{
				if ( value.compare( startupTokens[i] ) == 0 )
				{
					m_startup_mode = (StartupModeType)i;
					break;
				}
				i++;
			}

			if ( startupTokens[i] == NULL )
				return false;
		}
		break;

	case ConfirmFavourites:
		m_confirm_favs = config_str_to_bool( value );
		break;

	case ConfirmExit:
		m_confirm_exit = config_str_to_bool( value );
		break;

	case MouseThreshold:
		m_mouse_thresh = as_int( value );
		if ( m_mouse_thresh > 100 )
			m_mouse_thresh=100;
		else if ( m_mouse_thresh < 1 )
			m_mouse_thresh=1;
		break;

	case JoystickThreshold:
		m_joy_thresh = as_int( value );
		if ( m_joy_thresh > 100 )
			m_joy_thresh=100;
		else if ( m_joy_thresh < 1 )
			m_joy_thresh=1;
		break;

	case WindowMode:
		{
			int i=0;
			while ( windowModeTokens[i] != NULL )
			{
				if ( value.compare( windowModeTokens[i] ) == 0 )
				{
					m_window_mode = (WindowType)i;
					break;
				}
				i++;
			}

			if ( windowModeTokens[i] == NULL )
				return false;
		}
		break;

	case FilterWrapMode:
		{
			int i=0;
			while ( filterWrapTokens[i] != NULL )
			{
				if ( value.compare( filterWrapTokens[i] ) == 0 )
				{
					m_filter_wrap_mode = (FilterWrapModeType)i;
					break;
				}
				i++;
			}

			if ( filterWrapTokens[i] == NULL )
				return false;
		}
		break;

	case TrackUsage:
		m_track_usage = config_str_to_bool( value );
		break;

	case MultiMon:
		m_multimon = config_str_to_bool( value );
		break;

	case SmoothImages:
		m_smooth_images = config_str_to_bool( value );
		break;

	case AccelerateSelection:
		m_accel_selection = config_str_to_bool( value );
		break;

	case SelectionSpeed:
		m_selection_speed = as_int( value );
		if ( m_selection_speed < 0 )
			m_selection_speed = 0;
		break;

	case ScrapeSnaps:
		m_scrape_snaps = config_str_to_bool( value );
		break;

	case ScrapeMarquees:
		m_scrape_marquees = config_str_to_bool( value );
		break;

	case ScrapeFlyers:
		m_scrape_flyers = config_str_to_bool( value );
		break;

	case ScrapeWheels:
		m_scrape_wheels = config_str_to_bool( value );
		break;

	case ScrapeFanArt:
		m_scrape_fanart = config_str_to_bool( value );
		break;

	case ScrapeVids:
		m_scrape_vids = config_str_to_bool( value );
		break;

	case ScrapeMameDB:
		m_scrape_mamedb = config_str_to_bool( value );
		break;

	case ScrapeOverview:
		m_scrape_overview = config_str_to_bool( value );
		break;

#ifdef SFML_SYSTEM_WINDOWS
	case HideConsole:
		m_hide_console = config_str_to_bool( value );
		break;
#endif
	case VideoDecoder:
#ifndef NO_MOVIE
		FeMedia::set_current_decoder_by_label( value );
#endif
		break;

	case MenuLayout:
		if ( m_menu_layout.compare( value ) != 0 )
		{
			m_menu_layout = value;
			m_menu_layout_file.clear();
		}
		break;

	case MenuPrompt:
		m_menu_prompt = value;
		break;

	default:
		return false;
	}

	return true;
}


FeDisplayInfo *FeSettings::get_display( int index )
{
	if ( ( index < 0 ) || ( index >= (int)m_displays.size() ))
		return NULL;

	std::vector<FeDisplayInfo>::iterator itr=m_displays.begin() + index;
	return &(*itr);
}

void FeSettings::create_filter( FeDisplayInfo &d, const std::string &name ) const
{
	FeFilter new_filter( name );

	std::string defaults_file;
	if ( internal_resolve_config_file( m_config_path, defaults_file, NULL, FE_FILTER_DEFAULT ) )
		new_filter.load_from_file( defaults_file );

	d.append_filter( new_filter );
}

FeDisplayInfo *FeSettings::create_display( const std::string &n )
{
	if ( m_current_display == -1 )
		m_current_display=0;

	FeDisplayInfo new_display( n );

	std::string defaults_file;
	if ( internal_resolve_config_file( m_config_path, defaults_file, NULL, FE_LIST_DEFAULT ) )
		new_display.load_from_file( defaults_file );

	// If there is no layout set, set a good default one now
	//
	if ( new_display.get_info( FeDisplayInfo::Layout ).empty() )
	{
		if ( !m_displays.empty() )
		{
			// If other lists are configured, give the new list the same layout as
			// the last configured list
			//
			new_display.set_info( FeDisplayInfo::Layout,
				m_displays.back().get_info( FeDisplayInfo::Layout ) );
		}
		else
		{
			// Pick an available layout, use the first one alphabetically
			//
			std::vector<std::string> layouts;
			get_layouts_list( layouts ); // the returned list is sorted alphabetically
			if ( !layouts.empty() )
				new_display.set_info( FeDisplayInfo::Layout, layouts.front() );
		}
	}

	// If there is no romlist set, use the one from the last list created
	//
	if (( new_display.get_info( FeDisplayInfo::Romlist ).empty() )
		&& ( !m_displays.empty() ))
	{
		new_display.set_info( FeDisplayInfo::Romlist,
			m_displays.back().get_info( FeDisplayInfo::Romlist ) );
	}

	m_displays.push_back( new_display );
	return &(m_displays.back());
}

void FeSettings::get_layouts_list( std::vector<std::string> &layouts ) const
{
	std::string path = m_config_path + FE_LAYOUT_SUBDIR;

	get_subdirectories( layouts, path );

	int i=0;
	while ( FE_ARCHIVE_EXT[i] != NULL )
	{
		get_basename_from_extension( layouts, path, FE_ARCHIVE_EXT[i] );
		i++;
	}

	if ( FE_DATA_PATH != NULL )
	{
		std::string t = FE_DATA_PATH;
		t += FE_LAYOUT_SUBDIR;

		get_subdirectories( layouts, t );

		i=0;
		while ( FE_ARCHIVE_EXT[i] != NULL )
		{
			get_basename_from_extension( layouts, t, FE_ARCHIVE_EXT[i] );
			i++;
		}
	}

	if ( !layouts.empty() )
	{
		// Sort the list and remove duplicates
		std::sort( layouts.begin(), layouts.end() );
		layouts.erase( std::unique( layouts.begin(), layouts.end() ), layouts.end() );
	}
}

void FeSettings::delete_display( int index )
{
	if ( ( index < 0 ) || ( index >= (int)m_displays.size() ))
		return;

	std::vector<FeDisplayInfo>::iterator itr=m_displays.begin() + index;
	m_displays.erase( itr );

	if ( m_current_display >= index )
		m_current_display--;

	if ( m_current_display < 0 )
		m_current_display=0;
}

void FeSettings::get_current_display_filter_names(
		std::vector<std::string> &list ) const
{
	if ( m_current_display < 0 )
		return;

	m_displays[ m_current_display ].get_filters_list( list );
}

bool FeSettings::check_romlist_configured( const std::string &n ) const
{
	for ( std::vector<FeDisplayInfo>::const_iterator itr=m_displays.begin();
			itr!=m_displays.end(); ++itr )
	{
		if ( n.compare( (*itr).get_info( FeDisplayInfo::Romlist ) ) == 0 )
			return true;
	}

	return false;
}

void FeSettings::save() const
{
	confirm_directory( m_config_path, FE_ROMLIST_SUBDIR );
	confirm_directory( m_config_path, FE_EMULATOR_SUBDIR );
	confirm_directory( m_config_path, FE_LAYOUT_SUBDIR );
	confirm_directory( m_config_path, FE_SCREENSAVER_SUBDIR );
	confirm_directory( m_config_path, FE_INTRO_SUBDIR );
	confirm_directory( m_config_path, FE_MODULES_SUBDIR );
	confirm_directory( m_config_path, FE_SOUND_SUBDIR );
	confirm_directory( m_config_path, FE_PLUGIN_SUBDIR );
	confirm_directory( m_config_path, FE_MENU_ART_SUBDIR );
	// no FE_LANGUAGE_SUBDIR

	std::string menu_art = m_config_path;
	menu_art += FE_MENU_ART_SUBDIR;
	confirm_directory( menu_art, "flyer/" );
	confirm_directory( menu_art, "wheel/" );
	confirm_directory( menu_art, "marquee/" );
	confirm_directory( menu_art, "snap/" );
	confirm_directory( menu_art, "fanart/" );

	std::string filename( m_config_path );
	filename += FE_CFG_FILE;

#ifdef FE_DEBUG
   std::cout << "Writing config to: " << filename << std::endl;
#endif

	std::ofstream outfile( filename.c_str() );
	if ( outfile.is_open() )
	{
		outfile << "# Generated by " << FE_NAME << " " << FE_VERSION << std::endl
			<< "#" << std::endl;

		for ( std::vector<FeDisplayInfo>::const_iterator it=m_displays.begin();
						it != m_displays.end(); ++it )
		{
			(*it).save( outfile );
			outfile << std::endl;
		}

		outfile << otherSettingStrings[1] << std::endl; // "sounds"
		m_sounds.save( outfile );

		outfile << std::endl << otherSettingStrings[2] << std::endl; // "input_map"
		m_inputmap.save( outfile );

		outfile << std::endl << otherSettingStrings[3] << std::endl; // "general"
		for ( int i=0; i < LAST_INDEX; i++ )
		{
			std::string val = get_info( i );
			outfile << '\t' << std::setw(20) << std::left
						<< configSettingStrings[i] << ' ' << val << std::endl;
		}

		m_saver_params.save( outfile );

		for ( std::vector<FeLayoutInfo>::const_iterator itr=m_layout_params.begin(); itr != m_layout_params.end(); ++itr )
		{
			(*itr).save( outfile );
		}

		m_intro_params.save( outfile );

		std::vector<std::string> plugin_files;
		get_available_plugins( plugin_files );

		outfile << std::endl;
		std::vector< FePlugInfo >::const_iterator itr;
		for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
		{
			//
			// Get rid of configs for old plugins by not saving it if the
			// plugin itself is gone
			//
			std::vector< std::string >::const_iterator its;
			for ( its = plugin_files.begin(); its != plugin_files.end(); ++its )
			{
				if ( (*its).compare( (*itr).get_name() ) == 0 )
				{
					(*itr).save( outfile );
					break;
				}
			}
		}

		outfile.close();
	}
}

void FeSettings::get_resource( const std::string &token, std::string &str ) const
{
	m_resourcemap.get_resource( token, str );
}

void FeSettings::get_resource( const std::string &token,
					const std::string &rep, std::string &str ) const
{
	m_resourcemap.get_resource( token, str );

	if ( !rep.empty() )
		perform_substitution( str, "$1", rep );
}

int FeSettings::displays_count() const
{
	return m_displays.size();
}

void FeSettings::get_available_plugins( std::vector < std::string > &ll ) const
{
	//
	// Gather plugins that are subdirectories in the plugins directory
	//
	get_subdirectories( ll, m_config_path + FE_PLUGIN_SUBDIR );

	if ( FE_DATA_PATH != NULL )
	{
		std::string t = FE_DATA_PATH;
		t += FE_PLUGIN_SUBDIR;
		get_subdirectories( ll, t );
	}

	//
	// Also gather plugins that are lone .nut files in the plugins directory
	//
	internal_gather_config_files(
		ll,
		FE_PLUGIN_FILE_EXTENSION,
		FE_PLUGIN_SUBDIR );

	//
	// Can be an archive file (.zip, etc) in the plugins directory as well
	//
	int i=0;
	while ( FE_ARCHIVE_EXT[i] != NULL )
	{
		internal_gather_config_files(
			ll,
			FE_ARCHIVE_EXT[i],
			FE_PLUGIN_SUBDIR );
		i++;
	}

	if ( !ll.empty() )
	{
		// Sort the list and remove duplicates
		std::sort( ll.begin(), ll.end() );
		ll.erase( std::unique( ll.begin(), ll.end() ), ll.end() );
	}
}

void FeSettings::get_plugin( const std::string &label,
		FePlugInfo *&plug,
		int &index )
{
	std::vector< FePlugInfo >::iterator itr;
	for ( int i=0; i<(int)m_plugins.size(); i++ )
	{
		if ( label.compare( m_plugins[i].get_name() ) == 0 )
		{
			plug = &(m_plugins[i]);
			index = i;
			return;
		}
	}

	// No config for this plugin currently.  Add one
	//
	m_plugins.push_back( FePlugInfo( label ) );
	plug = &(m_plugins.back());
	index = m_plugins.size() - 1;
}

bool FeSettings::get_plugin_enabled( const std::string &label ) const
{
	std::vector< FePlugInfo >::const_iterator itr;
	for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
	{
		if ( label.compare( (*itr).get_name() ) == 0 )
			return (*itr).get_enabled();
	}

	// if there is no config then it isn't enabled
	return false;
}

void FeSettings::get_plugin_full_path(
				const std::string &label,
				std::string &path,
				std::string &filename ) const
{
	std::string temp;

	//
	// There are three valid locations for plugins:
	//
	// <config_dir>/plugins/<name>/plugin.nut
	// <config_dir>/plugins/<name>.nut
	// <config_dir>/plugins/<name>.zip (plugin.nut)
	//
	if ( internal_resolve_config_file( m_config_path, temp, FE_PLUGIN_SUBDIR, label + "/" ) )
	{
		path.swap( temp );
		filename = FE_PLUGIN_FILE;
		return;
	}

	if ( internal_resolve_config_file( m_config_path,
			temp,
			FE_PLUGIN_SUBDIR,
			label + FE_PLUGIN_FILE_EXTENSION ) )
	{
		size_t len = temp.find_last_of( "/\\" );
		ASSERT( len != std::string::npos );

		path = temp.substr( 0, len + 1 );
		filename = label + FE_PLUGIN_FILE_EXTENSION;
		return;
	}

	int i=0;
	while ( FE_ARCHIVE_EXT[i] != NULL )
	{
		if ( internal_resolve_config_file( m_config_path,
				temp,
				FE_PLUGIN_SUBDIR,
				label + FE_ARCHIVE_EXT[i] ) )
		{
			path.swap( temp );
			filename = FE_PLUGIN_FILE;
			return;
		}
		i++;
	}


	std::cerr << "Plugin file not found: " << label << std::endl;
}

void FeSettings::get_plugin_full_path( int id,
		std::string &path ) const
{
	if (( id < 0 ) || ( id >= (int)m_plugins.size() ))
		return;

	std::string ignored;
	get_plugin_full_path( m_plugins[id].get_name(), path, ignored );
}

void FeSettings::internal_load_language( const std::string &lang )
{
	m_resourcemap.clear();

	std::string fname;
	if ( internal_resolve_config_file( m_config_path, fname, FE_LANGUAGE_SUBDIR, lang + FE_LANGUAGE_FILE_EXTENSION ) )
		m_resourcemap.load_from_file( fname, ";" );
	else
		std::cerr << "Error loading language resource file: " << lang << std::endl;
}

void FeSettings::set_language( const std::string &s )
{
	if ( s.compare( m_language ) != 0 )
	{
		m_language = s;
		internal_load_language( m_language );
	}
}

void FeSettings::get_languages_list( std::vector < FeLanguage > &ll ) const
{
	std::vector<std::string> temp;
	internal_gather_config_files(
		temp,
		FE_LANGUAGE_FILE_EXTENSION,
		FE_LANGUAGE_SUBDIR );

	if ( temp.empty() )
	{
		ll.push_back( FeLanguage( "en" ) );
		get_resource( "en", ll.back().label );
	}
	else
	{
		for ( std::vector<std::string>::iterator itr=temp.begin();
			itr!=temp.end(); ++itr )
		{
			ll.push_back( FeLanguage( *itr ) );
			get_resource( *itr, ll.back().label );

			std::string fname = m_config_path + FE_LANGUAGE_SUBDIR;
			fname += (*itr);
			fname += FE_LANGUAGE_FILE_EXTENSION;

			if (( FE_DATA_PATH != NULL )
				&& ( !file_exists( fname ) ))
			{
				fname = FE_DATA_PATH;
				fname += (*itr);
				fname += FE_LANGUAGE_FILE_EXTENSION;
			}

			// Read first line of file to get key info
			//
			std::ifstream myfile( fname.c_str() );
			if ( !myfile.is_open() )
				continue;

			std::string line;
			getline( myfile, line );

			if (( line.size() < 2 )
				|| ( line.compare( 0, 2, "#@" ) != 0 ))
			{
				myfile.close();
				continue;
			}

			size_t pos(2);
			std::string tok;
			token_helper( line, pos, tok, ";" );

			//
			// Format should be:
			//
			// #@label;win_font=xxx,x2x2;mac_font=yyy;linux_font=zzz
			//
			if ( !tok.empty() )
				ll.back().label = tok;

			while ( pos < line.size() )
			{
				token_helper( line, pos, tok, ";" );
				if ( !tok.empty() )
				{
					std::string t2;
					size_t p2(0);
					token_helper( tok, p2, t2, "=" );

					if ( t2.compare(
#ifdef SFML_SYSTEM_WINDOWS
						"win_font"
#else
 #ifdef SFML_SYSTEM_MACOS
						"mac_font"
 #else
						"linux_font"
 #endif
#endif
							) == 0 )
					{
						while ( p2 < tok.size() )
						{
							token_helper( tok, p2, t2, "," );
							ll.back().font.push_back( t2 );
						}
					}
				}
			}
			myfile.close();
		}
	}
}

void FeSettings::get_romlists_list( std::vector < std::string > &ll ) const
{
	ll.clear();
	internal_gather_config_files(
		ll,
		FE_ROMLIST_FILE_EXTENSION,
		FE_ROMLIST_SUBDIR );
}


void FeSettings::internal_gather_config_files(
			std::vector<std::string> &ll,
			const std::string &extension,
			const char *subdir ) const
{
	std::string config_path = m_config_path + subdir;

	// check the config directory first
	if ( file_exists( config_path ) )
		get_basename_from_extension( ll, config_path, extension );

	// then the data directory
	if ( FE_DATA_PATH != NULL )
	{
		std::string data_path = FE_DATA_PATH;
		data_path += subdir;

		get_basename_from_extension( ll, data_path, extension );
	}

	// Sort the list and remove duplicates
	std::sort( ll.begin(), ll.end() );
	ll.erase( std::unique( ll.begin(), ll.end() ), ll.end() );
}

bool FeSettings::get_module_path(
	const std::string &module,
	std::string &module_dir,
	std::string &module_file ) const
{
	//
	// Try for "[module].nut" first
	//
	std::string fname = module;
	if ( !tail_compare( fname, FE_LAYOUT_FILE_EXTENSION ) )
		fname += FE_LAYOUT_FILE_EXTENSION;

	std::string res;
	if ( internal_resolve_config_file( m_config_path, res,
		FE_MODULES_SUBDIR, fname ) )
	{
		size_t len = res.find_last_of( "/\\" );
		ASSERT( len != std::string::npos );

		module_dir = absolute_path( res.substr( 0, len + 1 ) );

		len = fname.find_last_of( "/\\" );
		if ( len != std::string::npos )
			module_file = fname.substr( len + 1 );
		else
			module_file = fname;

		return true;
	}

	//
	// Fall back to "[module]/module.nut" first
	//
	const char *MOD_FNAME = "module.nut";

	fname = module + "/";
	fname += MOD_FNAME;

	if ( internal_resolve_config_file( m_config_path, res,
		FE_MODULES_SUBDIR, fname ) )
	{
		size_t len = res.find_last_of( "/\\" );
		ASSERT( len != std::string::npos );

		module_dir = absolute_path( res.substr( 0, len + 1 ) );
		module_file = MOD_FNAME;
		return true;
	}

	return false;
}

bool gather_artwork_filenames(
	const std::vector < std::string > &art_paths,
	const std::string &target_name,
	std::vector<std::string> &vids,
	std::vector<std::string> &images )
{
	for ( std::vector< std::string >::const_iterator itr = art_paths.begin();
			itr != art_paths.end(); ++itr )
	{
		std::vector < std::string > img_contents;
		std::vector < std::string > vid_contents;

		if ( is_supported_archive( *itr ) )
		{
			gather_archive_filenames_with_base(
				        img_contents,
				        vid_contents,
				        *itr,
				        target_name + ".",
					FE_ART_EXTENSIONS );

#ifdef NO_MOVIE
			vid_contents.clear();
#else
			for ( std::vector<std::string>::iterator itn = vid_contents.begin();
					itn != vid_contents.end(); )
			{
				if ( FeMedia::is_supported_media_file( *itn ) )
					++itn;
				else
					itn = vid_contents.erase( itn );
			}
#endif
			if ( !img_contents.empty() || !vid_contents.empty() )
			{
				while ( !img_contents.empty() )
				{
					images.push_back( (*itr) + "|" + img_contents.back() );
					img_contents.pop_back();
				}

				while ( !vid_contents.empty() )
				{
					vids.push_back( (*itr) + "|" + vid_contents.back() );
					vid_contents.pop_back();
				}
			}

			continue;
		}

		get_filename_from_base(
			img_contents,
			vid_contents,
			(*itr),
			target_name + '.',
			FE_ART_EXTENSIONS );

#ifdef NO_MOVIE
		vid_contents.clear();
#else
		for ( std::vector<std::string>::iterator itn = vid_contents.begin();
				itn != vid_contents.end(); )
		{
			if ( FeMedia::is_supported_media_file( *itn ) )
				++itn;
			else
				itn = vid_contents.erase( itn );
		}
#endif

		if ( !img_contents.empty() || !vid_contents.empty() )
		{
			images.insert( images.end(), img_contents.begin(), img_contents.end() );
			vids.insert( vids.end(), vid_contents.begin(), vid_contents.end() );
			continue;
		}

		//
		// If there is a subdirectory in art_path with the
		// given target name, then we load a random video or
		// image from it at this point (if available)
		//
		std::string sd_path = (*itr) + target_name;
		if ( directory_exists( sd_path ) )
		{
			sd_path += "/";

			get_filename_from_base(
				img_contents,
				vid_contents,
				sd_path,
				"",
				FE_ART_EXTENSIONS );

#ifdef NO_MOVIE
			vid_contents.clear();
#else
			for ( std::vector<std::string>::iterator itn = vid_contents.begin();
					itn != vid_contents.end(); )
			{
				if ( FeMedia::is_supported_media_file( *itn ) )
					++itn;
				else
					itn = vid_contents.erase( itn );
			}
#endif

			std::random_shuffle( vid_contents.begin(), vid_contents.end() );
			std::random_shuffle( img_contents.begin(), img_contents.end() );

			images.insert( images.end(), img_contents.begin(), img_contents.end() );
			vids.insert( vids.end(), vid_contents.begin(), vid_contents.end() );
		}
	}

	return ( !images.empty() || !vids.empty() );
}

bool art_exists( const std::string &path, const std::string &base )
{
	std::vector<std::string> u1;
	std::vector<std::string> u2;

	return ( get_filename_from_base(
		u1, u2, path, base, FE_ART_EXTENSIONS ) );
}


bool FeSettings::get_best_artwork_file(
	const FeRomInfo &rom,
	const std::string &art_name,
	std::vector<std::string> &vid_list,
	std::vector<std::string> &image_list,
	bool image_only,
	bool ignore_emu )
{
	std::vector < std::string > art_paths;

	// map boxart->flyer and banner->marquee so that artworks with those labels get
	// scraped artworks
	std::string scrape_art;
	if ( art_name.find( "box") != std::string::npos )
		scrape_art = "flyer";
	else if ( art_name.compare( "banner" ) == 0 )
		scrape_art = "marquee";
	else
		scrape_art = art_name;

	std::string emu_name = rom.get_info( FeRomInfo::Emulator );
	if ( emu_name.compare( 0, 1, "@" ) == 0 )
	{
		// If emu_name starts with "@", this is a display menu or signal option.
		// Check for the emulator stored in altromname to try and get a sensible artwork
		// in this circumstance
		//
		emu_name = rom.get_info( FeRomInfo::AltRomname );

		std::string add_path = get_config_dir() + FE_MENU_ART_SUBDIR + scrape_art + "/";
		if ( directory_exists( add_path ) )
			art_paths.push_back( add_path );

		if ( FE_DATA_PATH != NULL )
		{
			add_path = FE_DATA_PATH;
			add_path += FE_MENU_ART_SUBDIR;
			add_path += scrape_art;
			add_path += "/";

			if ( directory_exists( add_path ) )
				art_paths.push_back( add_path );
		}
	}

	std::string layout_path;
	get_path( Current, layout_path );

	FeEmulatorInfo *emu_info = get_emulator( emu_name );
	if ( emu_info )
	{
		std::vector < std::string > temp_list;
		emu_info->get_artwork( art_name, temp_list );
		for ( std::vector< std::string >::iterator itr = temp_list.begin();
			itr != temp_list.end(); ++itr )
		{
			art_paths.push_back(
				emu_info->clean_path_with_wd( *itr, !is_supported_archive( *itr ) ) );

			perform_substitution( art_paths.back(), "$LAYOUT", layout_path );
		}
	}

	std::string scraper_path = get_config_dir() + FE_SCRAPER_SUBDIR + emu_name + "/" + scrape_art + "/";
	if ( directory_exists( scraper_path ) )
		art_paths.push_back( scraper_path );

	if ( !art_paths.empty() )
	{
		const std::string &romname = rom.get_info( FeRomInfo::Romname );
		const std::string &altname = rom.get_info( FeRomInfo::AltRomname );
		const std::string &cloneof = rom.get_info( FeRomInfo::Cloneof );

		std::vector<std::string> romname_image_list;
		if ( gather_artwork_filenames( art_paths, romname, vid_list, romname_image_list ) )
		{
			// test for "romname" specific videos first
			if ( !image_only && !vid_list.empty() )
				return true;
		}

		bool check_altname = ( !altname.empty() && ( romname.compare( altname ) != 0 ));

		std::vector<std::string> altname_image_list;
		if ( check_altname && gather_artwork_filenames( art_paths, altname, vid_list, altname_image_list ) )
		{
			// test for "altname" specific videos second
			if ( !image_only && !vid_list.empty() )
				return true;
		}

		bool check_cloneof = ( !cloneof.empty() && (altname.compare( cloneof ) != 0 ));

		std::vector<std::string> cloneof_image_list;
		if ( check_cloneof && gather_artwork_filenames( art_paths, cloneof, vid_list, cloneof_image_list ) )
		{
			// then "cloneof" specific videos
			if ( !image_only && !vid_list.empty() )
				return true;
		}

		// now return "romname" specific images if we have them
		if ( !romname_image_list.empty() )
		{
			image_list.swap( romname_image_list );
			return true;
		}

		// next is "altname" specific images
		if ( !altname_image_list.empty() )
		{
			image_list.swap( altname_image_list );
			return true;
		}

		// then "cloneof" specific images
		if ( !cloneof_image_list.empty() )
		{
			image_list.swap( cloneof_image_list );
			return true;
		}

		// then "emulator"
		if ( !ignore_emu && !emu_name.empty()
			&& gather_artwork_filenames( art_paths,
				emu_name, vid_list, image_list ) )
			return true;
	}

	return false;
}

bool FeSettings::has_artwork( const FeRomInfo &rom, const std::string &art_name )
{
	std::vector<std::string> temp1, temp2;
	return ( get_best_artwork_file( rom, art_name, temp1, temp2, false, true ) );
}

bool FeSettings::has_video_artwork( const FeRomInfo &rom, const std::string &art_name )
{
	std::vector<std::string> vids, temp;
	get_best_artwork_file( rom, art_name, vids, temp, false, true );
	return (!vids.empty());
}

bool FeSettings::has_image_artwork( const FeRomInfo &rom, const std::string &art_name )
{
	std::vector<std::string> temp, images;
	get_best_artwork_file( rom, art_name, temp, images, true );
	return (!images.empty());
}

bool FeSettings::get_best_dynamic_image_file(
	int filter_index,
	int rom_index,
	const std::string &art_name,
	std::vector<std::string> &vid_list,
	std::vector<std::string> &image_list )
{
	std::string base = art_name;
	do_text_substitutions_absolute( base, filter_index, rom_index );

	std::string path;

	// split filename from directory paths correctly
	if ( is_relative_path( base ) )
		get_path( Current, path );

	size_t pos = base.find_last_of( "/\\" );
	if ( pos != std::string::npos )
	{
		path += base.substr( 0, pos+1 );
		base.erase( 0, pos+1 );
	}

	// strip extension from filename, if present
	pos = base.find_last_of( "." );
	if ( pos != std::string::npos )
		base.erase( pos );

	if ( base.empty() )
		return false;

	std::vector< std::string > paths;
	paths.push_back( path );

	return gather_artwork_filenames( paths, base, vid_list, image_list );
}

void FeSettings::update_romlist_after_edit(
	const FeRomInfo &original,
	const FeRomInfo &replacement,
	bool erase_it )
{
	if ( m_current_display < 0 )
		return;

	//
	// Update the in memory romlist now
	//
	FeRomInfoListType &rl = m_rl.get_list();

	for ( FeRomInfoListType::iterator it = rl.begin(); it != rl.end(); )
	{
		if ( (*it) == original )
		{
			if ( erase_it )
				it = rl.erase( it );
			else
			{
				(*it) = replacement;
				++it;
			}
		}
		else
			++it;
	}

	// RomInfo operator== compares romname and emulator
	if (!( original == replacement ))
		m_rl.mark_favs_and_tags_changed();

	m_rl.create_filters( m_displays[m_current_display] );

	std::string in_path( m_config_path );
	in_path += FE_ROMLIST_SUBDIR;

	const std::string &romlist_name = m_displays[m_current_display].get_info(FeDisplayInfo::Romlist);
	in_path += romlist_name;
	in_path += FE_ROMLIST_FILE_EXTENSION;

	std::string out_path( in_path );

	// Check for a romlist in the data path if there isn't one that matches in the
	// config directory
	//
	if (( !file_exists( in_path  ) ) && ( FE_DATA_PATH != NULL ))
	{
		std::string temp = FE_DATA_PATH;
		temp += FE_ROMLIST_SUBDIR;
		temp += romlist_name;
		temp += FE_ROMLIST_FILE_EXTENSION;

		if ( file_exists( temp ) )
			in_path = temp;
	}

	//
	// Load the romlist file into temp_list, update the changed entry, and resave now
	// We reload the file here because our in-memory romlist probably isn't complete (due
	// to global filtering)
	//
	FeRomInfoListType temp_list;

	bool found=false;

	std::ifstream infile( in_path.c_str() );
	if ( !infile.is_open() )
		return;

	while ( infile.good() )
	{
		std::string line, setting, value;

		getline( infile, line );
		if ( line_to_setting_and_value( line, setting, value, ";" ) )
		{
			FeRomInfo next_rom( setting );
			next_rom.process_setting( setting, value, "" );

			// It is possible to have multiple entries that match the original.
			// For now we just blindly update them all.
			//
			if ( next_rom == original )
			{
				if ( !erase_it )
					temp_list.push_back( replacement );

				found=true;
			}
			else
				temp_list.push_back( next_rom );
		}
	}

	infile.close();

	// If we didn't find the original, add this as a new rom at the end
	// This way if the user edits on an empty list, they can create a first entry
	//
	if ( !found )
		temp_list.push_back( replacement );

	//
	// Write out the update romlist file
	//
	std::ofstream outfile( out_path.c_str() );
	if ( outfile.is_open() )
	{
                // one line header showing what the columns represent
                //
		int i=0;
                outfile << "#" << FeRomInfo::indexStrings[i++];
                while ( i < FeRomInfo::Favourite )
                        outfile << ";" << FeRomInfo::indexStrings[i++];
                outfile << std::endl;

                // Now output the list
                //
                for ( FeRomInfoListType::const_iterator itl=temp_list.begin();
                                itl != temp_list.end(); ++itl )
                        outfile << (*itl).as_output() << std::endl;

                outfile.close();
	}
}
