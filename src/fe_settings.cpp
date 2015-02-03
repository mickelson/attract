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
#include "fe_util.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include <stdlib.h>

#include <SFML/System/Clock.hpp>
#include <SFML/Config.hpp>

#ifdef USE_FONTCONFIG
#include <fontconfig/fontconfig.h>
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
const char *FE_LAYOUT_FILE_BASE		= "layout";
const char *FE_LAYOUT_FILE_EXTENSION	= ".nut";
const char *FE_LANGUAGE_FILE_EXTENSION = ".msg";
const char *FE_PLUGIN_FILE_EXTENSION	= FE_LAYOUT_FILE_EXTENSION;
const char *FE_LAYOUT_SUBDIR			= "layouts/";
const char *FE_ROMLIST_SUBDIR			= "romlists/";
const char *FE_SOUND_SUBDIR			= "sounds/";
const char *FE_PLUGIN_SUBDIR 			= "plugins/";
const char *FE_LANGUAGE_SUBDIR		= "language/";
const char *FE_MODULES_SUBDIR			= "modules/";
const char *FE_STATS_SUBDIR			= "stats/";
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

const char *FeSettings::windowModeTokens[] =
{
	"default",
	"fullscreen",
	"window",
	NULL
};

const char *FeSettings::windowModeDispTokens[] =
{
	"Fill Screen (Default)",
	"Fullscreen Mode",
	"Window",
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

FeSettings::FeSettings( const std::string &config_path,
				const std::string &cmdln_font )
	:  m_rl( config_path ),
	m_inputmap(),
	m_current_display( -1 ),
	m_current_config_object( NULL ),
	m_ssaver_time( 600 ),
	m_last_launch_display( 0 ),
	m_last_launch_filter( 0 ),
	m_last_launch_rom( 0 ),
	m_joy_thresh( 75 ),
	m_mouse_thresh( 10 ),
	m_displays_menu_exit( true ),
	m_hide_brackets( false ),
	m_autolaunch_last_game( false ),
	m_confirm_favs( false ),
	m_track_usage( true ),
	m_window_mode( Default ),
	m_filter_wrap_mode( WrapWithinDisplay ),
	m_accel_selection( true ),
	m_selection_speed( 40 )
{
	int i=0;
	while ( FE_DEFAULT_FONT_PATHS[i] != NULL )
		m_font_paths.push_back( FE_DEFAULT_FONT_PATHS[i++] );

	if ( config_path.empty() )
		m_config_path = absolute_path( clean_path(FE_DEFAULT_CFG_PATH) );
	else
		m_config_path = absolute_path( clean_path( config_path, true ) );

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
	m_inputmap.default_mappings();

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
	"autolaunch_last_game",
	"confirm_favourites",
	"mouse_threshold",
	"joystick_threshold",
	"window_mode",
	"filter_wrap_mode",
	"track_usage",
	"accelerate_selection",
	"selection_speed_ms",
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
		FeLayoutInfo new_entry( value );
		m_layout_params.push_back( new_entry );
		m_current_config_object = &m_layout_params.back();
	}
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

			invalid_setting( fn,
				"general", setting, otherSettingStrings, configSettingStrings );
			return 1;
		}
	}

	return 0;
}

