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

const char *FE_ROMLIST_FILE_EXTENSION	= ".txt";
const char *FE_FAVOURITE_FILE_EXTENSION = ".tag";
const char FE_TAGS_SEP = ';';

const FeRomInfo::Index FeRomInfo::BuildAltName = FeRomInfo::Favourite;
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
	"Favourite",
	"Tags",
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

void FeRomInfo::dump( void ) const
{
	std::cout << '\t';
	for ( int i=0; i < LAST_INDEX; i++ )
		std::cout << " " << i << "=["
			<< get_info((Index)i) << "]";
	std::cout << std::endl;
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

SQRex *FeRomListCompare::m_rex = NULL;

void FeRomListCompare::init_rex( const std::string &re_mask )
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

void FeRomListCompare::close_rex()
{
	if ( m_rex )
		sqstd_rex_free( m_rex );

	m_rex = NULL;
}

bool FeRomListCompare::cmp( const FeRomInfo &one_info, const FeRomInfo &two_info )
{
	const std::string &one = one_info.get_info( FeRomInfo::Title );
	const std::string &two = two_info.get_info( FeRomInfo::Title );

	size_t one_begin( 0 ), one_len( one.size() ), two_begin( 0 ), two_len( two.size() );

	if ( m_rex )
	{
		const SQChar *one_begin_ptr;
		const SQChar *one_end_ptr;
		const SQChar *two_begin_ptr;
		const SQChar *two_end_ptr;

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
	}

	return ( one.compare( one_begin, one_len, two, two_begin, two_len ) < 0 );
}

const char FeRomListCompare::get_first_letter( const FeRomInfo &one_info )
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
		{
			if ( m_filter_what.empty() )
				return true;
			else
				return false;
		}
		return (( sqstd_rex_match(
					m_rex,
					(const SQChar *)target.c_str()
					) == SQTrue ) ? true : false );

	case FilterNotEquals:
		if ( target.empty() )
		{
			if ( m_filter_what.empty() )
				return false;
			else
				return true;
		}
		return (( sqstd_rex_match(
					m_rex,
					(const SQChar *)target.c_str()
					) == SQTrue ) ? false : true );

	case FilterContains:
		if ( target.empty() )
			return false;

		return (( sqstd_rex_search(
					m_rex,
					(const SQChar *)target.c_str(),
					&begin,
					&end
					) == SQTrue ) ? true : false );

	case FilterNotContains:
		if ( target.empty() )
			return true;

		return (( sqstd_rex_search(
					m_rex,
					(const SQChar *)target.c_str(),
					&begin,
					&end
					) == SQTrue ) ? false : true );

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

const char *FeFilter::indexString = "filter";

FeFilter::FeFilter( const std::string &name )
	: m_name( name ),
	m_rom_index( 0 )
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
		<< indexString << ' ' << n << std::endl;

	for ( std::vector<FeRule>::const_iterator itr=m_rules.begin();
			itr != m_rules.end(); ++itr )
		(*itr).save( f );
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

int FeListInfo::get_current_rom_index() const
{
	if ( !m_filters.empty() )
		return m_filters[ m_filter_index ].get_current_rom_index();

	return m_rom_index;
}

void FeListInfo::set_current_rom_index( int r )
{
	if ( !m_filters.empty() )
		m_filters[ m_filter_index ].set_current_rom_index( r );
	else
		m_rom_index = r;
}

std::string FeListInfo::get_current_layout_file() const
{
	return m_current_layout_file;
}

void FeListInfo::set_current_layout_file( const std::string &n )
{
	m_current_layout_file = n;
}

void FeListInfo::dump( void ) const
{
	std::cout << '\t';
	for ( int i=0; i < LAST_INDEX; i++ )
		std::cout << " " << i << "=["
			<< get_info((Index)i) << "]";

	std::cout << std::endl;
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
	else if ( setting.compare( FeFilter::indexString ) == 0 ) // filter
	{
		size_t pos=0;
		std::string name;
		token_helper( value, pos, name, FE_WHITESPACE );

		// Create a new filter with the given name
		//
		m_filters.push_back( FeFilter( name ) );

		//
		// Deal with filters in the format from version 1.0, where there could
		// only be one filter per list (with one filter rule) saved in the
		// following format:
		//
		// filter <target> <comparision> <what>
		//
		if ( value.size() > pos )
			m_filters.back().process_setting( setting, value, fn );

		// In versions after 1.0, filter rules occur under the "rule" tag
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
			m_filters[findex].set_current_rom_index( as_int( sub_val ) );
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
			state << (*itr).get_current_rom_index() << ",";
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

FeRomList::FeRomList()
	: m_filter( NULL ),
	m_fav_changed( false ),
	m_tags_changed( false )
{
}

FeRomList::~FeRomList()
{
}

void FeRomList::set_filter( const FeFilter *f )
{
	m_filter = f;
}

bool FeRomList::load_romlist( const std::string &path,
			const std::string &romlist_name,
			const std::string &user_path )
{
	m_user_path = user_path;
	m_romlist_name = romlist_name;

	load_favs( m_user_path + m_romlist_name + FE_FAVOURITE_FILE_EXTENSION );
	load_tags( m_user_path + m_romlist_name + "/" );

	m_list.clear();
	return FeBaseConfigurable::load_from_file(
			path + m_romlist_name + FE_ROMLIST_FILE_EXTENSION, ";" );
}

int FeRomList::process_setting( const std::string &setting,
				const std::string &value,
				const std::string &fn )
{
	FeRomInfo next_rom( setting );
	next_rom.process_setting( setting, value, fn );

	const std::string &rname = next_rom.get_info( FeRomInfo::Romname );

	//
	// Set the fav state
	//
	std::set< std::string >::const_iterator itf = m_extra_favs.find( rname );
	if ( itf != m_extra_favs.end() )
		next_rom.set_info( FeRomInfo::Favourite, "1" );

	//
	// Set the tags
	//
	std::pair<std::multimap<std::string,const char *>::iterator,std::multimap<std::string,const char *>::iterator> t_ret;
	t_ret = m_extra_tags.equal_range( rname );

	for ( std::multimap<std::string,const char *>::iterator itr = t_ret.first; itr != t_ret.second; ++itr )
		next_rom.append_tag( (*itr).second );

	//
	// Now check whether this entry is filtered out of the list
	//
	if (( m_filter == NULL ) || ( m_filter->apply_filter( next_rom ) == true ))
	{
		m_list.push_back( next_rom );

		//
		// Fix m_extra_favs and m_extra_tags so that they don't have entries for this rom (since
		// we now know the rom isn't filtered out)
		//
		if ( itf != m_extra_favs.end() )
			m_extra_favs.erase( itf );

		if ( t_ret.first != t_ret.second )
			m_extra_tags.erase( t_ret.first, t_ret.second );
	}

   return 0;
}

void FeRomList::load_favs( const std::string &filename )
{
	m_fav_changed=false;
	m_extra_favs.clear();

	std::ifstream myfile( filename.c_str() );

	if ( !myfile.is_open() )
		return;

	std::set<std::string>::iterator itr=m_extra_favs.begin();

	while ( myfile.good() )
	{
		size_t pos=0;
		std::string line, name;

		getline( myfile, line );
		token_helper( line, pos, name );

		if ( !name.empty() )
			itr=m_extra_favs.insert( itr, name );
	}

	myfile.close();
}

void FeRomList::load_tags( const std::string &path )
{
	m_tags.clear();
	m_tags_changed=false;
	m_extra_tags.clear();

	if ( !directory_exists( path ) )
		return;

	// Load the tags for this romlist
	//
	std::vector<std::string> temp_tags;
	get_basename_from_extension( temp_tags, path, FE_FAVOURITE_FILE_EXTENSION );

	for ( std::vector<std::string>::iterator itr=temp_tags.begin(); itr!=temp_tags.end(); ++itr )
	{
		if ( (*itr).empty() )
			continue;

		std::ifstream myfile( std::string(path + "/" + (*itr) + FE_FAVOURITE_FILE_EXTENSION).c_str() );

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
				m_extra_tags.insert(
						std::pair<std::string,const char *>(rname, (*itt).first.c_str()) );
		}

		myfile.close();
	}
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

	//
	// First insert all the favourites from the current filtered list into m_extra_favs
	//
	for ( std::deque<FeRomInfo>::const_iterator itr = m_list.begin(); itr != m_list.end(); ++itr )
	{
		if ( !((*itr).get_info( FeRomInfo::Favourite ).empty()) )
			m_extra_favs.insert( (*itr).get_info( FeRomInfo::Romname ) );
	}

	//
	// Now save the contents of m_extra_favs
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

void FeRomList::save_tags() const
{
	if (( !m_tags_changed ) || ( m_user_path.empty() ) || ( m_romlist_name.empty() ))
		return;

	// First construct a mapping of tags to rom entries (i.e. the reverse of the m_extra_tags mapping)
	//
	std::multimap< std::string, const char * > tag_to_rom_map;

	for ( std::multimap< std::string, const char * >::const_iterator ite = m_extra_tags.begin(); ite != m_extra_tags.end(); ++ite )
	{
		tag_to_rom_map.insert(
				std::pair<std::string,const char *>(
						(*ite).second,
						(*ite).first.c_str() ) );
	}

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

bool FeRomList::set_fav( int idx, bool fav )
{
	m_list[idx].set_info( FeRomInfo::Favourite, fav ? "1" : "" );
	m_fav_changed=true;

	if (( m_filter != NULL ) && ( m_filter->apply_filter( m_list[idx] ) == false ))
	{
		// this rom is now filtered out by the current list filter

		if ( fav == true ) // keep track of fav status if it is marked as a fav
			m_extra_favs.insert( m_list[idx].get_info( FeRomInfo::Romname ) );

		m_list.erase( m_list.begin() + idx );
		return true;
	}

	return false;
}

void FeRomList::get_tags_list( int idx,
		std::vector< std::pair<std::string, bool> > &tags_list ) const
{
	std::string curr_tags = m_list[idx].get_info(FeRomInfo::Tags);

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

bool FeRomList::set_tag( int idx, const std::string &tag, bool flag )
{
	std::string curr_tags = m_list[idx].get_info(FeRomInfo::Tags);
	size_t pos = curr_tags.find( FE_TAGS_SEP + tag + FE_TAGS_SEP );

	std::map<std::string, bool>::iterator itt = m_tags.begin();

	if ( flag == true )
	{
		if ( pos == std::string::npos )
		{
			m_list[idx].append_tag( tag );
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

		m_list[idx].set_info( FeRomInfo::Tags, curr_tags );
		m_tags_changed = true;

		itt = m_tags.find( tag );
		if ( itt != m_tags.end() )
			(*itt).second = true;
		else
			itt = m_tags.insert( itt, std::pair<std::string,bool>( tag, true ) );
	}

	if (( m_filter != NULL ) && ( m_filter->apply_filter( m_list[idx] ) == false ))
	{
		// this rom is now filtered out by the current list filter

		if ( flag == true )
		{
			// keep track of tag if entry is newly tagged
			m_extra_tags.insert(
					std::pair<std::string,const char *>(
								m_list[idx].get_info( FeRomInfo::Romname ),
								(*itt).first.c_str()) );
		}

		m_list.erase( m_list.begin() + idx );
		return true;
	}

	return false;
}

const char *FeEmulatorInfo::indexStrings[] =
{
	"name",
	"executable",
	"args",
	"rompath",
	"romext",
	"import_extras",
	"listxml",
	NULL
};

const char *FeEmulatorInfo::indexDispStrings[] =
{
	"Name",
	"Executable",
	"Command Arguments",
	"Rom Path",
	"Rom Extension(s)",
	"Additional Import Files",
	"XML Mode",
	NULL
};

FeEmulatorInfo::FeEmulatorInfo()
{
}

FeEmulatorInfo::FeEmulatorInfo( const std::string &n )
: m_name( n )
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
	case Import_extras:
		return vector_to_string( m_import_extras );
	case Listxml:
		return m_listxml;
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
	case Import_extras:
		m_import_extras.clear();
		string_to_vector( s, m_import_extras );
		break;
	case Listxml:
		m_listxml = s; break;
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
	// Special case for migration from versions <1.2.2
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
