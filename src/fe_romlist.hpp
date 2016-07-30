/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-2016 Andrew Mickelson
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

#ifndef FE_ROMLIST_HPP
#define FE_ROMLIST_HPP

#include "fe_info.hpp"

#include <map>
#include <set>
#include <list>

typedef std::list<FeRomInfo> FeRomInfoListType;
extern const char *FE_ROMLIST_FILE_EXTENSION;

//
// Comparison used when sorting/merging FeRomLists
//
class FeRomListSorter
{
private:
	FeRomInfo::Index m_comp;
	bool m_reverse;
	static SQRex *m_rex;

public:
	FeRomListSorter( FeRomInfo::Index c = FeRomInfo::Title, bool rev=false );

	bool operator()( const FeRomInfo &obj1, const FeRomInfo &obj2 ) const;

	const char get_first_letter( const FeRomInfo *one );

	static void init_title_rex( const std::string & );
	static void clear_title_rex();
};

class FeRomListSorter2
{
private:
	FeRomListSorter m_sorter;

public:
	FeRomListSorter2( FeRomInfo::Index c = FeRomInfo::Title, bool rev=false ) : m_sorter( c, rev ) {};
	bool operator()( const FeRomInfo *one, const FeRomInfo *two ) const { return m_sorter.operator()(*one,*two); };
};

class FeRomList : public FeBaseConfigurable
{
private:
	FeRomInfoListType m_list; // this is where we keep the info on all the games available for the current display
	std::vector<std::vector<FeRomInfo * > > m_filtered_list; // for each filter, store a pointer to the m_list entries in that filter
	std::vector<FeEmulatorInfo> m_emulators; // we keep the emulator info here because we need it for checking file availability

	std::map<std::string, bool> m_tags; // bool is flag of whether the tag has been changed
	std::set<std::string> m_extra_favs; // store for favourites that are filtered out by global filter
	std::multimap< std::string, const char * > m_extra_tags; // store for tags that are filtered out by global filter
	FeFilter *m_global_filter_ptr; // this will only get set if we are globally filtering out games during the initial load

	std::string m_user_path;
	std::string m_romlist_name;
	const std::string &m_config_path;
	bool m_fav_changed;
	bool m_tags_changed;
	bool m_availability_checked;
	int m_global_filtered_out_count; // for keeping stats during load

	FeRomList( const FeRomList & );
	FeRomList &operator=( const FeRomList & );

	// Fixes m_filtered_list as needed using the filters in the given "display", with the
	// assumption that the specified "target" attribute for all games might have been changed
	//
	// returns true if list changes might have been made
	//
	bool fix_filters( FeDisplayInfo &display, FeRomInfo::Index target );

	void save_favs();
	void save_tags();

public:
	FeRomList( const std::string &config_path );
	~FeRomList();

	void init_as_empty_list();

	bool load_romlist( const std::string &romlist_path,
		const std::string &romlist_name,
		const std::string &user_path,
		const std::string &stat_path,
		FeDisplayInfo &display );

	void create_filters( FeDisplayInfo &display ); // called by load_romlist()

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &fn );

	void mark_favs_and_tags_changed();
	void save_state();
	bool set_fav( FeRomInfo &rom, FeDisplayInfo &display, bool fav );

	void get_tags_list( FeRomInfo &rom,
		std::vector< std::pair<std::string, bool> > &tags_list ) const;
	bool set_tag( FeRomInfo &rom, FeDisplayInfo &display, const std::string &tag, bool flag );

	bool is_filter_empty( int filter_idx ) const { return m_filtered_list[filter_idx].empty(); };
	int filter_size( int filter_idx ) const { return (int)m_filtered_list[filter_idx].size(); };
	const FeRomInfo &lookup( int filter_idx, int idx) const { return *(m_filtered_list[filter_idx][idx]); };
	FeRomInfo &lookup( int filter_idx, int idx) { return *(m_filtered_list[filter_idx][idx]); };

	FeRomInfoListType &get_list() { return m_list; };

	void get_file_availability();

	FeEmulatorInfo *get_emulator( const std::string & );
	FeEmulatorInfo *create_emulator( const std::string & );
	void delete_emulator( const std::string & );
	void clear_emulators() { m_emulators.clear(); }
};

#endif
