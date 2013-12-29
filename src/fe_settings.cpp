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
#include <SFML/Config.hpp>

#ifndef NO_FONTCONFIG
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

const char *FE_CFG_FILE					= "attract.cfg";
const char *FE_STATE_FILE				= "attract.am";
const char *FE_SCREENSAVER_FILE		= "screensaver.nut";
const char *FE_RESOURCE_BASEFILE		= "locale$1.msg";
const char *FE_LAYOUT_FILE_EXTENSION	= ".nut";
const char *FE_ROMLIST_FILE_EXTENSION	= ".txt";
const char *FE_EMULATOR_FILE_EXTENSION	= ".cfg";
const char *FE_PLUGIN_FILE_EXTENSION	= FE_LAYOUT_FILE_EXTENSION;
const char *FE_LAYOUT_SUBDIR			= "layouts/";
const char *FE_ROMLIST_SUBDIR			= "romlists/";
const char *FE_EMULATOR_SUBDIR		= "emulators/";
const char *FE_SOUND_SUBDIR			= "sounds/";
const char *FE_PLUGIN_SUBDIR 			= "plugins/";

const std::string FE_EMPTY_STRING;

// NOTE: this has to remain aligned with the RotationState enum:
const char *FeSettings::rotationTokens[]	=
{
	"none",
	"right",
	"flip",
	"left",
	NULL
};

const char *FeSettings::rotationDispTokens[]	=
{
	"None",
	"Right",
	"Flip",
	"Left",
	NULL
};

FeSettings::FeSettings( const std::string &config_path,
				const std::string &cmdln_font, bool disable_mousecap )
	:  m_inputmap( disable_mousecap ),
	m_current_list( -1 ),
	m_autorotate( RotateNone ),
	m_current_config_object( NULL ),
	m_ssaver_time( 600 ),
	m_lists_menu_exit( true )
{
	int i=0;
	while ( FE_DEFAULT_FONT_PATHS[i] != NULL )
		m_font_paths.push_back( FE_DEFAULT_FONT_PATHS[i++] );

	if ( config_path.empty() )
		m_config_path = clean_path(FE_DEFAULT_CFG_PATH);
	else
		m_config_path = clean_path( config_path, true );

	m_default_font = cmdln_font;
}

void FeSettings::clear()
{
	m_current_config_object=NULL;
	m_current_list = -1;

	m_lists.clear();
	m_emulators.clear();
	m_plugins.clear();
}

bool FeSettings::load()
{
	clear();

	std::string filename( m_config_path );
	filename += FE_CFG_FILE;

	if ( load_from_file( filename ) == false )
	      std::cout << "Config file not found: " << filename << std::endl;

	load_state();

	// Make sure we have some keyboard mappings
	//
	m_inputmap.default_mappings();

	// If we haven't got our font yet from the config file
	// or command line then set to the default value now
	//
	if ( m_default_font.empty() )
		m_default_font = FE_DEFAULT_FONT;

	// Load locale strings - default filename is "locale.msg"
	//
	std::string test( FE_RESOURCE_BASEFILE );
	perform_substitution( test, "$1", FE_EMPTY_STRING );
	filename = m_config_path;
	filename += test;

	// Test if language specific file is available, and use it if
	// it is
	//
	std::string loc;
	try { loc = std::locale("").name(); } catch (...) {}
	if ( loc.size() > 1 )
	{
		test = FE_RESOURCE_BASEFILE;
		std::string token = "-";
		token += loc.substr( 0, 2 );
		perform_substitution( test, "$1", token );

		if ( file_exists( m_config_path + test ) )
		{
			filename = m_config_path + test;
		}
	}

	if ( loc.size() > 4 )
	{
		test = FE_RESOURCE_BASEFILE;
		std::string token = "-";
		token += loc.substr( 0, 5 );
		perform_substitution( test, "$1", token );

		if ( file_exists( m_config_path + test ) )
		{
			filename = m_config_path + test;
		}
	}

	m_resourcemap.load_from_file( filename, ";" );

	return true;
}

