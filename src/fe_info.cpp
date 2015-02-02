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

#include "fe_info.hpp"
#include "fe_input.hpp"
#include "fe_util.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <squirrel.h>
#include <sqstdstring.h>

#include <SFML/System/Clock.hpp>

const char *FE_ROMLIST_FILE_EXTENSION	= ".txt";
const char *FE_FAVOURITE_FILE_EXTENSION = ".tag";
const char *FE_STAT_FILE_EXTENSION = ".stat";
const char FE_TAGS_SEP = ';';

const FeRomInfo::Index FeRomInfo::BuildScratchPad = FeRomInfo::Category;

const char *FeRomInfo::indexStrings[] =
{
	"Name",
	"Title",
	"Emulator",
	"CloneOf",
	"Year",
	"Manufacturer",
	"Category",
	"Players",
	"Rotation",
	"Control",
	"Status",
	"DisplayCount",
	"DisplayType",
	"AltRomname",
	"AltTitle",
	"Extra",
	"Favourite",
	"Tags",
	"PlayedCount",
	"PlayedTime",
	"FileIsAvailable",
	NULL
};

FeRomInfo::FeRomInfo()
{
}

FeRomInfo::FeRomInfo( const std::string &rn )
{
	m_info[Romname] = rn;
}

const std::string &FeRomInfo::get_info( int i ) const
{
	return m_info[i];
}

std::string FeRomInfo::get_info_escaped( int i ) const
{
	if ( m_info[i].find_first_of( ';' ) != std::string::npos )
	{
		std::string temp = m_info[i];
		perform_substitution( temp, "\"", "\\\"" );
		return ( "\"" + temp + "\"" );
	}
	else
		return m_info[i];
}

void FeRomInfo::set_info( Index i, const std::string &v )
{
	m_info[i] = v;
}

void FeRomInfo::append_tag( const std::string &tag )
{
	//
	// The tags logic requires a FE_TAGS_SEP character on each side of
	// a tag.
	//
	if ( m_info[Tags].empty() )
		m_info[Tags] = FE_TAGS_SEP;

	m_info[Tags] += tag;
	m_info[Tags] += FE_TAGS_SEP;
}

void FeRomInfo::load_stats( const std::string &path )
{
	// Check if stats already loaded for this one
	if ( !m_info[PlayedCount].empty() )
		return;

	m_info[PlayedCount] = "0";
	m_info[PlayedTime] = "0";

	std::string filename = path + m_info[Romname] + FE_STAT_FILE_EXTENSION;
	std::ifstream myfile( filename.c_str() );

	if ( !myfile.is_open() )
		return;

	std::string line;
	if ( myfile.good() )
	{
		getline( myfile, line );
		m_info[PlayedCount] = line;
	}

	if ( myfile.good() )
	{
		getline( myfile, line );
		m_info[PlayedTime] = line;
	}

	myfile.close();
}

void FeRomInfo::update_stats( const std::string &path, int count_incr, int played_incr )
{
	int new_count = as_int( m_info[PlayedCount] ) + count_incr;
	int new_time = as_int( m_info[PlayedTime] ) + played_incr;

	m_info[PlayedCount] = as_str( new_count );
	m_info[PlayedTime] = as_str( new_time );

	std::string filename = path + m_info[Romname] + FE_STAT_FILE_EXTENSION;
	std::ofstream myfile( filename.c_str() );

	if ( !myfile.is_open() )
	{
		std::cerr << "Error writing stat file: " << filename << std::endl;
		return;
	}

	myfile << m_info[PlayedCount] << std::endl << m_info[PlayedTime] << std::endl;
	myfile.close();
}

int FeRomInfo::process_setting( const std::string &,
         const std::string &value, const std::string &fn )
{
	size_t pos=0;
	std::string token;

	for ( int i=1; i < Favourite; i++ )
	{
		token_helper( value, pos, token );
		m_info[(Index)i] = token;
	}

	return 0;
}

std::string FeRomInfo::as_output( void ) const
{
	std::string s = get_info_escaped( (Index)0 );
	for ( int i=1; i < Favourite; i++ )
	{
		s += ';';
		s += get_info_escaped( (Index)i );
	}

	return s;
}

void FeRomInfo::clear()
{
	for ( int i=0; i < LAST_INDEX; i++ )
		m_info[i].clear();
}

bool FeRomInfo::operator==( const FeRomInfo &o ) const
{
	return (( m_info[Romname].compare( o.m_info[Romname] ) == 0 )
				&& ( m_info[Emulator].compare( o.m_info[Emulator] ) == 0 ));
}

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

