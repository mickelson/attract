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

#include "scraper_xml.hpp"
#include "fe_util.hpp"
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <expat.h>

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

}

//
// Utility function to get strings to use to see if game names match filenames
//
std::string get_fuzzy( const std::string &orig )
{
	std::string retval;
	int word_start( 0 ), i( 0 );
	for ( std::string::const_iterator itr=orig.begin(); (( itr!=orig.end() ) && ( *itr != '(' )); ++itr )
	{
		if ( std::isalnum( *itr ) )
		{
			retval += std::tolower( *itr );
			i++;
		}
		else
		{
			fix_last_word( retval, word_start );
			i=word_start=retval.length();
		}
	}
	fix_last_word( retval, word_start );
	return retval;
}

//
// Base XML Parser
//
// Expat callback functions
void exp_handle_data( void *data, const char *content, int length )
{
	((FeXMLParser *)data)->handle_data( content, length );
}

void exp_start_element( void *data,
			const char *element,
			const char **attribute )
{
	((FeXMLParser *)data)->start_element( element, attribute );
}

void exp_end_element( void *data, const char *element )
{
	((FeXMLParser *)data)->end_element( element );
}

FeXMLParser::FeXMLParser( UiUpdate u, void *d )
	: m_ui_update( u ), m_ui_update_data( d ), m_continue_parse( true )
{
}

void FeXMLParser::handle_data( const char *content, int length )
{
	if ( m_element_open )
		m_current_data.append( content, length );
}

struct user_data_struct
{
	XML_Parser parser;
	bool parsed_xml;
};

bool my_parse_callback( const char *buff, void *opaque )
{
	struct user_data_struct *ds = (struct user_data_struct *)opaque;
	if ( XML_Parse( ds->parser, buff,
			strlen(buff), XML_FALSE ) == XML_STATUS_ERROR )
	{
		std::cout << "Error parsing xml output:"
					<< buff << std::endl;
	}
	else
		ds->parsed_xml = true;

	FeXMLParser *p = (FeXMLParser *)XML_GetUserData( ds->parser );
	return p->get_continue_parse(); // return false to cancel callback
}

bool FeXMLParser::parse_internal(
		const std::string &prog,
		const std::string &args )
{
	struct user_data_struct ud;
	ud.parsed_xml = false;

	m_element_open=m_keep_rom=false;
	m_continue_parse=true;

	ud.parser = XML_ParserCreate( NULL );
	XML_SetUserData( ud.parser, (void *)this );
	XML_SetElementHandler( ud.parser, exp_start_element, exp_end_element );
	XML_SetCharacterDataHandler( ud.parser, exp_handle_data );

	run_program( prog, args, my_parse_callback, (void *)&ud );

	// need to pass true to XML Parse on last line
	XML_Parse( ud.parser, 0, 0, XML_TRUE );
	XML_ParserFree( ud.parser );

	return ud.parsed_xml;
}

bool FeMapComp::operator()(const char *lhs, const char *rhs) const
{
	return ( strcmp( lhs, rhs ) < 0 );
}

FeImporterContext::FeImporterContext( const FeEmulatorInfo &e, FeRomInfoListType &rl )
	: emulator( e ),
	romlist( rl ),
	scrape_art( false ),
	uiupdate( NULL ),
	uiupdatedata( NULL ),
	full( false ),
	progress_past( 0 ),
	progress_range( 100 ),
	download_count( 0 )
{
}

//
// Mame XML Parser
//
FeMameXMLParser::FeMameXMLParser( FeImporterContext &ctx )
	: FeXMLParser( ctx.uiupdate, ctx.uiupdatedata ),
	m_ctx( ctx ),
	m_count( 0 ),
	m_displays( 0 ),
	m_chd( false ),
	m_mechanical( false )
{
}

