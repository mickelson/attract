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

extern const char *FE_ART_EXTENSIONS[];

extern const char *FE_ROMLIST_SUBDIR;
extern const char *FE_EMULATOR_SUBDIR;

extern const char *FE_EMULATOR_FILE_EXTENSION;
extern const char *FE_LAYOUT_FILE_BASE;
extern const char *FE_LAYOUT_FILE_EXTENSION;

extern const char *FE_DIR_TOKEN;


// A container for each task when importing/building romlists from the command line
class FeImportTask
{
public:
	FeImportTask( const std::string &fn, const std::string &en )
		: file_name( fn ), emulator_name( en )
	{
	};

	std::string file_name;		// if empty, then we build romlist for specified emulator
	std::string emulator_name;
};

class FeSettings : public FeBaseConfigurable
{
public:
   enum RotationState { RotateNone=0, RotateRight, RotateFlip, RotateLeft };

	enum WindowType { Default=0, Fullscreen, Window };
	static const char *windowModeTokens[];
	static const char *windowModeDispTokens[];

	enum ConfigSettingIndex
	{
		Language=0,
		ExitCommand,
		DefaultFont,
		FontPath,
		ScreenSaverTimeout,
		ListsMenuExit,
		HideBrackets,
		AutoLaunchLastGame,
		ConfirmFavourites,
		MouseThreshold,
		JoystickThreshold,
		WindowMode,
		LAST_INDEX
	};

	static const char *configSettingStrings[];
	static const char *otherSettingStrings[];

private:
	std::string m_config_path;
	std::string m_default_font;
	std::string m_exit_command;
	std::string m_language;

	std::vector<std::string> m_font_paths;
	std::vector<FeListInfo> m_lists;
	std::vector<FeEmulatorInfo> m_emulators;
	std::vector<FePlugInfo> m_plugins;
	std::vector<FeLayoutInfo> m_layout_params;
	FeRomList m_rl;

	FeInputMap m_inputmap;
	FeSoundInfo m_sounds;
	FeResourceMap m_resourcemap;
	FeLayoutInfo m_saver_params;
	sf::IntRect m_mousecap_rect;

	int m_current_list;
	FeBaseConfigurable *m_current_config_object;
	int m_ssaver_time;
	int m_last_launch_list;
	int m_last_launch_filter;
	int m_last_launch_rom;
	int m_joy_thresh;		// [1..100], 100=least sensitive
	int m_mouse_thresh;	// [1..100], 100=least sensitive
	bool m_lists_menu_exit;
	bool m_hide_brackets;
	bool m_autolaunch_last_game;
	bool m_confirm_favs;
	WindowType m_window_mode;

	FeSettings( const FeSettings & );
	FeSettings &operator=( const FeSettings & );

	int process_setting( const std::string &,
						const std::string &,
						const std::string & );

	void load_state();
	void clear();

	void set_current_rom(int );

	void internal_gather_config_files(
			std::vector<std::string> &ll,
			const std::string &extension,
			const char *subdir ) const;

	bool internal_resolve_config_file(
			std::string &result, const char *subdir, const std::string &config_file  ) const;

	void internal_load_language( const std::string &lang );

public:
	FeSettings( const std::string &config_dir,
				const std::string &cmdln_font );

	void load();
	void save_state( void ) const;

	FeInputMap::Command map_input( const sf::Event &e );
	bool config_map_input( const sf::Event &e, std::string &s, FeInputMap::Command &conflict );

	bool get_current_state( FeInputMap::Command c );
	void get_input_mappings( std::vector < FeMapping > &l ) const { m_inputmap.get_mappings( l ); };
	void set_input_mapping( FeMapping &m ) { m_inputmap.set_mapping( m ); };

	void set_volume( FeSoundInfo::SoundType, const std::string & );
	int get_set_volume( FeSoundInfo::SoundType ) const;
	int get_play_volume( FeSoundInfo::SoundType ) const;
	bool get_mute() const;
	void set_mute( bool );
	bool get_sound_file( FeInputMap::Command, std::string &s, bool full_path=true ) const;
	void set_sound_file( FeInputMap::Command, const std::string &s );
	void get_sounds_list( std::vector < std::string > &ll ) const;

	void change_rom( int step );

	// Switches the display list
	// returns true if the list change results in a new layout, false otherwise
	//
	bool set_list( int index );
	bool next_list();
	bool prev_list();
	int get_current_list_index() const;
	int lists_count() const;