const char FeRomListSorter::get_first_letter( const FeRomInfo &one_info )
{
	const std::string &name = one_info.get_info( FeRomInfo::Title );
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

const char *FeRule::indexString = "rule";

const char *FeRule::filterCompStrings[] =
{
	"equals",
	"not_equals",
	"contains",
	"not_contains",
	NULL
};

const char *FeRule::filterCompDisplayStrings[] =
{
	"equals",
	"does not equal",
	"contains",
	"does not contain",
	NULL
};

FeRule::FeRule( FeRomInfo::Index t, FilterComp c, const std::string &w )
	: m_filter_target( t ),
	m_filter_comp( c ),
	m_filter_what( w ),
	m_rex( NULL )
{
}

FeRule::FeRule( const FeRule &r )
	: m_filter_target( r.m_filter_target ),
	m_filter_comp( r.m_filter_comp ),
	m_filter_what( r.m_filter_what ),
	m_rex( NULL )
{
}

FeRule::~FeRule()
{
	if ( m_rex )
		sqstd_rex_free( m_rex );
}

FeRule &FeRule::operator=( const FeRule &r )
{
	m_filter_target = r.m_filter_target;
	m_filter_comp = r.m_filter_comp;
	m_filter_what = r.m_filter_what;

	if ( m_rex )
		sqstd_rex_free( m_rex );

	m_rex = NULL;
	return *this;
}

void FeRule::init()
{
	if (( m_rex ) || ( m_filter_what.empty() ))
		return;

	//
	// Compile the regular expression now
	//
	const SQChar *err( NULL );
	m_rex = sqstd_rex_compile(
		(const SQChar *)m_filter_what.c_str(), &err );

	if ( !m_rex )
		std::cout << "Error compiling regular expression \""
			<< m_filter_what << "\": " << err << std::endl;
}

bool FeRule::apply_rule( const FeRomInfo &rom ) const
{
	if (( m_filter_target == FeRomInfo::LAST_INDEX )
		|| ( m_filter_comp == FeRule::LAST_COMPARISON )
		|| ( m_rex == NULL ))
		return true;

	const SQChar *begin( NULL );
	const SQChar *end( NULL );
	const std::string &target = rom.get_info( m_filter_target );

	switch ( m_filter_comp )
	{
	case FilterEquals:
		if ( target.empty() )
			return ( m_filter_what.empty() );

		return ( sqstd_rex_match(
					m_rex,
					(const SQChar *)target.c_str() ) == SQTrue );

	case FilterNotEquals:
		if ( target.empty() )
			return ( !m_filter_what.empty() );

		return ( sqstd_rex_match(
					m_rex,
					(const SQChar *)target.c_str() ) != SQTrue );

	case FilterContains:
		if ( target.empty() )
			return false;

		return ( sqstd_rex_search(
					m_rex,
					(const SQChar *)target.c_str(),
					&begin,
					&end ) == SQTrue );

	case FilterNotContains:
		if ( target.empty() )
			return true;

		return ( sqstd_rex_search(
					m_rex,
					(const SQChar *)target.c_str(),
					&begin,
					&end ) != SQTrue );

	default:
		return true;
	}
}

void FeRule::save( std::ofstream &f ) const
{
	if (( m_filter_target != FeRomInfo::LAST_INDEX )
		&& ( m_filter_comp != LAST_COMPARISON ))
	{
		f << "\t\t" << std::setw(20) << std::left << indexString << ' '
			<< FeRomInfo::indexStrings[ m_filter_target ] << ' '
			<< filterCompStrings[ m_filter_comp ] << ' '
			<< m_filter_what << std::endl;
	}
}

void FeRule::set_values(
		FeRomInfo::Index i,
		FilterComp c,
		const std::string &w )
{
	if ( m_rex )
		sqstd_rex_free( m_rex );

	m_rex = NULL;

	m_filter_target = i;
	m_filter_comp = c;
	m_filter_what = w;
}

const char *FeFilter::indexStrings[] =
{
	"filter",
	"sort_by",
	"reverse_order",
	"list_limit",
	NULL
};

FeFilter::FeFilter( const std::string &name )
	: m_name( name ),
	m_rom_index( 0 ),
	m_list_limit( 0 ),
	m_size( 0 ),
	m_sort_by( FeRomInfo::LAST_INDEX ),
	m_reverse_order( false )
{
}

void FeFilter::init()
{
	for ( std::vector<FeRule>::iterator itr=m_rules.begin();
			itr != m_rules.end(); ++itr )
		(*itr).init();
}

bool FeFilter::apply_filter( const FeRomInfo &rom ) const
{
	for ( std::vector<FeRule>::const_iterator itr=m_rules.begin();
		itr != m_rules.end(); ++itr )
	{
		if ( (*itr).apply_rule( rom ) == false )
			return false;
	}

	return true;
}

int FeFilter::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{
	if ( value.empty() )
	{
		m_rules.push_back(
			FeRule( FeRomInfo::LAST_INDEX, FeRule::LAST_COMPARISON, "" )
			);
		return 0;
	}

	FeRomInfo::Index target;
	FeRule::FilterComp comp;
	std::string what;

	std::string token;
	size_t pos=0;

	token_helper( value, pos, token, FE_WHITESPACE );

	int i=0;
	while( FeRomInfo::indexStrings[i] != NULL )
	{
		if ( token.compare( FeRomInfo::indexStrings[i] ) == 0 )
			break;
		i++;
	}

	if ( i >= FeRomInfo::LAST_INDEX )
	{
		invalid_setting( fn, "filter", token,
				FeRomInfo::indexStrings, NULL, "target" );
		return 1;
	}

	target = (FeRomInfo::Index)i;
	token_helper( value, pos, token, FE_WHITESPACE );

	i=0;
	while( FeRule::filterCompStrings[i] != NULL )
	{
		if ( token.compare( FeRule::filterCompStrings[i] ) == 0 )
			break;
		i++;
	}

	if ( i >= FeRule::LAST_COMPARISON )
	{
		invalid_setting( fn, "filter", token, FeRule::filterCompStrings,
				NULL, "comparison" );
		return 1;
	}

	comp = (FeRule::FilterComp)i;

	// Remainder of the line is filter regular expression (which may contain
	// spaces...)
	//
	if ( pos < value.size() )
		what = value.substr( pos );

	m_rules.push_back( FeRule( target, comp, what ) );
	return 0;
}

void FeFilter::save( std::ofstream &f ) const
{
	std::string n;
	if ( m_name.find_first_of( ' ' ) != std::string::npos )
		n = '"' + m_name + '"';
	else
		n = m_name;

	f << '\t' << std::setw(20) << std::left
		<< indexStrings[0] << ' ' << n << std::endl;

	if ( m_sort_by != FeRomInfo::LAST_INDEX )
	{
		f << "\t\t" << std::setw(20) << std::left
			<< indexStrings[1] << ' ' << FeRomInfo::indexStrings[ m_sort_by ] << std::endl;
	}

	if ( m_reverse_order != false )
	{
		f << "\t\t" << std::setw(20) << std::left
			<< indexStrings[2] << " true" << std::endl;
	}

	if ( m_list_limit != 0 )
	{
		f << "\t\t" << std::setw(20) << std::left
			<< indexStrings[3] << " " << as_str( m_list_limit ) << std::endl;
	}

	for ( std::vector<FeRule>::const_iterator itr=m_rules.begin();
			itr != m_rules.end(); ++itr )
		(*itr).save( f );
}

bool FeFilter::test_for_target( FeRomInfo::Index target ) const
{
	for ( std::vector<FeRule>::const_iterator itr=m_rules.begin(); itr!=m_rules.end(); ++itr )
	{
		if ( (*itr).get_target() == target )
			return true;
	}

	return false;
}

const char *FeListInfo::indexStrings[] =
{
	"name",
	"layout",
	"romlist",
	NULL
};

FeListInfo::FeListInfo( const std::string &n )
	: m_rom_index( 0 ),
	m_filter_index( 0 )
{
	m_info[ Name ] = n;
}

const std::string &FeListInfo::get_info( int i ) const
{
	return m_info[i];
}

int FeListInfo::get_rom_index( int filter_index ) const
{
	if (( filter_index >= 0 ) && ( filter_index < (int)m_filters.size() ))
		return m_filters[ filter_index ].get_rom_index();

	return m_rom_index;
}

void FeListInfo::set_rom_index( int filter_index, int rom_index )
{
	if (( filter_index >= 0 ) && ( filter_index < (int)m_filters.size() ))
		m_filters[ filter_index ].set_rom_index( rom_index );
	else
		m_rom_index = rom_index;
}

std::string FeListInfo::get_current_layout_file() const
{
	return m_current_layout_file;
}

void FeListInfo::set_current_layout_file( const std::string &n )
{
	m_current_layout_file = n;
}

void FeListInfo::set_info( int setting,
         const std::string &value )
{
	m_info[ setting ] = value;
}

int FeListInfo::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{
	// name is igored here, it gets set directly
	//
	if ( setting.compare( indexStrings[Layout] ) == 0 ) // layout
		m_info[ Layout ] = value;
	else if ( setting.compare( indexStrings[Romlist] ) == 0 ) // romlist
		m_info[ Romlist ] = value;
	else if ( setting.compare( FeFilter::indexStrings[0] ) == 0 ) // filter
	{
		size_t pos=0;
		std::string name;
		token_helper( value, pos, name, FE_WHITESPACE );

		// Create a new filter with the given name
		//
		m_filters.push_back( FeFilter( name ) );
	}
	else if ( setting.compare( FeFilter::indexStrings[1] ) == 0 ) // (filter) sort_by
	{
		for ( int i=0; i < FeRomInfo::LAST_INDEX; i++ )
		{
			if ( value.compare( FeRomInfo::indexStrings[i] ) == 0 )
			{
				m_filters.back().set_sort_by( (FeRomInfo::Index)i );
				break;
			}
		}
	}
	else if ( setting.compare( FeFilter::indexStrings[2] ) == 0 ) // (filter) reverse_order
	{
		m_filters.back().set_reverse_order( true );
	}
	else if ( setting.compare( FeFilter::indexStrings[3] ) == 0 ) // (filter) list_limit
	{
		m_filters.back().set_list_limit( as_int( value ) );
	}
	else if ( setting.compare( FeRule::indexString ) == 0 ) // (filter) rule
	{
		if ( !m_filters.empty() )
			m_filters.back().process_setting( setting, value, fn );
	}
	else
	{
		invalid_setting( fn, "list", setting, indexStrings + 1 );
		return 1;
	}

	return 0;
}

int FeListInfo::process_state( const std::string &state_string )
{
	// state string is in format:
	//
	// "[curr_rom];[curr_layout_filename];[curr_filter];"
	//
	// With [curr_rom] = "[rom_index filter0],[rom_index filter1],..."
	//
	size_t pos=0;
	std::string val;

	token_helper( state_string, pos, val );

	//
	// Process the [curr_rom] string
	//
	if ( m_filters.empty() )
	{
		// If there are no filters we stash the current rom in m_rom_index
		m_rom_index = as_int( val );
	}
	else
	{
		// If there are filters we get a current rom for each filter
		size_t sub_pos=0;
		int findex=0;
		do
		{
			std::string sub_val;
			token_helper( val, sub_pos, sub_val, "," );
			m_filters[findex].set_rom_index( as_int( sub_val ) );
			findex++;
		} while ( sub_pos < val.size() );
	}

	if ( pos >= state_string.size() )
		return 0;

	token_helper( state_string, pos, val );
	m_current_layout_file = val;

	if ( pos >= state_string.size() )
		return 0;

	token_helper( state_string, pos, val );
	m_filter_index = as_int( val );

	return 0;
}

std::string FeListInfo::state_as_output() const
{
	std::ostringstream state;

	if ( m_filters.empty() )
	{
		state << m_rom_index;
	}
	else
	{
		for ( std::deque<FeFilter>::const_iterator itr=m_filters.begin();
			itr != m_filters.end(); ++itr )
		{
			state << (*itr).get_rom_index() << ",";
		}
	}

	state << ";" << m_current_layout_file << ";"
		<< m_filter_index << ";";

	return state.str();
}

FeFilter *FeListInfo::get_filter( int i )
{
	if (( i >= 0 ) && ( i < (int)m_filters.size() ))
		return &(m_filters[i]);

	return NULL;
}

void FeListInfo::append_filter( const FeFilter &f )
{
	m_filters.push_back( f );
}

void FeListInfo::delete_filter( int i )
{
	if (( m_filter_index > 0 ) && ( m_filter_index >= i ))
		m_filter_index--;

	m_filters.erase( m_filters.begin() + i );
}

void FeListInfo::get_filters_list( std::vector<std::string> &l ) const
{
	l.clear();

	for ( std::deque<FeFilter>::const_iterator itr=m_filters.begin();
			itr != m_filters.end(); ++itr )
	{
		l.push_back( (*itr).get_name() );
	}
}

void FeListInfo::save( std::ofstream &f ) const
{
	using std::setw;
	using std::left;
	using std::endl;

	f << "list" << '\t' << get_info( Name ) << endl;

	if ( !get_info( Layout ).empty() )
		f << '\t' << setw(20) << left
			<< indexStrings[Layout] << ' ' << get_info( Layout ) << endl;

	if ( !get_info( Romlist ).empty() )
		f << '\t' << setw(20) << left
			<< indexStrings[Romlist] << ' ' << get_info( Romlist ) << endl;

	for ( std::deque<FeFilter>::const_iterator itr=m_filters.begin();
			itr != m_filters.end(); ++itr )
		(*itr).save( f );
}

FeRomList::FeRomList( const std::string &config_path )
	: m_config_path( config_path ),
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
}