void FeMameXMLParser::start_element(
			const char *element,
			const char **attribute )
{
	if (( strcmp( element, "game" ) == 0 )
		|| ( strcmp( element, "machine" ) == 0 ))
	{
		int i;
		for ( i=0; attribute[i]; i+=2 )
		{
			if ( strcmp( attribute[i], "name" ) == 0 )
			{
				std::map<const char *, FeRomInfoListType::iterator, FeMapComp>::iterator itr;
				itr = m_map.find( attribute[i+1] );
				if ( itr != m_map.end() )
				{
					m_itr = (*itr).second;
					m_collect_data=true;
					m_displays=0;
					m_chd=false;
					m_mechanical=false;
					m_keep_rom=true;
				}
				else
				{
					if ( m_ctx.full )
					{
						m_ctx.romlist.push_back( FeRomInfo( attribute[i+1] ) );
						m_itr = m_ctx.romlist.end();
						m_itr--;
						m_collect_data=true;
						m_displays=0;
						m_chd=false;
						m_mechanical=false;
						m_keep_rom=true;
					}
					else
						m_collect_data=false;
				}

				break;
			}
		}
		if ( m_collect_data )
		{
			for ( i=0; attribute[i]; i+=2 )
			{
				if ((( strcmp( attribute[i], "isbios" ) == 0 )
						|| ( strcmp( attribute[i], "isdevice" ) == 0 ))
					&& ( strcmp( attribute[i+1], "yes" ) == 0 ))
				{
					m_keep_rom=false;
					break;
				}
				else if ( strcmp( attribute[i], "cloneof" ) == 0 )
					(*m_itr).set_info( FeRomInfo::Cloneof, attribute[i+1] );
				else if ( strcmp( attribute[i], "romof" ) == 0 )
					(*m_itr).set_info( FeRomInfo::AltRomname, attribute[i+1] );
				else if (( strcmp( attribute[i], "ismechanical" ) == 0 )
					&& ( strcmp( attribute[i+1], "yes" ) == 0 ))
				{
					m_mechanical = true;
				}
			}
		}
	}
	else if ( m_collect_data )
	{
		if ( strcmp( element, "input" ) == 0 )
		{
			for ( int i=0; attribute[i]; i+=2 )
			{
				if (( strcmp( attribute[i], "players" ) == 0 )
					&& (*m_itr).get_info( FeRomInfo::Players ).empty() )
				{
					(*m_itr).set_info( FeRomInfo::Players, attribute[i+1] );
					break;
				}
			}
		}
		else if ( strcmp( element, "display" ) == 0 )
		{
			for ( int i=0; attribute[i]; i+=2 )
			{
				if ( strcmp( attribute[i], "rotate" ) == 0 )
					(*m_itr).set_info( FeRomInfo::Rotation, attribute[i+1] );
				else if ( strcmp( attribute[i], "type" ) == 0 )
					(*m_itr).set_info( FeRomInfo::DisplayType, attribute[i+1] );
			}
			m_displays++;
		}
		else if ( strcmp( element, "driver" ) == 0 )
		{
			for ( int i=0; attribute[i]; i+=2 )
			{
				if ( strcmp( attribute[i], "status" ) == 0 )
				{
					(*m_itr).set_info( FeRomInfo::Status, attribute[i+1] );
					break;
				}
			}
		}
		else if ( strcmp( element, "control" ) == 0 )
		{
			std::string type, ways, old_type;

			old_type = (*m_itr).get_info( FeRomInfo::Control );
			if ( !old_type.empty() )
				old_type += ",";

			for ( int i=0; attribute[i]; i+=2 )
			{
				if ( strcmp( attribute[i], "type" ) == 0 )
					type = attribute[i+1];
				else if ( strcmp( attribute[i], "ways" ) == 0 )
					ways = attribute[i+1];
			}

			struct my_map_struct { const char *in; const char *out; };
			my_map_struct my_map[] =
			{
				{ "stick", "joystick (analog)" },
				{ "doublejoy", "double joystick" },
				{ NULL, NULL }
			};

			if ( type.compare( "joy" ) == 0 )
			{
				// construct the joystick name
				//
				type = "joystick (";
				type += ways;
				type += "-way)";
			}
			else
			{
				// we also do a bit of name remapping
				//
				int i=0;
				while ( my_map[i].in != NULL )
				{
					if ( type.compare( my_map[i].in ) == 0 )
					{
						type = my_map[i].out;
						break;
					}
					i++;
				}
			}
			(*m_itr).set_info( FeRomInfo::Control, old_type + type );
		}
		else if ( strcmp( element, "disk" ) == 0 )
		{
			m_chd=true;
		}
		// "cloneof" and "genre" elements appear in hyperspin .xml
		else if (( strcmp( element, "description" ) == 0 )
				|| ( strcmp( element, "cloneof" ) == 0 )
				|| ( strcmp( element, "genre" ) == 0 )
				|| ( strcmp( element, "year" ) == 0 )
				|| ( strcmp( element, "manufacturer" ) == 0 ))
		{
			m_element_open=true;
		}
	}
}

