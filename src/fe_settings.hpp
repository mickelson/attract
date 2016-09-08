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

#ifndef FE_SETTINGS_HPP
#define FE_SETTINGS_HPP

#include "fe_base.hpp"
#include "fe_info.hpp"
#include "fe_romlist.hpp"
#include "fe_input.hpp"

extern const char *FE_ART_EXTENSIONS[];

extern const char *FE_CFG_FILE;

extern const char *FE_ROMLIST_SUBDIR;
extern const char *FE_SCRAPER_SUBDIR;
extern const char *FE_LAYOUT_FILE_BASE;
extern const char *FE_LAYOUT_FILE_EXTENSION;
extern const char *FE_SWF_EXT;

extern const char *FE_CFG_YES_STR;
extern const char *FE_CFG_NO_STR;

class FeImporterContext;

// A container for each task when importing/building romlists from the command line
class FeImportTask
{
public:

	enum TaskType
	{
		BuildRomlist,
		ImportRomlist,
		ScrapeArtwork
	};

	FeImportTask( TaskType t, const std::string &en, const std::string &fn="" )
		: task_type( t ), emulator_name( en ), file_name( fn )
	{
	};

	TaskType task_type;
	std::string emulator_name;
	std::string file_name;
};

class FeLanguage
{
public:
	FeLanguage( const std::string &l );

	std::string language;
	std::string label;
	std::vector< std::string > font;
};

class FeSettings : public FeBaseConfigurable
{
public:
	enum RotationState { RotateNone=0, RotateRight, RotateFlip, RotateLeft };
	enum FePresentState
	{
		Intro_Showing,
		Layout_Showing,
		ScreenSaver_Showing
	};

	enum WindowType { Default=0, Fullscreen, Window, WindowNoBorder };
	static const char *windowModeTokens[];
	static const char *windowModeDispTokens[];

	enum FilterWrapModeType { WrapWithinDisplay=0, JumpToNextDisplay, NoWrap };
	static const char *filterWrapTokens[];
	static const char *filterWrapDispTokens[];

	enum StartupModeType { ShowLastSelection=0, LaunchLastGame, ShowDisplaysMenu };
	static const char *startupTokens[];
	static const char *startupDispTokens[];

	enum ConfigSettingIndex
	{
		Language=0,
		ExitCommand,
		DefaultFont,
		FontPath,
		ScreenSaverTimeout,
		DisplaysMenuExit,
		HideBrackets,
		StartupMode,
		ConfirmFavourites,
		MouseThreshold,
		JoystickThreshold,
		WindowMode,
		FilterWrapMode,
		TrackUsage,
		MultiMon,
		SmoothImages,
		AccelerateSelection,
		SelectionSpeed,
		ScrapeSnaps,
		ScrapeMarquees,
		ScrapeFlyers,
		ScrapeWheels,
		ScrapeFanArt,
		ScrapeVids,
#ifdef SFML_SYSTEM_WINDOWS
		HideConsole,
#endif
		VideoDecoder,
		LAST_INDEX
	};

	static const char *configSettingStrings[];
	static const char *otherSettingStrings[];

	enum GameExtra
	{
		Executable =0, // custom executable to override the configured emulator executable
		Arguments      // custom arguments to override the configured emulator arguments
	};

private:
	std::string m_config_path;
	std::string m_default_font;
	std::string m_exit_command;
	std::string m_language;
	std::string m_current_search_str;

	std::vector<std::string> m_font_paths;
	std::vector<FeDisplayInfo> m_displays;
	std::vector<FePlugInfo> m_plugins;
	std::vector<FeLayoutInfo> m_layout_params;
	std::vector<FeRomInfo *> m_current_search;
	std::vector<int> m_display_cycle; // display indices to show in cycle
	std::vector<int> m_display_menu; // display indices to show in menu
	std::map<GameExtra,std::string> m_game_extras; // "extra" rom settings for the current rom
	FeRomList m_rl;

	FeInputMap m_inputmap;
	FeSoundInfo m_sounds;
	FeResourceMap m_resourcemap;
	FeLayoutInfo m_saver_params;
	FeLayoutInfo m_intro_params;
	sf::IntRect m_mousecap_rect;

	int m_current_display;
	FeBaseConfigurable *m_current_config_object;
	int m_ssaver_time;
	int m_last_launch_display;
	int m_last_launch_filter;
	int m_last_launch_rom;
	int m_joy_thresh;		// [1..100], 100=least sensitive
	int m_mouse_thresh;	// [1..100], 100=least sensitive
	int m_current_search_index;
	bool m_displays_menu_exit;
	bool m_hide_brackets;
	StartupModeType m_startup_mode;
	bool m_confirm_favs;
	bool m_track_usage;
	bool m_multimon;
	WindowType m_window_mode;
	bool m_smooth_images;
	FilterWrapModeType m_filter_wrap_mode;
	bool m_accel_selection;
	int m_selection_speed;
	bool m_scrape_snaps;
	bool m_scrape_marquees;
	bool m_scrape_flyers;
	bool m_scrape_wheels;
	bool m_scrape_fanart;
	bool m_scrape_vids;
#ifdef SFML_SYSTEM_WINDOWS
	bool m_hide_console;
#endif
	bool m_loaded_game_extras;
	enum FePresentState m_present_state;