bool FeRomList::load_romlist( const std::string &path,
			const std::string &romlist_name,
			const std::string &user_path,
			const std::string &stat_path,
			FeListInfo &list_info )
{
	m_user_path = user_path;
	m_romlist_name = romlist_name;

	m_list.clear();
	m_filtered_list.clear();
	m_availability_checked = false;

	bool retval = FeBaseConfigurable::load_from_file(
			path + m_romlist_name + FE_ROMLIST_FILE_EXTENSION, ";" );

	std::cout << " - Loaded master romlist: " << m_romlist_name
			<< " (" << m_list.size() << " entries)" << std::endl;

	//
	// Create rom name to romlist entry lookup map
	//
	std::map < std::string, FeRomInfo * > rom_map;
	std::map < std::string, FeRomInfo * >::iterator rm_itr;
	for ( std::deque< FeRomInfo >::iterator itr = m_list.begin(); itr != m_list.end(); ++itr )
		rom_map[ (*itr).get_info( FeRomInfo::Romname ) ] = &(*itr);

	//
	// Load favourites
	//
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
			}
		}

		myfile.close();
	}

	//
	// Load tags
	//
	m_tags.clear();
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
				}
			}

			myfile.close();
		}
	}

	//
	// Apply filters
	//
	int filters_count = list_info.get_filter_count();

	//
	// If the list_info doesn't have any filters configured, we create a single "filter" in the romlist object
	// with every romlist entry in it
	//
	if ( filters_count == 0 )
		filters_count = 1;

	sf::Clock filter_timer;

	m_filtered_list.reserve( filters_count );
	for ( int i=0; i<filters_count; i++ )
	{
		m_filtered_list.push_back( std::vector< FeRomInfo *>()  );

		FeFilter *f = list_info.get_filter( i );
		if ( f )
		{
			if ( f->get_size() > 0 ) // if this is non zero then we've loaded before and know how many to expect
				m_filtered_list[i].reserve( f->get_size() );

			if ( f->test_for_target( FeRomInfo::FileIsAvailable ) )
				get_file_availability();

			f->init();
			for ( std::deque< FeRomInfo >::iterator itr=m_list.begin(); itr!=m_list.end(); ++itr )
				if ( f->apply_filter( *itr ) )
					m_filtered_list[i].push_back( &( *itr ) );
		}
		else // no filter situation, so we just add the entire list...
		{
			m_filtered_list[i].reserve( m_list.size() );
			for ( std::deque< FeRomInfo >::iterator itr=m_list.begin(); itr!=m_list.end(); ++itr )
				m_filtered_list[i].push_back( &( *itr ) );
		}

		//
		// make sure stats are loaded for the roms we will be showing
		//
		if ( !stat_path.empty() )
		{
			for ( std::vector< FeRomInfo * >::iterator itf=m_filtered_list[i].begin();
						itf!=m_filtered_list[i].end();
						++itf )
				(*itf)->load_stats( stat_path ); // this will just return if stats are already loaded for the rom
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

			if ( list_limit != 0 )
			{
				if ( list_limit > 0 )
					m_filtered_list[i].erase( m_filtered_list[i].begin() + list_limit, m_filtered_list[i].end()  );
				else
					m_filtered_list[i].erase( m_filtered_list[i].begin(), m_filtered_list[i].end() + list_limit );
			}
		}
	}

	std::cout << " - Constructed " << filters_count << " filtered lists in "
			<< filter_timer.getElapsedTime().asMilliseconds()
			<< " ms (" << filters_count * m_list.size() << " comparisons)" << std::endl;

	return retval;
}