void FeMameXMLParser::end_element( const char *element )
{
	if (( strcmp( element, "game" ) == 0 )
		|| ( strcmp( element, "machine" ) == 0 ))
	{
		if ( m_collect_data )
		{
			m_collect_data = false;

			if ( !m_keep_rom )
				m_discarded.push_back( m_itr );
			else
			{
				//
				// Construct the extra info now
				//
				std::string extra;
				if ( m_chd )
					extra += "chd";

				if ( m_mechanical )
				{
					if ( !extra.empty() )
						extra += ",";

					extra += "mechanical";
				}

				(*m_itr).set_info( FeRomInfo::Extra, extra );
				(*m_itr).set_info( FeRomInfo::DisplayCount, as_str( m_displays ) );
			}

			m_count++;

			int percent( 0 );
			if ( !m_ctx.full && ( m_ctx.romlist.size() > 0 ))
			{
				percent = m_ctx.progress_past
					+ m_count * m_ctx.progress_range
					/ m_ctx.romlist.size();

				std::cout << "\b\b\b\b" << std::setw(3)
					<< percent << '%' << std::flush;
			}

			if ( m_ui_update )
			{
				if ( m_ui_update( m_ui_update_data,
						percent,
						(*m_itr).get_info( FeRomInfo::Title ) ) == false )
					set_continue_parse( false );
			}
		}
	}

	if ( m_element_open )
	{
		if ( strcmp( element, "description" ) == 0 )
			(*m_itr).set_info( FeRomInfo::Title, m_current_data );
		else if ( strcmp( element, "year" ) == 0 )
			(*m_itr).set_info( FeRomInfo::Year, m_current_data );
		else if ( strcmp( element, "manufacturer" ) == 0 )
			(*m_itr).set_info( FeRomInfo::Manufacturer, m_current_data );
		else if ( strcmp( element, "cloneof" ) == 0 ) // Hyperspin .xml
			(*m_itr).set_info( FeRomInfo::Cloneof, m_current_data );
		else if ( strcmp( element, "genre" ) == 0 ) // Hyperspin .xml
			(*m_itr).set_info( FeRomInfo::Category, m_current_data );

		m_current_data.clear();
		m_element_open=false;
	}
}

void FeMameXMLParser::pre_parse()
{
	FeRomInfoListType::iterator itr;
	m_count=0;

	m_map.clear();
	for ( FeRomInfoListType::iterator itr=m_ctx.romlist.begin();
			itr != m_ctx.romlist.end(); ++itr )
		m_map[ (*itr).get_info( FeRomInfo::Romname ).c_str() ] = itr;

	std::cout << "    ";
}

void FeMameXMLParser::post_parse()
{
	std::cout << std::endl;

	if ( !m_discarded.empty() )
	{
		std::cout << " - Discarded " << m_discarded.size()
				<< " entries based on xml info: ";
		std::vector<FeRomInfoListType::iterator>::iterator itr;
		for ( itr = m_discarded.begin(); itr != m_discarded.end(); ++itr )
		{
			std::cout << (*(*itr)).get_info( FeRomInfo::Romname ) << " ";
			m_ctx.romlist.erase( (*itr) );
		}
		std::cout << std::endl;
	}
}

bool FeMameXMLParser::parse_command( const std::string &prog )
{
	pre_parse();

	std::string base_args = "-listxml";

	//
	// Special case for really small romlists... this gets used in
	// connection with -listsoftware parsing as well
	//
	bool ret_val=true;
	if (  m_ctx.romlist.size() < 10 )
	{
		for ( FeRomInfoListType::iterator itr=m_ctx.romlist.begin();
				itr != m_ctx.romlist.end(); ++itr )
		{
			parse_internal( prog,
				base_args + " "
				+ (*itr).get_info( FeRomInfo::Romname ) );
		}
	}
	else
		ret_val = parse_internal( prog, base_args );

	post_parse();
	return ret_val;
}

