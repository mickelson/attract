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

#ifndef FE_SETTINGS_HPP
#define FE_SETTINGS_HPP

#include "fe_base.hpp"
#include "fe_info.hpp"
#include "fe_input.hpp"
#include <deque>

extern const char *FE_ROMLIST_SUBDIR;
extern const char *FE_EMULATOR_SUBDIR;
extern const char *FE_LAYOUT_SUBDIR;
extern const char *FE_SOUND_SUBDIR;

extern const char *FE_ROMLIST_FILE_EXTENSION;
extern const char *FE_EMULATOR_FILE_EXTENSION;

class FeSettings : public FeFileConfigurable
{
public:
   enum RotationState { RotateNone=0, RotateRight, RotateFlip, RotateLeft };
	static const char *rotationTokens[];
	static const char *rotationDispTokens[];

	enum ConfigSettingIndex { AutoRotate=0, ExitCommand, DefaultFont, FontPath, ScreenSaverTimeout, LAST_INDEX };
	static const char *configSettingStrings[];

private:
	std::string m_config_path;
	std::string m_default_font;
	std::string m_exit_command;

	std::vector<std::string> m_font_paths;
	std::deque<FeListInfo> m_lists;
	std::deque<FeEmulatorInfo> m_emulators;
	FeRomList m_rl;

	FeInputMap m_inputmap;
	FeSoundInfo m_sounds;
	FeResourceMap m_resourcemap;

	int m_current_list;
	RotationState m_autorotate;
	FeBaseConfigurable *m_current_config_object;
	int m_ssaver_time;

	int process_setting( const std::string &, 
						const std::string &,
						const std::string & );

	void load_state();
	void clear();

	void set_current_rom(int );

	void internal_get_art_file( std::vector<std::string> &, 
							std::string &, int, const std::string &, bool  );

public:
	FeSettings( const std::string &config_dir, 
				const std::string &cmdln_font, bool disable_mousecap );

	bool load();
	void save_state( void );

	FeInputMap::Command map( sf::Event );

	//
	// config_map_input used in configuration mode.
	//
   void init_config_map_input();
   bool config_map_input( sf::Event, std::string &, 
					FeInputMap::Command &conflict );
  
	void set_volume( FeSoundInfo::SoundType, const std::string & );
	int get_set_volume( FeSoundInfo::SoundType );
	int get_play_volume( FeSoundInfo::SoundType );
	bool get_mute();
	void set_mute( bool );
	bool get_sound_file( FeInputMap::Command, std::string &s, bool full_path=true );
	void set_sound_file( FeInputMap::Command, const std::string &s );

	void change_rom( int step, bool wrap=true );

	// Switches to the next configured list
	// returns true if the list change results in a new layout, false otherwise
	//
	bool next_list();

	// Switches to the previously configured list
	// returns true if the list change results in a new layout, false otherwise
	//
	bool prev_list();

	void init_list();

	int run(); // run current selection
	int exit_command(); // run configured exit command (if any)

	void toggle_layout();

	int get_current_display_list( std::vector<std::string> &list );
	int get_rom_index( int offset=0, bool wrap=true ) const;

	std::string get_current_list_title() const;
	std::string get_rom_info( int offset, FeRomInfo::Index index ) const;

	//
	// Returns filenames to use for artwork. prefer the files earlier in the list
	//
	void get_art_file( int offset, const std::string &artwork, std::vector<std::string> &filenames );
	void get_movie_file( int offset, std::vector<std::string> &filenames );

	std::string get_movie_artwork();
	bool confirm_artwork( const std::string &artlabel );

	std::string get_current_layout_file();
	std::string get_current_layout_dir();

	std::string get_config_dir();
	bool get_font_file( std::string &fullpath, const std::string &filename="" );
	
	RotationState get_autorotate();
	int get_screen_saver_timeout();

	void dump( void );

	//
	// This function implements the command-line romlist generation
	// A non-existing filename is chosen for the resulting romlist
	//
	bool build_romlist( const std::vector< std::string > &emu_list );

	// This function implements the config-mode romlist generation
	// A romlist named "<emu_name>.txt" is created in the romlist dir,
	// overwriting any previous list of this name.
	//
	typedef void (*UiUpdate) ( void *, int );
	bool build_romlist( const std::string &emu_name, UiUpdate, void *, int & );

	FeEmulatorInfo *get_current_emulator();
	FeEmulatorInfo *get_emulator( const std::string & );
	FeEmulatorInfo *create_emulator( const std::string & );
	void delete_emulator( const std::string & );

	//
	// Functions used for configuration
	//
	void get_list_names( std::vector<std::string> &list );
   const std::string get_info( int index ) const; // see "ConfigSettingIndex"
   bool set_info( int index, const std::string & );
	FeListInfo *get_list( const std::string &n );
	FeListInfo *create_list( const std::string &n );
	void delete_list( const std::string &n );
   void get_mappings( std::vector< FeMapping > &mappings );
   void set_mapping( const FeMapping &mapping );

	void save();

	void get_resource( const std::string &token, std::string &str );
	void get_resource( const std::string &token, const std::string &rep, 
									std::string &str );

	int lists_count();
};

#endif
