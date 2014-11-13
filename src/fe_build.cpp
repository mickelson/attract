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
#include "fe_settings.hpp"
#include "fe_util.hpp"

#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <strings.h> // strcasecmp()

#ifndef NO_NET
#include <SFML/Network.hpp>
#endif // NO_NET

extern const char *FE_ROMLIST_SUBDIR;

namespace {

void build_basic_romlist( const FeEmulatorInfo &emulator,
				std::list<FeRomInfo> &romlist )
{
	const std::vector<std::string> &paths = emulator.get_paths();
	std::vector<std::string>::const_iterator itr, ite, its;

	const std::vector<std::string> &extensions = emulator.get_extensions();

	for ( itr = paths.begin(); itr != paths.end(); ++itr )
	{
		std::string path = clean_path( *itr, true );
		int count=0;

		for ( ite = extensions.begin(); ite != extensions.end(); ++ite )
		{
			std::vector<std::string> base_list;
			if ( (*ite).compare( FE_DIR_TOKEN ) == 0 )
				get_subdirectories( base_list, path );
			else
				get_basename_from_extension( base_list, path, (*ite), true );

			for ( its=base_list.begin(); its != base_list.end(); ++its )
			{
				FeRomInfo new_rom;
				new_rom.set_info( FeRomInfo::Romname, (*its) );
				new_rom.set_info( FeRomInfo::Title, (*its) );
				new_rom.set_info( FeRomInfo::Emulator, emulator.get_info(
																FeEmulatorInfo::Name ));

				romlist.push_back( new_rom );
				count++;
			}
		}

		std::cout << "Found " << count
			<< " files with rom extension(s):";

		for ( ite=extensions.begin(); ite != extensions.end(); ++ite )
			std::cout << " " << (*ite);

		std::cout << ".  Directory: " << path << std::endl;
	}
}

std::string url_escape( const std::string &raw )
{
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = raw.begin(); i != raw.end(); ++i)
	{
		const unsigned char c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum( c ) || c == '-' || c == '_' || c == '.' || c == '~')
		{
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << '%' << std::setw(2) << int(c);
	}

	return escaped.str();
}