bool FeMameXMLParser::parse_file( const std::string &filename )
{
	pre_parse();

	m_element_open=m_keep_rom=false;
	m_continue_parse=true;

	std::ifstream myfile( filename.c_str() );
	if ( !myfile.is_open() )
	{
		std::cerr << "Error opening file: " << filename << std::endl;
		return false;
	}

	XML_Parser parser = XML_ParserCreate( NULL );
	XML_SetUserData( parser, (void *)this );
	XML_SetElementHandler( parser, exp_start_element, exp_end_element );
	XML_SetCharacterDataHandler( parser, exp_handle_data );
	bool ret_val = true;

	while ( myfile.good() )
	{
		std::string line;
		getline( myfile, line );

		if ( XML_Parse( parser, line.c_str(),
				line.size(), XML_FALSE ) == XML_STATUS_ERROR )
		{
			std::cout << "Error parsing xml:"
				<< line << std::endl;
			ret_val = false;
			break;
		}
	}

	myfile.close();

	// need to pass true to XML Parse on last line
	XML_Parse( parser, 0, 0, XML_TRUE );
	XML_ParserFree( parser );

	post_parse();
	return ret_val;
}

//
// Mess XML Parser
//
FeMessXMLParser::FeMessXMLParser( FeImporterContext &ctx )
	: FeXMLParser( ctx.uiupdate, ctx.uiupdatedata ),
	m_ctx( ctx )
{
}

void FeMessXMLParser::clear_parse_state()
{
	m_element_open=m_keep_rom=false;
	m_name.clear();
	m_description.clear();
	m_year.clear();
	m_man.clear();
	m_fuzzydesc.clear();
	m_cloneof.clear();
	m_altname.clear();
	m_alttitle.clear();
}

void FeMessXMLParser::start_element(
			const char *element,
			const char **attribute )
{
	if ( strcmp( element, "software" ) == 0 )
	{
		int i;
		for ( i=0; attribute[i]; i+=2 )
		{
			if ( strcmp( attribute[i], "name" ) == 0 )
			{
				m_altname = m_name = attribute[i+1];

				for ( m_itr=m_ctx.romlist.begin(); m_itr!=m_ctx.romlist.end(); ++m_itr )
				{
					std::string romname = (*m_itr).get_info( FeRomInfo::Romname );
					if ( romname.compare( attribute[i+1] ) == 0 )
					{
						m_keep_rom=true;
						break;
					}
				}

				if ( !m_keep_rom && m_ctx.full )
				{
					m_ctx.romlist.push_back( FeRomInfo( attribute[i+1] ) );
					m_itr = m_ctx.romlist.end();
					m_itr--;
					m_keep_rom=true;
				}
			}
			else if ( strcmp( attribute[i], "cloneof" ) == 0 )
			{
				m_cloneof = attribute[i+1];
			}
		}
	}
	else if (( strcmp( element, "description" ) == 0 )
			|| ( strcmp( element, "year" ) == 0 )
			|| ( strcmp( element, "publisher" ) == 0 ))
	{
		m_element_open=true;
	}
	else if ( strcmp( element, "info" ) == 0 )
	{
		std::string value;
		bool found=false;

		for ( int i=0; attribute[i]; i+=2 )
		{
			if (( strcmp( attribute[i], "name" ) == 0 )
						&& ( strcmp( attribute[i+1], "alt_title" ) == 0 ))
				found = true;
			else if ( strcmp( attribute[i], "value" ) == 0 )
				value = attribute[i+1];
		}

		if ( found )
			m_alttitle.swap( value );
	}
}

