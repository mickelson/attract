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

#include "fe_xml.hpp"
#include "fe_util.hpp"
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <expat.h>

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

//
// Mame XML Parser
//
FeMameXMLParser::FeMameXMLParser(
		FeRomInfoListType &romlist,
		UiUpdate u,
		void *d )
	: FeXMLParser( u, d ),
	m_romlist( romlist ),
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
					m_collect_data=false;
				}

				break;
			}
		}
		if ( m_collect_data == true )
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
	else if ( m_collect_data==true )
	{
		if ( strcmp( element, "input" ) == 0 )
		{
			for ( int i=0; attribute[i]; i+=2 )
			{
				if ( strcmp( attribute[i], "players" ) == 0 )
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
		else if (( strcmp( element, "description" ) == 0 )
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

			int percent = m_count * 100 / m_romlist.size();
			if ( percent > m_percent )
			{
				m_percent = percent;
				std::cout << "\b\b\b\b" << std::setw(3)
					<< m_percent << '%' << std::flush;

				if ( m_ui_update )
				{
					if ( m_ui_update( m_ui_update_data, m_percent ) == false )
						set_continue_parse( false );
				}
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

		m_current_data.clear();
		m_element_open=false;
	}
}

bool FeMameXMLParser::parse( const std::string &prog )
{
	std::string base_args = "-listxml";
	FeRomInfoListType::iterator itr;
	m_percent=m_count=0;

	//
	// run "mame -listxml" and find each rom.
	//
	m_map.clear();
	for ( FeRomInfoListType::iterator itr=m_romlist.begin();
			itr != m_romlist.end(); ++itr )
		m_map[ (*itr).get_info( FeRomInfo::Romname ).c_str() ] = itr;

	std::cout << "    ";

	if ( parse_internal( prog, base_args ) == false )
	{
		std::cout << "No XML output found, command: " << prog << " "
					<< base_args << std::endl;
	}

	std::cout << std::endl;

	if ( !m_discarded.empty() )
	{
		std::cout << "Discarded " << m_discarded.size()
				<< " entries based on xml info: ";
		std::vector<FeRomInfoListType::iterator>::iterator itr;
		for ( itr = m_discarded.begin(); itr != m_discarded.end(); ++itr )
		{
			std::cout << (*(*itr)).get_info( FeRomInfo::Romname ) << " ";
			m_romlist.erase( (*itr) );
		}
		std::cout << std::endl;
	}

	return true;
}

//
// Mess XML Parser
//
FeMessXMLParser::FeMessXMLParser(
	FeRomInfoListType &romlist,
	UiUpdate u,
	void *d )
	: FeXMLParser( u, d ), m_romlist( romlist )
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

				for ( m_itr=m_romlist.begin(); m_itr!=m_romlist.end(); ++m_itr )
				{
					std::string romname = (*m_itr).get_info( FeRomInfo::Romname );
					if ( romname.compare( attribute[i+1] ) == 0 )
					{
						m_keep_rom=true;
						break;
					}
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
			for ( m_itr=m_romlist.begin(); m_itr!=m_romlist.end(); ++m_itr )
			{
				if ( m_description.compare(
							(*m_itr).get_info( FeRomInfo::Romname ) ) == 0 )
				{
					set_info_values( *m_itr );
				}
				else if ( (*m_itr).get_info( FeRomInfo::AltRomname ).empty() )
				{
					// Try using a "fuzzy" match that ignores brackets.
					//
					const std::string &temp = (*m_itr).get_info( FeRomInfo::BuildScratchPad );

					if (( temp.find( m_fuzzydesc ) != std::string::npos )
						|| ( m_fuzzydesc.find( temp ) != std::string::npos ))
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
			size_t pos=0;
			token_helper( m_current_data, pos, m_fuzzydesc, "(" );
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
		const std::string &system_name )
{
	// First get our machine -listxml settings
	//
	FeRomInfoListType temp_list;
	temp_list.push_back( FeRomInfo( system_name ) );

	FeMameXMLParser listxml( temp_list );
	listxml.parse( prog );

	FeRomInfoListType::iterator itr;
	if ( !temp_list.empty() )
	{
		const FeRomInfo &ri = temp_list.front();
		for ( itr=m_romlist.begin(); itr!=m_romlist.end(); ++itr )
		{
			(*itr).set_info( FeRomInfo::Players, ri.get_info( FeRomInfo::Players ));
			(*itr).set_info( FeRomInfo::Rotation, ri.get_info( FeRomInfo::Rotation ));
			(*itr).set_info( FeRomInfo::Control, ri.get_info( FeRomInfo::Control ));
			(*itr).set_info( FeRomInfo::Status, ri.get_info( FeRomInfo::Status ));
			(*itr).set_info( FeRomInfo::DisplayCount, ri.get_info( FeRomInfo::DisplayCount ));
			(*itr).set_info( FeRomInfo::DisplayType, ri.get_info( FeRomInfo::DisplayType ));

			// A bit of a hack here: the Category field gets repurposed for this stage of a MESS
			// import...We temporarily store a "fuzzy" match romname (i.e. the name stripped of
			// bracketted text) for each rom
			//
			size_t pos=0;
			std::string temp;
			token_helper( (*itr).get_info( FeRomInfo::Romname ), pos, temp, "(" );
			(*itr).set_info( FeRomInfo::BuildScratchPad, temp );
		}
	}

	// Now get the individual game -listsoftware settings
	//
	int retval=parse_internal( prog, system_name + " -listsoftware" );

	// We're done with our "fuzzy" matching, so clear where we were storing them
	//
	for ( itr=m_romlist.begin(); itr!=m_romlist.end(); ++itr )
		(*itr).set_info( FeRomInfo::BuildScratchPad, "" );

	return retval;
}

FeHyperSpinXMLParser::FeHyperSpinXMLParser(  FeRomInfoListType & li )
	: m_romlist( li ), m_collect_data( false )
{
}

void FeHyperSpinXMLParser::start_element(
			const char *element,
			const char **attribute )
{
	if ( strcmp( element, "game" ) == 0 )
	{
		m_current_rom.clear();
		m_collect_data = true;

		for ( int i=0; attribute[i]; i+=2 )
		{
			if ( strcmp( attribute[i], "name" ) == 0 )
			{
				m_current_rom.set_info( FeRomInfo::Romname, attribute[i+1] );
				break;
			}
		}
	}
	else if ( m_collect_data )
	{
		if (( strcmp( element, "description" ) == 0 )
			|| ( strcmp( element, "cloneof" ) == 0 )
			|| ( strcmp( element, "year" ) == 0 )
			|| ( strcmp( element, "manufacturer" ) == 0 )
			|| ( strcmp( element, "genre" ) == 0 ))
		{
			m_element_open=true;
		}
	}
}

void FeHyperSpinXMLParser::end_element( const char *element )
{
	if ( strcmp( element, "game" ) == 0 )
	{
		m_romlist.push_back( m_current_rom );
		m_collect_data = false;
	}
	else if ( m_element_open )
	{
		if ( strcmp( element, "description" ) == 0 )
			m_current_rom.set_info( FeRomInfo::Title, m_current_data );
		else if ( strcmp( element, "cloneof" ) == 0 )
			m_current_rom.set_info( FeRomInfo::Cloneof, m_current_data );
		else if ( strcmp( element, "manufacturer" ) == 0 )
			m_current_rom.set_info( FeRomInfo::Manufacturer, m_current_data );
		else if ( strcmp( element, "year" ) == 0 )
			m_current_rom.set_info( FeRomInfo::Year, m_current_data );
		else if ( strcmp( element, "genre" ) == 0 )
			m_current_rom.set_info( FeRomInfo::Category, m_current_data );

		m_current_data.clear();
		m_element_open=false;
	}
}

bool FeHyperSpinXMLParser::parse( const std::string &filename )
{
	m_element_open=m_keep_rom=false;
	m_continue_parse=true;

	XML_Parser parser = XML_ParserCreate( NULL );
	XML_SetUserData( parser, (void *)this );
	XML_SetElementHandler( parser, exp_start_element, exp_end_element );
	XML_SetCharacterDataHandler( parser, exp_handle_data );

	std::ifstream myfile( filename.c_str() );
	if ( !myfile.is_open() )
	{
		std::cerr << "Error opening file: " << filename << std::endl;
		XML_ParserFree( parser );
		return false;
	}

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

	return ret_val;
}

void FeGameDBPlatformParser::start_element(
			const char *element,
			const char **attribute )
{
	if ( strcmp( element, "name" ) == 0 )
		m_element_open=true;
}

void FeGameDBPlatformParser::end_element( const char *element )
{
	if ( strcmp( element, "name" ) == 0 )
	{
		m_set.insert( m_current_data );
		m_current_data.clear();
		m_element_open=false;
	}
}

bool FeGameDBPlatformParser::parse( const std::string &data )
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

FeGameDBParser::FeGameDBParser( FeRomInfo &rom )
	: m_rom( rom ),
	m_collect_data( false ),
	m_exact_match( false )
{
}

void FeGameDBParser::start_element(
			const char *element,
			const char **attribute )
{
	if ( m_collect_data )
	{
		if (( strcmp( element, "ReleaseDate" ) == 0 )
				|| ( strcmp( element, "genre" ) == 0 )
				|| ( strcmp( element, "Players" ) == 0 )
				|| ( strcmp( element, "Publisher" ) == 0 ))
		{
			m_element_open=true;
		}

		if ( strcmp( element, "Genres" ) == 0 )
			m_rom.set_info( FeRomInfo::Category, "" );
	}

	if ( strcmp( element, "GameTitle" ) == 0 )
		m_element_open=true;
}

void FeGameDBParser::end_element( const char *element )
{
	if ( m_element_open )
	{
		if ( strcmp( element, "GameTitle" ) == 0 )
		{
			bool m = ( m_current_data.compare(
								name_with_brackets_stripped(
								m_rom.get_info( FeRomInfo::Romname ) ) ) == 0 );

			if ( m )
				m_exact_match = true;

			if ( m || m_collect_data )
			{
				m_rom.set_info( FeRomInfo::Title, m_current_data );
				m_collect_data = true;
			}
		}
		else if ( m_collect_data )
		{
			if ( strcmp( element, "ReleaseDate" ) == 0 )
			{
				size_t cut = m_current_data.find_last_of( "/" );

				if ( cut != std::string::npos )
					m_rom.set_info( FeRomInfo::Year, m_current_data.substr( cut+1 ) );
				else
					m_rom.set_info( FeRomInfo::Year, m_current_data );
			}
			else if ( strcmp( element, "genre" ) == 0 )
			{
				std::string cat = m_rom.get_info( FeRomInfo::Category );
				if ( cat.size() == 0 )
					cat = m_current_data;
				else
				{
					cat += " / ";
					cat += m_current_data;
				}
				m_rom.set_info( FeRomInfo::Category, cat );
			}
			else if ( strcmp( element, "Players" ) == 0 )
				m_rom.set_info( FeRomInfo::Players, m_current_data );
			else if ( strcmp( element, "Publisher" ) == 0 )
				m_rom.set_info( FeRomInfo::Manufacturer, m_current_data );
		}

		m_element_open=false;
		m_current_data.clear();
	}

	if ( strcmp( element, "Game" ) == 0 )
		m_collect_data=false;
}

bool FeGameDBParser::parse( const std::string &data, bool &exact_match )
{
	m_exact_match=m_element_open=m_keep_rom=false;
	m_collect_data= !exact_match;
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

	exact_match = m_exact_match;
	return ret_val;
}
