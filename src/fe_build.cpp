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

#ifndef NO_NET
#include "fe_net.hpp"
#endif // NO_NET

extern const char *FE_ROMLIST_SUBDIR;

namespace {

void build_basic_romlist( FeImporterContext &c )
{
	std::vector<std::string> names_list;
	c.emulator.gather_rom_names( names_list );

	for ( std::vector<std::string>::const_iterator its=names_list.begin(); its != names_list.end(); ++its )
	{
		FeRomInfo new_rom;
		new_rom.set_info( FeRomInfo::Romname, (*its) );
		new_rom.set_info( FeRomInfo::Title, (*its) );

		c.romlist.push_back( new_rom );
	}

	std::cout << " - Found " << names_list.size() << " files." << std::endl;
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

void write_romlist( const std::string &filename,
				const FeRomInfoListType &romlist )
{

	std::cout << " + Writing " << romlist.size() << " entries to: "
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
		for ( FeRomInfoListType::const_iterator itl=romlist.begin();
				itl != romlist.end(); ++itl )
			outfile << (*itl).as_output() << std::endl;

		outfile.close();
	}
}

struct myclasscmp
{
	bool operator() ( const std::string &lhs, const std::string &rhs ) const
	{
		return ( icompare( lhs, rhs ) < 0 );
	}
};

void ini_import( const std::string &filename,
				FeRomInfoListType &romlist,
				FeRomInfo::Index index,
				const std::string &init_tag )
{
	std::map <std::string, std::string, myclasscmp> my_map;

	// create entries in the map for each name we want to find
	for ( FeRomInfoListType::iterator itr=romlist.begin();
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
	for ( FeRomInfoListType::iterator itr=romlist.begin();
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

void apply_import_extras( FeImporterContext &c )
{
	const std::vector< std::string > &extras = c.emulator.get_import_extras();

	for ( std::vector< std::string >::const_iterator itr = extras.begin();
			itr != extras.end(); ++itr )
	{
		std::string path = clean_path( *itr );

		if ( tail_compare( path, "catver.ini" ) )
			ini_import( path, c.romlist, FeRomInfo::Category, "[Category]" );
		else if ( tail_compare( path, "nplayers.ini" ) )
			ini_import( path, c.romlist, FeRomInfo::Players, "[NPlayers]" );
		else
			std::cout << "Unsupported import_extras file: " << path << std::endl;
	}
}

bool import_mamewah( const std::string &input_filename,
				const std::string &emulator_name,
				FeRomInfoListType &romlist )
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

void apply_emulator_name( const std::string &name,
				FeRomInfoListType &romlist )
{
	for ( FeRomInfoListType::iterator itr=romlist.begin(); itr!=romlist.end(); ++itr )
		(*itr).set_info( FeRomInfo::Emulator, name );
}

void build_parent_map(
	std::map < std::string, FeRomInfo * > &parent_map,
	FeRomInfoListType &romlist )
{
	for ( FeRomInfoListType::iterator itr=romlist.begin(); itr!=romlist.end(); ++itr )
	{
		if ( (*itr).get_info( FeRomInfo::Cloneof ).empty() )
			parent_map[ (*itr).get_info( FeRomInfo::Romname ) ] = &(*itr);
	}
}

bool has_same_name_as_parent( FeRomInfo &rom,
	std::map < std::string, FeRomInfo * > &parent_map )
{
	const std::string &cloneof = rom.get_info( FeRomInfo::Cloneof );
	if ( !cloneof.empty() )
	{
		std::map<std::string, FeRomInfo * >::iterator itm = parent_map.find( cloneof );
		if ( itm != parent_map.end() )
		{
			std::string clone_fuzz = get_fuzzy( rom.get_info( FeRomInfo::Title ) );
			std::string parent_fuzz = get_fuzzy( (*itm).second->get_info( FeRomInfo::Title ) );

			if ( clone_fuzz.compare( parent_fuzz ) == 0 )
				return true;
		}
	}
	return false;
}

bool process_q_simple( FeNetQueue &q,
	FeImporterContext &c,
	int taskc )
{
	int done( 0 );
	//
	// Create worker threads to process the queue.
	//
	FeNetWorker worker1( q ), worker2( q ), worker3( q ), worker4( q );

	//
	// Process the output queue from our worker threads
	//
	std::string aux;
	while ( !( q.input_done() && q.output_done() ) )
	{
		int id;
		std::string result;

		if ( q.pop_completed_task( id, result ) )
		{
			if ( id < 0 )
			{
				if ( id == -1 )
				{
					std::cout << " + Downloaded: " << result << std::endl;
					c.download_count++;

					// find second last forward slash in filename
					// we assume that there will always be at least two
					size_t pos = result.find_last_of( "\\/" );
					pos = result.find_last_of( "\\/", pos-1 );
					aux = result.substr( pos );
				}

				done++;
			}
		}

		if ( c.uiupdate )
		{
			int p = c.progress_past + done * c.progress_range / taskc;
			if ( c.uiupdate( c.uiupdatedata, p, aux ) == false )
				return false;
		}
	}
	return true;
}

bool scummvm_cb( const char *buff, void *opaque )
{
	std::string *outp = (std::string *)opaque;
	*outp += buff;
	return true;
}

void scummvm_target( const std::string &base_command,
	const std::string &params,
	std::map< std::string, std::string, myclasscmp > &my_map )
{
	std::string output;
	run_program( base_command, params, scummvm_cb, &output );

	size_t pos( 0 );
	while ( pos < output.size() )
	{
		std::string line;
		token_helper( output, pos, line, "\n" );

		std::string shortn;
		std::string longn;
		size_t pos2( 0 );
		token_helper( line, pos2, shortn, " " );
		token_helper( line, pos2, longn, "\n" );

		my_map[ shortn ] = name_with_brackets_stripped( longn );
	}
}

//
// Map scummvm gameids -> full name using -z and -t output.
//
bool scummvm_lookup( FeImporterContext &c )
{
	std::string base_command = clean_path( c.emulator.get_info(
				FeEmulatorInfo::Executable ) );

	std::map< std::string, std::string, myclasscmp > my_map;

	scummvm_target( base_command, "-z", my_map );
	scummvm_target( base_command, "-t", my_map );

	for ( FeRomInfoListType::iterator itr = c.romlist.begin(); itr != c.romlist.end(); ++itr )
	{
		std::map< std::string, std::string >::iterator itm;
		itm = my_map.find( (*itr).get_info( FeRomInfo::Romname ) );
		if ( itm != my_map.end() )
			(*itr).set_info( FeRomInfo::Title, (*itm).second );
	}
	return true;
}

}; // end namespace

bool FeSettings::mameps_scraper( FeImporterContext &c )
{
#ifndef NO_NET
	if (( c.emulator.get_info_source() != FeEmulatorInfo::Listxml )
				|| ( !m_scrape_vids ))
		return true;

	//
	// Build a map for looking up parents
	//
	std::map < std::string, FeRomInfo * > parent_map;
	build_parent_map( parent_map, c.romlist );

	const char *MAMEPS = "http://www.progettosnaps.net";
	const char *REQ = "/videosnaps/mp4/";

	std::string emu_name = c.emulator.get_info( FeEmulatorInfo::Name );
	std::string base_path = get_config_dir() + FE_SCRAPER_SUBDIR;
	base_path += emu_name + "/";

	FeNetQueue q;
	int taskc( 0 );

	for ( FeRomInfoListType::iterator itr=c.romlist.begin(); itr!=c.romlist.end(); ++itr )
	{
		// ugh, this must be set for has_artwork() to correctly function
		(*itr).set_info( FeRomInfo::Emulator, emu_name );

		// Don't scrape for a clone if its parent has the same name
		//
		if ( has_same_name_as_parent( *itr, parent_map ) )
			continue;

		if ( m_scrape_vids && !has_video_artwork( *itr, "snap" ) )
		{
			const char *SNAP = "snap/";
			std::string fname = base_path + SNAP + (*itr).get_info( FeRomInfo::Romname );
			confirm_directory( base_path, SNAP );
			q.add_file_task( MAMEPS, REQ + (*itr).get_info( FeRomInfo::Romname ) +".mp4", fname );
			taskc++;
		}
	}

	return process_q_simple( q, c, taskc );
#else
	return true;
#endif
}

bool FeSettings::mamedb_scraper( FeImporterContext &c )
{
#ifndef NO_NET
	if (( c.emulator.get_info_source() != FeEmulatorInfo::Listxml )
				|| ( !m_scrape_snaps && !m_scrape_marquees ))
		return true;

	//
	// Build a map for looking up parents
	//
	std::map < std::string, FeRomInfo * > parent_map;
	build_parent_map( parent_map, c.romlist );

	const char *MAMEDB = "http://mamedb.com";

	std::string emu_name = c.emulator.get_info( FeEmulatorInfo::Name );
	std::string base_path = get_config_dir() + FE_SCRAPER_SUBDIR;
	base_path += emu_name + "/";

	FeNetQueue q;
	int taskc( 0 );

	for ( FeRomInfoListType::iterator itr=c.romlist.begin(); itr!=c.romlist.end(); ++itr )
	{
		// ugh, this must be set for has_artwork() to correctly function
		(*itr).set_info( FeRomInfo::Emulator, emu_name );

		// Don't scrape for a clone if its parent has the same name
		//
		if ( has_same_name_as_parent( *itr, parent_map ) )
			continue;

		if ( m_scrape_marquees && !has_artwork( *itr, "marquee" ) )
		{
			const char *MARQUEE = "marquee/";
			std::string fname = base_path + MARQUEE + (*itr).get_info( FeRomInfo::Romname );
			confirm_directory( base_path, MARQUEE );
			q.add_file_task( MAMEDB, "marquees/" + (*itr).get_info( FeRomInfo::Romname ) +".png", fname );
			taskc++;
		}

		if ( m_scrape_snaps && !has_image_artwork( *itr, "snap" ) )
		{
			const char *SNAP = "snap/";
			std::string fname = base_path + SNAP + (*itr).get_info( FeRomInfo::Romname );
			confirm_directory( base_path, SNAP );
			q.add_file_task( MAMEDB, SNAP + (*itr).get_info( FeRomInfo::Romname ) +".png", fname );
			taskc++;
		}
	}

	return process_q_simple( q, c, taskc );
#else
	return true;
#endif
}

bool FeSettings::thegamesdb_scraper( FeImporterContext &c )
{
#ifndef NO_NET
	const char *HOSTNAME = "http://thegamesdb.net";
	const char *PLATFORM_REQ = "api/GetPlatformsList.php";
	const char *GAME_REQ = "api/GetGame.php?name=$1";

	//
	// Get a list of valid platforms
	//
	FeNetQueue q;
	q.add_buffer_task( HOSTNAME, PLATFORM_REQ, 0 );
	sf::Http::Response::Status status;
	q.do_next_task( status );

	if ( status != sf::Http::Response::Ok )
	{
		get_resource( "Error getting platform list from thegamesdb.net.  Code: $1",
							as_str( status ), c.user_message );

		std::cout << " * " << c.user_message << std::endl;
		return true;
	}

	int temp;
	std::string body;
	q.pop_completed_task( temp, body );

	FeGameDBPlatformParser gdbpp;
	gdbpp.parse( body );

	const std::vector<std::string> &sl_temp = c.emulator.get_systems();
	std::vector<std::string> system_list;
	for ( std::vector<std::string>::const_iterator itr = sl_temp.begin(); itr!=sl_temp.end(); ++itr )
	{
		if ( gdbpp.m_set.find( *itr ) != gdbpp.m_set.end() )
			system_list.push_back( *itr );
		else
			std::cout << " * System identifier '" << (*itr) << "' not recognized by "
				<< HOSTNAME << std::endl;
	}

	if ( system_list.size() < 1 )
	{
		// Correct if we can based on the configured info source,
		// otherwise we error out
		switch( c.emulator.get_info_source() )
		{
		case FeEmulatorInfo::Listxml:
			system_list.push_back( "Arcade" ); break;
		case FeEmulatorInfo::Steam:
			system_list.push_back( "PC" ); break;
		default:
			get_resource( "Error: None of the configured system identifier(s) are recognized by thegamesdb.net.",
								c.user_message );

			std::cout << " * " << c.user_message << std::endl;
			return true;
		}
	}

	std::string emu_name = c.emulator.get_info( FeEmulatorInfo::Name );

	//
	// Build a map for looking up parents
	//
	std::map < std::string, FeRomInfo * > parent_map;
	build_parent_map( parent_map, c.romlist );

	//
	// Build a worklist of the roms where we need to lookup
	//
	std::vector<FeRomInfo *> worklist;
	worklist.reserve( c.romlist.size() );
	for ( FeRomInfoListType::iterator itr=c.romlist.begin(); itr!=c.romlist.end(); ++itr )
	{
		(*itr).set_info( FeRomInfo::Emulator, emu_name );

		// Don't scrape for a clone if its parent has the same name
		//
		if ( has_same_name_as_parent( *itr, parent_map ) )
			continue;

		if ( !c.scrape_art || m_scrape_fanart
				|| ( m_scrape_flyers && (!has_artwork( *itr, "flyer" ) ) )
				|| ( m_scrape_wheels && (!has_artwork( *itr, "wheel" ) ) )
				|| ( m_scrape_snaps && (!has_image_artwork( *itr, "snap" ) ) )
				|| ( m_scrape_marquees && (!has_artwork( *itr, "marquee" ) ) ) )
			worklist.push_back( &(*itr) );
	}

	const int NUM_ARTS=5; // the number of scrape-able artwork types
	int done_count( 0 );

	//
	// Set up our initial queue of network tasks
	//
	for ( unsigned int i=0; i<worklist.size(); i++ )
	{
		std::string req_string = GAME_REQ;

		std::string game = url_escape(
				name_with_brackets_stripped( worklist[i]->get_info( FeRomInfo::Title ) ) );

		perform_substitution( req_string, "$1", game );

		//
		// If we don't need to scrape a wheel artwork, then add the specific platform to our request
		// If we are scraping a wheel, we want to be able to grab them where the game name (but
		// not the system) matches, so we don't limit ourselves by system...
		//
		if (( system_list.size() == 1 )
			&& ( !c.scrape_art || !m_scrape_wheels || has_artwork( *(worklist[i]), "wheel" ) ))
		{
			req_string += "&platform=";
			req_string += url_escape( system_list.front() );
		}

		q.add_buffer_task( HOSTNAME, req_string, i );
	}

	std::string base_path = get_config_dir() + FE_SCRAPER_SUBDIR;
	base_path += emu_name + "/";

	//
	// Create worker threads to process the queue, adding new tasks to download
	// artwork files where appropriate
	//
	FeNetWorker worker1( q ), worker2( q ), worker3( q ), worker4( q );
	std::string aux;

	//
	// Process the output queue from our worker threads
	//
	while ( !q.input_done() || !q.output_done() )
	{
		int id;
		std::string result;

		if ( q.pop_completed_task( id, result ) )
		{
			if ( id < 0 )
			{
				if (( id == FeNetTask::FileTask ) || ( id == FeNetTask::SpecialFileTask ))
				{
					std::cout << " + Downloaded: " << result << std::endl;
					c.download_count++;

					// find second last forward slash in filename
					// we assume that there will always be at least two
					size_t pos = result.find_last_of( "\\/" );
					pos = result.find_last_of( "\\/", pos-1 );
					aux = result.substr( pos );
				}

				if ( id == FeNetTask::FileTask ) // we don't increment if id = FeNetTask::SpecialFileTask
					done_count++;
			}
			else
			{
				FeGameDBArt my_art;
				FeGameDBParser gdbp( system_list, *(worklist[id]), ( c.scrape_art ? &my_art : NULL ) );
				gdbp.parse( result );

				if ( c.scrape_art && !my_art.base.empty() )
				{
					std::string hostn = HOSTNAME;
					std::string base_req = "banners/";
					size_t pos=0;
					for ( int i=0; i<3 && ( pos != std::string::npos ); i++ )
						pos = my_art.base.find_first_of( '/', pos+1 );

					if (( pos != std::string::npos ) && ( pos < my_art.base.size()-1 ) )
					{
						hostn = my_art.base.substr( 0, pos+1 );
						base_req = my_art.base.substr( pos+1 );
					}

					FeRomInfo &rom = *(worklist[id]);

					if ( m_scrape_flyers && ( !my_art.flyer.empty() ) && (!has_artwork( rom, "flyer" )) )
					{
						const char *FLYER = "flyer/";
						std::string fname = base_path + FLYER + rom.get_info( FeRomInfo::Romname );
						confirm_directory( base_path, FLYER );
						q.add_file_task( hostn, base_req + my_art.flyer, fname );
					}
					else
						done_count++;

					if ( m_scrape_wheels && ( !my_art.wheel.empty() ) && (!has_artwork( rom, "wheel" )) )
					{
						const char *WHEEL = "wheel/";
						std::string fname = base_path + WHEEL + rom.get_info( FeRomInfo::Romname );
						confirm_directory( base_path, WHEEL );
						q.add_file_task( hostn, base_req + my_art.wheel, fname );
					}
					else
						done_count++;

					if ( m_scrape_marquees && (!my_art.marquee.empty() ) && (!has_artwork( rom, "marquee" )) )
					{
						const char *MARQUEE = "marquee/";
						std::string fname = base_path + MARQUEE + rom.get_info( FeRomInfo::Romname );
						confirm_directory( base_path, MARQUEE );
						q.add_file_task( hostn, base_req + my_art.marquee, fname );
					}
					else
						done_count++;

					if ( m_scrape_snaps && (!my_art.snap.empty() ) && (!has_image_artwork( rom, "snap" )) )
					{
						const char *SNAP = "snap/";
						std::string fname = base_path + SNAP + rom.get_info( FeRomInfo::Romname );
						confirm_directory( base_path, SNAP );
						q.add_file_task( hostn, base_req + my_art.snap, fname );
					}
					else
						done_count++;

					if ( m_scrape_fanart && !my_art.fanart.empty() )
					{
						const char *FANART = "fanart/";
						std::string fname_base = base_path + FANART + rom.get_info( FeRomInfo::Romname ) + "/";
						confirm_directory( base_path, "" );
						confirm_directory( base_path + FANART, rom.get_info( FeRomInfo::Romname ) );
						bool done_first=false; // we only count the first fanart against our percentage completed...

						for ( std::vector<std::string>::iterator itr = my_art.fanart.begin();
								itr != my_art.fanart.end(); ++itr )
						{
							size_t start_pos = (*itr).find_last_of( "/\\" );
							size_t end_pos = (*itr).find_last_of( '.' );

							if (( start_pos != std::string::npos )
								&& ( !file_exists( fname_base + (*itr).substr( start_pos+1 ) ) ))
							{
								if (( end_pos != std::string::npos ) && ( end_pos > start_pos ))
								{
									q.add_file_task( hostn,
												base_req + (*itr),
												fname_base + (*itr).substr( start_pos+1, end_pos-start_pos-1 ),
												done_first );
									done_first=true;
								}
							}
						}
					}
					else
						done_count++;
				}
				else
				{
					aux = (worklist[id])->get_info( FeRomInfo::Title );
					done_count+=NUM_ARTS;
				}
			}

			if ( c.uiupdate )
			{
				int p = c.progress_past + done_count * c.progress_range / ( NUM_ARTS * worklist.size() );
				if ( c.uiupdate( c.uiupdatedata, p, aux ) == false )
					return false;
			}
		}
		else if ( q.output_done() )
		{
			sf::Http::Response::Status status;
			q.do_next_task( status );
		}
		else
			sf::sleep( sf::milliseconds( 10 ) );
	}
#endif
	return true;
}


void FeSettings::apply_xml_import( FeImporterContext &c )
{
	std::string base_command = clean_path( c.emulator.get_info(
				FeEmulatorInfo::Executable ) );

	switch ( c.emulator.get_info_source() )
	{

	case FeEmulatorInfo::Listxml:
	{
		std::cout << " - Obtaining -listxml info...";
		FeMameXMLParser mamep( c );
		if ( !mamep.parse( base_command ) )
			std::cout << "No XML output found, command: "
				<< base_command << " -listxml" << std::endl;
	}
	break;

	case FeEmulatorInfo::Listsoftware:
	{
		const std::vector < std::string > &system_names = c.emulator.get_systems();
		if ( system_names.empty() )
		{
			std::cout << "Note: No system configured for emulator: "
				<< c.emulator.get_info( FeEmulatorInfo::Name )
				<< ", unable to obtain -listsoftware info."
				<< std::endl;
			return;
		}

		FeMessXMLParser messp( c );
		messp.parse( base_command, system_names );
	}
	break;

	case FeEmulatorInfo::Steam:
	{
		const std::vector<std::string> &paths = c.emulator.get_paths();
		const std::vector<std::string> &exts = c.emulator.get_extensions();
		if ( paths.empty() || exts.empty() )
			return;

		std::string path = clean_path( paths.front(), true );
		const std::string &extension = exts.front();

		// A bit mislabelled: the steam import isn't really xml.
		for ( FeRomInfoListType::iterator itr = c.romlist.begin(); itr != c.romlist.end(); ++itr )
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

					if ( icompare( val, "appid" ) == 0 )
					{
						std::string id;
						token_helper( line, pos, id, FE_WHITESPACE );
						(*itr).set_info( FeRomInfo::Romname, id );
						fields_left--;
					}
					else if ( icompare( val, "name" ) == 0 )
					{
						std::string name;
						token_helper( line, pos, name, FE_WHITESPACE );
						(*itr).set_info( FeRomInfo::Title, name );
						fields_left--;
					}
					else if ( icompare( val, "installdir" ) == 0 )
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
		thegamesdb_scraper( c );
	}
	break;

	case FeEmulatorInfo::Thegamesdb:
		thegamesdb_scraper( c );
		break;

	case FeEmulatorInfo::Scummvm:
		scummvm_lookup( c );
		thegamesdb_scraper( c );
		break;

	case FeEmulatorInfo::None:
	default:
		break;
	}
}

bool FeSettings::build_romlist( const std::vector< FeImportTask > &task_list,
						const std::string &output_name,
						FeFilter &filter,
						bool full )
{
	FeRomInfoListType total_romlist;
	std::string best_name, list_name, path;

	for ( std::vector<FeImportTask>::const_iterator itr=task_list.begin();
			itr < task_list.end(); ++itr )
	{
		if ( (*itr).task_type == FeImportTask::BuildRomlist )
		{
			// Build romlist task
			std::cout << "*** Generating Collection/Rom List" << std::endl;

			FeEmulatorInfo *emu = m_rl.get_emulator( (*itr).emulator_name );
			if ( emu == NULL )
			{
				std::cout << "Error: Invalid -build-rom-list target: " <<  (*itr).emulator_name
					<< std::endl;
			}
			else
			{
				FeRomInfoListType romlist;

				best_name = emu->get_info( FeEmulatorInfo::Name );

				FeImporterContext ctx( *emu, romlist );
				build_basic_romlist( ctx );

				apply_xml_import( ctx );
				apply_import_extras( ctx );

				apply_emulator_name( best_name, romlist );
				total_romlist.splice( total_romlist.end(), romlist );
			}
		}
		else if ( (*itr).task_type == FeImportTask::ImportRomlist )
		{
			// import romlist from file task
			std::cout << "*** Importing Collection/Rom List" << std::endl;

			FeRomInfoListType romlist;
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
				FeRomList temp_list( m_config_path );
				temp_list.load_from_file( (*itr).file_name, ";" );

				FeRomInfoListType &entries = temp_list.get_list();

				for ( FeRomInfoListType::iterator itr = entries.begin(); itr != entries.end(); ++itr )
					romlist.push_back( *itr );
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
					apply_emulator_name( emu_name, romlist );
			}
			else
			{
				std::cerr << "Error: Unsupported --import-rom-list file: "
					<<  (*itr).file_name << std::endl;
			}

			std::cout << "[Import " << (*itr).file_name << "] - Imported " << romlist.size() << " entries."
				<< std::endl;

			FeEmulatorInfo *emu = m_rl.get_emulator( emu_name );
			if ( emu == NULL )
			{
				std::cout << "Warning: The emulator specified with --import-rom-list was not found: "
					<<  emu_name << std::endl;
			}
			else
			{
				FeImporterContext ctx( *emu, romlist );
				apply_import_extras( ctx );
			}

			total_romlist.splice( total_romlist.end(), romlist );
		}
		else // scrape artwork
		{
			FeEmulatorInfo *emu = m_rl.get_emulator( (*itr).emulator_name );
			if ( emu == NULL )
				return false;

			std::cout << "*** Scraping artwork for: " << (*itr).emulator_name << std::endl;

			FeRomInfoListType romlist;
			std::string fn = get_config_dir() + FE_ROMLIST_SUBDIR + (*itr).emulator_name + FE_ROMLIST_FILE_EXTENSION;

			FeImporterContext ctx( *emu, romlist );
			if ( file_exists( fn ) )
			{
				FeRomList loader( get_config_dir() );
				loader.load_from_file( fn, ";" );
				ctx.romlist.swap( loader.get_list() );
			}
			else
			{
				build_basic_romlist( ctx );
				apply_xml_import( ctx );
			}

			ctx.scrape_art = true;
			confirm_directory( get_config_dir(), FE_SCRAPER_SUBDIR );

			// do the mame-specific scrapers first, followed
			// by the more general thegamesdb scraper.
			mameps_scraper( ctx );
			mamedb_scraper( ctx );
			thegamesdb_scraper( ctx );

			std::cout << "*** Scraping done." << std::endl;
		}
	}

	// return now if all we did was scrape artwork
	if ( total_romlist.empty() )
		return true;

	total_romlist.sort( FeRomListSorter() );

	// strip duplicate entries
	std::cout << " - Removing any duplicate entries..." << std::endl;
	total_romlist.unique();

	// Apply the specified filter
	if ( filter.get_rule_count() > 0 )
	{
		std::cout << " - Applying filter..." << std::endl;
		filter.init();

		FeRomInfoListType::iterator last_it=total_romlist.begin();
		for ( FeRomInfoListType::iterator it=total_romlist.begin(); it!=total_romlist.end(); )
		{
			if ( filter.apply_filter( *it ) )
			{
				if ( last_it != it )
					it = total_romlist.erase( last_it, it );
				else
					++it;

				last_it = it;
			}
			else
				++it;
		}

		if ( last_it != total_romlist.end() )
			total_romlist.erase( last_it, total_romlist.end() );
	}

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
	FeEmulatorInfo *emu = m_rl.get_emulator( emu_name );
	if ( emu == NULL )
		return false;

	//
	// Put up the "building romlist" message at 0 percent while we get going...
	//
	if ( uiu )
		uiu( uid, 0, "" );

	std::cout << "*** Generating Collection/Rom List" << std::endl;

	FeRomInfoListType romlist;

	FeImporterContext ctx( *emu, romlist );
	ctx.uiupdate = uiu;
	ctx.uiupdatedata = uid;

	build_basic_romlist( ctx );
	apply_xml_import( ctx );
	apply_import_extras( ctx );
	apply_emulator_name( emu_name, romlist );

	romlist.sort( FeRomListSorter() );

	// strip duplicate entries
	std::cout << " - Removing any duplicate entries..." << std::endl;
	romlist.unique();

	size = romlist.size();

	std::string filename = get_config_dir();
	confirm_directory( filename, FE_ROMLIST_SUBDIR );

	if ( uiu )
		uiu( uid, 100, "" );

	filename += FE_ROMLIST_SUBDIR;
	filename += emu_name;
	filename += FE_ROMLIST_FILE_EXTENSION;
	write_romlist( filename, romlist );

	return true;
}

bool FeSettings::scrape_artwork( const std::string &emu_name, UiUpdate uiu, void *uid, std::string &msg )
{
	FeEmulatorInfo *emu = m_rl.get_emulator( emu_name );
	if ( emu == NULL )
		return false;

	//
	// Put up the "scraping artwork" message at 0 percent while we get going...
	//
	if ( uiu )
		uiu( uid, 0, "" );

	std::cout << "*** Scraping artwork for: " << emu_name << std::endl;

	FeRomInfoListType romlist;

	std::string fn = get_config_dir() + FE_ROMLIST_SUBDIR + emu_name + FE_ROMLIST_FILE_EXTENSION;

	FeImporterContext ctx( *emu, romlist );
	ctx.uiupdate = uiu;
	ctx.uiupdatedata = uid;

	if ( file_exists( fn ) )
	{
		FeRomList loader( get_config_dir() );
		loader.load_from_file( fn, ";" );
		ctx.romlist.swap( loader.get_list() );
	}
	else
	{
		ctx.progress_range=33;
		build_basic_romlist( ctx );
		apply_xml_import( ctx );
		ctx.progress_past=33;
	}

	ctx.progress_range = ( 100-ctx.progress_past ) / 2;
	ctx.scrape_art = true;
	confirm_directory( get_config_dir(), FE_SCRAPER_SUBDIR );

	// do the mame-specific scrapers first followed
	// by the more general thegamesdb scraper.
	// These return false if the user cancels...
	//
	if ( mameps_scraper( ctx ) && mamedb_scraper( ctx ) )
	{
		ctx.progress_past = ctx.progress_past + ctx.progress_range;
		thegamesdb_scraper( ctx );
	}

	if ( uiu )
		uiu( uid, 100, "" );

	std::cout << "*** Scraping done." << std::endl;

	if ( ctx.user_message.empty() )
		get_resource( "Scraped $1 artwork file(s)", as_str( ctx.download_count ), msg );
	else
		msg = ctx.user_message;

	return true;
}
