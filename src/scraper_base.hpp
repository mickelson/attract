/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-2019 Andrew Mickelson
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

#ifndef FE_SCRAPER_BASE_HPP
#define FE_SCRAPER_BASE_HPP

#include <string>
#include "fe_romlist.hpp"

typedef bool (*UiUpdate) (void *, int, const std::string &);

class FeImporterContext
{
public:
	FeImporterContext( const FeEmulatorInfo &e, FeRomInfoListType &rl );
	const FeEmulatorInfo &emulator;
	FeRomInfoListType &romlist;
	bool scrape_art;
	UiUpdate uiupdate;
	void *uiupdatedata;
	bool full;
	bool use_net;
	int progress_past;
	int progress_range;
	int download_count;
	std::string user_message;
	std::string out_name;
};

void romlist_console_report( FeRomInfoListType &rl );

//
// Utility function to get strings to use to see if game names match filenames
//
std::string get_fuzzy( const std::string &orig );

std::string get_crc( const std::string &full_path, const std::vector < std::string > &exts );

typedef std::map < std::string, FeRomInfo * > ParentMapType;

void build_parent_map( ParentMapType &parent_map, FeRomInfoListType &romlist, bool prefer_alt_filename );
bool has_same_name_as_parent( FeRomInfo &rom, ParentMapType &parent_map );

#endif
