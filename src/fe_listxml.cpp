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

#include "fe_listxml.hpp"
#include "fe_info.hpp"
#include "fe_util.hpp"
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <expat.h>

// Expat callback functions
void exp_start_element_mame( void *data, 
			const char *element, 
			const char **attribute )
{
	((FeListXMLParse *)data)->start_element_mame( element, attribute );
}

void exp_start_element_mess( void *data, 
			const char *element, 
			const char **attribute )
{
	((FeListXMLParse *)data)->start_element_mess( element, attribute );
}

void exp_handle_data( void *data, const char *content, int length )
{
	((FeListXMLParse *)data)->handle_data( content, length );
}

void exp_end_element_mame( void *data, const char *element )
{
	((FeListXMLParse *)data)->end_element_mame( element );
}

void exp_end_element_mess( void *data, const char *element )
{
	((FeListXMLParse *)data)->end_element_mess( element );
}

FeListXMLParse::FeListXMLParse( std::list <FeRomInfo> &romlist, UiUpdate u, void *d )
	: m_romlist( romlist ), m_ui_update( u ), m_ui_update_data( d )
{
}

void FeListXMLParse::clear_parse_state()
{
	m_collect_data=m_element_open=m_keep_rom=false;
	m_name.clear();
	m_description.clear();
	m_year.clear();
	m_man.clear();
	m_fuzzydesc.clear();
}

void FeListXMLParse::start_element_mame( 
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
				std::string romname = (*m_itr).get_info( FeRomInfo::Romname );
				if ( romname.compare( attribute[i+1] ) == 0 )
				{
					m_collect_data=true;
					m_keep_rom=true;
					break;
				}
			}
		}
		if ( m_collect_data == true )
		{
			for ( i=0; attribute[i]; i+=2 )
			{
				if (( strcmp( attribute[i], "isbios" ) == 0 )
					&& ( strcmp( attribute[i+1], "yes" ) == 0 ))
				{
					m_collect_data=false;
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
				{ "stick", "Joystick (Analog)" },
				{ "doublejoy", "Double Joystick" },
				{ "paddle", "Paddle" },
				{ "trackball", "Trackball" },
				{ "dial", "Dial" },
				{ NULL, NULL }
			};

			if ( type.compare( "joy" ) == 0 )
			{
				type = "Joystick (";
				type += ways;
				type += "-Way)";
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

void FeListXMLParse::start_element_mess( 
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

void FeListXMLParse::handle_data( const char *content, int length )
{
	if ( m_element_open )
		m_current_data.append( content, length );
}

void FeListXMLParse::end_element_mame( const char *element )
{
	if (( m_collect_data ) && ( strcmp( element, "game" ) == 0 ))
		m_collect_data = false;

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

void FeListXMLParse::end_element_mess( const char *element )
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
					break;
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

bool FeListXMLParse::parse_mame( const std::string &prog )
{
	std::vector< std::string > discarded;
	int count( 0 ), total( m_romlist.size() );

	std::cout << "    ";

	std::string base_args = "-listxml ";

	for ( m_itr = m_romlist.begin(); m_itr != m_romlist.end(); ++m_itr )
	{
		std::string args = base_args;
		args += (*m_itr).get_info( FeRomInfo::Romname );

		if ( parse_internal( 
				exp_start_element_mame, 
				exp_end_element_mame, 
				prog, args ) == false )
		{
			std::cout << "No XML output found, command: " << prog << " "
						<< args << std::endl;
		}

		if ( !m_keep_rom ) 
		{
			discarded.push_back( (*m_itr).get_info( FeRomInfo::Romname ) );
			std::list<FeRomInfo>::iterator del = m_itr--;
			m_romlist.erase( del );
		}

		count++;
		int percent = count * 100 / total;
		std::cout << "\b\b\b\b" << std::setw(3) << percent << '%' << std::flush;
		if ( m_ui_update )
		{
			if ( m_ui_update( m_ui_update_data, percent ) == false )
				return false;  // false if user has cancelled
		}
	}

	std::cout << std::endl;

	if ( !discarded.empty() )
	{
		std::cout << "Discarded " << discarded.size() 
				<< " entries based on xml info: ";
		for ( std::vector<std::string>::iterator itr = discarded.begin(); 
					itr < discarded.end(); ++itr )
		{
			std::cout << (*itr) << " ";
		}
		std::cout << std::endl;
	}

	return true;
}

bool FeListXMLParse::parse_mess( const std::string &prog, 
		const std::string &args )
{
	return parse_internal( 
		exp_start_element_mess, 
		exp_end_element_mess, prog, args );
}

struct user_data_struct 
{
	XML_Parser parser;
	bool parsed_xml;
};

void my_parse_callback( const char *buff, void *opaque )
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
}

bool FeListXMLParse::parse_internal( 
		StartElementHandler s, 
		EndElementHandler e, 
		const std::string &prog,
		const std::string &args )
{
	struct user_data_struct ud;
	ud.parsed_xml = false;

	clear_parse_state();

	ud.parser = XML_ParserCreate( NULL );
	XML_SetUserData( ud.parser, (void *)this );
	XML_SetElementHandler( ud.parser, s, e );
	XML_SetCharacterDataHandler( ud.parser, exp_handle_data );

	run_program( prog, args, my_parse_callback, (void *)&ud );

	// need to pass true to XML Parse on last line
	XML_Parse( ud.parser, 0, 0, XML_TRUE );
	XML_ParserFree( ud.parser );

	return ud.parsed_xml;
}