const char *FeSettings::configSettingStrings[] =
{
	"autorotate",
	"exit_command",
	"default_font",
	"font_path",
	"screen_saver_timeout",
	"lists_menu_exit",
	NULL
};

const char *FeSettings::otherSettingStrings[] =
{
	"list",
	"sound",
	"input_map",
	"general",
	"plugin",
	NULL
};


int FeSettings::process_setting( const std::string &setting,
					const std::string &value,
					const std::string &fn )
{
	if ( setting.compare( otherSettingStrings[0] ) == 0 ) // list
	{
		FeListInfo newList( value );
		m_lists.push_back( newList );
		m_current_config_object = &m_lists.back();
	}
	else if ( setting.compare( otherSettingStrings[1] ) == 0 ) // sound
		m_current_config_object = &m_sounds;
	else if ( setting.compare( otherSettingStrings[2] ) == 0 ) // input_map
		m_current_config_object = &m_inputmap;
	else if ( setting.compare( otherSettingStrings[3] ) == 0 ) // general
		m_current_config_object = NULL;
	else if ( setting.compare( otherSettingStrings[4] ) == 0 ) // plugin
	{
		FePlugInfo newPlug( value );
		m_plugins.push_back( newPlug );
		m_current_config_object = &m_plugins.back();
	}
	else if ( setting.compare( configSettingStrings[0] ) == 0 ) // autorotate
	{
		if ( set_info( AutoRotate, value ) == false )
		{
			invalid_setting( fn, "autorotate", value, rotationTokens, NULL, "value" );
			return 1;
		}
	}
	else if ( setting.compare( configSettingStrings[1] ) == 0 ) // exit_command
		m_exit_command = value;
	else if ( setting.compare( configSettingStrings[2] ) == 0 ) // default_font
	{
		if ( m_default_font.empty() ) // don't overwrite command line font
			m_default_font = value;
	}
	else if ( setting.compare( configSettingStrings[3] ) == 0 ) // font_path
		set_info( FontPath, value );
	else if ( setting.compare( configSettingStrings[4] ) == 0 ) // screen_saver_timeout
		set_info( ScreenSaverTimeout, value );
	else if ( setting.compare( configSettingStrings[5] ) == 0 ) // lists_menu_exit 
		set_info( ListsMenuExit, value );
	else if ( m_current_config_object != NULL )
	{
		// otherwise setting belongs to the most recently declared
		// list or emulator
		m_current_config_object->process_setting( setting, value, fn );
	}
	else
	{
		invalid_setting( fn, "general", setting, otherSettingStrings, configSettingStrings );
		return 1;
	}

	return 0;
}

void FeSettings::init_list()
{
	m_rl.clear();

	if ( m_current_list < 0 )
		return;

	FeFilter *f = m_lists[m_current_list].get_filter(
			m_lists[m_current_list].get_current_filter_index() );

	if ( f )
	{
		f->init();
		m_rl.set_filter( f );
	}

	std::string romlist = m_lists[m_current_list].get_info(FeListInfo::Romlist);

	if ( romlist.empty() )
		return;

	std::string filename( m_config_path );
	filename += FE_ROMLIST_SUBDIR;
	filename += romlist;
	filename += FE_ROMLIST_FILE_EXTENSION;

	if ( m_rl.load_from_file( filename, ";" ) == false )
		std::cout << "Error opening romlist: " << filename << std::endl;
}

void FeSettings::save_state() const
{
	std::string filename( m_config_path );
	confirm_directory( m_config_path, FE_EMPTY_STRING );

	filename += FE_STATE_FILE;

	std::ofstream outfile( filename.c_str() );
	if ( outfile.is_open() )
	{
		outfile << m_current_list << std::endl;

		for ( std::vector<FeListInfo>::const_iterator itl=m_lists.begin();
					itl != m_lists.end(); ++itl )
			outfile << (*itl).state_as_output() << std::endl;

		outfile.close();
	}
}