	void init_list();

	void set_filter( int index );
	int get_current_filter_index() const;
	const std::string &get_current_filter_name();
	void get_current_list_filter_names( std::vector<std::string> &list ) const;

	bool select_last_launch();
	int get_joy_thresh() const { return m_joy_thresh; }
	void init_mouse_capture( int window_x, int window_y );
	bool test_mouse_reset( int mouse_x, int mouse_y ) const;

	int run(); // run current selection
	int exit_command() const; // run configured exit command (if any)

	void toggle_layout();

	void get_current_display_list( std::vector<std::string> &list ) const;
	int get_current_list_size() const { return m_rl.size(); };
	int get_rom_index( int offset=0 ) const;

	const std::string &get_current_list_title() const;
	const std::string &get_rom_info( int offset, FeRomInfo::Index index ) const;
	bool hide_brackets() const { return m_hide_brackets; }
	bool autolaunch_last_game() const { return m_autolaunch_last_game; }

	// get a list of available plugins
	void get_available_plugins( std::vector < std::string > &list ) const;
	std::vector<FePlugInfo> &get_plugins() { return m_plugins; }
	FePlugInfo *get_plugin( const std::string &label );
	void get_plugin_full_path( const std::string &label,
			std::string &path,
			std::string &filename ) const;

	void get_screensaver_file( std::string &path, std::string &filename ) const;
	FeLayoutInfo &get_screensaver_config() { return m_saver_params; }
	std::string get_current_layout_file() const;
	std::string get_current_layout_dir() const;
	std::string get_layout_dir( const std::string &layout_name ) const;
	void get_layouts_list( std::vector<std::string> &layouts ) const;
	FeLayoutInfo &get_layout_config( const std::string &layout_name );
	FeLayoutInfo &get_current_layout_config();

	std::string get_module_dir( const std::string &module_file ) const;

	const std::string &get_config_dir() const;
	bool config_file_exists() const;
	bool get_font_file( std::string &fullpath, const std::string &filename="" ) const;

	WindowType get_window_mode() const;
	int get_screen_saver_timeout() const;
	bool get_lists_menu_exit() const;

	void dump( void ) const;

	bool get_current_fav() const;

	// returns true if the current list chnaged as a result of setting the tag
	bool set_current_fav( bool );
	int get_prev_fav_offset() const;
	int get_next_fav_offset() const;
	bool confirm_favs() const { return m_confirm_favs; }

	int get_next_letter_offset( int step ) const;

	void get_current_tags_list(
		std::vector< std::pair<std::string, bool> > &tags_list ) const;

	// returns true if the current list chnaged as a result of setting the tag
	bool set_current_tag(
			const std::string &tag, bool flag );
	//
	// This function implements command-line romlist generation/imports
	// If output_name is empty, then a non-existing filename is chosen for
	// the resulting romlist file
	//
	bool build_romlist( const std::vector< FeImportTask > &task_list,
							const std::string &output_name );

	// This function implements the config-mode romlist generation
	// A romlist named "<emu_name>.txt" is created in the romlist dir,
	// overwriting any previous list of this name.
	//
	typedef bool (*UiUpdate) ( void *, int );
	bool build_romlist( const std::string &emu_name, UiUpdate, void *, int & );

	FeEmulatorInfo *get_current_emulator();
	FeEmulatorInfo *get_emulator( const std::string & );
	FeEmulatorInfo *create_emulator( const std::string & );
	void delete_emulator( const std::string & );

	//
	// Functions used for configuration
	//
   const std::string get_info( int index ) const; // see "ConfigSettingIndex"
   bool set_info( int index, const std::string & );

	void get_list_names( std::vector<std::string> &list ) const;
	FeListInfo *get_list( int index );
	FeListInfo *create_list( const std::string &n );
	void delete_list( int index );

	void create_filter( FeListInfo &l, const std::string &name ) const;

	// return true if specified romlist name is configured for use as a display
	// list
	bool check_romlist_configured( const std::string &n ) const;
	void get_romlists_list( std::vector < std::string > &ll ) const;

	void save() const;

	void get_resource( const std::string &token, std::string &str ) const;
	void get_resource( const std::string &token, const std::string &rep,
									std::string &str ) const;

	void set_language( const std::string &s );
	const std::string &get_language() const { return m_language; }
	void get_languages_list( std::vector < std::string > &ll ) const;
};

#endif
