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

#include "fe_romlist.hpp"
#include "fe_util.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

#include <squirrel.h>
#include <sqstdstring.h>

#include <SFML/System/Clock.hpp>

const char *FE_ROMLIST_FILE_EXTENSION	= ".txt";
const char *FE_FAVOURITE_FILE_EXTENSION = ".tag";

SQRex *FeRomListSorter::m_rex = NULL;

void FeRomListSorter::init_title_rex( const std::string &re_mask )
{
	ASSERT( m_rex == NULL );

	if ( re_mask.empty() )
		return;

	const SQChar *err( NULL );
	m_rex = sqstd_rex_compile(
		(const SQChar *)re_mask.c_str(), &err );

	if ( !m_rex )
		std::cout << "Error compiling regular expression \""
			<< re_mask << "\": " << err << std::endl;
}

void FeRomListSorter::clear_title_rex()
{
	if ( m_rex )
		sqstd_rex_free( m_rex );

	m_rex = NULL;
}

FeRomListSorter::FeRomListSorter( FeRomInfo::Index c, bool rev )
	: m_comp( c ),
	m_reverse( rev )
{
}

bool FeRomListSorter::operator()( const FeRomInfo &one_obj, const FeRomInfo &two_obj ) const
{
	const std::string &one = one_obj.get_info( m_comp );
	const std::string &two = two_obj.get_info( m_comp );

	if (( m_comp == FeRomInfo::Title ) && m_rex )
	{
		size_t one_begin( 0 ), one_len( one.size() ), two_begin( 0 ), two_len( two.size() );

		const SQChar *one_begin_ptr( NULL );
		const SQChar *one_end_ptr( NULL );
		const SQChar *two_begin_ptr( NULL );
		const SQChar *two_end_ptr( NULL );

		//
		// I couldn't get Squirrel's no capture regexp (?:) working the way I would expect it to.
		// I'm probably doing something dumb but I can't figure it out and docs seem nonexistent
		//
		// So we do this kind of backwards, instead of defining what we want to compare based on,
		// the regexp instead defines the part of the string we want to strip out up front
		//
		if ( sqstd_rex_search( m_rex, one.c_str(), &one_begin_ptr, &one_end_ptr ) == SQTrue )
		{
			one_begin = one_end_ptr - one.c_str();
			one_len -= one_begin;
		}

		if ( sqstd_rex_search( m_rex, two.c_str(), &two_begin_ptr, &two_end_ptr ) == SQTrue )
		{
			two_begin = two_end_ptr - two.c_str();
			two_len -= two_begin;
		}

		return ( one.compare( one_begin, one_len, two, two_begin, two_len ) < 0 );
	}
	else if (( m_comp == FeRomInfo::PlayedCount )
				|| ( m_comp == FeRomInfo::PlayedTime ))
	{
		return ( as_int( one ) > as_int( two ) );
	}

	if ( m_reverse )
		return ( one.compare( two ) > 0 );
	else
		return ( one.compare( two ) < 0 );
}

const char FeRomListSorter::get_first_letter( const FeRomInfo *one_info )
{
	if ( !one_info )
		return '0';

	const std::string &name = one_info->get_info( FeRomInfo::Title );
	if ( name.empty() )
		return '0';

	size_t b( 0 );

	if ( m_rex )
	{
		const SQChar *bp;
		const SQChar *ep;
		if ( sqstd_rex_search( m_rex, name.c_str(), &bp, &ep ) == SQTrue )
			b = ep - name.c_str();
	}

	return name.at( b );
}

FeRomList::FeRomList( const std::string &config_path )
	: m_global_filter_ptr( NULL ),
	m_config_path( config_path ),
	m_fav_changed( false ),
	m_tags_changed( false ),
	m_availability_checked( false )
{
}

FeRomList::~FeRomList()
{
}

void FeRomList::init_as_empty_list()
{
	m_user_path.clear();
	m_romlist_name.clear();
	m_list.clear();
	m_filtered_list.clear();
	m_filtered_list.push_back( std::vector< FeRomInfo *>()  ); // there always has to be at least one filter
	m_tags.clear();
	m_availability_checked = false;
	m_fav_changed=false;
	m_tags_changed=false;

	m_global_filter_ptr=NULL;
	m_extra_favs.clear();
	m_extra_tags.clear();
}

