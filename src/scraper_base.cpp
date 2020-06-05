/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-19 Andrew Mickelson
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

#include "scraper_base.hpp"
#include "fe_util.hpp"
#include "zip.hpp"

#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include "nowide/fstream.hpp"

#include <expat.h>

#include <SFML/System/Clock.hpp>

namespace {

void fix_last_word( std::string &str, int pos )
{
	struct rep_struct { const char *tok; const char *rep; } rep_list[] =
	{
		{ "the", NULL },
		{ "versus", "vs" },
		{ "iv", "4" },
		{ "iii", "3" },
		{ "ii", "2" },
		{ NULL, NULL }
	};

	std::string word = str.substr( pos );

	int j=0;
	while ( rep_list[j].tok != NULL )
	{
		if ( word.compare( rep_list[j].tok ) == 0 )
		{
			str.erase( pos );
			if ( rep_list[j].rep != NULL )
				str += rep_list[j].rep;
			break;
		}
		j++;
	}
}

std::string truncate( FeRomInfo &ri, FeRomInfo::Index idx, size_t width )
{
	const std::string &str = ri.get_info( idx );
	if ( str.length() > width )
		return str.substr(0, width-3) + "...";

	return str;
}

bool correct_buff_for_format( char *&buff, int &size,
	const std::string &filename )
{
	if ( tail_compare( filename, "nes" ) )
	{
		//
		// .nes files: 16 bit header
		// we only want the first prg block
		//
		if (( size <= 16 ) || ( buff[0] != 'N' )
				|| ( buff[1] != 'E' ) || ( buff[2] != 'S' ))
			return false;

		int new_size = 16384 * buff[4];
		bool trainer_present = buff[6] & 0x04;

		int buff_move = 16 + ( trainer_present ? 512 : 0 );
		if ( new_size + buff_move > size )
			return false;

		buff += buff_move;
		size = new_size;
	}

	return true;
}

} // end namespace

std::string get_crc( const std::string &full_path,
	const std::vector<std::string> &exts )
{
	const int MAX_CRC_FILE_SIZE = 10485760; // don't do CRC checks on files more than 10 meg

	if ( is_supported_archive( full_path ) )
	{
		std::vector<std::string> contents;
		if ( !fe_zip_get_dir( full_path.c_str(), contents ) )
			return "";

		// check for extension matches
		std::vector<std::string>::iterator itr;
		for ( itr=contents.begin(); itr != contents.end(); ++itr )
		{
			//
			// Run the crc on this file if there is only one file
			// in the archive or if this file matches one of the
			// supported extensions
			//
			if ( tail_compare( *itr, exts ) || ( contents.size() == 1 ) )
			{
				FeZipStream zs( full_path );
				zs.open( *itr );

				char *buff = zs.getData();
				int size = zs.getSize();

				if ( size > MAX_CRC_FILE_SIZE )
					return "";

				correct_buff_for_format( buff, size, *itr );
				std::string retval = get_crc32( buff, size );
				FeDebug() << "CRC: " << full_path << "=" << retval << std::endl;
				return retval;
			}
		}

		return "";
	}

	nowide::ifstream myfile( full_path.c_str(),
		std::ios_base::in | std::ios_base::binary );

	if ( !myfile.is_open() )
		return "";

	myfile.seekg(0, myfile.end);
	int size = myfile.tellg();
	myfile.seekg(0, myfile.beg);

	if ( size > MAX_CRC_FILE_SIZE )
	{
		myfile.close();
		return "";
	}

	char *buff_ptr = new char[size];
	char *orig_buff_ptr = buff_ptr;

	myfile.read( buff_ptr, size );
	myfile.close();

	correct_buff_for_format( buff_ptr, size, full_path );
	std::string retval = get_crc32( buff_ptr, size );

	delete [] orig_buff_ptr;

	FeDebug() << "CRC: " << full_path << "=" << retval << std::endl;
	return retval;
}

//
// Utility function to get strings to use to see if game names match filenames
//
std::string get_fuzzy( const std::string &orig )
{
	std::string retval;
	int word_start( 0 );
	for ( std::string::const_iterator itr=orig.begin(); (( itr!=orig.end() ) && ( *itr != '(' )); ++itr )
	{
		if ( std::isalnum( *itr ) )
			retval += std::tolower( *itr );
		else
		{
			fix_last_word( retval, word_start );
			word_start=retval.length();
		}
	}
	fix_last_word( retval, word_start );
	return retval;
}

void romlist_console_report( FeRomInfoListType &rl )
{
	FeRomInfoListType::iterator itr;
	for ( itr=rl.begin(); itr!=rl.end(); ++itr )
	{
		FeLog() << " > " << std::left << std::setw( 25 )
			<< truncate( *itr, FeRomInfo::Romname, 25 ) << " ==> "
			<< std::setw( 25 )
			<< truncate( *itr, FeRomInfo::Title, 25 );

		if ( (*itr).get_info( FeRomInfo::BuildScore ).empty() )
			FeLog() << std::endl;
		else
			FeLog() << " [" << std::right << std::setw( 3 )
				<< (*itr).get_info( FeRomInfo::BuildScore ) << "]"
				<< std::endl;
	}
}

void build_parent_map(
	ParentMapType &parent_map,
	FeRomInfoListType &romlist,
	bool prefer_alt_filename )
{
	for ( FeRomInfoListType::iterator itr=romlist.begin(); itr!=romlist.end(); ++itr )
	{
		if ( (*itr).get_info( FeRomInfo::Cloneof ).empty() )
		{
			parent_map[ (*itr).get_info( FeRomInfo::Romname ) ] = &(*itr);

			if (( prefer_alt_filename )
					&& (!(*itr).get_info( FeRomInfo::AltRomname ).empty() ))
				parent_map[ (*itr).get_info( FeRomInfo::AltRomname ) ] = &(*itr);
		}
	}
}

bool has_same_name_as_parent( FeRomInfo &rom, ParentMapType &parent_map )
{
	const std::string &cloneof = rom.get_info( FeRomInfo::Cloneof );
	if ( !cloneof.empty() )
	{
		std::map<std::string, FeRomInfo * >::iterator itm = parent_map.find( cloneof );
		if ( itm != parent_map.end() )
		{
			std::string clone_fuzz = get_fuzzy( rom.get_info( FeRomInfo::Title ) );
			std::string parent_fuzz = get_fuzzy( (*itm).second->get_info( FeRomInfo::Title ) );

			if ( clone_fuzz.compare( parent_fuzz ) == 0 )
				return true;
		}
	}
	return false;
}

FeImporterContext::FeImporterContext( const FeEmulatorInfo &e, FeRomInfoListType &rl )
	: emulator( e ),
	romlist( rl ),
	scrape_art( false ),
	uiupdate( NULL ),
	uiupdatedata( NULL ),
	full( false ),
	use_net( true ),
	progress_past( 0 ),
	progress_range( 100 ),
	download_count( 0 )
{
}
