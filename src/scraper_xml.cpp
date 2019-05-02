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
#include "zip.hpp"

#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include "nowide/fstream.hpp"

#include <expat.h>

#include <SFML/System/Clock.hpp>

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
		FeLog() << "Error parsing xml output:"
			<< buff << std::endl;
	}
	else
		ds->parsed_xml = true;

	FeXMLParser *p = (FeXMLParser *)XML_GetUserData( ds->parser );
	return p->get_continue_parse(); // return false to cancel callback
}

bool FeXMLParser::parse_internal(
		const std::string &prog,
		const std::string &args,
		const std::string &work_dir )
{
	struct user_data_struct ud;
	ud.parsed_xml = false;

	m_element_open=m_keep_rom=false;
	m_continue_parse=true;

	ud.parser = XML_ParserCreate( NULL );
	XML_SetUserData( ud.parser, (void *)this );
	XML_SetElementHandler( ud.parser, exp_start_element, exp_end_element );
	XML_SetCharacterDataHandler( ud.parser, exp_handle_data );

	run_program( prog, args, work_dir, my_parse_callback, (void *)&ud );

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
// Mame -listxml Parser
//
FeListXMLParser::FeListXMLParser( FeImporterContext &ctx )
	: FeXMLParser( ctx.uiupdate, ctx.uiupdatedata ),
	m_ctx( ctx ),
	m_count( 0 ),
	m_displays( 0 ),
	m_collect_data( false ),
	m_chd( false ),
	m_mechanical( false )
{
}

void FeListXMLParser::start_element(
			const char *element,
			const char **attribute )
{
	if (( strcmp( element, "game" ) == 0 )
		|| ( strcmp( element, "software" ) == 0 )
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
						--m_itr;
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
					(*m_itr).set_info( FeRomInfo::Players, attribute[i+1] );
				else if (( strcmp( attribute[i], "buttons" ) == 0 )
						&& (*m_itr).get_info( FeRomInfo::Buttons ).empty() )
					(*m_itr).set_info( FeRomInfo::Buttons, attribute[i+1] );

				// Older MAME XML included control as an attribute of the input tag:
				else if (( strcmp( attribute[i], "control" ) == 0 )
						&& (*m_itr).get_info( FeRomInfo::Control ).empty() )
					(*m_itr).set_info( FeRomInfo::Control, attribute[i+1] );
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

				if (( strcmp( attribute[i], "buttons" ) == 0 )
						&& (*m_itr).get_info( FeRomInfo::Buttons ).empty() )
					(*m_itr).set_info( FeRomInfo::Buttons, attribute[i+1] );
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
		else if ( strcmp( element, "extension" ) == 0 )
		{
			//
			// The extension attribute is encountered when parsing machines
			// in connection with -listsoftware processing.  This indicates
			// file extensions supported when emulating software on the given
			// machine (so for sega genesis for example, this should be giving
			// us "smd", "bin", "md" and "gen")
			//
			for ( int i=0; attribute[i]; i+=2 )
			{
				if ( strcmp( attribute[i], "name" ) == 0 )
				{
					m_sl_exts.push_back( attribute[i+1] );
					break;
				}
			}
		}
		// "cloneof" and "genre" elements appear in hyperspin .xml
		// "publisher" in listsoftware xml
		else if (( strcmp( element, "description" ) == 0 )
				|| ( strcmp( element, "cloneof" ) == 0 )
				|| ( strcmp( element, "genre" ) == 0 )
				|| ( strcmp( element, "year" ) == 0 )
				|| ( strcmp( element, "publisher" ) == 0 )
				|| ( strcmp( element, "manufacturer" ) == 0 )
				|| ( strcmp( element, "buttons" ) == 0 ))
		{
			m_element_open=true;
		}
		// "info"/"alt_title" in listsoftware xml
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
				(*m_itr).set_info( FeRomInfo::AltTitle, value );
		}
	}
}