void FeSettings::init_display()
{
	if ( m_current_display < 0 )
	{
		m_rl.init_as_empty_list();
		return;
	}

	const std::string &romlist_name = m_displays[m_current_display].get_info(FeDisplayInfo::Romlist);
	if ( romlist_name.empty() )
	{
		m_rl.init_as_empty_list();
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
}

void FeSettings::save_state()
{
	m_rl.save_state();

	std::string filename( m_config_path );
	confirm_directory( m_config_path, FE_EMPTY_STRING );

	filename += FE_STATE_FILE;

	std::ofstream outfile( filename.c_str() );
	if ( outfile.is_open() )
	{
		outfile << m_current_display << ";"
			<< m_last_launch_display << "," << m_last_launch_filter
			<< "," << m_last_launch_rom << std::endl;

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

		int i=0;
		while (( pos < line.size() ) && ( i < 3 ))
		{
			token_helper( line, pos, tok, "," );
			int temp = as_int( tok );

			switch (i)
			{
				case 0: m_last_launch_display = temp; break;
				case 1: m_last_launch_filter = temp; break;
				case 2: m_last_launch_rom = temp; break;
			}
			i++;
		}

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

bool FeSettings::config_map_input( const sf::Event &e, std::string &s, FeInputMap::Command &conflict )
{
	FeInputSource index( e, m_mousecap_rect, m_joy_thresh );
	if ( index.get_type() == FeInputSource::Unsupported )
		return false;

	s = index.as_string();

	conflict = map_input( e );
	return true;
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
		return 0;

	return m_displays[m_current_display].get_filter_count();
}

int FeSettings::get_filter_size( int filter_index ) const
{
	return m_rl.filter_size( filter_index );
}

int FeSettings::get_rom_index( int filter_index, int offset ) const
{
	if ( m_current_display < 0 )
		return -1;

	int retval = m_displays[m_current_display].get_rom_index( filter_index );

	// keep in bounds
	int rl_size = m_rl.filter_size( filter_index );
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

const std::string &FeSettings::get_current_display_title() const
{
	if ( m_current_display < 0 )
		return FE_EMPTY_STRING;

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
	if ( m_rl.is_filter_empty( filter_index ) )
		return FE_EMPTY_STRING;

	// Make sure we have file availability information if user is requesting it.
	if ( index == FeRomInfo::FileIsAvailable )
		m_rl.get_file_availability();

	return m_rl.lookup( filter_index, rom_index ).get_info( index );
}

void FeSettings::get_screensaver_file( std::string &path, std::string &file ) const
{
	std::string temp;
	if ( internal_resolve_config_file( m_config_path, temp, FE_LAYOUT_SUBDIR, FE_SCREENSAVER_FILE ) )
	{
		size_t len = temp.find_last_of( "/\\" );
		ASSERT( len != std::string::npos );

		path = temp.substr( 0, len + 1 );
		file = FE_SCREENSAVER_FILE;
	}
	else
	{
		std::cerr << "Error loading screensaver: " << FE_SCREENSAVER_FILE << std::endl;
	}
}

std::string FeSettings::get_current_layout_file() const
{
	if ( m_current_display < 0 )
		return FE_EMPTY_STRING;

	std::string file = m_displays[m_current_display].get_current_layout_file();
	if ( file.empty() )
	{
		std::vector<std::string> my_list;
		get_basename_from_extension( my_list, get_current_layout_dir(), FE_LAYOUT_FILE_EXTENSION );

		if ( my_list.empty() )
			return FE_EMPTY_STRING;

		for ( std::vector<std::string>::iterator itr=my_list.begin(); itr!=my_list.end(); ++itr )
		{
			if ( (*itr).compare( FE_LAYOUT_FILE_BASE ) == 0 )
			{
				file = (*itr);
				break;
			}
		}

		if ( file.empty() )
			file = my_list.front();
	}

	file += FE_LAYOUT_FILE_EXTENSION;
	return file;
}

std::string FeSettings::get_current_layout_dir() const
{
	if (( m_current_display < 0 )
		|| ( m_displays[ m_current_display ].get_info( FeDisplayInfo::Layout ).empty() ))
	{
		return FE_EMPTY_STRING;
	}

	return get_layout_dir( m_displays[ m_current_display ].get_info( FeDisplayInfo::Layout ));
}

std::string FeSettings::get_layout_dir( const std::string &layout_name ) const
{
	std::string temp;
	internal_resolve_config_file( m_config_path, temp, FE_LAYOUT_SUBDIR, layout_name + "/" );
	return temp;
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

FeLayoutInfo &FeSettings::get_current_layout_config()
{
	if ( m_current_display < 0 )
	{
		ASSERT( 0 ); // This should not happen
		return get_layout_config( "" );
	}

	return get_layout_config( m_displays[ m_current_display ].get_info( FeDisplayInfo::Layout ) );
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
	std::string old = get_current_layout_dir() + get_current_layout_file();

	if ( index >= (int)m_displays.size() )
		m_current_display = 0;
	else if ( index < 0 )
		m_current_display = m_displays.size() - 1;
	else
		m_current_display = index;

	m_rl.save_state();
	init_display();

	return ( old.compare( get_current_layout_dir() + get_current_layout_file() ) != 0 );
}

// return true if layout needs to be reloaded as a result
bool FeSettings::navigate_display( int step, bool wrap_mode )
{
	bool retval = set_display( m_current_display + step );

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

	set_current_selection( new_filter, -1 );
	return false;
}

int FeSettings::get_current_display_index() const
{
	return m_current_display;
}

// if rom_index < 0, then the rom index is left unchanged and only the filter index is changed
void FeSettings::set_current_selection( int filter_index, int rom_index )
{
	if ( m_current_display < 0 )
		return;

	m_displays[m_current_display].set_current_filter_index( filter_index );

	if ( rom_index >= 0 )
		m_displays[m_current_display].set_rom_index( filter_index, rom_index );
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
	if ( m_current_display != m_last_launch_display )
	{
		set_display( m_last_launch_display );
		retval = true;
	}

	set_current_selection( m_last_launch_filter, m_last_launch_rom );
	return retval;
}

bool FeSettings::get_current_fav() const
{
	int filter_index = get_current_filter_index();

	if ( m_rl.is_filter_empty( filter_index ) )
		return false;

	const std::string &s = m_rl.lookup( filter_index, get_rom_index( filter_index, 0 ) ).get_info(FeRomInfo::Favourite);
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

	return m_rl.set_fav(
			filter_index,
			get_rom_index( filter_index, 0 ),
			m_displays[m_current_display],
			status );
}

int FeSettings::get_prev_fav_offset() const
{
	int filter_index = get_current_filter_index();
	int idx = get_rom_index( filter_index, 0 );

	for ( int i=1; i < m_rl.filter_size( filter_index ); i++ )
	{
		int t_idx = ( i <= idx ) ? ( idx - i ) : ( m_rl.filter_size( filter_index ) - ( i - idx ) );
		if ( m_rl.lookup( filter_index, t_idx ).get_info(FeRomInfo::Favourite).compare("1")==0 )
			return ( t_idx - idx );
	}

	return 0;
}

int FeSettings::get_next_fav_offset() const
{
	int filter_index = get_current_filter_index();
	int idx = get_rom_index( filter_index, 0 );

	for ( int i=1; i < m_rl.filter_size( filter_index ); i++ )
	{
		int t_idx = ( idx + i ) % m_rl.filter_size( filter_index );
		if ( m_rl.lookup( filter_index, t_idx ).get_info(FeRomInfo::Favourite).compare("1")==0 )
			return ( t_idx - idx );
	}

	return 0;
}

int FeSettings::get_next_letter_offset( int step ) const
{
	int filter_index = get_current_filter_index();

	if ( m_rl.is_filter_empty( filter_index ) )
		return 0;

	FeRomListSorter s;

	int idx = get_rom_index( filter_index, 0 );
	const char curr_l = s.get_first_letter( m_rl.lookup( filter_index, idx ) );
	bool is_alpha = std::isalpha( curr_l );
	int retval = 0;

	for ( int i=1; i < m_rl.filter_size( filter_index ); i++ )
	{
		int t_idx;
		if ( step > 0 )
			t_idx = ( idx + i ) % m_rl.filter_size( filter_index );
		else
			t_idx = ( i <= idx ) ? ( idx - i ) : ( m_rl.filter_size( filter_index ) - ( i - idx ) );

		const char test_l = s.get_first_letter( m_rl.lookup( filter_index, t_idx ) );

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
	std::vector< std::pair<std::string, bool> > &tags_list ) const
{
	int filter_index = get_current_filter_index();
	m_rl.get_tags_list( filter_index, get_rom_index( filter_index, 0 ), tags_list );
}

bool FeSettings::set_current_tag(
		const std::string &tag, bool flag )
{
	if ( m_current_display < 0 )
		return false;

	int filter_index = get_current_filter_index();

	return m_rl.set_tag(
			filter_index,
			get_rom_index( filter_index, 0 ),
			m_displays[m_current_display],
			tag,
			flag );
}

void FeSettings::toggle_layout()
{
	if ( m_current_display < 0 )
		return;

	std::vector<std::string> list;
	std::string layout_file = m_displays[m_current_display].get_current_layout_file();

	get_basename_from_extension(
			list,
			get_current_layout_dir(),
			FE_LAYOUT_FILE_EXTENSION );

	int test_len = strlen( FE_LAYOUT_FILE_BASE );
	for ( std::vector< std::string >::iterator itr=list.begin(); itr != list.end(); )
	{
		if ( (*itr).compare( 0, test_len, FE_LAYOUT_FILE_BASE ) != 0 )
			itr = list.erase( itr );
		else
			++itr;
	}

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

void FeSettings::run( int &minimum_run_seconds )
{
	int filter_index = get_current_filter_index();

	if ( m_rl.is_filter_empty( filter_index ) )
		return;

	int rom_index = get_rom_index( filter_index, 0 );
	FeRomInfo &rom = m_rl.lookup( filter_index, rom_index );

	const FeEmulatorInfo *emu = get_emulator( rom.get_info( FeRomInfo::Emulator ) );
	if ( emu == NULL )
		return;

	const std::string &rom_name = rom.get_info( FeRomInfo::Romname );
	minimum_run_seconds = as_int( emu->get_info( FeEmulatorInfo::Minimum_run_time ) );

	m_last_launch_display = get_current_display_index();
	m_last_launch_filter = filter_index;
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
		std::string path = clean_path( *itr, true );

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

	args = emu->get_info( FeEmulatorInfo::Command );
	perform_substitution( args, "[name]", rom_name );
	perform_substitution( args, "[rompath]", rom_path );
	perform_substitution( args, "[romext]", extension );
	perform_substitution( args, "[romfilename]", romfilename );
	perform_substitution( args, "[emulator]",
				emu->get_info( FeEmulatorInfo::Name ) );

	command = clean_path( emu->get_info( FeEmulatorInfo::Executable ) );

	std::cout << "*** Running: " << command << " " << args << std::endl;

	sf::Clock play_timer;
	run_program(
				command,
				args,
				NULL,
				NULL,
				true,
				emu->get_info( FeEmulatorInfo::Exit_hotkey ),
				m_joy_thresh );

	if ( m_track_usage )
	{
		const std::string rl_name = m_displays[m_current_display].get_info( FeDisplayInfo::Romlist );
		std::string path = m_config_path + FE_STATS_SUBDIR;
		confirm_directory( path, rl_name );

		path += rl_name + "/";
		rom.update_stats( path, 1, play_timer.getElapsedTime().asSeconds() );
	}
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

	for ( int i=0; ((i< FeRomInfo::LAST_INDEX) && ( n > 0 )); i++ )
	{
		if (( i == FeRomInfo::Title ) // these are special cases dealt with below
				|| ( i == FeRomInfo::PlayedTime ))
			continue;

		std::string from = "[";
		from += FeRomInfo::indexStrings[i];
		from += "]";

		n -= perform_substitution( str, from,
				get_rom_info_absolute( filter_index, rom_index, (FeRomInfo::Index)i) );
	}

	if ( n > 0 )
	{
		n -= perform_substitution( str, "[DisplayName]",
				get_current_display_title() );

		n -= perform_substitution( str, "[FilterName]",
				get_filter_name( filter_index ) );

		n -= perform_substitution( str, "[ListSize]",
				as_str( get_filter_size( filter_index ) ) );

		n -= perform_substitution( str, "[ListEntry]",
				as_str( rom_index + 1 ) );


		// [ListTitle] deprecated as of 1.5
		n -= perform_substitution( str, "[ListTitle]",
				get_current_display_title() );

		// [ListFilterName] deprecated as of 1.5
		n -= perform_substitution( str, "[ListFilterName]",
				get_filter_name( filter_index ) );
}

	if ( n > 0 )
	{
		const std::string &title_full =
				get_rom_info_absolute( filter_index, rom_index, FeRomInfo::Title );

		n -= perform_substitution( str, "[TitleFull]", title_full );

		if ( hide_brackets() )
			n -= perform_substitution( str, "[Title]", name_with_brackets_stripped( title_full ) );
		else
			n -= perform_substitution( str, "[Title]", title_full );
	}

	std::string played_string;
	if ( n > 0 )
	{
		std::string label;

		int raw = as_int(
			get_rom_info_absolute( filter_index, rom_index, FeRomInfo::PlayedTime ) );
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

		played_string = as_str( num, 1 ) + " " + op_label;

		n -= perform_substitution( str, "[PlayedTime]", played_string );
	}

	if ( n > 0 )
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

		n -= perform_substitution( str, "[SortName]", sort_name );

		if ( sort_by == FeRomInfo::PlayedTime )
			n -= perform_substitution( str, "[SortValue]", played_string );
		else
			n -= perform_substitution( str, "[SortValue]",
					get_rom_info_absolute( filter_index, rom_index, sort_by ) );
	}
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

bool FeSettings::get_font_file( std::string &fontpath,
				const std::string &fontname ) const
{
	if ( fontname.empty() )
	{
		if ( m_default_font.empty() )
			return false;
		else
			return get_font_file( fontpath, m_default_font );
	}

	//
	// First check if there is a matching font file in the
	// layout directory
	//
	std::string test;
	std::string layout_dir = get_current_layout_dir();
	if ( !layout_dir.empty() && search_for_file( layout_dir,
				fontname, FE_FONT_EXTENSIONS, test ) )
	{
		fontpath = test;
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
					fontpath = (char *)file;
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
			fontpath = test;
			return true;
		}
	}

	// fall back to default font
	if ( m_default_font.compare( fontname ) != 0 )
	{
		std::cerr << "Could not find font: " << fontname
					<< ", trying default_font" << std::endl;
		return get_font_file( fontpath, m_default_font );
	}

	// should only get here if the default font is not found
	std::cerr << "Could not find default font \""
					<< fontname << "\"" << std::endl;
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

int FeSettings::get_screen_saver_timeout() const
{
	return m_ssaver_time;
}

bool FeSettings::get_displays_menu_exit() const
{
	return m_displays_menu_exit;
}

void FeSettings::get_display_names( std::vector<std::string> &list ) const
{
	list.clear();
	list.reserve( m_displays.size() );

	for ( std::vector<FeDisplayInfo>::const_iterator itr=m_displays.begin();
			itr < m_displays.end(); ++itr )
		list.push_back( (*itr).get_info( FeDisplayInfo::Name ) );
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
	case DisplaysMenuExit:
		return ( m_displays_menu_exit ? FE_CFG_YES_STR : FE_CFG_NO_STR );
	case HideBrackets:
		return ( m_hide_brackets ? FE_CFG_YES_STR : FE_CFG_NO_STR );
	case AutoLaunchLastGame:
		return ( m_autolaunch_last_game ? FE_CFG_YES_STR : FE_CFG_NO_STR );
	case ConfirmFavourites:
		return ( m_confirm_favs ? FE_CFG_YES_STR : FE_CFG_NO_STR );
	case MouseThreshold:
		return as_str( m_mouse_thresh );
	case JoystickThreshold:
		return as_str( m_joy_thresh );
	case WindowMode:
		return windowModeTokens[ m_window_mode ];
	case FilterWrapMode:
		return filterWrapTokens[ m_filter_wrap_mode ];
	case TrackUsage:
		return ( m_track_usage ? FE_CFG_YES_STR : FE_CFG_NO_STR );
	case AccelerateSelection:
		return ( m_accel_selection ? FE_CFG_YES_STR : FE_CFG_NO_STR );
	case SelectionSpeed:
		return as_str( m_selection_speed );
	default:
		break;
	}
	return FE_EMPTY_STRING;
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

	case AutoLaunchLastGame:
		m_autolaunch_last_game = config_str_to_bool( value );
		break;

	case ConfirmFavourites:
		m_confirm_favs = config_str_to_bool( value );
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

	case AccelerateSelection:
		m_accel_selection = config_str_to_bool( value );
		break;

	case SelectionSpeed:
		m_selection_speed = as_int( value );
		if ( m_selection_speed < 0 )
			m_selection_speed = 0;
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
	get_subdirectories( layouts, m_config_path + FE_LAYOUT_SUBDIR );

	if ( FE_DATA_PATH != NULL )
	{
		std::string t = FE_DATA_PATH;
		t += FE_LAYOUT_SUBDIR;
		get_subdirectories( layouts, t );
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
	confirm_directory( m_config_path, FE_SOUND_SUBDIR );
	confirm_directory( m_config_path, FE_PLUGIN_SUBDIR );
	// no FE_LANGUAGE_SUBDIR

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

	if ( !ll.empty() )
	{
		// Sort the list and remove duplicates
		std::sort( ll.begin(), ll.end() );
		ll.erase( std::unique( ll.begin(), ll.end() ), ll.end() );
	}
}

FePlugInfo *FeSettings::get_plugin( const std::string &label )
{
	std::vector< FePlugInfo >::iterator itr;
	for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
	{
		if ( label.compare( (*itr).get_name() ) == 0 )
			return &(*itr);
	}

	// No config for this plugin currently.  Add one
	//
	m_plugins.push_back( FePlugInfo( label ) );
	return &(m_plugins.back());
}

void FeSettings::get_plugin_full_path(
				const std::string &label,
				std::string &path,
				std::string &filename ) const
{
	std::string temp;

	//
	// There are two valid locations for plugins:
	//
	// <config_dir>/plugins/<name>/plugin.nut
	// <config_dir>/plugins/<name>.nut
	//
	if ( internal_resolve_config_file( m_config_path, temp, FE_PLUGIN_SUBDIR, label + "/" ) )
	{
		path.swap( temp );
		filename = FE_PLUGIN_FILE;
		return;
	}

	if ( internal_resolve_config_file( m_config_path, temp, FE_PLUGIN_SUBDIR, label + FE_PLUGIN_FILE_EXTENSION ) )
	{
		size_t len = temp.find_last_of( "/\\" );
		ASSERT( len != std::string::npos );

		path = temp.substr( 0, len + 1 );
		filename = label + FE_PLUGIN_FILE_EXTENSION;
		return;
	}

	std::cerr << "Plugin file not found: " << label << std::endl;
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

void FeSettings::get_languages_list( std::vector < std::string > &ll ) const
{
	ll.clear();
	internal_gather_config_files(
		ll,
		FE_LANGUAGE_FILE_EXTENSION,
		FE_LANGUAGE_SUBDIR );

	if ( ll.empty() )
		ll.push_back( "en" );
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

std::string FeSettings::get_module_dir( const std::string &module_file ) const
{
	std::string temp;
	internal_resolve_config_file( m_config_path, temp, FE_MODULES_SUBDIR, module_file );
	return temp;
}