void FeRomList::mark_favs_and_tags_changed()
{
	m_fav_changed=true;
	m_tags_changed=true;
}

bool FeRomList::load_romlist( const std::string &path,
	const std::string &romlist_name,
	const std::string &user_path,
	const std::string &stat_path,
	FeDisplayInfo &display )
{
	m_user_path = user_path;
	m_romlist_name = romlist_name;

	m_list.clear();
	m_availability_checked = false;

	m_global_filter_ptr = NULL;
	m_global_filtered_out_count = 0;

	FeFilter *first_filter = display.get_global_filter();
	if ( first_filter )
	{
		first_filter->init();

		if ( !first_filter->test_for_target( FeRomInfo::FileIsAvailable )
			&& !first_filter->test_for_target( FeRomInfo::Favourite )
			&& !first_filter->test_for_target( FeRomInfo::Tags ) )
		{
			// If the global filter doesn't care about file availability,
			// favourites or tags then we can apply it right up front when we
			// load the romlist.  We signal this by setting m_global_filter_ptr
			m_global_filter_ptr = first_filter;
			first_filter = NULL;
		}
	}

	sf::Clock load_timer;

	bool retval = FeBaseConfigurable::load_from_file(
			path + m_romlist_name + FE_ROMLIST_FILE_EXTENSION, ";" );

	//
	// Create rom name to romlist entry lookup map
	//
	std::map < std::string, FeRomInfo * > rom_map;
	std::map < std::string, FeRomInfo * >::iterator rm_itr;
	for ( FeRomInfoListType::iterator itr = m_list.begin(); itr != m_list.end(); ++itr )
		rom_map[ (*itr).get_info( FeRomInfo::Romname ) ] = &(*itr);

	//
	// Load favourites
	//
	m_extra_favs.clear();
	m_fav_changed=false;

	std::string load_name( m_user_path + m_romlist_name + FE_FAVOURITE_FILE_EXTENSION );
	std::ifstream myfile( load_name.c_str() );

	if ( myfile.is_open() )
	{
		while ( myfile.good() )
		{
			size_t pos=0;
			std::string line, name;

			getline( myfile, line );
			token_helper( line, pos, name );

			if ( !name.empty() )
			{
				rm_itr = rom_map.find( name );
				if ( rm_itr != rom_map.end() )
					(*rm_itr).second->set_info( FeRomInfo::Favourite, "1" );
				else
					m_extra_favs.insert( name );
			}
		}

		myfile.close();
	}

	//
	// Load tags
	//
	m_tags.clear();
	m_extra_tags.clear();
	m_tags_changed=false;
	load_name = m_user_path + m_romlist_name + "/";

	if ( directory_exists( load_name ) )
	{
		std::vector<std::string> temp_tags;
		get_basename_from_extension( temp_tags, load_name, FE_FAVOURITE_FILE_EXTENSION );

		for ( std::vector<std::string>::iterator itr=temp_tags.begin(); itr!=temp_tags.end(); ++itr )
		{
			if ( (*itr).empty() )
				continue;

			std::ifstream myfile( std::string(load_name + (*itr) + FE_FAVOURITE_FILE_EXTENSION).c_str() );

			if ( !myfile.is_open() )
				continue;

			std::map<std::string, bool>::iterator itt = m_tags.begin();
			itt = m_tags.insert( itt, std::pair<std::string,bool>( (*itr), false ) );

			while ( myfile.good() )
			{
				size_t pos=0;
				std::string line, rname;

				getline( myfile, line );
				token_helper( line, pos, rname );

				if ( !rname.empty() )
				{
					rm_itr = rom_map.find( rname );
					if ( rm_itr != rom_map.end() )
						(*rm_itr).second->append_tag( (*itt).first.c_str() );
					else
						m_extra_tags.insert( std::pair<std::string,const char*>(rname, (*itt).first.c_str() ) );
				}
			}

			myfile.close();
		}
	}

	// Apply global filter if it hasn't been applied already
	if ( first_filter )
	{
		if ( first_filter->test_for_target( FeRomInfo::FileIsAvailable ) )
			get_file_availability();

		FeRomInfoListType::iterator last_it=m_list.begin();
		for ( FeRomInfoListType::iterator it=m_list.begin(); it!=m_list.end(); )
		{
			if ( first_filter->apply_filter( *it ) )
			{
				if ( last_it != it )
					it = m_list.erase( last_it, it );
				else
					++it;

				last_it = it;
			}
			else
			{
				//
				// This rom is being filtered out and we may need to keep track of a few things
				//
				// 1. Track if this is a favourite...
				//
				if ( !(*it).get_info( FeRomInfo::Favourite ).empty() )
					m_extra_favs.insert( (*it).get_info( FeRomInfo::Romname ) );

				//
				// 2. Track if this rom has tags we'll need to keep
				//
				if ( !(*it).get_info( FeRomInfo::Tags ).empty() )
				{
					const std::string &name = (*it).get_info( FeRomInfo::Romname );
					const std::string &tags = (*it).get_info( FeRomInfo::Tags );
					const char sep[] = { FE_TAGS_SEP, 0 };

					size_t pos=0;
					while ( pos < tags.size() )
					{
						std::string one_tag;
						token_helper( tags, pos, one_tag, sep );

						std::map<std::string, bool>::iterator itt = m_tags.find( one_tag );
						if ( itt != m_tags.end() )
							m_extra_tags.insert( std::pair<std::string,const char *>( name, (*itt).first.c_str() ) );
					}
				}

				m_global_filtered_out_count++;
				++it;
			}
		}

		if ( last_it != m_list.end() )
			m_list.erase( last_it, m_list.end() );
	}

	//
	// make sure stats are loaded now
	//
	if ( !stat_path.empty() )
	{
		for ( FeRomInfoListType::iterator it=m_list.begin(); it!=m_list.end(); ++it )
			(*it).load_stats( stat_path );
	}

	std::cout << " - Loaded master romlist '" << m_romlist_name
			<< "' in " << load_timer.getElapsedTime().asMilliseconds()
			<< " ms (" << m_list.size() << " entries kept, " << m_global_filtered_out_count
			<< " discarded)" << std::endl;

	create_filters( display );
	return retval;
}

