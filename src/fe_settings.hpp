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

	enum ConfigSettingIndex 
	{ 
		AutoRotate=0, 
		ExitCommand, 
		DefaultFont, 
		FontPath, 
		ScreenSaverTimeout, 
		ListsMenuExit,
		LAST_INDEX 
	};

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
	bool m_lists_menu_exit;

	int process_setting( const std::string &, 
						const std::string &,
						const std::string & );

	void load_state();
	void clear();

	void set_current_rom(int );

	void internal_get_art_file( std::vector<std::string> &, 
							std::string &, int, const std::string &, bool  ) const;

public:
	FeSettings( const std::string &config_dir, 
				const std::string &cmdln_font, bool disable_mousecap );

	bool load();
	void save_state( void ) const;

	FeInputMap::Command map( sf::Event e ) const { return m_inputmap.map( e ); };
	FeInputMap &get_input_map() { return m_inputmap; };

	void set_volume( FeSoundInfo::SoundType, const std::string & );
	int get_set_volume( FeSoundInfo::SoundType ) const;
	int get_play_volume( FeSoundInfo::SoundType ) const;
	bool get_mute() const;
	void set_mute( bool );
	bool get_sound_file( FeInputMap::Command, std::string &s, bool full_path=true ) const;
	void set_sound_file( FeInputMap::Command, const std::string &s );

	void change_rom( int step, bool wrap=true );

	// Switches the display list
	// returns true if the list change results in a new layout, false otherwise
	//
	bool set_list( int index );
	bool next_list();
	bool prev_list();
	int get_current_list_index() const;

	void init_list();

	int run(); // run current selection
	int exit_command() const; // run configured exit command (if any)

	void toggle_layout();

	int get_current_display_list( std::vector<std::string> &list ) const;
	int get_rom_index( int offset=0, bool wrap=true ) const;

	std::string get_current_list_title() const;
	const std::string &get_rom_info( int offset, FeRomInfo::Index index ) const;

	//
	// Returns filenames to use for artwork. prefer the files earlier in the list
	//
	void get_art_file( int offset, const std::string &artwork, std::vector<std::string> &filenames );
	void get_movie_file( int offset, std::vector<std::string> &filenames );

	const std::string &get_movie_artwork();

	std::string get_screensaver_file() const;
	std::string get_current_layout_file() const;
	std::string get_layout_global_file() const;
	std::string get_current_layout_dir() const;

	const std::string &get_config_dir() const;
	bool config_file_exists() const;
	bool get_font_file( std::string &fullpath, const std::string &filename="" ) const;
	
	RotationState get_autorotate() const;
	int get_screen_saver_timeout() const;
	bool get_lists_menu_exit() const;

	void dump( void ) const;

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
	void get_list_names( std::vector<std::string> &list ) const;
   const std::string get_info( int index ) const; // see "ConfigSettingIndex"
   bool set_info( int index, const std::string & );
	FeListInfo *get_list( const std::string &n );
	FeListInfo *create_list( const std::string &n );
	void delete_list( const std::string &n );

	// return true if specified romlist name is configured for use as a display 
	// list
	bool check_romlist_configured( const std::string &n ) const;

	void save() const;

	void get_resource( const std::string &token, std::string &str ) const;
	void get_resource( const std::string &token, const std::string &rep, 
									std::string &str ) const;

	int lists_count() const;
};

#endif
