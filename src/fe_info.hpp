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

#ifndef FE_INFO_HPP
#define FE_INFO_HPP

#include "fe_base.hpp"
#include <map>
#include <set>
#include <vector>
#include <deque>

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
		Favourite,		// everything from Favourite on is not loaded from romlist
		LAST_INDEX
	};
	static const char *indexStrings[];

	FeRomInfo();
	FeRomInfo( const std::string &romname );

	const std::string &get_info( int ) const;
	void set_info( enum Index, const std::string & );

	int process_setting( const std::string &setting,
								const std::string &value,
								const std::string &fn );
	void dump( void ) const;
	std::string as_output( void ) const;

	bool operator< ( FeRomInfo ) const;

private:
	FeRomInfo &operator=( const FeRomInfo & );
	std::string get_info_escaped( int ) const;

	std::string m_info[LAST_INDEX];
};

//
// Class for a single rule in a list filter
//
class FeRule
{
public:
	enum FilterComp {
		FilterEquals=0,
		FilterNotEquals,
		FilterContains,
		FilterNotContains,
		LAST_COMPARISON
	};

	static const char *indexString;
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

	void set_values( FeRomInfo::Index i, FilterComp c, const std::string &w );

private:
	FeRomInfo::Index m_filter_target;
	FilterComp m_filter_comp;
	std::string m_filter_what;
	SQRex *m_rex;
};

//
// Class for a list filter
//
class FeFilter : public FeBaseConfigurable
{
public:
	static const char *indexString;

	FeFilter( const std::string &name );
	void init();
	bool apply_filter( const FeRomInfo &rom ) const;

	int process_setting( const std::string &setting,
				const std::string &value,
				const std::string &fn );

	void save( std::ofstream & ) const;
	const std::string &get_name() const { return m_name; };
	void set_name( const std::string &n ) { m_name = n; };

	int get_current_rom_index() const { return m_rom_index; };
	void set_current_rom_index( int i ) { m_rom_index=i; };

	std::vector<FeRule> &get_rules() { return m_rules; };

private:
	std::string m_name;
	std::vector<FeRule> m_rules;
	int m_rom_index;
};

//
// Class for storing information regarding a specific Attract-Mode list
//
class FeListInfo : public FeBaseConfigurable
{
public:

	enum Index {
		Name=0,
		Layout,
		Romlist,
		LAST_INDEX
	};
	static const char *indexStrings[];

	FeListInfo( const std::string &name );

	const std::string &get_info( int ) const;
	void set_info( int setting, const std::string &value );

	int process_setting( const std::string &setting,
								const std::string &value,
								const std::string &fn );

	int process_state( const std::string &state_string );
	std::string state_as_output() const;

	void dump( void ) const;

	void set_current_filter_index( int i ) { m_filter_index=i; };
	int get_current_filter_index() const { return m_filter_index; };
	int get_filter_count() const { return m_filters.size(); };
	FeFilter *get_filter( int );
	void append_filter( const FeFilter &f );
	void delete_filter( int i );
	void get_filters_list( std::vector<std::string> &l ) const;

	std::string get_current_layout_file() const;
	void set_current_layout_file( const std::string & );

	int get_current_rom_index() const;
	void set_current_rom_index( int );

	void save( std::ofstream & ) const;

private:
	std::string m_info[LAST_INDEX];
	std::string m_current_layout_file;
	int m_rom_index; // only used if there are no filters on this list
	int m_filter_index;

	std::deque< FeFilter > m_filters;
};

class FeRomList : protected FeBaseConfigurable
{
private:
	std::deque<FeRomInfo> m_list;
	std::set<std::string> m_favs;
	std::string m_fav_file;
	const FeFilter *m_filter;
	bool m_fav_changed;

	FeRomList( const FeRomList & );
	FeRomList &operator=( const FeRomList & );

	bool apply_filter( const FeRomInfo &rom ) const;

public:
	FeRomList();
	~FeRomList();

	void clear();

	void set_filter( const FeFilter *f );

	// base class has this function too!
	bool load_from_file( const std::string &filename,
					const char *sep=FE_WHITESPACE );

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &fn );

	void load_fav_map( const std::string &filename );
	void save_fav_map() const;
	void set_fav( int idx, bool fav );

	bool empty() const { return m_list.empty(); };
	int size() const { return (int)m_list.size(); };
	const FeRomInfo &operator[](int idx) const { return m_list[idx]; };
	FeRomInfo &operator[](int idx) { return m_list[idx]; };
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
		Rom_path,
		Rom_extension,  // this value gets split and duplicated in m_extensions
		Import_extras,
		Listxml,
		Movie_path,
		Movie_artwork,
		LAST_INDEX
	};
	static const char *indexStrings[];
	static const char *indexDispStrings[];

	FeEmulatorInfo();
	FeEmulatorInfo( const std::string &name );

	const std::string &get_info( int ) const;
	void set_info( enum Index, const std::string & );

	bool get_artwork( const std::string &, std::string & ) const;
	void set_artwork( const std::string &, const std::string & );
	void get_artwork_list( std::vector<std::pair<std::string,std::string> > & ) const;
	void delete_artwork( const std::string & );

	const std::vector<std::string> &get_extensions() const;

	int process_setting( const std::string &setting,
								const std::string &value,
								const std::string &filename );
	void dump( void ) const;

	void save( const std::string &filename ) const;

private:
	std::string m_info[LAST_INDEX];
	std::map<std::string, std::string> m_artwork;
	std::vector<std::string> m_extensions;
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

	const std::string &get_command() const { return m_command; };
	void set_command( const std::string &c ) { m_command=c; };

	bool get_enabled() const { return m_enabled; };
	void set_enabled( bool e ) { m_enabled=e; };

	int process_setting( const std::string &setting,
								const std::string &value,
								const std::string &filename );

	void save( std::ofstream & ) const;

private:
	static const char *indexStrings[];

	std::string m_name;
	std::string m_command;
	bool m_enabled;
};

//
// Class for storing layout/screensaver configuration
//
class FeLayoutInfo : public FeScriptConfigurable
{
public:
	static const char *indexStrings[];

	FeLayoutInfo();
	FeLayoutInfo( const std::string &name );
	const std::string &get_name() const { return m_name; };

	void save( std::ofstream & ) const;

private:
	std::string m_name;
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