void apply_xml_import( const FeEmulatorInfo &emulator,
				std::list<FeRomInfo> &romlist,
				FeXMLParser::UiUpdate uiupdate=NULL, void *uiupdatedata=NULL )
{
	std::string source = emulator.get_info(
				FeEmulatorInfo::Info_source );

	if ( source.empty() )
		return;

	std::string base_command = clean_path( emulator.get_info(
				FeEmulatorInfo::Executable ) );

	if ( source.compare( "mame" ) == 0 )
	{
		std::cout << "Obtaining -listxml info...";
		FeMameXMLParser mamep( romlist, uiupdate, uiupdatedata );
		mamep.parse( base_command );
		std::cout << std::endl;
	}
	else if ( source.compare( "mess" ) == 0 )
	{
		std::string system_name = emulator.get_info( FeEmulatorInfo::System );
		if ( system_name.empty() )
		{
			std::cout << "Note: No system configured for emulator: "
				<< emulator.get_info( FeEmulatorInfo::Name )
				<< ", unable to obtain -listsoftware info."
				<< std::endl;
			return;
		}

		std::cout << "Obtaining -listsoftware info...";
		FeMessXMLParser messp( romlist, uiupdate, uiupdatedata );
		messp.parse( base_command, system_name );
		std::cout << std::endl;
	}
	else if ( source.compare( "steam" ) == 0 )
	{
		const std::vector<std::string> &paths = emulator.get_paths();
		const std::vector<std::string> &exts = emulator.get_extensions();
		if ( paths.empty() || exts.empty() )
			return;

		std::string path = clean_path( paths.front(), true );
		const std::string &extension = exts.front();

		// A bit mislabelled: the steam import isn't really xml.
		for ( std::list<FeRomInfo>::iterator itr = romlist.begin(); itr != romlist.end(); ++itr )
		{
			// We expect appmanifest_* entries in the romname field.  Open each of these files and
			// extract the data we can use in our list
			const std::string &n = (*itr).get_info( FeRomInfo::Romname );
			std::string fname = path + n + extension;

			//
			// First, Fix the Romname entry in case we don't find it in the manifest
			//
			size_t start_pos = n.find_last_of( "_" );
			if ( start_pos != std::string::npos )
				(*itr).set_info( FeRomInfo::Romname,
						n.substr( start_pos + 1 ) );

			std::ifstream myfile( fname.c_str() );

			int fields_left( 3 );

			if ( myfile.is_open() )
			{
				while ( myfile.good() && ( fields_left > 0 ) )
				{
					std::string line, val;
					size_t pos( 0 );

					getline( myfile, line );

					token_helper( line, pos, val, FE_WHITESPACE );

					if ( strcasecmp( val.c_str(), "appid" ) == 0 )
					{
						std::string id;
						token_helper( line, pos, id, FE_WHITESPACE );
						(*itr).set_info( FeRomInfo::Romname, id );
						fields_left--;
					}
					else if ( strcasecmp( val.c_str(), "name" ) == 0 )
					{
						std::string name;
						token_helper( line, pos, name, FE_WHITESPACE );
						(*itr).set_info( FeRomInfo::Title, name );
						fields_left--;
					}
					else if ( strcasecmp( val.c_str(), "installdir" ) == 0 )
					{
						std::string altname;
						token_helper( line, pos, altname, FE_WHITESPACE );
						(*itr).set_info( FeRomInfo::AltRomname, altname );
						fields_left--;
					}
				}

				ASSERT( !fields_left );
			}
			else
				std::cerr << "Error opening file: " << fname << std::endl;
		}
	}
#ifndef NO_NET
	else if ( source.compare( "thegamesdb.net" ) == 0 )
	{
		const char *HOSTNAME = "http://thegamesdb.net";
		const char *PLATFORM_REQ = "api/GetPlatformsList.php";
		const char *GAME_REQ = "api/GetGame.php?name=$1&platform=$2";
		const char *FROM_FIELD = "From";
		const char *FROM_VALUE = "user@attractmode.org";
		const char *UA_FIELD = "User-Agent";
		const char *UA_VALUE = "Attract-Mode/1.x";

		sf::Http http;
		http.setHost( HOSTNAME );

		//
		// Get a list of valid platforms
		//
		sf::Http::Request req( PLATFORM_REQ );
		req.setField( FROM_FIELD, FROM_VALUE );
		req.setField( UA_FIELD, UA_VALUE );

		sf::Http::Response resp = http.sendRequest( req );
		sf::Http::Response::Status status = resp.getStatus();

		if ( status != sf::Http::Response::Ok )
		{
			std::cout << "[thegamesdb.net scraper] Error getting platform list.  Code: " << status << std::endl;
			return;
		}

		FeGameDBPlatformParser gdbpp;
		gdbpp.parse( resp.getBody() );

		const std::vector<std::string> &sl_temp = emulator.get_systems();
		std::vector<std::string> system_list;
		for ( std::vector<std::string>::const_iterator itr = sl_temp.begin(); itr!=sl_temp.end(); ++itr )
		{
			if ( gdbpp.m_set.find( *itr ) != gdbpp.m_set.end() )
				system_list.push_back( *itr );
			else
				std::cout << "[thegamesdb.net scraper] System identifier '" << (*itr) << "' not recognized by "
					<< HOSTNAME << std::endl;
		}

		if ( system_list.size() < 1 )
		{
			std::cout << "[thegamesdb.net scraper] Error, no valid system identifier available." << std::endl;
			return;
		}

		int i=0;
		for ( std::list<FeRomInfo>::iterator itr=romlist.begin(); itr!=romlist.end(); ++itr )
		{
			bool exact_match( false );
			for ( std::vector<std::string>::iterator its=system_list.begin();
					its!=system_list.end(); ++its )
			{
				std::string req_string = GAME_REQ;
				std::string game = url_escape(
						name_with_brackets_stripped( (*itr).get_info( FeRomInfo::Romname ) ) );

				std::string system = url_escape( *its );

				perform_substitution( req_string, "$1", game );
				perform_substitution( req_string, "$2", system );

				req.setUri( req_string );
				resp = http.sendRequest( req );
				status = resp.getStatus();

				if ( uiupdate )
				{
					int p = i * 100 / romlist.size();
					if ( uiupdate( uiupdatedata, p ) == false )
						break;
				}

				if ( status != sf::Http::Response::Ok )
				{
					std::cout << "[thegamesdb.net scraper] Error getting game information.  Game: "
						<< (*itr).get_info( FeRomInfo::Romname ) << ", Code: " << status << std::endl;
					break;
				}

				FeGameDBParser gdbp( *itr );
				gdbp.parse( resp.getBody(), exact_match );

				if ( exact_match )
					break;
			}
			i++;
		}
	}
#endif
	else
	{
		std::cout << "Unrecognized import_source setting: " << source
					<< std::endl;
	}
}