void FeMessXMLParser::end_element( const char *element )
{
	if ( strcmp( element, "software" ) == 0 )
	{
		if ( m_keep_rom )
		{
			set_info_values( *m_itr );
		}
		else
		{
			// otherwise try matching based on description
			for ( m_itr=m_ctx.romlist.begin(); m_itr!=m_ctx.romlist.end(); ++m_itr )
			{
				if ( m_description.compare(
							(*m_itr).get_info( FeRomInfo::Romname ) ) == 0 )
				{
					set_info_values( *m_itr );
				}
				else if ( (*m_itr).get_info( FeRomInfo::AltRomname ).empty() )
				{
					// Try using a "fuzzy" match (ignores brackets).
					//
					const std::string &temp = (*m_itr).get_info( FeRomInfo::BuildScratchPad );
					if ( temp.compare( m_fuzzydesc ) == 0 )
					{
						set_info_values( *m_itr );
					}
				}
			}
		}
		clear_parse_state();
	}
	else if ( m_element_open )
	{
		if ( strcmp( element, "description" ) == 0 )
		{
			m_description = m_current_data;
			m_fuzzydesc = get_fuzzy( m_current_data );
		}
		else if ( strcmp( element, "year" ) == 0 )
			m_year = m_current_data;
		else if ( strcmp( element, "publisher" ) == 0 )
			m_man = m_current_data;

		m_current_data.clear();
		m_element_open=false;
	}
}

void FeMessXMLParser::set_info_values( FeRomInfo &r )
{
	r.set_info( FeRomInfo::Title, m_description );
	r.set_info( FeRomInfo::Year, m_year );
	r.set_info( FeRomInfo::Manufacturer, m_man );
	r.set_info( FeRomInfo::Cloneof, m_cloneof );
	r.set_info( FeRomInfo::AltRomname, m_altname );
	r.set_info( FeRomInfo::AltTitle, m_alttitle );
}

bool FeMessXMLParser::parse( const std::string &prog,
		const std::vector < std::string > &system_names )
{
	// First get our machine -listxml settings
	//
	std::string system_name;
	FeRomInfoListType::iterator itr;

	for ( std::vector<std::string>::const_iterator its=system_names.begin(); its!=system_names.end(); ++its )
	{
		FeRomInfoListType temp_list;
		temp_list.push_back( FeRomInfo( *its ) );

		FeEmulatorInfo ignored;
		FeImporterContext temp( ignored, temp_list );
		FeMameXMLParser listxml( temp );

		if (( listxml.parse_command( prog ) )
				&& ( !temp_list.empty() ))
		{
			const FeRomInfo &ri = temp_list.front();
			for ( itr=m_ctx.romlist.begin(); itr!=m_ctx.romlist.end(); ++itr )
			{
				(*itr).set_info( FeRomInfo::Players, ri.get_info( FeRomInfo::Players ));
				(*itr).set_info( FeRomInfo::Rotation, ri.get_info( FeRomInfo::Rotation ));
				(*itr).set_info( FeRomInfo::Control, ri.get_info( FeRomInfo::Control ));
				(*itr).set_info( FeRomInfo::Status, ri.get_info( FeRomInfo::Status ));
				(*itr).set_info( FeRomInfo::DisplayCount, ri.get_info( FeRomInfo::DisplayCount ));
				(*itr).set_info( FeRomInfo::DisplayType, ri.get_info( FeRomInfo::DisplayType ));

				// A bit of a hack here: the Category field gets repurposed for this stage of a MESS
				// import...We temporarily store a "fuzzy" match romname
				//
				(*itr).set_info( FeRomInfo::BuildScratchPad,
							get_fuzzy( (*itr).get_info( FeRomInfo::Romname ) ) );
			}
			system_name=(*its);
			break;
		}
	}

	if ( system_name.empty() )
	{
		std::cerr << " * Error: No system identifier found that is recognized by MESS -listxml" << std::endl;
		return false;
	}

	std::cout << " - Obtaining -listsoftware info [" << system_name << "]" << std::endl;

	// Now get the individual game -listsoftware settings
	//
	int retval=parse_internal( prog, system_name + " -listsoftware" );

	if ( !retval )
	{
		std::cout << " * Error: No XML output found, command: " << prog << " "
					<< system_name + " -listsoftware" << std::endl;
	}

	// We're done with our "fuzzy" matching, so clear where we were storing them
	//
	for ( itr=m_ctx.romlist.begin(); itr!=m_ctx.romlist.end(); ++itr )
		(*itr).set_info( FeRomInfo::BuildScratchPad, "" );

	return retval;
}

FeGameDBPlatformListParser::FeGameDBPlatformListParser()
	: m_id( -1 )
{
}

void FeGameDBPlatformListParser::start_element(
			const char *element,
			const char **attribute )
{
	if (( strcmp( element, "name" ) == 0 )
			|| ( strcmp( element, "id" ) == 0 ))
		m_element_open=true;
}