	FeSettings( const FeSettings & );
	FeSettings &operator=( const FeSettings & );

	int process_setting( const std::string &,
		const std::string &,
		const std::string & );

	void init_display();
	void load_state();
	void clear();

	void construct_display_maps();

	void internal_gather_config_files(
		std::vector<std::string> &ll,
		const std::string &extension,
		const char *subdir ) const;

	void internal_load_language( const std::string &lang );

	std::string get_played_display_string( int filter_index, int rom_index );


	bool mameps_scraper( FeImporterContext & );
	bool mamedb_scraper( FeImporterContext & );
	bool thegamesdb_scraper( FeImporterContext & );
	void apply_xml_import( FeImporterContext &, bool );


public:
	FeSettings( const std::string &config_dir,
				const std::string &cmdln_font );

	void load();
	void save_state();

	FeInputMap::Command map_input( const sf::Event &e );

	void get_input_config_metrics( sf::IntRect &mousecap_rect, int &joy_thresh );
	FeInputMap::Command input_conflict_check( const FeInputMapEntry &e );

	// for use with Up, Down, Left, Right, Back commands to get what they are actually mapped to
	FeInputMap::Command get_default_command( FeInputMap::Command );
	void set_default_command( FeInputMap::Command c, FeInputMap::Command v );

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

	void step_current_selection( int step );
	void set_current_selection( int filter_index, int rom_index ); // use rom_index<0 to only change the filter

	// Switches the display
	// returns true if the display change results in a new layout, false otherwise
	//
	bool set_display( int index );

	int get_current_display_index() const;
	int get_display_index_from_name( const std::string &name ) const;
	int displays_count() const;

	bool navigate_display( int step, bool wrap_mode=false );
	bool navigate_filter( int step );

	int get_current_filter_index() const;
	const std::string &get_filter_name( int filter_index );
	void get_current_display_filter_names( std::vector<std::string> &list ) const;
	int get_filter_index_from_offset( int offset ) const;
	int get_filter_size( int filter_index ) const;
	int get_filter_count() const;

	// apply a search rule (see FeRule) to the currently selected
	// filter
	//
	// set to an empty string to clear the current search
	//
	void set_search_rule( const std::string &rule );
	const std::string &get_search_rule() const;

	bool select_last_launch();
	int get_joy_thresh() const { return m_joy_thresh; }
	void init_mouse_capture( int window_x, int window_y );
	bool test_mouse_reset( int mouse_x, int mouse_y ) const;

	void run( int &minimum_run_seconds ); // run current selection
	int exit_command() const; // run configured exit command (if any)

	void toggle_layout();
	void set_current_layout_file( const std::string &layout_file );

	int get_rom_index( int filter_index, int offset ) const;

	void do_text_substitutions( std::string &str, int filter_offset, int index_offset );
	void do_text_substitutions_absolute( std::string &str, int filter_index, int rom_index );

	void get_current_sort( FeRomInfo::Index &idx, bool &rev, int &limit );

	const std::string &get_current_display_title() const;
	const std::string &get_rom_info( int filter_offset, int rom_offset, FeRomInfo::Index index );
	const std::string &get_rom_info_absolute( int filter_index, int rom_index, FeRomInfo::Index index );
	FeRomInfo *get_rom_absolute( int filter_index, int rom_index );

	int selection_speed() const { return m_selection_speed; }

	// get a list of available plugins
	void get_available_plugins( std::vector < std::string > &list ) const;
	std::vector<FePlugInfo> &get_plugins() { return m_plugins; }
	void get_plugin( const std::string &label,
			FePlugInfo *&plug, int &index );
	bool get_plugin_enabled( const std::string &label ) const;
	void get_plugin_full_path( const std::string &label,
			std::string &path,
			std::string &filename ) const;
	void get_plugin_full_path( int id,
			std::string &path ) const;

	//
	FePresentState get_present_state() const { return m_present_state; };
	void set_present_state( FePresentState s ) { m_present_state=s; };

#ifdef SFML_SYSTEM_WINDOWS
	bool get_hide_console() const { return m_hide_console; };
#endif

	enum FePathType
	{
		Current, // could be Layout, ScreenSaver or Intro depending on
			// m_present_state
		Layout,
		ScreenSaver,
		Intro,
		Loader
	};

	bool get_path( FePathType t,
		std::string &path,
		std::string &filename ) const;

	bool get_path( FePathType t,
		std::string &path ) const;

	FeLayoutInfo &get_current_config( FePathType t );

	bool get_layout_dir( const std::string &layout_name, std::string &layout_dir ) const;
	void get_layouts_list( std::vector<std::string> &layouts ) const;
	FeLayoutInfo &get_layout_config( const std::string &layout_name );