void write_romlist( const std::string &filename,
				const std::list<FeRomInfo> &romlist )
{

	std::cout << "Writing " << romlist.size() << " entries to: "
				<< filename << std::endl;

	int i=0;
	std::ofstream outfile( filename.c_str() );
	if ( outfile.is_open() )
	{
		// one line header showing what the columns represent
		//
		outfile << "#" << FeRomInfo::indexStrings[i++];
		while ( i < FeRomInfo::Favourite )
			outfile << ";" << FeRomInfo::indexStrings[i++];
		outfile << std::endl;

		// Now output the list
		//
		for ( std::list<FeRomInfo>::const_iterator itl=romlist.begin();
				itl != romlist.end(); ++itl )
			outfile << (*itl).as_output() << std::endl;

		outfile.close();
	}
}

struct myclasscmp
{
	bool operator() ( const std::string &lhs, const std::string &rhs ) const
	{
		return ( strcasecmp( lhs.c_str(), rhs.c_str() ) < 0 );
	}
};

void ini_import( const std::string &filename,
				std::list<FeRomInfo> &romlist,
				FeRomInfo::Index index,
				const std::string &init_tag )
{
	std::map <std::string, std::string, myclasscmp> my_map;

	// create entries in the map for each name we want to find
	for ( std::list<FeRomInfo>::iterator itr=romlist.begin();
			itr!=romlist.end(); ++itr )
	{
		my_map[ (*itr).get_info( FeRomInfo::Romname ) ] = "";

		const std::string &cloneof_key = (*itr).get_info( FeRomInfo::Cloneof );
		if ( !cloneof_key.empty() )
			my_map[ cloneof_key ] = "";

		const std::string &alt_key = (*itr).get_info( FeRomInfo::AltRomname );
		if ( !alt_key.empty() )
			my_map[ alt_key ] = "";
	}

	std::ifstream myfile( filename.c_str() );

	if ( myfile.is_open() )
	{
		std::string line;

		// Jump forward to the init_tag (if provided)
		//
		if ( !init_tag.empty() )
		{
			while (( myfile.good() )
					&& ( line.compare(0, init_tag.size(), init_tag ) != 0 ))
				getline( myfile, line );
		}

		// Now read until the next tag is found
		getline( myfile, line );
		bool done=false;

		while ( myfile.good() && !done )
		{
			if ( !line.empty() && ( line[0] == '[' ))
			{
				done=true;
				break;
			}
			std::string name;
			size_t pos=0;
			token_helper( line, pos, name, "=" );

			std::map<std::string, std::string, myclasscmp>::iterator itr;
			itr = my_map.find( name );
			if ( itr != my_map.end() )
			{
				std::string val;
				token_helper( line, pos, val, "=" );
				my_map[ name ] = val;
			}
			getline( myfile, line );
		}
	}
	myfile.close();

	int count=0;
	for ( std::list<FeRomInfo>::iterator itr=romlist.begin();
			itr!=romlist.end(); ++itr )
	{
		std::string val = my_map[ (*itr).get_info( FeRomInfo::Romname ) ];

		if ( val.empty() )
			val = my_map[ (*itr).get_info( FeRomInfo::AltRomname ) ];

		if ( val.empty() )
			val = my_map[ (*itr).get_info( FeRomInfo::Cloneof ) ];

		if ( !val.empty() )
		{
			count++;
			(*itr).set_info( index, val );
		}
	}

	std::cout << "[Import " << filename << "] - found info for " << count
		<< " entries." << std::endl;
}