void FeGameDBPlatformListParser::end_element( const char *element )
{
	if ( strcmp( element, "name" ) == 0 )
	{
		m_name = m_current_data;
		m_current_data.clear();
		m_element_open=false;
	}
	else if ( strcmp( element, "id" ) == 0 )
	{
		m_id = as_int( m_current_data );
		m_current_data.clear();
		m_element_open=false;
	}
	else if ( strcmp( element, "Platform" ) == 0 )
	{
		m_set[ m_name ] = m_id;
		m_name.clear();
		m_id=-1;
	}
}

bool FeGameDBPlatformListParser::parse( const std::string &data )
{
	m_set.clear();
	m_element_open=m_keep_rom=false;
	m_continue_parse=true;
	bool ret_val=true;

	XML_Parser parser = XML_ParserCreate( NULL );
	XML_SetUserData( parser, (void *)this );
	XML_SetElementHandler( parser, exp_start_element, exp_end_element );
	XML_SetCharacterDataHandler( parser, exp_handle_data );

	if ( XML_Parse( parser, data.c_str(),
			data.size(), XML_FALSE ) == XML_STATUS_ERROR )
	{
		std::cout << "Error parsing xml." << std::endl;
		ret_val = false;
	}

	// need to pass true to XML Parse on last line
	XML_Parse( parser, 0, 0, XML_TRUE );
	XML_ParserFree( parser );

	return ret_val;
}

void FeGameDBArt::clear()
{
	base.clear();

	flyer.clear();
	wheel.clear();
	snap.clear();
	marquee.clear();
	fanart.clear();
}

FeGameDBPlatformParser::FeGameDBPlatformParser( FeGameDBArt &art )
	: m_art( art )
{
}

void FeGameDBPlatformParser::start_element(
			const char *element,
			const char **attribute )
{
	if (( strcmp( element, "baseImgUrl" ) == 0 )
			|| ( strcmp( element, "banner" ) == 0 )
			|| ( strcmp( element, "original" ) == 0 )
			|| ( strcmp( element, "consoleart" ) == 0 ))
	{
		m_element_open=true;
	}
	else if ( strcmp( element, "boxart" ) == 0 )
	{
		std::string side;
		int width( 0 ), height( 0 );

		for ( int i=0; attribute[i]; i+=2 )
		{
			if ( strcmp( attribute[i], "side" ) == 0 )
				side = attribute[i+1];
			else if ( strcmp( attribute[i], "width" ) == 0 )
				width = atoi( attribute[i+1] );
			else if ( strcmp( attribute[i], "height" ) == 0 )
				height = atoi( attribute[i+1] );
		}

		// don't bother if it is too big
		if (( width * height ) >= 4194304 )
			return;

		if (( side.compare( "front" ) == 0 )
				|| ( m_art.flyer.empty() ))
			m_element_open = true;
	}
}
void FeGameDBPlatformParser::end_element( const char *element )
{
	if ( !m_element_open )
		return;

	if ( strcmp( element, "baseImgUrl" ) == 0 )
		m_art.base = m_current_data;
	else if ( strcmp( element, "banner" ) == 0 )
		m_art.marquee = m_current_data;
	else if ( strcmp( element, "consoleart" ) == 0 )
		m_art.wheel = m_current_data;
	else if ( strcmp( element, "original" ) == 0 )
		m_art.fanart.push_back( m_current_data );
	else if ( strcmp( element, "boxart" ) == 0 )
		m_art.flyer = m_current_data;

	m_element_open=false;
	m_current_data.clear();
}

bool FeGameDBPlatformParser::parse( const std::string &data )
{
	m_element_open=m_keep_rom=false;
	m_continue_parse=true;
	bool ret_val=true;

	XML_Parser parser = XML_ParserCreate( NULL );
	XML_SetUserData( parser, (void *)this );
	XML_SetElementHandler( parser, exp_start_element, exp_end_element );
	XML_SetCharacterDataHandler( parser, exp_handle_data );

	if ( XML_Parse( parser, data.c_str(),
			data.size(), XML_FALSE ) == XML_STATUS_ERROR )
	{
		std::cout << "Error parsing xml." << std::endl;
		ret_val = false;
	}

	// need to pass true to XML Parse on last line
	XML_Parse( parser, 0, 0, XML_TRUE );
	XML_ParserFree( parser );

	return ret_val;
}

