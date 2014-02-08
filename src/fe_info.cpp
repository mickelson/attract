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

bool FeRomInfo::operator< ( FeRomInfo s ) const
{
	std::string one = get_info( FeRomInfo::Title );
	std::string two = s.get_info( FeRomInfo::Title );
	
	return (one < two);
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
	"Rom Extension(s)",
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
	if ( i == Rom_extension )
	{
		size_t pos=0;
		m_extensions.clear();
		do
		{
			std::string ext;
  			token_helper( s, pos, ext );
			m_extensions.push_back( ext );
		} while ( pos < s.size() );
	}
}

const std::vector<std::string> &FeEmulatorInfo::get_extensions() const
{
	return m_extensions;
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
			set_info( (Index)i, value );
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

const char *FePlugInfo::indexStrings[] = { "command","enabled","param",NULL };

FePlugInfo::FePlugInfo( const std::string & n )
	: m_name( n ), m_enabled( false )
{
}

bool FePlugInfo::get_param( const std::string &label, std::string &v ) const
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

void FePlugInfo::set_param( const std::string &label, const std::string &v )
{
	m_params[ label ] = v;
}

void FePlugInfo::get_param_labels( std::vector<std::string> &labels ) const
{
	std::map<std::string,std::string>::const_iterator itr;
	for ( itr=m_params.begin(); itr!=m_params.end(); ++itr )
		labels.push_back( (*itr).first );
}

int FePlugInfo::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{
	if ( setting.compare( indexStrings[0] ) == 0 ) // command
		set_command( value );
	else if ( setting.compare( indexStrings[1] ) == 0 ) // enabled
	{
		if (( value.compare( "yes" ) == 0 )
				|| ( value.compare( "true" ) == 0 ))
			set_enabled( true );
		else
			set_enabled( false );
	}
	else if ( setting.compare( indexStrings[2] ) == 0 ) // param
	{
		std::string label, v;
		size_t pos=0;

		token_helper( value, pos, label, FE_WHITESPACE );

		if ( pos < value.size() )
			v = value.substr( pos );

		m_params[ label ] = v;
	}
	else
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

	f << "plugin" << '\t' << m_name << std::endl;

	if ( !m_command.empty() )
		f << '\t' << std::setw(20) << std::left 
			<< indexStrings[0] << ' ' << get_command() << std::endl;

	f << '\t' << std::setw(20) << std::left << indexStrings[1] 
		<< ( m_enabled ? " yes" : " no" ) << std::endl;

	std::map<std::string,std::string>::const_iterator itr;
	for ( itr=m_params.begin(); itr!=m_params.end(); ++itr )
	{
		f << '\t' << std::setw(20) << std::left << indexStrings[2] << ' '
			<< (*itr).first << ' ' << (*itr).second << std::endl;
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