void FeListXMLParser::end_element( const char *element )
{
	if (( strcmp( element, "game" ) == 0 )
		|| ( strcmp( element, "software" ) == 0 )
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

			static int last_percent( 0 );
			if ( !m_ctx.full && ( !m_ctx.romlist.empty() ))
			{
				int per = m_ctx.progress_past
					+ m_count * m_ctx.progress_range
					/ m_ctx.romlist.size();

				if ( per != last_percent )
				{
					last_percent = per;

					std::cout << "\b\b\b\b" << std::setw(3)
						<< last_percent << '%' << std::flush;

					if ( m_ui_update )
					{
						if ( m_ui_update( m_ui_update_data,
								last_percent,
								(*m_itr).get_info( FeRomInfo::Title ) ) == false )
							set_continue_parse( false );
					}
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
		else if (( strcmp( element, "manufacturer" ) == 0 )
				|| ( strcmp( element, "publisher" ) == 0 ))
			(*m_itr).set_info( FeRomInfo::Manufacturer, m_current_data );
		else if ( strcmp( element, "cloneof" ) == 0 ) // Hyperspin .xml
			(*m_itr).set_info( FeRomInfo::Cloneof, m_current_data );
		else if ( strcmp( element, "genre" ) == 0 ) // Hyperspin .xml
			(*m_itr).set_info( FeRomInfo::Category, m_current_data );
		else if ( strcmp( element, "buttons" ) == 0 ) // Hyperspin .xml
			(*m_itr).set_info( FeRomInfo::Buttons, m_current_data );

		m_current_data.clear();
		m_element_open=false;
	}
}

void FeListXMLParser::pre_parse()
{
	m_count=0;

	m_map.clear();
	for ( FeRomInfoListType::iterator itr=m_ctx.romlist.begin();
			itr != m_ctx.romlist.end(); ++itr )
		m_map[ (*itr).get_info( FeRomInfo::Romname ).c_str() ] = itr;

	std::cout << "    ";
}

void FeListXMLParser::post_parse()
{
	std::cout << std::endl;

	if ( !m_discarded.empty() )
	{
		FeLog() << " - Discarded " << m_discarded.size()
				<< " entries based on xml info: ";
		std::vector<FeRomInfoListType::iterator>::iterator itr;
		for ( itr = m_discarded.begin(); itr != m_discarded.end(); ++itr )
		{
			FeLog() << (*(*itr)).get_info( FeRomInfo::Romname ) << " ";
			m_ctx.romlist.erase( (*itr) );
		}
		FeLog() << std::endl;
	}
}

bool FeListXMLParser::parse_command( const std::string &prog, const std::string &work_dir )
{
	pre_parse();

	std::string base_args = "-listxml";

	//
	// Special case for really small romlists... this gets used in
	// connection with -listsoftware parsing as well
	//
	bool ret_val=true;
	if ( (!m_ctx.full) &&  (m_ctx.romlist.size() < 10) )
	{
		for ( FeRomInfoListType::iterator itr=m_ctx.romlist.begin();
				itr != m_ctx.romlist.end(); ++itr )
		{
			if ( !parse_internal( prog,
					base_args + " "
					+ (*itr).get_info( FeRomInfo::Romname ), work_dir ) )
				ret_val = false;
		}
	}
	else
		ret_val = parse_internal( prog, base_args, work_dir );

	post_parse();
	return ret_val;
}

bool FeListXMLParser::parse_file( const std::string &filename )
{
	pre_parse();

	m_element_open=m_keep_rom=false;
	m_continue_parse=true;

	nowide::ifstream myfile( filename.c_str() );
	if ( !myfile.is_open() )
	{
		FeLog() << "Error opening file: " << filename << std::endl;
		return false;
	}

	XML_Parser parser = XML_ParserCreate( NULL );
	XML_SetUserData( parser, (void *)this );
	XML_SetElementHandler( parser, exp_start_element, exp_end_element );
	XML_SetCharacterDataHandler( parser, exp_handle_data );
	bool ret_val = true;

	while ( myfile.good() && m_continue_parse )
	{
		std::string line;
		getline( myfile, line );

		if ( XML_Parse( parser, line.c_str(),
				line.size(), XML_FALSE ) == XML_STATUS_ERROR )
		{
			FeLog() << "Error parsing xml:"
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
// Mame -listsofware XML Parser
//
FeListSoftwareParser::FeListSoftwareParser( FeImporterContext &ctx )
	: FeXMLParser( ctx.uiupdate, ctx.uiupdatedata ),
	m_ctx( ctx )
{
}

void FeListSoftwareParser::clear_parse_state()
{
	m_element_open=m_keep_rom=false;
	m_description.clear();
	m_year.clear();
	m_man.clear();
	m_cloneof.clear();
	m_altname.clear();
	m_alttitle.clear();
	m_crc.clear();
}

void FeListSoftwareParser::start_element(
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
				m_altname = attribute[i+1];
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
	else if ( strcmp( element, "rom" ) == 0 )
	{
		int i;
		for ( i=0; attribute[i]; i+=2 )
		{
			if ( strcmp( attribute[i], "crc" ) == 0 )
			{
				if ( !m_crc.empty() )
					m_crc += ";";

				m_crc += attribute[i+1];
				break;
			}
		}
	}
}

void FeListSoftwareParser::end_element( const char *element )
{
	if ( strcmp( element, "software" ) == 0 )
	{
		std::string fuzzyname = get_fuzzy( m_altname );
		std::string fuzzydesc = get_fuzzy( m_description );

		bool found=false;

		//
		// 1.) check for CRC match(es)
		//
		// we can have multiple crcs in m_crc at this stage,
		// separated by ';' characters.  Check each one for a match
		//
		std::pair< std::multimap<std::string, FeRomInfo *>::iterator,
			std::multimap<std::string, FeRomInfo *>::iterator> itc;
		std::multimap<std::string, FeRomInfo *>::iterator itr;

		std::vector<std::string> crcs;
		string_to_vector( m_crc, crcs, false );

		while ( !crcs.empty() )
		{
			itc = m_crc_map.equal_range( crcs.back() );
			for ( itr = itc.first; itr != itc.second; ++itr )
			{
				const std::string &rn = (*itr).second->get_info( FeRomInfo::Romname );
				int score = 100;
				std::string itr_fuzz = get_fuzzy( rn );

				//
				// Do additional scoring if there is a name
				// or fuzzy match as well
				//
				if ( fuzzyname.compare( itr_fuzz ) == 0 )
					score += 11;
				else if ( fuzzydesc.compare( itr_fuzz ) == 0 )
				{
					score += 1;

					if ( m_description.compare( rn ) == 0 )
						score += 10;
				}

				found = true;
				set_info_values( *((*itr).second), score );
			}

			crcs.pop_back();
		}

		//
		// 2.) Now check for fuzzy and exact name matches
		//
		itc = m_fuzzy_map.equal_range( fuzzydesc );
		for ( itr = itc.first; itr != itc.second; ++itr )
		{
			int score = 1;

			if ( m_description.compare(
						(*itr).second->get_info( FeRomInfo::Romname ) ) == 0 )
				score += 10;

			found = true;
			set_info_values( (*(*itr).second), score );
		}

		itc = m_fuzzy_map.equal_range( fuzzyname );
		for ( itr = itc.first; itr != itc.second; ++itr )
		{
			set_info_values( (*(*itr).second), 11 );
			found = true;
		}

		//
		if ( m_ctx.full && !found )
		{
			m_ctx.romlist.push_back( FeRomInfo( m_description ) );
			set_info_values( m_ctx.romlist.back(), 1 );
		}

		clear_parse_state();
	}
	else if ( m_element_open )
	{
		if ( strcmp( element, "description" ) == 0 )
			m_description = m_current_data;
		else if ( strcmp( element, "year" ) == 0 )
			m_year = m_current_data;
		else if ( strcmp( element, "publisher" ) == 0 )
			m_man = m_current_data;

		m_current_data.clear();
		m_element_open=false;
	}
}

void FeListSoftwareParser::set_info_values( FeRomInfo &r, int score )
{
	int old_score = 0;

	std::string oss = r.get_info( FeRomInfo::BuildScore );
	if ( !oss.empty() )
		old_score = as_int( oss );

	if ( old_score < score )
	{
		r.set_info( FeRomInfo::Title, m_description );
		r.set_info( FeRomInfo::Year, m_year );
		r.set_info( FeRomInfo::Manufacturer, m_man );
		r.set_info( FeRomInfo::Cloneof, m_cloneof );
		r.set_info( FeRomInfo::AltRomname, m_altname );
		r.set_info( FeRomInfo::AltTitle, m_alttitle );
		r.set_info( FeRomInfo::BuildScore, as_str( score ) );
	}
}

bool FeListSoftwareParser::parse( const std::string &prog,
		const std::string &work_dir,
		const std::vector < std::string > &system_names )
{
	// First get our machine -listxml settings
	//
	std::string system_name;
	FeRomInfoListType::iterator itr;

	sf::Clock my_timer;
	int s = m_ctx.romlist.empty() ? 1 : m_ctx.romlist.size();
	int c = 0;

	for ( std::vector<std::string>::const_iterator its=system_names.begin();
			its!=system_names.end(); ++its )
	{
		FeRomInfoListType temp_list;
		temp_list.push_back( FeRomInfo( *its ) );

		FeEmulatorInfo ignored;
		FeImporterContext temp( ignored, temp_list );
		FeListXMLParser listxml( temp );

		if (( listxml.parse_command( prog, work_dir ) )
				&& ( !temp_list.empty() ))
		{
			FeRomInfo &ri = temp_list.front();
			for ( itr=m_ctx.romlist.begin(); itr!=m_ctx.romlist.end(); ++itr )
			{
				(*itr).copy_info( ri, FeRomInfo::Players );
				(*itr).copy_info( ri, FeRomInfo::Rotation );
				(*itr).copy_info( ri, FeRomInfo::Control );
				(*itr).copy_info( ri, FeRomInfo::Status );
				(*itr).copy_info( ri, FeRomInfo::DisplayCount );
				(*itr).copy_info( ri, FeRomInfo::DisplayType );
				(*itr).copy_info( ri, FeRomInfo::Buttons );

				std::string crc = get_crc(
					(*itr).get_info( FeRomInfo::BuildFullPath ),
					listxml.get_sl_extensions() );

				//
				// Add rom to our crc and fuzzy name maps
				//
				if ( !crc.empty() )
					m_crc_map.insert(
						std::pair<std::string, FeRomInfo *>( crc, &(*itr) ) );

				m_fuzzy_map.insert(
					std::pair<std::string, FeRomInfo *>(
						get_fuzzy(
							(*itr).get_info( FeRomInfo::Romname ) ),
						&(*itr) ) );


				if ( m_ui_update )
				{
					if ( m_ui_update( m_ui_update_data,
							c*90/s, "") == false )
					{
						set_continue_parse( false );
						break;
					}
				}
				c++;
			}
			system_name=(*its);
			break;
		}
	}

	FeLog() << " * Calculated CRCs for " << m_crc_map.size() << " files in "
		<< my_timer.getElapsedTime().asMilliseconds() << "ms." << std::endl;

	if ( system_name.empty() )
	{
		FeLog() << " * Error: No system identifier found that is recognized by -listxml" << std::endl;
		return false;
	}

	FeLog() << " - Obtaining -listsoftware info ["
		<< system_name << "]" << std::endl;

	// Now get the individual game -listsoftware settings
	//
	int retval=parse_internal( prog, system_name + " -listsoftware", work_dir );

	if ( !retval )
	{
		FeLog() << " * Error: No XML output found, command: " << prog << " "
					<< system_name + " -listsoftware" << std::endl;
	}

	romlist_console_report( m_ctx.romlist );
	return retval;
}
