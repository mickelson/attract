/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-15 Andrew Mickelson
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

#ifndef FE_INFO_HPP
#define FE_INFO_HPP

#include "fe_base.hpp"
#include <map>
#include <vector>

extern const char FE_TAGS_SEP;
struct SQRex;

//
// Class for storing information regarding a specific rom
//
class FeRomInfo : public FeBaseConfigurable
{
public:
	enum Index
	{
		Romname=0,
		Title,
		Emulator,
		Cloneof,
		Year,
		Manufacturer,
		Category,
		Players,
		Rotation,
		Control,
		Status,
		DisplayCount,
		DisplayType,
		AltRomname,
		AltTitle,
		Extra,
		Buttons,
		Favourite,		// everything from Favourite on is not loaded from romlist
		Tags,
		PlayedCount,
		PlayedTime,
		FileIsAvailable,
		LAST_INDEX
	};

	// Certain indices gets repurposed during -listsoftware
	// romlist building/importing...
	static const Index BuildFullPath;
	static const Index BuildScore;

	static const char *indexStrings[];

	FeRomInfo();
	FeRomInfo( const std::string &romname );

	const std::string &get_info( int ) const;
	void set_info( enum Index, const std::string & );

	void append_tag( const std::string &tag );

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &fn );
	std::string as_output( void ) const;

	void load_stats( const std::string &path );
	void update_stats( const std::string &path, int count_incr, int played_incr );

	void clear();

	// convenience method to copy info attribute at idx from src
	void copy_info( const FeRomInfo &src, Index idx );

	bool operator==( const FeRomInfo & ) const;

private:
	std::string get_info_escaped( int ) const;

	std::string m_info[LAST_INDEX];
};

//
// Class for a single rule in a list filter
//
class FeRule : public FeBaseConfigurable
{
public:
	enum FilterComp {
		FilterEquals=0,
		FilterNotEquals,
		FilterContains,
		FilterNotContains,
		LAST_COMPARISON
	};

	static const char *filterCompStrings[];
	static const char *filterCompDisplayStrings[];

	FeRule( FeRomInfo::Index target=FeRomInfo::LAST_INDEX,
		FilterComp comp=LAST_COMPARISON,
		const std::string &what="" );
	FeRule( const FeRule & );

	~FeRule();

	FeRule &operator=( const FeRule & );

	void init();
	bool apply_rule( const FeRomInfo &rom ) const;
	void save( std::ofstream & ) const;

	FeRomInfo::Index get_target() const { return m_filter_target; };
	FilterComp get_comp() const { return m_filter_comp; };
	const std::string &get_what() const { return m_filter_what; };

	bool is_exception() const { return m_is_exception; };
	void set_is_exception( bool f ) { m_is_exception=f; };

	void set_values( FeRomInfo::Index i, FilterComp c, const std::string &w );

	int process_setting( const std::string &,
         const std::string &value, const std::string &fn );

private:
	FeRomInfo::Index m_filter_target;
	FilterComp m_filter_comp;
	std::string m_filter_what;
	SQRex *m_rex;
	bool m_is_exception;
};

//
// Class for a list filter
//
class FeFilter : public FeBaseConfigurable
{
public:
	enum Index { Rule=0, Exception, SortBy, ReverseOrder, ListLimit };
	static const char *indexStrings[];

	FeFilter( const std::string &name );
	void init();
	bool apply_filter( const FeRomInfo &rom ) const;

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &fn );

	void save( std::ofstream &, const char *filter_tag ) const;
	const std::string &get_name() const { return m_name; };
	void set_name( const std::string &n ) { m_name = n; };

	int get_rom_index() const { return m_rom_index; };
	void set_rom_index( int i ) { m_rom_index=i; };

	int get_size() const { return m_size; };
	void set_size( int s ) { m_size=s; };

	std::vector<FeRule> &get_rules() { return m_rules; };
	int get_rule_count() const { return m_rules.size(); };

	FeRomInfo::Index get_sort_by() const { return m_sort_by; }
	bool get_reverse_order() const { return m_reverse_order; }
	int get_list_limit() const { return m_list_limit; }

	void set_sort_by( FeRomInfo::Index i ) { m_sort_by=i; }
	void set_reverse_order( bool r ) { m_reverse_order=r; }
	void set_list_limit( int p ) { m_list_limit=p; }

	bool test_for_target( FeRomInfo::Index target ) const; // do changes to the specified target affect this filter?

	void clear();

private:
	std::string m_name;
	std::vector<FeRule> m_rules;
	int m_rom_index;
	int m_list_limit; // limit the number of list entries if non-zero.
		// Positive value limits to the first
		// x values, Negative value limits to the last abs(x) values.
	int m_size;
	FeRomInfo::Index m_sort_by;
	bool m_reverse_order;
};

//
// Class for storing information regarding a specific Attract-Mode display
//
class FeDisplayInfo : public FeBaseConfigurable
{
public:

	enum Index {
		Name=0,
		Layout,
		Romlist,
		InCycle,
		InMenu,
		LAST_INDEX
	};
	static const char *indexStrings[];
	static const char *otherStrings[];

	FeDisplayInfo( const std::string &name );

	const std::string &get_info( int ) const;
	void set_info( int setting, const std::string &value );

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &fn );

	int process_state( const std::string &state_string );
	std::string state_as_output() const;

	void set_current_filter_index( int i ) { m_filter_index=i; };
	int get_current_filter_index() const { return m_filter_index; };
	int get_filter_count() const { return m_filters.size(); };
	FeFilter *get_filter( int ); // use get_filter( -1 ) to get global filter
	void append_filter( const FeFilter &f );
	void delete_filter( int i );
	void get_filters_list( std::vector<std::string> &l ) const;

	FeFilter *get_global_filter() { return &m_global_filter; };

	std::string get_current_layout_file() const;
	void set_current_layout_file( const std::string & );

	int get_rom_index( int filter_index ) const;
	void set_rom_index( int filter_index, int rom_index );

	void save( std::ofstream & ) const;

	const std::string &get_name() const { return m_info[Name]; };
	const std::string &get_layout() const { return m_info[Layout]; };
	const std::string &get_romlist_name() const { return m_info[Romlist]; };

	bool show_in_cycle() const;
	bool show_in_menu() const;