void FeRomList::create_filters(
	FeDisplayInfo &display )
{
	sf::Clock load_timer;

	//
	// Apply filters
	//
	int filters_count = display.get_filter_count();

	//
	// If the display doesn't have any filters configured, we create a single "filter" in the romlist object
	// with every romlist entry in it
	//
	if ( filters_count == 0 )
		filters_count = 1;

	m_filtered_list.clear();
	m_filtered_list.reserve( filters_count );

	for ( int i=0; i<filters_count; i++ )
	{
		m_filtered_list.push_back( std::vector< FeRomInfo *>()  );

		FeFilter *f = display.get_filter( i );
		if ( f )
		{
			if ( f->get_size() > 0 ) // if this is non zero then we've loaded before and know how many to expect
				m_filtered_list[i].reserve( f->get_size() );

			if ( f->test_for_target( FeRomInfo::FileIsAvailable ) )
				get_file_availability();

			f->init();
			for ( FeRomInfoListType::iterator itr=m_list.begin(); itr!=m_list.end(); ++itr )
				if ( f->apply_filter( *itr ) )
					m_filtered_list[i].push_back( &( *itr ) );
		}
		else // no filter situation, so we just add the entire list...
		{
			m_filtered_list[i].reserve( m_list.size() );
			for ( FeRomInfoListType::iterator itr=m_list.begin(); itr!=m_list.end(); ++itr )
				m_filtered_list[i].push_back( &( *itr ) );
		}

		if ( f )
		{
			// track the size of the filtered list in our filter info object
			f->set_size( m_filtered_list[i].size() );

			//
			// Sort and/or prune now if configured for this filter
			//
			FeRomInfo::Index sort_by=f->get_sort_by();
			bool rev = f->get_reverse_order();
			int list_limit = f->get_list_limit();

			if ( sort_by != FeRomInfo::LAST_INDEX )
			{
				std::stable_sort( m_filtered_list[i].begin(),
						m_filtered_list[i].end(),
						FeRomListSorter2( sort_by, rev ) );
			}
			else if ( rev != false )
				std::reverse( m_filtered_list[i].begin(), m_filtered_list[i].end() );

			if (( list_limit != 0 ) && ( (int)m_filtered_list[i].size() > abs( list_limit ) ))
			{
				if ( list_limit > 0 )
					m_filtered_list[i].erase( m_filtered_list[i].begin() + list_limit, m_filtered_list[i].end() );
				else
					m_filtered_list[i].erase( m_filtered_list[i].begin(), m_filtered_list[i].end() + list_limit );
			}
		}
	}

	std::cout << " - Constructed " << filters_count << " filters in "
			<< load_timer.getElapsedTime().asMilliseconds()
			<< " ms (" << filters_count * m_list.size() << " comparisons)" << std::endl;
}