int FeRomList::process_setting( const std::string &setting,
				const std::string &value,
				const std::string &fn )
{
	FeRomInfo next_rom( setting );
	next_rom.process_setting( setting, value, fn );
	m_list.push_back( next_rom );

   return 0;
}

void FeRomList::save_state() const
{
	save_favs();
	save_tags();
}

void FeRomList::save_favs() const
{
	if (( !m_fav_changed ) || ( m_user_path.empty() ) || ( m_romlist_name.empty() ))
		return;

	std::set<std::string> favs_list;

	//
	// First gather all the favourites from the current list
	//
	for ( std::deque<FeRomInfo>::const_iterator itr = m_list.begin(); itr != m_list.end(); ++itr )
	{
		if ( !((*itr).get_info( FeRomInfo::Favourite ).empty()) )
			favs_list.insert( (*itr).get_info( FeRomInfo::Romname ) );
	}

	//
	// Now save the contents of favs list
	//
	std::string fname = m_user_path + m_romlist_name + FE_FAVOURITE_FILE_EXTENSION;

	if ( favs_list.empty() )
	{
		delete_file( fname );
	}
	else
	{
		std::ofstream outfile( fname.c_str() );
		if ( !outfile.is_open() )
			return;

		for ( std::set<std::string>::const_iterator itf = favs_list.begin(); itf != favs_list.end(); ++itf )
			outfile << (*itf) << std::endl;

		outfile.close();
	}
}