void FeSettings::load_state()
{
	if ( m_lists.empty() )
	{
		m_current_list = -1;
		return;
	}

	std::string filename( m_config_path );
	filename += FE_STATE_FILE;

	std::ifstream myfile( filename.c_str() );
	std::string line;

	if ( myfile.is_open() && myfile.good() )
	{
		getline( myfile, line );
		m_current_list = as_int( line );

		for ( std::vector<FeListInfo>::iterator itl=m_lists.begin();
					itl != m_lists.end(); ++itl )
		{
			if ( myfile.good() )
			{
				getline( myfile, line );
				(*itl).process_state( line );
			}
		}
	}

	// bound checking on the current list state
	if ( m_current_list >= (int)m_lists.size() )
		m_current_list = m_lists.size() - 1;
	if ( m_current_list < 0 )
		m_current_list = 0;
}

int FeSettings::get_rom_index( int offset ) const
{
	if ( m_current_list < 0 )
		return -1;

	int retval = m_lists[m_current_list].get_current_rom_index();

	// keep in bounds
	if ( retval >= m_rl.size() )
		retval = m_rl.size() - 1;

	if ( retval < 0 )
		retval = 0;

	// apply the offset
	if ( m_rl.size() > 0 )
	{
		int off = abs( offset ) % m_rl.size();
		if ( offset < 0 )
			retval -= off;
		else
			retval += off;

		if ( retval < 0 )
			retval = retval + m_rl.size();
		if ( retval >= (int)m_rl.size() )
			retval = retval - m_rl.size();
	}
	return retval;
}

void FeSettings::set_current_rom(int r)
{
	if ( m_current_list < 0 )
		return;

	m_lists[ m_current_list ].set_current_rom_index( r );
}

void FeSettings::dump() const
{
	std::cout << "*** Dump of available lists:" << std::endl;

	for ( std::vector<FeListInfo>::const_iterator itl=m_lists.begin();
			itl != m_lists.end(); ++itl )
			(*itl).dump();

	std::cout << "*** Dump of available emulators:" << std::endl;
	for ( std::vector<FeEmulatorInfo>::const_iterator ite=m_emulators.begin();
			ite != m_emulators.end(); ++ite )
			(*ite).dump();


	std::cout << "*** Dump of current rom list:" << std::endl;

	for ( int i=0; i < m_rl.size(); i++ )
		m_rl[i].dump();

	std::cout << "*** Dump of font paths: " << std::endl;
	if ( !m_font_paths.empty() )
		for ( std::vector<std::string>::const_iterator its=m_font_paths.begin();
			its != m_font_paths.end(); ++its )
			std::cout << "[" << (*its) << "]";
	std::cout << std::endl;

	std::cout << "*** Dump of current state: " << std::endl
			<< '\t' << get_current_list_title()
			<< ", " << get_rom_index()
			<< ", " << get_current_layout_file() << std::endl;

}

const std::string &FeSettings::get_current_list_title() const
{
	if ( m_current_list < 0 )
		return FE_EMPTY_STRING;

	return m_lists[m_current_list].get_info( FeListInfo::Name );
}

const std::string &FeSettings::get_rom_info( int offset, FeRomInfo::Index index ) const
{
	if ( m_rl.empty() )
		return FE_EMPTY_STRING;

	int rom = get_rom_index( offset );
	return m_rl[rom].get_info( index );
}

std::string FeSettings::get_screensaver_file() const
{
	std::string path = m_config_path;
	path += FE_LAYOUT_SUBDIR;
	path += FE_SCREENSAVER_FILE;
	return path;
}