int FeRomList::process_setting( const std::string &setting,
				const std::string &value,
				const std::string &fn )
{
	FeRomInfo next_rom( setting );
	next_rom.process_setting( setting, value, fn );

	if (( !m_global_filter_ptr ) || ( m_global_filter_ptr->apply_filter( next_rom ) ))
		m_list.push_back( next_rom );
	else
		m_global_filtered_out_count++;

   return 0;
}

void FeRomList::save_state()
{
	save_favs();
	save_tags();
}

void FeRomList::save_favs()
{
	if (( !m_fav_changed ) || ( m_user_path.empty() ) || ( m_romlist_name.empty() ))
		return;

	//
	// First gather all the favourites from the current list into our extra favs list
	//
	for ( FeRomInfoListType::const_iterator itr = m_list.begin(); itr != m_list.end(); ++itr )
	{
		if ( !((*itr).get_info( FeRomInfo::Favourite ).empty()) )
			m_extra_favs.insert( (*itr).get_info( FeRomInfo::Romname ) );
	}

	//
	// Now save the contents of favs list
	//
	std::string fname = m_user_path + m_romlist_name + FE_FAVOURITE_FILE_EXTENSION;

	if ( m_extra_favs.empty() )
	{
		delete_file( fname );
	}
	else
	{
		std::ofstream outfile( fname.c_str() );
		if ( !outfile.is_open() )
			return;

		for ( std::set<std::string>::const_iterator itf = m_extra_favs.begin(); itf != m_extra_favs.end(); ++itf )
			outfile << (*itf) << std::endl;

		outfile.close();
	}
}

void FeRomList::save_tags()
{
	if (( !m_tags_changed ) || ( m_user_path.empty() ) || ( m_romlist_name.empty() ))
		return;

	// First construct a mapping of tags to rom entries
	//
	std::multimap< std::string, const char * > tag_to_rom_map;

	for ( std::multimap< std::string, const char * >::const_iterator ite = m_extra_tags.begin(); ite != m_extra_tags.end(); ++ite )
	{
		tag_to_rom_map.insert(
				std::pair<std::string,const char *>(
						(*ite).second,
						(*ite).first.c_str() ) );
	}

	for ( FeRomInfoListType::const_iterator itr = m_list.begin(); itr != m_list.end(); ++itr )
	{
		const std::string &my_tags = (*itr).get_info( FeRomInfo::Tags );

		size_t pos=0;
		do
		{
			std::string one_tag;
			const char sep[] = { FE_TAGS_SEP, 0 };
			token_helper( my_tags, pos, one_tag, sep );

			if ( !one_tag.empty() )
			{
				tag_to_rom_map.insert(
						std::pair<std::string,const char *>(
								one_tag,
								((*itr).get_info( FeRomInfo::Romname )).c_str() ) );
			}
		} while ( pos < my_tags.size() );
	}

	confirm_directory( m_user_path, m_romlist_name );
	std::string my_path = m_user_path + m_romlist_name + "/";

	//
	// Now save the tags
	//
	std::map<std::string,bool>::const_iterator itt;
	for ( itt = m_tags.begin(); itt != m_tags.end(); ++itt )
	{
		if ( (*itt).second == false )
			continue;

		std::string file_name = my_path + (*itt).first + FE_FAVOURITE_FILE_EXTENSION;

		std::pair<std::multimap<std::string,const char *>::const_iterator,std::multimap<std::string,const char *>::const_iterator> ret;
		ret = tag_to_rom_map.equal_range( (*itt).first );
		if ( ret.first == ret.second )
		{
			delete_file( file_name );
		}
		else
		{
			std::ofstream outfile( file_name.c_str() );
			if ( !outfile.is_open() )
				continue;

			for ( std::multimap<std::string,const char *>::const_iterator ito = ret.first; ito != ret.second; ++ito )
				outfile << (*ito).second << std::endl;

			outfile.close();
		}
	}
}