void apply_import_extras( const FeEmulatorInfo &emulator,
				std::list<FeRomInfo> &romlist )
{
	const std::vector< std::string > &extras = emulator.get_import_extras();

	for ( std::vector< std::string >::const_iterator itr = extras.begin();
			itr != extras.end(); ++itr )
	{
		std::string path = clean_path( *itr );

		if ( tail_compare( path, "catver.ini" ) )
			ini_import( path, romlist, FeRomInfo::Category, "[Category]" );
		else if ( tail_compare( path, "nplayers.ini" ) )
			ini_import( path, romlist, FeRomInfo::Players, "[NPlayers]" );
		else
			std::cout << "Unsupported import_extras file: " << path << std::endl;
	}
}

bool import_mamewah( const std::string &input_filename,
				const std::string &emulator_name,
				std::list<FeRomInfo> &romlist )
{
	// Map the lines of the Mamewah / Wahcade! romlist file to our RomInfo structure
	//
	const FeRomInfo::Index wah_map[] =
	{
		FeRomInfo::Romname,			// Name
		FeRomInfo::Title,				// Description
		FeRomInfo::Year,
		FeRomInfo::Manufacturer,
		FeRomInfo::Cloneof,
		FeRomInfo::LAST_INDEX,		// Romof
		FeRomInfo::DisplayType,
		FeRomInfo::Rotation,
		FeRomInfo::Control,
		FeRomInfo::Status,
		FeRomInfo::LAST_INDEX,		// Colour status
		FeRomInfo::LAST_INDEX,		// Sound status
		FeRomInfo::Category
	};

	std::ifstream myfile( input_filename.c_str() );
	if ( !myfile.is_open() )
	{
		std::cerr << "Error opening file: " << input_filename << std::endl;
		return false;
	}

	int count=0;
	FeRomInfo current_rom;

	while ( myfile.good() )
	{
		std::string line;
		getline( myfile, line );

		int index = ( count % 13 );

		if ( wah_map[ index ] != FeRomInfo::LAST_INDEX )
		{
			size_t l = line.find_last_not_of( FE_WHITESPACE );
			if ( l != std::string::npos )
			{
				current_rom.set_info(
					wah_map[ index ],
					line.substr( 0, l+1 ) );
			}
		}

		if ( index == 12 )
		{
			current_rom.set_info(
					FeRomInfo::Emulator,
					emulator_name );

			romlist.push_back( current_rom );
			current_rom.clear();
			count = 0;
		}
		else
			count++;
	}

	if ( count > 1 )
	{
		std::cout << "Warning: Unexpected end of file encountered: " << input_filename
			<< ", this probably means the import failed." << std::endl;
	}

	myfile.close();
	return true;
}

}; // end namespace