std::string FeSettings::get_current_layout_file() const
{
	if ( m_current_list < 0 )
		return FE_EMPTY_STRING;

	std::string path = get_current_layout_dir();
	std::string file = m_lists[m_current_list].get_current_layout_file();
	if ( file.empty() )
	{
		std::vector<std::string> list;
		get_basename_from_extension( list, path, 
			std::vector<std::string>(1, FE_LAYOUT_FILE_EXTENSION) );

		if ( list.empty() )
			return FE_EMPTY_STRING;

		for ( unsigned int i=0; i< list.size(); i++ )
		{
			if ( list[i].compare( "layout" ) == 0 )
			{
				file = list[i];
				break;
			}
		}

		if ( file.empty() )
			file = list.front();
	}

	path += file;
	path += FE_LAYOUT_FILE_EXTENSION;

	return path;
}

std::string FeSettings::get_current_layout_dir() const
{
	if (( m_current_list < 0 ) 
		|| ( m_lists[ m_current_list ].get_info( FeListInfo::Layout ).empty() ))
	{
		return FE_EMPTY_STRING;
	}

	std::string layout_dir = m_config_path;
	layout_dir += FE_LAYOUT_SUBDIR;
	layout_dir += m_lists[ m_current_list ].get_info( FeListInfo::Layout );
	layout_dir += "/";

	return layout_dir;
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

bool FeSettings::set_list( int index )
{
	if ( m_current_list < 0 )
		return false;

	std::string old = get_current_layout_file();

	if ( index >= (int)m_lists.size() )
		m_current_list = 0;
	else if ( index < 0 )
		m_current_list = m_lists.size() - 1;
	else	
		m_current_list = index;

	init_list();
	return ( old.compare( get_current_layout_file() ) == 0 ) ? false : true;
}

bool FeSettings::next_list()
{
	return set_list( m_current_list + 1 );
}

bool FeSettings::prev_list()
{
	return set_list( m_current_list - 1 );
}

int FeSettings::get_current_list_index() const
{
	return m_current_list;
}

void FeSettings::set_filter( int index )
{
	if ( m_current_list < 0 )
		return;

	int filter_count = m_lists[m_current_list].get_filter_count();

	if ( index >= filter_count )
		index = 0;
	else if ( index < 0 )
		index = filter_count - 1;

	m_lists[m_current_list].set_current_filter_index( index );
	init_list();
}

int FeSettings::get_current_filter_index() const
{
	if ( m_current_list < 0 )
		return 0;

	return m_lists[m_current_list].get_current_filter_index();
}

const std::string &FeSettings::get_current_filter_name()
{
	if ( m_current_list < 0 )
		return FE_EMPTY_STRING;

	FeFilter *f = m_lists[m_current_list].get_filter(
			m_lists[m_current_list].get_current_filter_index() );

	if ( !f )
		return FE_EMPTY_STRING;

	return f->get_name();
}

void FeSettings::change_rom( int step )
{
	set_current_rom( get_rom_index( step ) );
}

void FeSettings::toggle_layout()
{
	if ( m_current_list < 0 )
		return;

	std::vector<std::string> list;
	std::string layout_file = m_lists[m_current_list].get_current_layout_file();

	get_basename_from_extension(
			list,
			get_current_layout_dir(),
			std::vector<std::string>(1,FE_LAYOUT_FILE_EXTENSION) );

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
	m_lists[ m_current_list ].set_current_layout_file( layout_file );
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
			s = m_config_path;
			s += FE_SOUND_SUBDIR;
			s += filename;
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

int FeSettings::run()
{
	std::string command, args, rom_path, rom, extension, romfilename;
	const FeEmulatorInfo *emu = get_current_emulator();

	if (( emu == NULL ) || (m_rl.empty()))
		return -1;

	rom = m_rl[ get_rom_index() ].get_info( FeRomInfo::Romname );
	rom_path = clean_path( emu->get_info( FeEmulatorInfo::Rom_path ));

	const std::vector<std::string> &exts = emu->get_extensions();

	// construct base romfilename (minus extension)
	romfilename = rom_path + rom;

	std::vector<std::string>::const_iterator itr;
	bool found=false;

	for ( itr = exts.begin(); itr != exts.end(); ++itr )
	{
		if ( file_exists( romfilename + (*itr) ) )
		{
			extension = (*itr);
			found=true;
			break;
		}
	}

	if ( !found && !exts.empty() )
		extension = exts.front();

	// we have the extension now
	romfilename += extension;

	args = emu->get_info( FeEmulatorInfo::Command );
	perform_substitution( args, "[name]", rom );
	perform_substitution( args, "[rompath]", rom_path );
	perform_substitution( args, "[romext]", extension );
	perform_substitution( args, "[romfilename]", romfilename );
	perform_substitution( args, "[emulator]",
				emu->get_info( FeEmulatorInfo::Name ) );

	command = clean_path( emu->get_info( FeEmulatorInfo::Executable ) );

	std::cout << "Running: " << command << " " << args << std::endl;
	return run_program( command, args );
}

int FeSettings::exit_command() const
{
	int r( -1 );
	if ( !m_exit_command.empty() )
		r = system( m_exit_command.c_str() );

	return r;
}

void FeSettings::get_current_display_list( std::vector<std::string> &l ) const
{
	l.clear();
	l.reserve( m_rl.size() );

	for ( int i=0; i < m_rl.size(); i++ )
		l.push_back( m_rl[i].get_info( FeRomInfo::Title ) );
}

FeEmulatorInfo *FeSettings::get_current_emulator()
{
	if ( m_rl.empty() )
		return NULL;

	std::string current_emu = m_rl[ get_rom_index() ].get_info( FeRomInfo::Emulator );

	return get_emulator( current_emu );
}

FeEmulatorInfo *FeSettings::get_emulator( const std::string & emu )
{
	if ( emu.empty() )
		return NULL;

	// Check if we already haved loaded the matching emulator object
	//
	for ( std::vector<FeEmulatorInfo>::iterator ite=m_emulators.begin();
			ite != m_emulators.end(); ++ite )
	{
		if ( emu.compare( (*ite).get_info( FeEmulatorInfo::Name ) ) == 0 )
			return &(*ite);
	}

	// Emulator not loaded yet, load it now
	//
	std::string filename = m_config_path;
	filename += FE_EMULATOR_SUBDIR;
	filename += emu;
	filename += FE_EMULATOR_FILE_EXTENSION;

	FeEmulatorInfo new_emu( emu );
	if ( new_emu.load_from_file( filename ) )
	{
		m_emulators.push_back( new_emu );
		return &(m_emulators.back());
	}

	// Could not find emulator config
	return NULL;
}

FeEmulatorInfo *FeSettings::create_emulator( const std::string &emu )
{
	// If an emulator with the given name already exists we return it
	//
	FeEmulatorInfo *tmp = get_emulator( emu );
	if ( tmp != NULL )
		return tmp;

	//
	// Fill in with default values if there is a "default" emulator
	//
	std::string filename = m_config_path;
	filename += FE_EMULATOR_SUBDIR;
	filename += "default";
	filename += FE_EMULATOR_FILE_EXTENSION;

	FeEmulatorInfo new_emu( emu );
	new_emu.load_from_file( filename );

	m_emulators.push_back( new_emu );
	return &(m_emulators.back());
}

void FeSettings::delete_emulator( const std::string & emu )
{
	//
	// Delete file
	//
	std::string path = m_config_path;
	path += FE_EMULATOR_SUBDIR;
	path += emu;
	path += FE_EMULATOR_FILE_EXTENSION;

	delete_file( path );

	//
	// Delete from our list if it has been loaded
	//
	for ( std::vector<FeEmulatorInfo>::iterator ite=m_emulators.begin();
			ite != m_emulators.end(); ++ite )
	{
		if ( emu.compare( (*ite).get_info( FeEmulatorInfo::Name ) ) == 0 )
		{
			m_emulators.erase( ite );
			break;
		}
	}
}

void FeSettings::internal_get_art_file(
		std::vector<std::string> &result,
		std::string &ap,				// artwork/movie path
		int curr_rom,				// rom index
		const std::string &artlabel,
		bool is_movie ) const
{
	std::string layout_path = get_current_layout_dir();

	std::string current_emu;
	if ( !m_rl.empty() )
		current_emu = m_rl[ curr_rom ].get_info(
				FeRomInfo::Emulator );

	if (( !ap.empty() ) && ( !m_rl.empty() ))
	{
		std::vector<std::string> test_list;
		std::string artpath = clean_path( ap, true );
		perform_substitution( artpath, "$LAYOUT", layout_path );

		// test for "romname" specific
		test_list.push_back( m_rl[curr_rom].get_info( FeRomInfo::Romname ) );

		// then "cloneof" specific
		std::string cloneof = m_rl[curr_rom].get_info( FeRomInfo::Cloneof );
		if ( !cloneof.empty() )
			test_list.push_back( cloneof );

		// then "emulator"
		test_list.push_back( current_emu );

		get_filename_from_base( result, artpath, test_list,
								FE_ART_EXTENSIONS, is_movie );
	}

	if ( !layout_path.empty() )
	{
		std::vector<std::string> test_list;

		if ( !current_emu.empty() )
		{
			// check for "[emulator]-[artlabel]" image in layout directory
			std::string test = current_emu;
			test += "-";
			test += artlabel;
			test_list.push_back( test );
		}

		// check for file named with the artwork label in layout directory
		test_list.push_back( artlabel );

		get_filename_from_base( result, layout_path, test_list,
								FE_ART_EXTENSIONS, is_movie );
	}
}

void FeSettings::get_art_file( int offset,
					const std::string &artlabel,
					std::vector<std::string> &filelist )
{
	std::string artpath;
	FeEmulatorInfo *cur_emu_info = get_current_emulator();
	if ( cur_emu_info != NULL )
		cur_emu_info->get_artwork( artlabel, artpath );

	internal_get_art_file( filelist, artpath, get_rom_index( offset ),
							artlabel, false );
}

void FeSettings::get_movie_file( int offset, std::vector<std::string> &filename_list )
{
	std::string mpath;
	std::string artlabel;
	FeEmulatorInfo *cur_emu_info = get_current_emulator();

	if ( cur_emu_info != NULL )
	{
		artlabel = cur_emu_info->get_info( FeEmulatorInfo::Movie_artwork );
		mpath = cur_emu_info->get_info( FeEmulatorInfo::Movie_path );

		internal_get_art_file( filename_list, mpath,
							get_rom_index( offset ),
							artlabel, true );
	}
}

const std::string &FeSettings::get_movie_artwork()
{
	FeEmulatorInfo *cur_emu_info = get_current_emulator();
	if ( cur_emu_info != NULL )
		return cur_emu_info->get_info( FeEmulatorInfo::Movie_artwork );

	return FE_EMPTY_STRING;
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

#ifndef NO_FONTCONFIG
	bool fc_found = false;
	FcConfig *config = FcInitLoadConfigAndFonts();

	FcPattern *pat = FcNameParse( (const FcChar8 *)(fontname.c_str()) );
	FcConfigSubstitute( config, pat, FcMatchPattern );
	FcDefaultSubstitute( pat );

	FcPattern *font = FcFontMatch( config, pat, NULL );
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
	FcConfigDestroy( config );

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
		std::cout << "Could not find font: " << fontname
					<< ", trying default_font" << std::endl;
		return get_font_file( fontpath, m_default_font );
	}

	// should only get here if the default font is not found
	std::cout << "Could not find default font \""
					<< fontname << "\"" << std::endl;
	return false;
}

FeSettings::RotationState FeSettings::get_autorotate() const
{
	return m_autorotate;
}

int FeSettings::get_screen_saver_timeout() const
{
	return m_ssaver_time;
}

bool FeSettings::get_lists_menu_exit() const
{
	return m_lists_menu_exit;
}

void FeSettings::get_list_names( std::vector<std::string> &list ) const
{
	list.clear();
	list.reserve( m_lists.size() );

	for ( std::vector<FeListInfo>::const_iterator itr=m_lists.begin();
			itr < m_lists.end(); ++itr )
		list.push_back( (*itr).get_info( FeListInfo::Name ) );
}

const std::string FeSettings::get_info( int index ) const
{
	switch ( index )
	{
	case ExitCommand:
		return m_exit_command;
	case AutoRotate:
		return rotationTokens[ m_autorotate ];
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
	case ListsMenuExit:
		return ( m_lists_menu_exit ? "yes" : "no" );
	default:
		break;
	}
	return FE_EMPTY_STRING;
}

bool FeSettings::set_info( int index, const std::string &value )
{
	switch ( index )
	{
	case ExitCommand:
		m_exit_command = value;
		return true;

	case AutoRotate:
		{
			int i=0;
			while ( rotationTokens[i] != NULL )
			{
				if ( value.compare( rotationTokens[i] ) == 0 )
				{
					m_autorotate = (RotationState)i;
					break;
				}
				i++;
			}

			if ( rotationTokens[i] != NULL )
				return true;
		}
		break;

	case DefaultFont:
		m_default_font = value;
		return true;

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
		return true;

	case ScreenSaverTimeout:
		m_ssaver_time = as_int( value );
		break;

	case ListsMenuExit:
		if (( value.compare( "yes" ) == 0 ) 
				|| ( value.compare( "true" ) == 0 ))
			m_lists_menu_exit = true;
		else
			m_lists_menu_exit = false;
		break;

	default:
		break;
	}

	return false;
}


FeListInfo *FeSettings::get_list( const std::string &n )
{
	for ( std::vector<FeListInfo>::iterator itr=m_lists.begin();
			itr < m_lists.end(); ++itr )
	{
		if ( n.compare( (*itr).get_info( FeListInfo::Name ) ) == 0 )
			return &(*itr);
	}
	return NULL;
}

FeListInfo *FeSettings::create_list( const std::string &n )
{
	if ( m_current_list == -1 )
		m_current_list=0;

	FeListInfo new_list( n );

	if ( !m_lists.empty() )
	{
		// If other lists are configured, give the new list the same layout as
		// the last configured list
		//
		new_list.set_info( FeListInfo::Layout, 
			m_lists.back().get_info( FeListInfo::Layout ) );
	}
	else
	{
		// Pick an available layout, use the first one alphabetically
		//
		std::vector<std::string> layouts;
		get_subdirectories( layouts, m_config_path + FE_LAYOUT_SUBDIR );
		if ( !layouts.empty() )
		{
			std::sort( layouts.begin(), layouts.end() );
			new_list.set_info( FeListInfo::Layout, layouts.front() );
		}
	}

	m_lists.push_back( new_list );
	return &(m_lists.back());
}

void FeSettings::delete_list( const std::string &n )
{
	int i=0;
	for ( std::vector<FeListInfo>::iterator itr=m_lists.begin();
			itr < m_lists.end(); ++itr )
	{
		if ( n.compare( (*itr).get_info( FeListInfo::Name ) ) == 0 )
		{
			m_lists.erase( itr );

			if ( m_current_list >= i )
				m_current_list--;

			if ( m_current_list < 0 )
				m_current_list=0;

			return;
		}
		i++;
	}
}

void FeSettings::get_current_list_filter_names( 
		std::vector<std::string> &list ) const
{
	if ( m_current_list < 0 )
		return;

	m_lists[ m_current_list ].get_filters_list( list );
}

bool FeSettings::check_romlist_configured( const std::string &n ) const
{
	for ( std::vector<FeListInfo>::const_iterator itr=m_lists.begin();
			itr!=m_lists.end(); ++itr )
	{
		if ( n.compare( (*itr).get_info( FeListInfo::Romlist ) ) == 0 )
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

		for ( std::vector<FeListInfo>::const_iterator it=m_lists.begin();
						it != m_lists.end(); ++it )
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

		outfile << std::endl;
		std::vector< FePlugInfo >::const_iterator itr;
		for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
		{
			//
			// Get rid of configs for old plugins by not saving it if the 
			// plugin itself is gone
			//
			if ( file_exists( m_config_path + FE_PLUGIN_SUBDIR 
					+ (*itr).get_name() + FE_PLUGIN_FILE_EXTENSION ) )
			{
				(*itr).save( outfile );
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

int FeSettings::lists_count() const
{
	return m_lists.size();
}

const std::string &FeSettings::get_plugin_command( 
			const std::string &label ) const
{
	std::vector< FePlugInfo >::const_iterator itr;

	for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
	{
		if ( label.compare( (*itr).get_name() ) == 0 )
			return (*itr).get_command();
	}

	return FE_EMPTY_STRING;
}

void FeSettings::set_plugin_command( 
			const std::string &label, const std::string &command )
{
	std::vector< FePlugInfo >::iterator itr;

	for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
	{
		if ( label.compare( (*itr).get_name() ) == 0 )
		{
			(*itr).set_command( command );
			return;
		}
	}

	// If there isn't a plugin configuration, create it now
	//
	m_plugins.push_back( FePlugInfo( label ) );
	m_plugins.back().set_command( command );
}

bool FeSettings::get_plugin_enabled( const std::string &label ) const
{
	std::vector< FePlugInfo >::const_iterator itr;

	for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
	{
		if ( label.compare( (*itr).get_name() ) == 0 )
			return (*itr).get_enabled();
	}

	return false;
}

void FeSettings::set_plugin_enabled( 
			const std::string &label, bool e )
{
	std::vector< FePlugInfo >::iterator itr;

	for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
	{
		if ( label.compare( (*itr).get_name() ) == 0 )
		{
			(*itr).set_enabled( e );
			return;
		}
	}

	// If there isn't a plugin configuration, create it now
	//
	m_plugins.push_back( FePlugInfo( label ) );
	m_plugins.back().set_enabled( e );
}

bool FeSettings::get_plugin_param( const std::string &plugin, 
		const std::string &param, std::string &v ) const
{
	std::vector< FePlugInfo >::const_iterator itr;
	for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
	{
		if ( plugin.compare( (*itr).get_name() ) == 0 )
			return (*itr).get_param( param, v );
	}
	return false;
}

void FeSettings::set_plugin_param( const std::string &plugin, 
		const std::string &param, const std::string &v )
{
	std::vector< FePlugInfo >::iterator itr;
	for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
	{
		if ( plugin.compare( (*itr).get_name() ) == 0 )
		{
			(*itr).set_param( param, v );
			return;
		}
	}
}

void FeSettings::get_plugin_param_labels( const std::string &plugin,
		std::vector<std::string> &labels ) const
{
	std::vector< FePlugInfo >::const_iterator itr;
	for ( itr = m_plugins.begin(); itr != m_plugins.end(); ++itr )
	{
		if ( plugin.compare( (*itr).get_name() ) == 0 )
		{
			(*itr).get_param_labels( labels );
			return;
		}
	}
}

void FeSettings::get_plugins( std::vector < std::string > &list ) const
{
	list.clear();
	std::string path = m_config_path + FE_PLUGIN_SUBDIR;

	get_basename_from_extension( list, path, 
			std::vector<std::string>(1, FE_PLUGIN_FILE_EXTENSION) );
}