bool FeRomList::set_fav( FeRomInfo &r, FeDisplayInfo &display, bool fav )
{
	r.set_info( FeRomInfo::Favourite, fav ? "1" : "" );
	m_fav_changed=true;

	return fix_filters( display, FeRomInfo::Favourite );
}

void FeRomList::get_tags_list( FeRomInfo &rom,
		std::vector< std::pair<std::string, bool> > &tags_list ) const
{
	std::string curr_tags = rom.get_info(FeRomInfo::Tags);

	std::set<std::string> my_set;
	size_t pos=0;
	do
	{
		std::string one_tag;
		const char sep[] = { FE_TAGS_SEP, 0 };
		token_helper( curr_tags, pos, one_tag, sep );
		if ( !one_tag.empty() )
		{
			my_set.insert( one_tag );
		}
	} while ( pos < curr_tags.size() );

	for ( std::map<std::string, bool>::const_iterator itr=m_tags.begin(); itr!=m_tags.end(); ++itr )
	{
		tags_list.push_back(
				std::pair<std::string, bool>((*itr).first,
						( my_set.find( (*itr).first ) != my_set.end() ) ) );
	}
}

bool FeRomList::set_tag( FeRomInfo &rom, FeDisplayInfo &display, const std::string &tag, bool flag )
{
	std::string curr_tags = rom.get_info(FeRomInfo::Tags);
	size_t pos = curr_tags.find( FE_TAGS_SEP + tag + FE_TAGS_SEP );

	std::map<std::string, bool>::iterator itt = m_tags.begin();

	if ( flag == true )
	{
		if ( pos == std::string::npos )
		{
			rom.append_tag( tag );
			m_tags_changed = true;

			itt = m_tags.find( tag );
			if ( itt != m_tags.end() )
				(*itt).second = true;
			else
				itt = m_tags.insert( itt, std::pair<std::string,bool>( tag, true ) );
		}
	}
	else if ( pos != std::string::npos )
	{
		pos++; // because we searched for the preceeding FE_TAGS_SEP as well
		int len = tag.size();

		if (( ( pos + len ) < curr_tags.size() )
				&& ( curr_tags.at( pos + len ) == FE_TAGS_SEP ))
			len++;

		curr_tags.erase( pos, len );

		//
		// Clean up our leading FE_TAGS_SEP if there are no tags left
		//
		if (( curr_tags.size() == 1 ) && (curr_tags[0] == FE_TAGS_SEP ))
			curr_tags.clear();

		rom.set_info( FeRomInfo::Tags, curr_tags );
		m_tags_changed = true;

		itt = m_tags.find( tag );
		if ( itt != m_tags.end() )
			(*itt).second = true;
		else
			itt = m_tags.insert( itt, std::pair<std::string,bool>( tag, true ) );
	}

	return fix_filters( display, FeRomInfo::Tags );
}

bool FeRomList::fix_filters( FeDisplayInfo &display, FeRomInfo::Index target )
{
	bool retval = false;
	for ( int i=0; i<display.get_filter_count(); i++ )
	{
		FeFilter *f = display.get_filter( i );
		ASSERT( f );

		if ( f->test_for_target( target ) )
		{
			m_filtered_list[i].clear();
			retval = true;

			for ( FeRomInfoListType::iterator itr=m_list.begin(); itr!=m_list.end(); ++itr )
			{
				if ( f->apply_filter( *itr ) )
					m_filtered_list[i].push_back( &( *itr ) );
			}
		}
	}

	return retval;
}