FeGameDBParser::FeGameDBParser( std::vector<std::string> &system_list, FeRomInfo &rom, FeGameDBArt *art )
	: m_system_list( system_list ),
	m_rom( rom ),
	m_art( art ),
	m_screenshot( false ),
	m_fanart( false ),
	m_ignore( false )
{
}

void FeGameDBParser::start_element(
			const char *element,
			const char **attribute )
{
	if ( m_ignore )
		return;
	else if ( strcmp( element, "Similar") == 0 )
		m_ignore=true;
	else if ( strcmp( element, "Game" ) == 0 )
	{
		m_work.clear();
		m_work_art.clear();
		m_work_platform.clear();
	}
	else if (( strcmp( element, "ReleaseDate" ) == 0 )
			|| ( strcmp( element, "genre" ) == 0 )
			|| ( strcmp( element, "Players" ) == 0 )
			|| ( strcmp( element, "Publisher" ) == 0 )
			|| ( strcmp( element, "Platform" ) == 0 )
			|| ( strcmp( element, "GameTitle" ) == 0 )
			|| ( strcmp( element, "title" ) == 0 )
		)
	{
		m_element_open=true;
	}
	else if ( strcmp( element, "Genres" ) == 0 )
	{
		m_work.set_info( FeRomInfo::Category, "" );
	}
	else if ( m_art )
	{
		if (( strcmp( element, "baseImgUrl" ) == 0 )
				|| ( strcmp( element, "banner" ) == 0 )
				|| ( strcmp( element, "clearlogo" ) == 0 ))
			m_element_open=true;


		else if ( strcmp( element, "fanart" ) == 0 )
			m_fanart=true;

		else if ( strcmp( element, "screenshot" ) == 0 )
			m_screenshot=true;

		else if ( ( m_screenshot || m_fanart ) && ( strcmp( element, "original" ) == 0 ))
			m_element_open=true;

		else if ( strcmp( element, "boxart" ) == 0 )
		{
			std::string side, thumb;
			int width( 0 ), height( 0 );

			for ( int i=0; attribute[i]; i+=2 )
			{
				if ( strcmp( attribute[i], "side" ) == 0 )
					side = attribute[i+1];
				else if ( strcmp( attribute[i], "thumb" ) == 0 )
					thumb = attribute[i+1];
				else if ( strcmp( attribute[i], "width" ) == 0 )
					width = atoi( attribute[i+1] );
				else if ( strcmp( attribute[i], "height" ) == 0 )
					height = atoi( attribute[i+1] );
			}

			if ( side.compare( "front" ) == 0 )
			{
				// If the flyer is goint to be really big, use the thumb instead.
				if (( width * height ) < 4194304 )
					m_element_open = true;
				else
					m_work_art.flyer = thumb;
			}
		}
	}
}