private:
	std::string m_info[LAST_INDEX];
	std::string m_current_layout_file;
	int m_rom_index; // only used if there are no filters on this display
	int m_filter_index;
	FeFilter *m_current_config_filter;

	std::vector< FeFilter > m_filters;
	FeFilter m_global_filter;
};

//
// Class for storing information regarding a specific emulator
//
class FeEmulatorInfo : public FeBaseConfigurable
{
public:

	enum Index
	{
		Name=0,
		Executable,
		Command,
		Working_dir,
		Rom_path,
		Rom_extension,
		System,
		Info_source,
		Import_extras,
		Minimum_run_time,
		Exit_hotkey,
		LAST_INDEX
	};

	enum InfoSource
	{
		None=0,
		Listxml,	// "mame"
		Listsoftware,	// "mess"
		Steam,
		Thegamesdb,
		Scummvm,
		Listsoftware_tgdb,	// "mess" + thegamesdb.net
		LAST_INFOSOURCE
	};

	static const char *indexStrings[];
	static const char *indexDispStrings[];
	static const char *infoSourceStrings[];

	FeEmulatorInfo();
	FeEmulatorInfo( const std::string &name );

	const std::string get_info( int ) const;
	void set_info( enum Index, const std::string & );

	InfoSource get_info_source() const { return m_info_source; };

	// get artwork path.  Multiple paths are semicolon separated
	bool get_artwork( const std::string &label, std::string &paths ) const;

	// get artwork paths in the provided vector
	bool get_artwork( const std::string &label, std::vector< std::string > &paths ) const;

	// add artwork paths to the specified artwork
	void add_artwork( const std::string &label, const std::string &paths );

	// get a list of all artworks labels with the related paths in a string
	void get_artwork_list( std::vector<std::pair<std::string,std::string> > & ) const;

	void delete_artwork( const std::string & );

	const std::vector<std::string> &get_paths() const;
	const std::vector<std::string> &get_extensions() const;
	const std::vector<std::string> &get_systems() const;
	const std::vector<std::string> &get_import_extras() const;

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &filename );

	void save( const std::string &filename ) const;

	// search paths and exts to gather all base rom names (i.e. "galaga")
	void gather_rom_names( std::vector<std::string> &name_list ) const;

	// same as above but pair base and full paths
	// (i.e. "galaga", "c:\beer\galaga.zip")
	void gather_rom_names( std::vector<std::string> &name_list,
		std::vector<std::string> &full_path_list ) const;

	// clean_path() and use Working_dir setting if relative in_path
	std::string clean_path_with_wd( const std::string &in_path, bool add_trailing_slash=false ) const;

	bool is_mame() const;
	bool is_mess() const;

private:
	std::string vector_to_string( const std::vector< std::string > &vec ) const;
	std::string m_name;
	std::string m_executable;
	std::string m_command;
	std::string m_workdir;
	std::string m_exit_hotkey;

	std::vector<std::string> m_paths;
	std::vector<std::string> m_extensions;
	std::vector<std::string> m_systems;
	std::vector<std::string> m_import_extras;

	InfoSource m_info_source;
	int m_min_run;

	//
	// Considered using a std::multimap here but C++98 doesn't guarantee the
	// relative order of equivalent values, so we do a map of vectors so that
	// we can maintain the precedence ordering of our artwork paths...
	//
	std::map<std::string, std::vector<std::string> > m_artwork;
};

class FeScriptConfigurable : public FeBaseConfigurable
{
public:
	bool get_param( const std::string &label, std::string &v ) const;
	void set_param( const std::string &label, const std::string &v );
	void get_param_labels( std::vector<std::string> &labels ) const;
	void clear_params() { m_params.clear(); };

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &filename );

	void save( std::ofstream & ) const;

protected:
	static const char *indexString;
	std::map<std::string,std::string> m_params;
};

//
// Class for storing plug-in configuration
//
class FePlugInfo : public FeScriptConfigurable
{
public:
	FePlugInfo( const std::string &name );
	const std::string &get_name() const { return m_name; };

	bool get_enabled() const { return m_enabled; };
	void set_enabled( bool e ) { m_enabled=e; };

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &filename );

	void save( std::ofstream & ) const;

private:
	static const char *indexStrings[];

	std::string m_name;
	bool m_enabled;
};

//
// Class for storing layout/screensaver configuration
//
class FeLayoutInfo : public FeScriptConfigurable
{
public:
	static const char *indexStrings[];
	enum Type { ScreenSaver=0, Layout, Intro };

	FeLayoutInfo( Type t );
	FeLayoutInfo( const std::string &name ); // type=Layout
	const std::string &get_name() const { return m_name; };

	void save( std::ofstream & ) const;

private:
	std::string m_name;
	Type m_type;
};

//
// Class to contain the interface language strings
//
class FeResourceMap : public FeBaseConfigurable
{
public:
	FeResourceMap();
	void clear() { m_map.clear(); }

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &filename );

	void get_resource( const std::string &token,
		std::string &str ) const;

private:
	FeResourceMap( const FeResourceMap & );
	FeResourceMap &operator=( const FeResourceMap & );

	std::map<std::string, std::string> m_map;
};

#endif