void FeRomList::get_file_availability()
{
	if ( m_availability_checked )
		return;

	m_availability_checked = true;

	std::map<std::string,std::vector<FeRomInfo *> > emu_map;

	for ( FeRomInfoListType::iterator itr=m_list.begin(); itr != m_list.end(); ++itr )
		emu_map[ ((*itr).get_info( FeRomInfo::Emulator )) ].push_back( &(*itr) );

	// figure out what roms we have for each emulator
	for ( std::map<std::string,std::vector<FeRomInfo *> >::iterator ite=emu_map.begin();
					ite != emu_map.end(); ++ite )
	{
		FeEmulatorInfo *emu = get_emulator( (*ite).first );
		if ( emu )
		{
			std::vector<std::string> name_vector;
			emu->gather_rom_names( name_vector );

			std::set<std::string> name_set;
			for ( std::vector<std::string>::iterator itv=name_vector.begin(); itv!=name_vector.end(); ++itv )
				name_set.insert( *itv );

			for ( std::vector<FeRomInfo *>::iterator itp=(*ite).second.begin(); itp!=(*ite).second.end(); ++itp )
			{
				if ( name_set.find( (*itp)->get_info( FeRomInfo::Romname ) ) != name_set.end() )
					(*itp)->set_info( FeRomInfo::FileIsAvailable, "1" );
			}
		}
	}
}

// NOTE: this function is implemented in fe_settings.cpp
bool internal_resolve_config_file(
						const std::string &config_path,
						std::string &result,
						const char *subdir,
						const std::string &name  );


FeEmulatorInfo *FeRomList::get_emulator( const std::string & emu )
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
	std::string filename;
	if ( internal_resolve_config_file( m_config_path, filename, FE_EMULATOR_SUBDIR, emu + FE_EMULATOR_FILE_EXTENSION ) )
	{
		FeEmulatorInfo new_emu( emu );
		if ( new_emu.load_from_file( filename ) )
		{
			m_emulators.push_back( new_emu );
			return &(m_emulators.back());
		}
	}

	// Could not find emulator config
	return NULL;
}

FeEmulatorInfo *FeRomList::create_emulator( const std::string &emu )
{
	// If an emulator with the given name already exists we return it
	//
	FeEmulatorInfo *tmp = get_emulator( emu );
	if ( tmp != NULL )
		return tmp;

	//
	// Fill in with default values if there is a "default" emulator
	//
	FeEmulatorInfo new_emu( emu );

	std::string defaults_file;
	if ( internal_resolve_config_file( m_config_path, defaults_file, NULL, FE_EMULATOR_DEFAULT ) )
	{
		new_emu.load_from_file( defaults_file );

		//
		// Find and replace the [emulator] token, replace with the specified
		// name.  This is only done in the path fields.  It is not done for
		// the FeEmulator::Command field.
		//
		const char *EMU_TOKEN = "[emulator]";

		std::string temp = new_emu.get_info( FeEmulatorInfo::Rom_path );
		if ( perform_substitution( temp, EMU_TOKEN, emu ) )
			new_emu.set_info( FeEmulatorInfo::Rom_path, temp );

		std::vector<std::pair<std::string,std::string> > al;
		std::vector<std::pair<std::string,std::string> >::iterator itr;
		new_emu.get_artwork_list( al );
		for ( itr=al.begin(); itr!=al.end(); ++itr )
		{
			std::string temp = (*itr).second;
			if ( perform_substitution( temp, EMU_TOKEN, emu ) )
			{
				new_emu.delete_artwork( (*itr).first );
				new_emu.add_artwork( (*itr).first, temp );
			}
		}
	}

	m_emulators.push_back( new_emu );
	return &(m_emulators.back());
}

void FeRomList::delete_emulator( const std::string & emu )
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