void FeGameDBParser::end_element( const char *element )
{
	if ( strcmp( element, "Similar" ) == 0 )
	{
		m_ignore=false;
		return;
	}
	if ( m_ignore )
		return;

	if ( m_element_open )
	{
		if ( strcmp( element, "GameTitle" ) == 0 )
			m_work.set_info( FeRomInfo::Title, m_current_data );
		else if ( strcmp( element, "title" ) == 0 )
		{
			if ( m_work.get_info( FeRomInfo::AltTitle ).empty() )
				m_work.set_info( FeRomInfo::AltTitle, m_current_data );
		}
		else if ( strcmp( element, "ReleaseDate" ) == 0 )
		{
			size_t cut = m_current_data.find_last_of( "/" );

			if ( cut != std::string::npos )
				m_work.set_info( FeRomInfo::Year, m_current_data.substr( cut+1 ) );
			else
				m_work.set_info( FeRomInfo::Year, m_current_data );
		}
		else if ( strcmp( element, "genre" ) == 0 )
		{
			std::string cat = m_work.get_info( FeRomInfo::Category );
			if ( cat.size() == 0 )
				cat = m_current_data;
			else
			{
				cat += " / ";
				cat += m_current_data;
			}
			m_work.set_info( FeRomInfo::Category, cat );
		}
		else if ( strcmp( element, "Players" ) == 0 )
			m_work.set_info( FeRomInfo::Players, m_current_data );
		else if ( strcmp( element, "Publisher" ) == 0 )
			m_work.set_info( FeRomInfo::Manufacturer, m_current_data );
		else if ( strcmp( element, "Platform" ) == 0 )
			m_work_platform = m_current_data;

		else if ( m_art )
		{
			if ( strcmp( element, "baseImgUrl" ) == 0 )
				m_art->base = m_current_data;
			else if ( strcmp( element, "banner" ) == 0 )
				m_work_art.marquee = m_current_data;
			else if ( strcmp( element, "clearlogo" ) == 0 )
				m_work_art.wheel = m_current_data;
			else if ( strcmp( element, "original" ) == 0 )
			{
				if ( m_fanart )
				{
					m_work_art.fanart.push_back( m_current_data );
					m_fanart = false;
				}
				else
				{
					if ( m_work_art.snap.empty() )
						m_work_art.snap = m_current_data;

					m_screenshot=false;
				}
			}
			else if ( strcmp( element, "boxart" ) == 0 )
				m_work_art.flyer = m_current_data;
		}

		m_element_open=false;
		m_current_data.clear();
	}

	if ( strcmp( element, "Game" ) == 0 )
	{
		std::string title_str = get_fuzzy( m_rom.get_info( FeRomInfo::Title ) );
		std::string work_title = get_fuzzy( m_work.get_info(FeRomInfo::Title ) );

		bool match_plat=false;
		for ( std::vector<std::string>::iterator itr=m_system_list.begin(); itr!=m_system_list.end(); ++itr )
		{
			if ( m_work_platform.compare( *itr ) == 0 )
			{
				match_plat=true;
				break;
			}
		}

		if ( work_title.compare( title_str ) == 0 )
		{
			if ( match_plat )
			{
				m_work.set_info( FeRomInfo::Emulator, m_rom.get_info( FeRomInfo::Emulator ) );
				m_work.set_info( FeRomInfo::Romname, m_rom.get_info( FeRomInfo::Romname ) );
				m_rom = m_work;

				if ( m_art )
				{
					m_art->snap = m_work_art.snap;
					m_art->marquee = m_work_art.marquee;
					m_art->flyer = m_work_art.flyer;
					m_art->fanart = m_work_art.fanart;

					if ( !m_work_art.wheel.empty() )
						m_art->wheel = m_work_art.wheel;
				}
			}
			else if ( m_art && !m_work_art.wheel.empty() && m_art->wheel.empty() )
				m_art->wheel = m_work_art.wheel;
		}
		else
		{
			std::string work_alt = get_fuzzy( m_work.get_info(FeRomInfo::AltTitle ) );
			bool match_alt = ( work_alt.compare( title_str ) == 0 );

			if ( match_alt && !m_work_art.wheel.empty() && m_art->wheel.empty() )
				m_art->wheel = m_work_art.wheel;

			if ( match_plat && match_alt )
			{
				if ( !m_work_art.flyer.empty() && m_art->flyer.empty() )
					m_art->flyer = m_work_art.flyer;

				if ( !m_work_art.marquee.empty() && m_art->marquee.empty() )
					m_art->marquee = m_work_art.marquee;

				if ( !m_work_art.snap.empty() && m_art->snap.empty() )
					m_art->snap = m_work_art.snap;
			}
		}
	}
}

bool FeGameDBParser::parse( const std::string &data )
{
	m_element_open=m_keep_rom=m_ignore=false;
	m_screenshot=m_fanart=false;
	m_continue_parse=true;
	bool ret_val=true;

	XML_Parser parser = XML_ParserCreate( NULL );
	XML_SetUserData( parser, (void *)this );
	XML_SetElementHandler( parser, exp_start_element, exp_end_element );
	XML_SetCharacterDataHandler( parser, exp_handle_data );

	if ( XML_Parse( parser, data.c_str(),
			data.size(), XML_FALSE ) == XML_STATUS_ERROR )
	{
		std::cout << "Error parsing xml." << std::endl;
		ret_val = false;
	}

	// need to pass true to XML Parse on last line
	XML_Parse( parser, 0, 0, XML_TRUE );
	XML_ParserFree( parser );

	return ret_val;
}