bool FeSettings::build_romlist( const std::vector< FeImportTask > &task_list,
						const std::string &output_name )
{
	std::list<FeRomInfo> total_romlist;
	std::string best_name, list_name, path;

	for ( std::vector<FeImportTask>::const_iterator itr=task_list.begin();
			itr < task_list.end(); ++itr )
	{
		if ( (*itr).file_name.empty() )
		{
			// Build romlist task
			FeEmulatorInfo *emu = get_emulator( (*itr).emulator_name );
			if ( emu == NULL )
			{
				std::cout << "Error: Invalid -build-rom-list target: " <<  (*itr).emulator_name
					<< std::endl;
			}
			else
			{
				std::list<FeRomInfo> romlist;

				best_name = emu->get_info( FeEmulatorInfo::Name );

				build_basic_romlist( *emu, romlist );

				apply_xml_import( *emu, romlist );
				apply_import_extras( *emu, romlist );

				total_romlist.splice( total_romlist.end(), romlist );
			}
		}
		else
		{
			// import romlist from file task
			std::list<FeRomInfo> romlist;
			std::string emu_name;

			if ( (*itr).emulator_name.empty() )
			{
				// deduce the emulator name from the filename provided
				size_t my_start = (*itr).file_name.find_last_of( "\\/" );
				if ( my_start == std::string::npos ) // if there is no / we start at the beginning
					my_start = 0;
				else
					my_start += 1;

				size_t my_end = (*itr).file_name.find_last_of( "." );
				if ( my_end != std::string::npos )
					emu_name = (*itr).file_name.substr( my_start, my_end - my_start  );
			}
			else
				emu_name = (*itr).emulator_name;

			best_name = emu_name;

			if ( tail_compare( (*itr).file_name, ".txt" ) )
			{
				// Attract-Mode format list
				//
				FeRomList temp_list;
				temp_list.load_from_file( (*itr).file_name, ";" );

				for ( int i=0; i< temp_list.size(); i++ )
					romlist.push_back( temp_list[i] );
			}
			else if ( tail_compare( (*itr).file_name, ".lst" ) )
			{
				// Mamewah/Wahcade! format list
				//
				import_mamewah( (*itr).file_name, emu_name, romlist );
			}
			else if ( tail_compare( (*itr).file_name, ".xml" ) )
			{
				// HyperSpin format list
				//
				FeHyperSpinXMLParser my_parser( romlist );
				if ( my_parser.parse( (*itr).file_name ) )
				{
					// Update the emulator entries
					for ( std::list<FeRomInfo>::iterator itr=romlist.begin(); itr != romlist.end(); ++itr )
						(*itr).set_info( FeRomInfo::Emulator, emu_name );
				}
			}
			else
			{
				std::cerr << "Error: Unsupported --import-rom-list file: "
					<<  (*itr).file_name << std::endl;
			}

			std::cout << "[Import " << (*itr).file_name << "] - Imported " << romlist.size() << " entries."
				<< std::endl;

			FeEmulatorInfo *emu = get_emulator( emu_name );
			if ( emu == NULL )
			{
				std::cout << "Warning: The emulator specified with --import-rom-list was not found: "
					<<  emu_name << std::endl;
			}
			else
				apply_import_extras( *emu, romlist );

			total_romlist.splice( total_romlist.end(), romlist );
		}
	}

	total_romlist.sort( FeRomListSorter() );

	// strip duplicate entries
	std::cout << "Removing any duplicate entries..." << std::endl;
	total_romlist.unique();

	if ( task_list.size() > 1 )
		best_name = "multi";

	path = get_config_dir();
	confirm_directory( path, FE_ROMLIST_SUBDIR );

	path += FE_ROMLIST_SUBDIR;

	// if we weren't given a specific output name, then we come up with a name
	// that doesn't exist already
	//
	if ( output_name.empty() )
		get_available_filename( path, best_name, FE_ROMLIST_FILE_EXTENSION, list_name );
	else
		list_name = path + output_name + FE_ROMLIST_FILE_EXTENSION;

	write_romlist( list_name, total_romlist );

	return true;
}

bool FeSettings::build_romlist( const std::string &emu_name, UiUpdate uiu, void *uid, int &size )
{
	FeEmulatorInfo *emu = get_emulator( emu_name );
	if ( emu == NULL )
		return false;

	//
	// Put up the "building romlist" message at 0 percent while we get going...
	//
	if ( uiu )
		uiu( uid, 0 );

	std::list<FeRomInfo> romlist;
	build_basic_romlist( *emu, romlist );

	apply_xml_import( *emu, romlist, uiu, uid );
	apply_import_extras( *emu, romlist );

	romlist.sort( FeRomListSorter() );

	// strip duplicate entries
	std::cout << "Removing any duplicate entries..." << std::endl;
	romlist.unique();

	size = romlist.size();

	std::string filename = get_config_dir();
	confirm_directory( filename, FE_ROMLIST_SUBDIR );

	if ( uiu )
		uiu( uid, 100 );

	filename += FE_ROMLIST_SUBDIR;
	filename += emu_name;
	filename += FE_ROMLIST_FILE_EXTENSION;
	write_romlist( filename, romlist );

	return true;
}