	bool get_best_artwork_file(
		const FeRomInfo &rom,
		const std::string &art_name,
		std::vector<std::string> &vid_list,
		std::vector<std::string> &image_list,
		bool image_only,
		bool ignore_emu=false );

	bool has_artwork( const FeRomInfo &rom, const std::string &art_name );
	bool has_video_artwork( const FeRomInfo &rom, const std::string &art_name );
	bool has_image_artwork( const FeRomInfo &rom, const std::string &art_name );

	bool get_best_dynamic_image_file(
		int filter_index,
		int rom_index,
		const std::string &logo_str,
		std::vector<std::string> &vid_list,
		std::vector<std::string> &image_list );

	bool get_module_path( const std::string &module,
		std::string &module_dir,
		std::string &module_file ) const;

	const std::string &get_config_dir() const;
	bool config_file_exists() const;

	// [out] fpath = if font is in a zip file, this is the path to the zip
	// [out] ffile = font file to open
	// [in] fontname = name of font to find.  If empty return default font
	bool get_font_file(
		std::string &fpath,
		std::string &ffile,
		const std::string &fontname="" ) const;

	WindowType get_window_mode() const;
	FilterWrapModeType get_filter_wrap_mode() const;
	StartupModeType get_startup_mode() const;
	int get_screen_saver_timeout() const;

	bool get_current_fav();

	// returns true if the current list chnaged as a result of setting the tag
	bool set_current_fav( bool );
	int get_prev_fav_offset();
	int get_next_fav_offset();

	int get_next_letter_offset( int step );

	void get_current_tags_list(
		std::vector< std::pair<std::string, bool> > &tags_list );

	// returns true if the current list changed as a result of setting the tag
	bool set_current_tag(
			const std::string &tag, bool flag );
	//
	// This function implements command-line romlist generation/imports
	// If output_name is empty, then a non-existing filename is chosen for
	// the resulting romlist file
	//
	bool build_romlist( const std::vector< FeImportTask > &task_list,
		const std::string &output_name,
		FeFilter &filter,
		bool full );

	//
	// Save an updated rom in the current romlist file (used with "Edit Game" command)
	// original is assumed to be the currently selected rom
	//
	void update_romlist_after_edit(
		const FeRomInfo &original,		// original rom values
		const FeRomInfo &replacement,		// new rom values
		bool erase=false );			// if true, erase original instead

	void update_stats( int count_incr, int time_incr );

	//
	// The frontend maintains extra per game settings/extra info
	//
	// This info is only ever loaded for the currently selected game, and is not intended
	// to be used in a filter
	//
	std::string get_game_extra( GameExtra id );
	void set_game_extra( GameExtra id, const std::string &value );
	void save_game_extras();

	// This function implements the config-mode romlist generation
	// A romlist named "<emu_name>.txt" is created in the romlist dir,
	// overwriting any previous list of this name.
	//
	typedef bool (*UiUpdate) ( void *, int, const std::string & );
	bool build_romlist( const std::vector < std::string > &emu_name, const std::string &out_filename,
		UiUpdate, void *, std::string & );
	bool scrape_artwork( const std::string &emu_name, UiUpdate uiu, void *uid, std::string &msg );

	FeEmulatorInfo *get_emulator( const std::string & );
	FeEmulatorInfo *create_emulator( const std::string & );
	void delete_emulator( const std::string & );

	void get_list_of_emulators( std::vector<std::string> &emu_list );

	//
	// Functions used for configuration
	//
	const std::string get_info( int index ) const; // see "ConfigSettingIndex"
	bool get_info_bool( int index ) const;
	bool set_info( int index, const std::string & );

	void get_display_menu( std::vector<std::string> &names,
		std::vector<int> &indices,
		int &current_idx ) const;
	FeDisplayInfo *get_display( int index );
	FeDisplayInfo *create_display( const std::string &n );
	void delete_display( int index );

	void create_filter( FeDisplayInfo &l, const std::string &name ) const;

	// return true if specified romlist name is configured for use as a display
	// list
	bool check_romlist_configured( const std::string &n ) const;
	void get_romlists_list( std::vector < std::string > &ll ) const;

	void save() const;

	void get_resource( const std::string &token, std::string &str ) const;
	void get_resource( const std::string &token, const std::string &rep,
		std::string &str ) const;

	void set_language( const std::string &l );
	const std::string &get_language() const { return m_language; }
	void get_languages_list( std::vector < FeLanguage > &ll ) const;

	// Utility function to get a list of layout*.nut files from the specified path...
	static void get_layout_file_basenames_from_path(
		const std::string &path,
		std::vector<std::string> &names_list );
};

inline bool is_windowed_mode( int m )
{
	return (( m == FeSettings::Window ) || ( m == FeSettings::WindowNoBorder ));
}

//
// Utility function used to collect artwork files with 'target_name' from
// the specified art_paths
//
bool gather_artwork_filenames(
	const std::vector < std::string > &art_paths,
	const std::string &target_name,
	std::vector<std::string> &vids,
	std::vector<std::string> &images );

bool art_exists( const std::string &path, const std::string &base );

#endif
