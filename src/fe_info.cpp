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

bool FeRomInfo::operator==( const FeRomInfo &o )
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
	: m_filter( NULL ), m_fav_changed( false )
{
}

FeRomList::~FeRomList()
{
}

void FeRomList::set_filter( const FeFilter *f )
{
	m_filter = f;
}

bool FeRomList::load_from_file( const std::string &filename,
			const char *sep )
{
	m_list.clear();
	return FeBaseConfigurable::load_from_file( filename, sep );
}

int FeRomList::process_setting( const std::string &setting,
				const std::string &value,
				const std::string &fn )
{
	FeRomInfo next_rom( setting );
	next_rom.process_setting( setting, value, fn );

	if ( m_favs.find( next_rom.get_info( FeRomInfo::Romname ) ) != m_favs.end() )
		next_rom.set_info( FeRomInfo::Favourite, "1" );

	if (( m_filter == NULL ) || ( m_filter->apply_filter( next_rom ) == true ))
		m_list.push_back( next_rom );

   return 0;
}

void FeRomList::load_fav_map( const std::string &filename )
{
	m_favs.clear();
	m_fav_changed=false;
	m_fav_file = filename;

	std::ifstream myfile( filename.c_str() );

	if ( !myfile.is_open() )
		return;

	std::set<std::string>::iterator itr=m_favs.begin();

	while ( myfile.good() )
	{
		size_t pos=0;
		std::string line, name;

		getline( myfile, line );
		token_helper( line, pos, name );

		itr=m_favs.insert( itr, name );
	}

	myfile.close();
}

void FeRomList::save_fav_map() const
{
	if (( !m_fav_changed ) || ( m_fav_file.empty() ))
		return;

	std::ofstream outfile( m_fav_file.c_str() );
	if ( !outfile.is_open() )
		return;

	std::set<std::string>::const_iterator itr;
	for ( itr = m_favs.begin(); itr != m_favs.end(); ++itr )
	{
		if ( !(*itr).empty() )
			outfile << (*itr) << std::endl;
	}

	outfile.close();
}

void FeRomList::set_fav( int idx, bool fav )
{
	if ( fav )
	{
		m_list[idx].set_info( FeRomInfo::Favourite, "1" );
		m_favs.insert( m_list[idx].get_info( FeRomInfo::Romname ) );
	}
	else
	{
		m_list[idx].set_info( FeRomInfo::Favourite, "" );
		m_favs.erase( m_list[idx].get_info( FeRomInfo::Romname ) );
	}

	m_fav_changed=true;
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
		string_to_vector( s, m_paths ); break;
	case Rom_extension:
		string_to_vector( s, m_extensions, true ); break;
	case Import_extras:
		string_to_vector( s, m_import_extras ); break;
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
	vec.clear();
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
		f << '\t' << std::setw(20) << std::left << indexString << ' '
			<< (*itr).first << ' ' << (*itr).second << std::endl;
	}
}

const char *FePlugInfo::indexStrings[] = { "command","enabled","param",NULL };

FePlugInfo::FePlugInfo( const std::string & n )
	: m_name( n ), m_enabled( false )
{
}

int FePlugInfo::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{
	if ( setting.compare( indexStrings[0] ) == 0 ) // command
		set_command( value );
	else if ( setting.compare( indexStrings[1] ) == 0 ) // enabled
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
	if (( m_command.empty() ) && ( !m_enabled ))
		return;

	f << std::endl << "plugin" << '\t' << m_name << std::endl;

	if ( !m_command.empty() )
		f << '\t' << std::setw(20) << std::left
			<< indexStrings[0] << ' ' << get_command() << std::endl;

	f << '\t' << std::setw(20) << std::left << indexStrings[1]
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
