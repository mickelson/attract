/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-15 Andrew Mickelson
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
#include "fe_util.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <squirrel.h>
#include <sqstdstring.h>

const char *FE_STAT_FILE_EXTENSION = ".stat";
const char FE_TAGS_SEP = ';';

const FeRomInfo::Index FeRomInfo::BuildFullPath = FeRomInfo::Tags;
const FeRomInfo::Index FeRomInfo::BuildScore = FeRomInfo::PlayedCount;

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
	"Buttons",
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

void FeRomInfo::copy_info( const FeRomInfo &src, Index idx )
{
	m_info[idx]=src.m_info[idx];
}

bool FeRomInfo::operator==( const FeRomInfo &o ) const
{
	return (( m_info[Romname].compare( o.m_info[Romname] ) == 0 )
				&& ( m_info[Emulator].compare( o.m_info[Emulator] ) == 0 ));
}

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
	m_rex( NULL ),
	m_is_exception( false )
{
}

FeRule::FeRule( const FeRule &r )
	: m_filter_target( r.m_filter_target ),
	m_filter_comp( r.m_filter_comp ),
	m_filter_what( r.m_filter_what ),
	m_rex( NULL ),
	m_is_exception( r.m_is_exception )
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
	m_is_exception = r.m_is_exception;

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
		const char *label=FeFilter::indexStrings[FeFilter::Rule];
		if ( m_is_exception )
			label=FeFilter::indexStrings[FeFilter::Exception];

		f << "\t\t" << std::setw(20) << std::left << label << ' '
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

int FeRule::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{
	if ( setting.compare(
			FeFilter::indexStrings[ FeFilter::Exception ] ) == 0 )
		m_is_exception=true;

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
		invalid_setting( fn, "rule", token,
				FeRomInfo::indexStrings, NULL, "target" );
		return 1;
	}

	m_filter_target = (FeRomInfo::Index)i;
	token_helper( value, pos, token, FE_WHITESPACE );

	i=0;
	while( filterCompStrings[i] != NULL )
	{
		if ( token.compare( filterCompStrings[i] ) == 0 )
			break;
		i++;
	}

	if ( i >= LAST_COMPARISON )
	{
		invalid_setting( fn, "rule", token, filterCompStrings,
				NULL, "comparison" );
		return 1;
	}

	m_filter_comp = (FilterComp)i;

	// Remainder of the line is filter regular expression (which may contain
	// spaces...)
	//
	if ( pos < value.size() )
		m_filter_what = value.substr( pos );

	return 0;
}

const char *FeFilter::indexStrings[] =
{
	"rule",
	"exception",
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
		if ( (*itr).apply_rule( rom ) == (*itr).is_exception() )
			return (*itr).is_exception();
	}

	return true;
}

int FeFilter::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{
	if (( setting.compare( indexStrings[Rule] ) == 0 ) // rule
		|| ( setting.compare( indexStrings[Exception] ) == 0 ))
	{
		FeRule new_rule;

		if ( !value.empty() )
			new_rule.process_setting( setting, value, fn );

		m_rules.push_back( new_rule );
	}
	else if ( setting.compare( indexStrings[SortBy] ) == 0 ) // sort_by
	{
		for ( int i=0; i < FeRomInfo::LAST_INDEX; i++ )
		{
			if ( value.compare( FeRomInfo::indexStrings[i] ) == 0 )
			{
				set_sort_by( (FeRomInfo::Index)i );
				break;
			}
		}
	}
	else if ( setting.compare( indexStrings[ReverseOrder] ) == 0 ) // reverse_order
	{
		set_reverse_order( true );
	}
	else if ( setting.compare( indexStrings[ListLimit] ) == 0 ) // list_limit
	{
		set_list_limit( as_int( value ) );
	}
	else
	{
		invalid_setting( fn, "filter", setting, indexStrings );
		return 1;
	}

	return 0;
}

