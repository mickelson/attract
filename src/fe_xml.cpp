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
#include "fe_info.hpp"
#include "fe_util.hpp"
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
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
	std::list <FeRomInfo> &romlist, 
	UiUpdate u, 
	void *d )
	: FeXMLParser( u, d ), m_romlist( romlist ), m_count( 0 )
{
}

void FeMameXMLParser::start_element( 
			const char *element, 
			const char **attribute )
{
	if ( strcmp( element, "game" ) == 0 )
	{
		int i;
		for ( i=0; attribute[i]; i+=2 )
		{
			if ( strcmp( attribute[i], "name" ) == 0 )
			{
				std::map<const char *, std::list<FeRomInfo>::iterator, FeMapComp>::iterator itr;
				itr = m_map.find( attribute[i+1] );
				if ( itr != m_map.end() )
				{
					m_itr = (*itr).second;
					m_collect_data=true;
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
				{
					(*m_itr).set_info( FeRomInfo::Rotation, attribute[i+1] );
					break;
				}
			}
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
			std::string type, ways;

			for ( int i=0; attribute[i]; i+=2 )
			{
				if ( strcmp( attribute[i], "type" ) == 0 )
					type = attribute[i+1];
				else if ( strcmp( attribute[i], "ways" ) == 0 )
					ways = attribute[i+1];
			}

			struct my_map_struct { const char *in; const char *out; };
			my_map_struct map[] = 
			{
				{ "stick", "joystick (analog)" },
				{ "doublejoy", "double joystick" },
				{ "paddle", "paddle" }, 
				{ "trackball", "trackball" },
				{ "dial", "dial" },
				{ NULL, NULL }
			};

			if ( type.compare( "joy" ) == 0 )
			{
				type = "joystick (";
				type += ways;
				type += "-way)";
				(*m_itr).set_info( FeRomInfo::Control, type );
			}
			else
			{
				int i=0;
				while ( map[i].in != NULL )
				{
					if ( type.compare( map[i].in ) == 0 )
					{
						(*m_itr).set_info( FeRomInfo::Control, map[i].out );
						break;
					}
					i++;
				}

				if ( (*m_itr).get_info( FeRomInfo::Control ).empty() )
					(*m_itr).set_info( FeRomInfo::Control, type );
			}
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
	if ( strcmp( element, "game" ) == 0 )
	{
		if ( m_collect_data )
		{
			m_collect_data = false;

			if ( !m_keep_rom ) 
				m_discarded.push_back( m_itr );

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
	std::list<FeRomInfo>::iterator itr;
	m_percent=m_count=0;

	if ( m_romlist.size() < 1500 )
	{
		//
		// Small List Strategy: run "mame -listxml <romname>" for each rom.
		//
		std::cout << "    ";
		for ( itr = m_romlist.begin(); itr != m_romlist.end(); ++itr )
		{
			std::string args = base_args;
			args += " ";
			args += (*itr).get_info( FeRomInfo::Romname );

			m_map.clear();
			m_map[ (*itr).get_info( FeRomInfo::Romname ).c_str() ] = itr;

			if ( parse_internal( prog, args ) == false )
			{
				std::cout << "No XML output found, command: " << prog << " "
						<< args << std::endl;
			}

			if ( get_continue_parse() == false )
				break;
		}
	}
	else
	{
		//
		// Large List Strategy: run "mame -listxml" and find each rom.
		//
		m_map.clear();
		for ( std::list<FeRomInfo>::iterator itr=m_romlist.begin();
				itr != m_romlist.end(); ++itr )
			m_map[ (*itr).get_info( FeRomInfo::Romname ).c_str() ] = itr;

		std::cout << "    ";

		if ( parse_internal( prog, base_args ) == false )
		{
			std::cout << "No XML output found, command: " << prog << " "
						<< base_args << std::endl;
		}
	}

	std::cout << std::endl;

	if ( !m_discarded.empty() )
	{
		std::cout << "Discarded " << m_discarded.size() 
				<< " entries based on xml info: ";
		std::vector<std::list<FeRomInfo>::iterator>::iterator itr; 
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
	std::list <FeRomInfo> &romlist, 
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
				m_name = attribute[i+1];

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
		}
	}
	else if (( strcmp( element, "description" ) == 0 )
			|| ( strcmp( element, "year" ) == 0 )
			|| ( strcmp( element, "publisher" ) == 0 )) 
	{
		m_element_open=true;
	}
}

void FeMessXMLParser::end_element( const char *element )
{
	if ( strcmp( element, "software" ) == 0 )
	{
		if ( m_keep_rom )
		{
			(*m_itr).set_info( FeRomInfo::Title, m_description );
			(*m_itr).set_info( FeRomInfo::Year, m_year );
			(*m_itr).set_info( FeRomInfo::Manufacturer, m_man );
			(*m_itr).set_info( FeRomInfo::Cloneof, m_name );
		}
		else
		{
			// otherwise try matching based on description
			for ( m_itr=m_romlist.begin(); m_itr!=m_romlist.end(); ++m_itr )
			{
				if ( m_description.compare( 
							(*m_itr).get_info( FeRomInfo::Romname ) ) == 0 )
				{ 
					(*m_itr).set_info( FeRomInfo::Title, m_description );
					(*m_itr).set_info( FeRomInfo::Year, m_year );
					(*m_itr).set_info( FeRomInfo::Manufacturer, m_man );
					(*m_itr).set_info( FeRomInfo::Cloneof, m_name );
				}
				else if (( (*m_itr).get_info( FeRomInfo::Cloneof ).empty() )
						&& ( (*m_itr).get_info( FeRomInfo::Title ).find( 
									m_fuzzydesc ) != std::string::npos ))
				{
					(*m_itr).set_info( FeRomInfo::Year, m_year );
					(*m_itr).set_info( FeRomInfo::Manufacturer, m_man );
					(*m_itr).set_info( FeRomInfo::Cloneof, m_name );
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

bool FeMessXMLParser::parse( const std::string &prog, 
		const std::string &args )
{
	return parse_internal( prog, args );
}

