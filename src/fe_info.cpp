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

	for ( int i=1; i<LAST_INDEX; i++ )
	{
		token_helper( value, pos, token );
		m_info[(Index)i] = token;
	}

	return 0;
}

std::string FeRomInfo::as_output( void ) const
{
	std::string s = get_info_escaped( (Index)0 );
	for ( int i=1; i < LAST_INDEX; i++ )
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

bool FeRomInfo::operator< ( FeRomInfo s ) const
{
	std::string one = get_info( FeRomInfo::Title );
	std::string two = s.get_info( FeRomInfo::Title );
	
	return (one < two);
}

const char *FeListInfo::indexStrings[] =
{
	"name",
	"layout",
	"romlist",
	"filter",
	NULL
};

const char *FeListInfo::filterCompStrings[] = 
{
	"equals",
	"not_equals",
	"contains",
	"not_contains",
	NULL 
};

FeListInfo::FeListInfo( const std::string &n )
	: m_rom_index( 0 ), 
	m_filter_target( FeRomInfo::LAST_INDEX ), 
	m_filter_comp( LAST_COMPARISON )
{
	m_info[ Name ] = n;
}

const std::string &FeListInfo::get_info( int i ) const
{
	return m_info[i];
}

int FeListInfo::get_current_rom_index() const
{
	return m_rom_index;
}

void FeListInfo::set_current_rom_index( int r )
{
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
	else if ( setting.compare( indexStrings[Filter] ) == 0 ) // filter
		return parse_filter( fn, value );
	else
	{
		invalid_setting( fn, "list", setting, indexStrings + 1 );
		return 1;
	}

	return 0;
}

int FeListInfo::process_state( const std::string &state_string )
{
	// state string is in format "[curr_rom];[curr_layout_file];"
	size_t pos=0;
	std::string val;

	token_helper( state_string, pos, val );
	m_rom_index = as_int( val );

	if ( pos < state_string.size() )
	{
		token_helper( state_string, pos, val );
		m_current_layout_file = val;
	}

	return 0;
}

std::string FeListInfo::state_as_output() const
{
	std::ostringstream state;
	state << m_rom_index << ";" << m_current_layout_file << ";";
	return state.str();
}

int FeListInfo::parse_filter( const std::string &fn, 
				const std::string &filter_str )
{
	if ( filter_str.empty() )
	{
		m_filter_target = FeRomInfo::LAST_INDEX;
		m_filter_comp = LAST_COMPARISON;
		m_filter_what.clear();
		return 0;
	}

	std::string token;
	size_t pos=0;

	token_helper( filter_str, pos, token, FE_WHITESPACE );

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

	m_filter_target = (FeRomInfo::Index)i;
	token_helper( filter_str, pos, token, FE_WHITESPACE );

	i=0;
	while( filterCompStrings[i] != NULL )
	{
		if ( token.compare( filterCompStrings[i] ) == 0 )
			break;
		i++;
	}

	if ( i >= LAST_COMPARISON )
	{
		invalid_setting( fn, "filter", token, filterCompStrings, 
				NULL, "comparison" );
		return 1;
	}

	m_filter_comp = (FilterComp)i;

	// Remainder of the line is filter regular expression (which may contain
	// spaces...)
	//
	if ( pos < filter_str.size() )
		m_filter_what = filter_str.substr( pos );

	return 0;
}


bool FeListInfo::get_filter( FeRomInfo::Index &i, FilterComp &c, 
					std::string &w ) const
{
	i = m_filter_target;
	c = m_filter_comp;
	w = m_filter_what;

	if (( m_filter_target == FeRomInfo::LAST_INDEX )
				|| ( m_filter_comp == LAST_COMPARISON ))
		return false;
	else
		return true;
}

void FeListInfo::set_filter( FeRomInfo::Index i, FilterComp c, 
					const std::string &w )
{
	m_filter_target = i;
	m_filter_comp = c;
	m_filter_what = w;
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

	if (( m_filter_target != FeRomInfo::LAST_INDEX )
		&& ( m_filter_comp != LAST_COMPARISON ))
	{
		f << '\t' << setw(20) << left << indexStrings[Filter] << ' ' 
			<< FeRomInfo::indexStrings[ m_filter_target ] << ' ' 
			<< filterCompStrings[ m_filter_comp ] << ' ' 
			<< m_filter_what << endl;
	}
}

FeRomList::FeRomList()
	: m_filter_target( FeRomInfo::LAST_INDEX ),
	m_filter_comp( FeListInfo::LAST_COMPARISON ),
	m_rex( NULL )
{
}

FeRomList::~FeRomList()
{
	// need to free m_rex
	clear();
}

void FeRomList::clear()
{
	if ( m_rex )
		sqstd_rex_free( m_rex );

	m_list.clear();
	m_filter_target = FeRomInfo::LAST_INDEX;
	m_filter_comp = FeListInfo::LAST_COMPARISON;
	m_rex = NULL;
}

bool FeRomList::apply_filter( const FeRomInfo &rom ) const
{
	
	if (( m_filter_target == FeRomInfo::LAST_INDEX )
		|| ( m_filter_comp == FeListInfo::LAST_COMPARISON )
		|| ( m_rex == NULL ))
		return true;

	const SQChar *begin( NULL );
	const SQChar *end( NULL );
	const std::string &target = rom.get_info( m_filter_target );

	switch ( m_filter_comp )
	{
	case FeListInfo::FilterEquals:
		return (( sqstd_rex_match( 
					m_rex, 
					(const SQChar *)target.c_str() 
					) == SQTrue ) ? true : false );

	case FeListInfo::FilterNotEquals:
		return (( sqstd_rex_match( 
					m_rex, 
					(const SQChar *)target.c_str() 
					) == SQTrue ) ? false : true );

	case FeListInfo::FilterContains:
		return (( sqstd_rex_search( 
					m_rex, 
					(const SQChar *)target.c_str(),
					&begin,
					&end
					) == SQTrue ) ? true : false );

	case FeListInfo::FilterNotContains:
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

void FeRomList::set_filter( FeRomInfo::Index i,
            FeListInfo::FilterComp c,
            const std::string &w )
{
	clear();

	m_filter_target = i;
	m_filter_comp = c;
	m_filter_what = w;

	if ( !m_filter_what.empty() )
	{
		//
		// Compile the regular expression now
		//
		const SQChar *err( NULL );
		m_rex = sqstd_rex_compile( 
			(const SQChar *)m_filter_what.c_str(),
			&err );

		if ( !m_rex )
			std::cout << "Error compiling regular expression: " 
				<< err << std::endl;
	}
}

int FeRomList::process_setting( const std::string &setting,
				const std::string &value,
				const std::string &fn )
{
	FeRomInfo next_rom( setting );
	next_rom.process_setting( setting, value, fn );

	if ( apply_filter( next_rom ) == true )
		m_list.push_back( next_rom );

   return 0;
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
	"movie_path",
	"movie_artwork",
	NULL
};

const char *FeEmulatorInfo::indexDispStrings[] =
{
	"Name",
	"Executable",
	"Command Arguments",
	"Rom Path",
	"Rom Extension",
	"Additional Import Files",	
	"XML Mode",	
	"Movie Path",
	"Movie Fallback Artwork",
	NULL
};

FeEmulatorInfo::FeEmulatorInfo()
{
}

FeEmulatorInfo::FeEmulatorInfo( const std::string &n )
{
	m_info[Name] = n;
}

const std::string &FeEmulatorInfo::get_info( int i ) const
{
	return m_info[i];
}

void FeEmulatorInfo::set_info( enum Index i, const std::string &s )
{
	m_info[i] = s;
}

bool FeEmulatorInfo::get_artwork( const std::string &label, std::string &artwork ) const
{
	std::map<std::string, std::string>::const_iterator it;

	if (( label.empty() ) && ( !m_artwork.empty() ))
		it = m_artwork.begin();
	else
		it=m_artwork.find( label );

	if ( it == m_artwork.end() )
	{
		return false;
	}

	artwork = (*it).second;
	return true;
}

void FeEmulatorInfo::set_artwork( const std::string &label, const std::string &artwork )
{
	m_artwork[ label ] = artwork;
}

void FeEmulatorInfo::get_artwork_list( 
			std::vector< std::pair< std::string, std::string > > &list ) const
{
	list.clear();
	std::map<std::string, std::string>::const_iterator it;

	for ( it=m_artwork.begin(); it != m_artwork.end(); ++it )
		list.push_back( std::pair<std::string, std::string>( 
									(*it).first, 
									(*it).second ) );
}

void FeEmulatorInfo::delete_artwork( const std::string &label )
{
	std::map<std::string, std::string>::iterator it;

	it=m_artwork.find( label );
	if ( it == m_artwork.end() )
		return;

	m_artwork.erase( it );
}

void FeEmulatorInfo::dump( void ) const
{
	std::cout << '\t';
	for ( int i=0; i < LAST_INDEX; i++ )
		std::cout << " " << i << "=["
			<< get_info((Index)i) << "]";

	for ( std::map<std::string,std::string>::const_iterator itl=m_artwork.begin();
			itl != m_artwork.end(); ++itl )
			std::cout << "artwork " << (*itl).first << "=[" 
				<< (*itl).second << "]";

	std::cout << std::endl;
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
			m_info[ (Index)i ] = value;
			return 0;
		}
	}

	if ( setting.compare( stokens[0] ) == 0 ) // artwork
	{
		size_t pos=0;
		std::string label, path;
  		token_helper( value, pos, label, FE_WHITESPACE );
  		token_helper( value, pos, path );
		m_artwork[ label ] = path;
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

		for ( std::map<string, string>::const_iterator itr=m_artwork.begin();
					itr != m_artwork.end(); ++itr )
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