void FeRomList::save_tags() const
{
	if (( !m_tags_changed ) || ( m_user_path.empty() ) || ( m_romlist_name.empty() ))
		return;

	// First construct a mapping of tags to rom entries
	//
	std::multimap< std::string, const char * > tag_to_rom_map;

	for ( std::deque<FeRomInfo>::const_iterator itr = m_list.begin(); itr != m_list.end(); ++itr )
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

bool FeRomList::set_fav( int filter_index, int rom_index, FeListInfo &list_info, bool fav )
{
	if (( rom_index < 0 ) || ( rom_index >= filter_size( filter_index ) ))
		return false;

	(m_filtered_list[filter_index][rom_index])->set_info( FeRomInfo::Favourite, fav ? "1" : "" );
	m_fav_changed=true;

	return fix_filters( list_info, FeRomInfo::Favourite );
}

void FeRomList::get_tags_list( int filter_index, int rom_index,
		std::vector< std::pair<std::string, bool> > &tags_list ) const
{
	if (( rom_index < 0 ) || ( rom_index >= filter_size( filter_index ) ))
		return;

	std::string curr_tags = (m_filtered_list[filter_index][rom_index])->get_info(FeRomInfo::Tags);

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

bool FeRomList::set_tag( int filter_index, int rom_index, FeListInfo &list_info, const std::string &tag, bool flag )
{
	if (( rom_index < 0 ) || ( rom_index >= filter_size( filter_index ) ))
		return false;

	std::string curr_tags = (m_filtered_list[filter_index][rom_index])->get_info(FeRomInfo::Tags);
	size_t pos = curr_tags.find( FE_TAGS_SEP + tag + FE_TAGS_SEP );

	std::map<std::string, bool>::iterator itt = m_tags.begin();

	if ( flag == true )
	{
		if ( pos == std::string::npos )
		{
			(m_filtered_list[filter_index][rom_index])->append_tag( tag );
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

		(m_filtered_list[filter_index][rom_index])->set_info( FeRomInfo::Tags, curr_tags );
		m_tags_changed = true;

		itt = m_tags.find( tag );
		if ( itt != m_tags.end() )
			(*itt).second = true;
		else
			itt = m_tags.insert( itt, std::pair<std::string,bool>( tag, true ) );
	}

	return fix_filters( list_info, FeRomInfo::Tags );
}

bool FeRomList::fix_filters( FeListInfo &list_info, FeRomInfo::Index target )
{
	bool retval = false;
	for ( int i=0; i<list_info.get_filter_count(); i++ )
	{
		FeFilter *f = list_info.get_filter( i );
		ASSERT( f );

		if ( f->test_for_target( target ) )
		{
			m_filtered_list[i].clear();
			retval = true;

			for ( std::deque< FeRomInfo >::iterator itr=m_list.begin(); itr!=m_list.end(); ++itr )
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

	for ( std::deque<FeRomInfo>::iterator itr=m_list.begin(); itr != m_list.end(); ++itr )
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

const char *FeEmulatorInfo::indexStrings[] =
{
	"name",
	"executable",
	"args",
	"rompath",
	"romext",
	"system",
	"info_source",
	"import_extras",
	"minimum_run_time",
	"exit_hotkey",
	NULL
};

const char *FeEmulatorInfo::indexDispStrings[] =
{
	"Name",
	"Executable",
	"Command Arguments",
	"Rom Path(s)",
	"Rom Extension(s)",
	"System Identifier",
	"Info Source/Scraper",
	"Additional Import File(s)",
	"Minimum Run Time",
	"Exit Hotkey",
	NULL
};

FeEmulatorInfo::FeEmulatorInfo()
	: m_min_run( 0 )
{
}

FeEmulatorInfo::FeEmulatorInfo( const std::string &n )
: m_name( n ), m_min_run( 0 )
{
}

const std::string FeEmulatorInfo::get_info( int i ) const
{
	switch ( (Index)i )
	{
	case Name:
		return m_name;
	case Executable:
		return m_executable;
	case Command:
		return m_command;
	case Rom_path:
		return vector_to_string( m_paths );
	case Rom_extension:
		return vector_to_string( m_extensions );
	case System:
		return vector_to_string( m_systems );
	case Info_source:
		return m_info_source;
	case Import_extras:
		return vector_to_string( m_import_extras );
	case Minimum_run_time:
		return as_str( m_min_run );
	case Exit_hotkey:
		return m_exit_hotkey;
	default:
		return "";
	}
}

void FeEmulatorInfo::set_info( enum Index i, const std::string &s )
{
	switch ( i )
	{
	case Name:
		m_name = s; break;
	case Executable:
		m_executable = s; break;
	case Command:
		m_command = s; break;
	case Rom_path:
		m_paths.clear();
		string_to_vector( s, m_paths );
		break;
	case Rom_extension:
		m_extensions.clear();
		string_to_vector( s, m_extensions, true );
		break;
	case System:
		m_systems.clear();
		string_to_vector( s, m_systems );
		break;
	case Info_source:
		m_info_source = s; break;
	case Import_extras:
		m_import_extras.clear();
		string_to_vector( s, m_import_extras );
		break;
	case Minimum_run_time:
		m_min_run = as_int( s );
		break;
	case Exit_hotkey:
		m_exit_hotkey = s; break;
	default:
		break;
	}
}

const std::vector<std::string> &FeEmulatorInfo::get_paths() const
{
	return m_paths;
}

const std::vector<std::string> &FeEmulatorInfo::get_extensions() const
{
	return m_extensions;
}

const std::vector<std::string> &FeEmulatorInfo::get_systems() const
{
	return m_systems;
}

const std::vector<std::string> &FeEmulatorInfo::get_import_extras() const
{
	return m_import_extras;
}

bool FeEmulatorInfo::get_artwork( const std::string &label, std::string &artwork ) const
{
	std::map<std::string, std::vector<std::string> >::const_iterator itm;
	itm = m_artwork.find( label );
	if ( itm == m_artwork.end() )
		return false;

	artwork = vector_to_string( (*itm).second );
	return true;
}

bool FeEmulatorInfo::get_artwork( const std::string &label, std::vector< std::string > &artwork ) const
{
	std::map<std::string, std::vector<std::string> >::const_iterator itm;
	itm = m_artwork.find( label );
	if ( itm == m_artwork.end() )
		return false;

	artwork.clear();
	for ( std::vector<std::string>::const_iterator its = (*itm).second.begin();
			its != (*itm).second.end(); ++its )
		artwork.push_back( *its );

	return true;
}

void FeEmulatorInfo::add_artwork( const std::string &label,
							const std::string &artwork )
{
	// don't clear m_artwork[ label ], it may have entries already
	// see process_setting() and special case for migrating movie settings
	// from pre 1.2.2 versions
	//
	string_to_vector( artwork, m_artwork[ label ] );
}

void FeEmulatorInfo::get_artwork_list(
			std::vector< std::pair< std::string, std::string > > &out_list ) const
{
	out_list.clear();
	std::map<std::string, std::vector<std::string> >::const_iterator itm;

	for ( itm=m_artwork.begin(); itm != m_artwork.end(); ++itm )
	{
		std::string path_list = vector_to_string( (*itm).second );
		out_list.push_back( std::pair<std::string,std::string>( (*itm).first, path_list ) );
	}
}

void FeEmulatorInfo::delete_artwork( const std::string &label )
{
	std::map<std::string, std::vector<std::string> >::iterator itm;
	itm = m_artwork.find( label );
	if ( itm != m_artwork.end() )
		m_artwork.erase( itm );
}

int FeEmulatorInfo::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{

	const char *stokens[] = { "artwork", NULL };

	// name is ignored here, it gets set directly
	//
	for ( int i=1; i < LAST_INDEX; i++ )
	{
		if ( setting.compare( indexStrings[i] ) == 0 )
		{
			set_info( (Index)i, value );
			return 0;
		}
	}

	//
	// Special case for migration from versions <=1.3.2
	//
	if ( setting.compare( "listxml" ) == 0 )
	{
		//
		// value will one of the following:
		//    mame
		//    mess <system>
		//
		size_t pos=0;
		token_helper( value, pos, m_info_source, FE_WHITESPACE );

		std::string temp;
		token_helper( value, pos, temp, "\n" );
		m_systems.push_back( temp );
		return 0;
	}

	//
	// Special case for migration from versions <=1.2.2
	//
	// version 1.2.2 and earlier had "movie_path" and
	// "movie_artwork" settings which are now deprecated.
	// Handle them in a way that gets things working in the
	// new method of configuration...
	//
	// Assumption: these settings will always be encountered
	// before the other artwork settings, which is true unless
	// the user did manual sorting of the .cfg file...
	//
	if ( setting.compare( "movie_path" ) == 0 )
	{
		add_artwork( FE_DEFAULT_ARTWORK, value );
		return 0;
	}
	else if ( setting.compare( "movie_artwork" ) == 0 )
	{
		if ( value.compare( FE_DEFAULT_ARTWORK ) != 0 )
		{
			// We guessed wrong, user didn't have snaps set as the movie artwork
			// so go in and change the snaps artwork to the one the user configured
			//
			std::string temp;
			get_artwork( FE_DEFAULT_ARTWORK, temp );
			delete_artwork( FE_DEFAULT_ARTWORK );
			add_artwork( value, temp );
		}
		return 0;
	}
	//
	// End migration code
	//

	if ( setting.compare( stokens[0] ) == 0 ) // artwork
	{
		size_t pos=0;
		std::string label, path;
		token_helper( value, pos, label, FE_WHITESPACE );
		token_helper( value, pos, path, "\n" );
		add_artwork( label, path );
	}
	else
	{
		invalid_setting( fn, "emulator", setting,
				indexStrings + 1, stokens );
		return 1;
	}
	return 0;
}

void FeEmulatorInfo::save( const std::string &filename ) const
{
#ifdef FE_DEBUG
	std::cout << "Writing emulator config to: " << filename << std::endl;
#endif

	std::ofstream outfile( filename.c_str() );
	if ( outfile.is_open() )
	{
		using std::string;
		using std::setw;
		using std::left;
		using std::endl;

		outfile << "# Generated by " << FE_NAME << " " << FE_VERSION << endl
					<< "#" << endl;

		// skip name, which gets derived from the filename
		//
		for ( int i=1; i < LAST_INDEX; i++ )
		{
			// don't output minimum run time if it is zero
			if (( i == Minimum_run_time ) && ( m_min_run == 0 ))
				continue;

			string val = get_info( (Index) i );
			if ( !val.empty() )
				outfile << setw(20) << left << indexStrings[i]
							<< ' ' << val << endl;
		}

		std::vector< std::pair< std::string, std::string > > art_list;

		get_artwork_list( art_list );

		for (	std::vector< std::pair< std::string, std::string > >::iterator itr=art_list.begin();
				itr != art_list.end(); ++itr )
		{
			std::string label;
			if ( (*itr).first.find_first_of( ' ' ) != std::string::npos )
				label = '"' + (*itr).first + '"';
			else
				label = (*itr).first;

			outfile << setw(10) << left << "artwork"
						<< ' ' << setw(15) << left << label
						<< ' ' << (*itr).second << endl;
		}

		outfile.close();
   }
}

std::string FeEmulatorInfo::vector_to_string( const std::vector< std::string > &vec ) const
{
	std::string ret_str;
	for ( unsigned int i=0; i < vec.size(); i++ )
	{
		if ( i > 0 ) // there could be empty entries in the vector...
			ret_str += ";";

		ret_str += vec[i];
	}
	return ret_str;
}

void FeEmulatorInfo::string_to_vector(
			const std::string &input, std::vector< std::string > &vec, bool allow_empty ) const
{
	size_t pos=0;
	do
	{
		std::string val;
		token_helper( input, pos, val );

		if ( ( !val.empty() ) || allow_empty )
			vec.push_back( val );

	} while ( pos < input.size() );
}

void FeEmulatorInfo::gather_rom_names( std::vector<std::string> &name_list ) const
{
	for ( std::vector<std::string>::const_iterator itr=m_paths.begin(); itr!=m_paths.end(); ++itr )
	{
		std::string path = clean_path( *itr, true );

		for ( std::vector<std::string>::const_iterator ite = m_extensions.begin();
						ite != m_extensions.end(); ++ite )
		{
			if ( (*ite).compare( FE_DIR_TOKEN ) == 0 )
				get_subdirectories( name_list, path );
			else
				get_basename_from_extension( name_list, path, (*ite), true );
		}
	}
}

const char *FeScriptConfigurable::indexString = "param";

bool FeScriptConfigurable::get_param( const std::string &label, std::string &v ) const
{
	std::map<std::string,std::string>::const_iterator itr;

	itr = m_params.find( label );
	if ( itr != m_params.end() )
	{
		v = (*itr).second;
		return true;
	}

	return false;
}

void FeScriptConfigurable::set_param( const std::string &label, const std::string &v )
{
	m_params[ label ] = v;
}

void FeScriptConfigurable::get_param_labels( std::vector<std::string> &labels ) const
{
	std::map<std::string,std::string>::const_iterator itr;
	for ( itr=m_params.begin(); itr!=m_params.end(); ++itr )
		labels.push_back( (*itr).first );
}

int FeScriptConfigurable::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{
	if ( setting.compare( indexString ) == 0 ) // param
	{
		std::string label, v;
		size_t pos=0;

		token_helper( value, pos, label, FE_WHITESPACE );

		if ( pos < value.size() )
			v = value.substr( pos );

		m_params[ label ] = v;

		return 0;
	}
	else
		return 1;
}

void FeScriptConfigurable::save( std::ofstream &f ) const
{
	std::map<std::string,std::string>::const_iterator itr;
	for ( itr=m_params.begin(); itr!=m_params.end(); ++itr )
	{
		if ( !(*itr).first.empty() )
		{
			f << '\t' << std::setw(20) << std::left << indexString << ' '
				<< (*itr).first << ' ' << (*itr).second << std::endl;
		}
	}
}

const char *FePlugInfo::indexStrings[] = { "enabled","param",NULL };

FePlugInfo::FePlugInfo( const std::string & n )
	: m_name( n ), m_enabled( false )
{
}

int FePlugInfo::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{
	if ( setting.compare( indexStrings[0] ) == 0 ) // enabled
		set_enabled( config_str_to_bool( value ) );
	else if ( FeScriptConfigurable::process_setting( setting, value, fn ) ) // params
	{
		invalid_setting( fn, "plugin", setting, indexStrings );
		return 1;
	}
	return 0;
}

void FePlugInfo::save( std::ofstream &f ) const
{
	f << std::endl << "plugin" << '\t' << m_name << std::endl;

	f << '\t' << std::setw(20) << std::left << indexStrings[0]
		<< ( m_enabled ? " yes" : " no" ) << std::endl;

	FeScriptConfigurable::save( f );
}

const char *FeLayoutInfo::indexStrings[] = {
	"saver_config",
	"layout_config",
	NULL
};

FeLayoutInfo::FeLayoutInfo()
{
}

FeLayoutInfo::FeLayoutInfo( const std::string &name )
	: m_name( name )
{
}

void FeLayoutInfo::save( std::ofstream &f ) const
{
	if ( !m_params.empty() )
	{
		if ( m_name.empty() )
			f << std::endl << indexStrings[0] << std::endl;
		else
			f << std::endl << indexStrings[1] << '\t' << m_name << std::endl;

		FeScriptConfigurable::save( f );
	}
}

FeResourceMap::FeResourceMap()
{
}

int FeResourceMap::process_setting( const std::string &setting,
                        const std::string &value,
                        const std::string &filename )
{
	m_map[setting] = value;
	return 0;
}

void FeResourceMap::get_resource( const std::string &token,
            std::string &str ) const
{
	if ( token.empty() )
		return;

	std::map<std::string, std::string>::const_iterator it = m_map.find( token );

	if ( it != m_map.end() )
		str = (*it).second;
	else if ( token[0] != '_' )
		str = token;
}