void FeFilter::save( std::ofstream &f, const char *filter_tag ) const
{
	std::string n;
	if ( m_name.find_first_of( ' ' ) != std::string::npos )
		n = '"' + m_name + '"';
	else
		n = m_name;

	f << '\t' << std::setw(20) << std::left
		<< filter_tag << ' ' << n << std::endl;

	if ( m_sort_by != FeRomInfo::LAST_INDEX )
	{
		f << "\t\t" << std::setw(20) << std::left
			<< indexStrings[SortBy] << ' ' << FeRomInfo::indexStrings[ m_sort_by ] << std::endl;
	}

	if ( m_reverse_order != false )
	{
		f << "\t\t" << std::setw(20) << std::left
			<< indexStrings[ReverseOrder] << " true" << std::endl;
	}

	if ( m_list_limit != 0 )
	{
		f << "\t\t" << std::setw(20) << std::left
			<< indexStrings[ListLimit] << " " << as_str( m_list_limit ) << std::endl;
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

void FeFilter::clear()
{
	m_name.clear();
	m_rules.clear();
	m_rom_index=0;
	m_list_limit=0;
	m_size=0;
	m_sort_by=FeRomInfo::LAST_INDEX;
	m_reverse_order=false;
}

const char *FeDisplayInfo::indexStrings[] =
{
	"name",
	"layout",
	"romlist",
	"in_cycle",
	"in_menu",
	NULL
};

const char *FeDisplayInfo::otherStrings[] =
{
	"filter",
	"global_filter",
	NULL
};

FeDisplayInfo::FeDisplayInfo( const std::string &n )
	: m_rom_index( 0 ),
	m_filter_index( 0 ),
	m_current_config_filter( NULL ),
	m_global_filter( "" )
{
	m_info[ Name ] = n;
	m_info[ InCycle ] = "yes";
	m_info[ InMenu ] = "yes";
}

const std::string &FeDisplayInfo::get_info( int i ) const
{
	return m_info[i];
}

int FeDisplayInfo::get_rom_index( int filter_index ) const
{
	if (( filter_index >= 0 ) && ( filter_index < (int)m_filters.size() ))
		return m_filters[ filter_index ].get_rom_index();

	return m_rom_index;
}

void FeDisplayInfo::set_rom_index( int filter_index, int rom_index )
{
	if (( filter_index >= 0 ) && ( filter_index < (int)m_filters.size() ))
		m_filters[ filter_index ].set_rom_index( rom_index );
	else
		m_rom_index = rom_index;
}

std::string FeDisplayInfo::get_current_layout_file() const
{
	return m_current_layout_file;
}

void FeDisplayInfo::set_current_layout_file( const std::string &n )
{
	m_current_layout_file = n;
}

void FeDisplayInfo::set_info( int setting,
         const std::string &value )
{
	m_info[ setting ] = value;
}

int FeDisplayInfo::process_setting( const std::string &setting,
         const std::string &value, const std::string &fn )
{
	// name is igored here, it gets set directly
	//
	if ( setting.compare( indexStrings[Layout] ) == 0 ) // layout
		m_info[ Layout ] = value;
	else if ( setting.compare( indexStrings[Romlist] ) == 0 ) // romlist
		m_info[ Romlist ] = value;
	else if ( setting.compare( indexStrings[InCycle] ) == 0 ) // in_cycle
		m_info[ InCycle ] = value;
	else if ( setting.compare( indexStrings[InMenu] ) == 0 ) // in_menu
		m_info[ InMenu ] = value;
	else if ( setting.compare( otherStrings[0] ) == 0 ) // filter
	{
		size_t pos=0;
		std::string name;
		token_helper( value, pos, name, FE_WHITESPACE );

		// Create a new filter with the given name
		//
		m_filters.push_back( FeFilter( name ) );
		m_current_config_filter = &(m_filters.back());
	}
	else if ( setting.compare( otherStrings[1] ) == 0 ) // global_filter
	{
		m_global_filter.clear();
		m_current_config_filter = &(m_global_filter);
	}
	else
	{
		if ( m_current_config_filter )
			m_current_config_filter->process_setting( setting, value, fn );
		else
		{
			invalid_setting( fn, "list", setting, indexStrings + 1, otherStrings );
			return 1;
		}
	}

	return 0;
}

int FeDisplayInfo::process_state( const std::string &state_string )
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

std::string FeDisplayInfo::state_as_output() const
{
	std::ostringstream state;

	if ( m_filters.empty() )
	{
		state << m_rom_index;
	}
	else
	{
		for ( std::vector<FeFilter>::const_iterator itr=m_filters.begin();
			itr != m_filters.end(); ++itr )
		{
			state << (*itr).get_rom_index() << ",";
		}
	}

	state << ";" << m_current_layout_file << ";"
		<< m_filter_index << ";";

	return state.str();
}

FeFilter *FeDisplayInfo::get_filter( int i )
{
	if ( i < 0 )
		return &m_global_filter;

	if ( i >= (int)m_filters.size() )
		return NULL;

	return &(m_filters[i]);
}

void FeDisplayInfo::append_filter( const FeFilter &f )
{
	m_filters.push_back( f );
}

void FeDisplayInfo::delete_filter( int i )
{
	if (( m_filter_index > 0 ) && ( m_filter_index >= i ))
		m_filter_index--;

	m_filters.erase( m_filters.begin() + i );
}

void FeDisplayInfo::get_filters_list( std::vector<std::string> &l ) const
{
	l.clear();

	for ( std::vector<FeFilter>::const_iterator itr=m_filters.begin();
			itr != m_filters.end(); ++itr )
	{
		l.push_back( (*itr).get_name() );
	}
}

void FeDisplayInfo::save( std::ofstream &f ) const
{
	using std::setw;
	using std::left;
	using std::endl;

	f << "display" << '\t' << get_info( Name ) << endl;

	if ( !get_info( Layout ).empty() )
		f << '\t' << setw(20) << left
			<< indexStrings[Layout] << ' ' << get_info( Layout ) << endl;

	if ( !get_info( Romlist ).empty() )
		f << '\t' << setw(20) << left
			<< indexStrings[Romlist] << ' ' << get_info( Romlist ) << endl;

	f << '\t' << setw(20) << left
		<< indexStrings[InCycle] << ' ' << get_info( InCycle ) << endl;

	f << '\t' << setw(20) << left
		<< indexStrings[InMenu] << ' ' << get_info( InMenu ) << endl;

	if ( m_global_filter.get_rule_count() > 0 )
		m_global_filter.save( f, otherStrings[1] );

	for ( std::vector<FeFilter>::const_iterator itr=m_filters.begin();
			itr != m_filters.end(); ++itr )
		(*itr).save( f, otherStrings[0] );
}

bool FeDisplayInfo::show_in_cycle() const
{
	return config_str_to_bool( m_info[InCycle] );
}

bool FeDisplayInfo::show_in_menu() const
{
	return config_str_to_bool( m_info[InMenu] );
}

const char *FeEmulatorInfo::indexStrings[] =
{
	"name",
	"executable",
	"args",
	"workdir",
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
	"Working Directory",
	"Rom Path(s)",
	"Rom Extension(s)",
	"System Identifier",
	"Info Source/Scraper",
	"Additional Import File(s)",
	"Minimum Run Time",
	"Exit Hotkey",
	NULL
};

const char *FeEmulatorInfo::infoSourceStrings[] =
{
	"",
	"listxml",
	"listsoftware",
	"steam",
	"thegamesdb.net",
	"scummvm",
	"listsoftware+thegamesdb.net",
	NULL
};

FeEmulatorInfo::FeEmulatorInfo()
	: m_info_source( None ),
	m_min_run( 0 )
{
}

FeEmulatorInfo::FeEmulatorInfo( const std::string &n )
	: m_name( n ),
	m_info_source( None ),
	m_min_run( 0 )
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
	case Working_dir:
		return m_workdir;
	case Rom_path:
		return vector_to_string( m_paths );
	case Rom_extension:
		return vector_to_string( m_extensions );
	case System:
		return vector_to_string( m_systems );
	case Info_source:
		return infoSourceStrings[m_info_source];
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
	case Working_dir:
		m_workdir = s; break;
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
		for ( int i=0; i<LAST_INFOSOURCE; i++ )
		{
			if ( s.compare( infoSourceStrings[i] ) == 0 )
			{
				m_info_source = (InfoSource)i;
				return;
			}
		}

		//
		// Special case handling for versions <= 1.6.0
		//
		if ( s.compare( "mame" ) == 0 )
			m_info_source = Listxml;
		else if ( s.compare( "mess" ) == 0 )
			m_info_source = Listsoftware;
		else
			m_info_source = None;

		break;

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

void FeEmulatorInfo::gather_rom_names( std::vector<std::string> &name_list ) const
{
	std::vector< std::string > ignored;
	gather_rom_names( name_list, ignored );
}

void FeEmulatorInfo::gather_rom_names(
	std::vector<std::string> &name_list,
	std::vector<std::string> &full_path_list ) const
{
	for ( std::vector<std::string>::const_iterator itr=m_paths.begin();
			itr!=m_paths.end(); ++itr )
	{
		std::string path = clean_path_with_wd( *itr, true );

		for ( std::vector<std::string>::const_iterator ite = m_extensions.begin();
				ite != m_extensions.end(); ++ite )
		{
			std::vector<std::string> temp_list;
			std::vector<std::string>::iterator itn;

			if ( (*ite).compare( FE_DIR_TOKEN ) == 0 )
			{
				get_subdirectories( temp_list, path );

				for ( itn = temp_list.begin(); itn != temp_list.end(); ++itn )
				{
					full_path_list.push_back( path + *itn );
					name_list.push_back( std::string() );
					name_list.back().swap( *itn );
				}
			}
			else
			{
				get_basename_from_extension( temp_list, path, (*ite), true );

				for ( itn = temp_list.begin(); itn != temp_list.end(); ++itn )
				{
					full_path_list.push_back( path + *itn + *ite );
					name_list.push_back( std::string() );
					name_list.back().swap( *itn );
				}
			}
		}
	}
}

std::string FeEmulatorInfo::clean_path_with_wd( const std::string &in_path, bool add_trailing_slash ) const
{
	std::string res = clean_path( in_path, add_trailing_slash );

	if ( is_relative_path( res ) )
	{
		std::string temp;
		temp.swap( res );
		res = clean_path( m_workdir, true ) + temp;
	}

	return res;
}

bool FeEmulatorInfo::is_mame() const
{
	return ( m_info_source == Listxml );
}

bool FeEmulatorInfo::is_mess() const
{
	return (( m_info_source == Listsoftware )
		|| ( m_info_source == Listsoftware_tgdb ));
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
	"intro_config",
	NULL
};

FeLayoutInfo::FeLayoutInfo( Type t )
	: m_type( t )
{
}

FeLayoutInfo::FeLayoutInfo( const std::string &name )
	: m_name( name ),
	m_type( Layout )
{
}

void FeLayoutInfo::save( std::ofstream &f ) const
{
	if ( !m_params.empty() )
	{
		if ( m_type == Layout )
			f << std::endl << indexStrings[ m_type ]
				<< '\t' << m_name << std::endl;
		else
			f << std::endl << indexStrings[ m_type ] << std::endl;

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
