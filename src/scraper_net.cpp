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

typedef std::map < std::string, FeRomInfo * > ParentMapType;

void build_parent_map(
	ParentMapType &parent_map,
	FeRomInfoListType &romlist,
	bool prefer_alt_filename )
{
	for ( FeRomInfoListType::iterator itr=romlist.begin(); itr!=romlist.end(); ++itr )
	{
		if ( (*itr).get_info( FeRomInfo::Cloneof ).empty() )
		{
			parent_map[ (*itr).get_info( FeRomInfo::Romname ) ] = &(*itr);

			if (( prefer_alt_filename )
					&& (!(*itr).get_info( FeRomInfo::AltRomname ).empty() ))
				parent_map[ (*itr).get_info( FeRomInfo::AltRomname ) ] = &(*itr);
		}
	}
}

bool has_same_name_as_parent( FeRomInfo &rom, ParentMapType &parent_map )
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

					if ( pos != std::string::npos )
					{
						pos = result.find_last_of( "\\/", pos-1 );

						if ( pos != std::string::npos )
							aux = result.substr( pos );
					}
				}

				done++;
			}
		}
		else
			sf::sleep( sf::milliseconds( 10 ) );

		if ( c.uiupdate && ( taskc > 0 ) )
		{
			int p = c.progress_past + done * c.progress_range / taskc;
			if ( c.uiupdate( c.uiupdatedata, p, aux ) == false )
				return false;
		}
	}
	return true;
}

}; // end namespace

bool FeSettings::mameps_scraper( FeImporterContext &c )
{
#ifndef NO_NET
	if ( !c.emulator.is_mame() || !m_scrape_vids )
		return true;

	//
	// Build a map for looking up parents
	//
	ParentMapType parent_map;
	build_parent_map( parent_map, c.romlist, false );

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
	if ( !c.emulator.is_mame() || ( !m_scrape_snaps && !m_scrape_marquees ))
		return true;

	//
	// Build a map for looking up parents
	//
	ParentMapType parent_map;
	build_parent_map( parent_map, c.romlist, false );

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
	const char *PLATFORM_LIST_REQ = "api/GetPlatformsList.php";
	const char *PLAT_REQ = "api/GetPlatform.php?id=$1";
	const char *GAME_REQ = "api/GetGame.php?name=$1";
	const char *FLYER = "flyer/";
	const char *WHEEL = "wheel/";
	const char *MARQUEE = "marquee/";
	const char *SNAP = "snap/";
	const char *FANART = "fanart/";

	//
	// Get a list of valid platforms
	//
	FeNetQueue q;
	q.add_buffer_task( HOSTNAME, PLATFORM_LIST_REQ, 0 );
	sf::Http::Response::Status status;
	std::string err_req;

	q.do_next_task( status, err_req );

	if ( status != sf::Http::Response::Ok )
	{
		get_resource( "Error getting platform list from thegamesdb.net.  Code: $1",
			as_str( status ), c.user_message );

		std::cerr << " ! " << c.user_message << " (" << err_req << ")" << std::endl;
		return true;
	}

	int temp;
	std::string body;
	q.pop_completed_task( temp, body );

	FeGameDBPlatformListParser gdbplp;
	gdbplp.parse( body );

	std::vector<std::string> system_list;
	std::vector<int> system_ids;

	const std::vector<std::string> &sl_temp = c.emulator.get_systems();
	for ( std::vector<std::string>::const_iterator itr = sl_temp.begin();
			itr != sl_temp.end(); ++itr )
	{
		std::string comp_fuzz = get_fuzzy( *itr );

		for ( size_t i=0; i<gdbplp.m_names.size(); i++ )
		{
			ASSERT( gdbplp.m_names.size() == gdbplp.m_ids.size() );

			std::string &n = gdbplp.m_names[i];
			int id = ( i < gdbplp.m_ids.size() ) ? gdbplp.m_ids[i] : 0;

			if ( comp_fuzz.compare( get_fuzzy( n ) ) == 0 )
			{
				system_list.push_back( n );
				system_ids.push_back( id );
				break;
			}
			else
			{
				size_t pos = n.find_first_of( "(" );

				if (( pos != std::string::npos ) &&
					(( comp_fuzz.compare( get_fuzzy( n.substr(0,pos-1))) == 0 )
					|| ( comp_fuzz.compare(get_fuzzy( n.substr(pos+1,n.size()-pos-1 ))) == 0 )))
				{
					system_list.push_back( n );
					system_ids.push_back( id );
					break;
				}
			}
		}
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

			std::cerr << " ! " << c.user_message << std::endl;
			return true;
		}
	}

	std::string emu_name = c.emulator.get_info( FeEmulatorInfo::Name );
	std::string base_path = get_config_dir() + FE_SCRAPER_SUBDIR;
	base_path += emu_name + "/";

	if ( c.scrape_art )
	{
		//
		// Get emulator-specific images
		//
		for ( std::vector<int>::iterator iti=system_ids.begin();
				iti != system_ids.end(); ++iti )
		{
			std::string plat_string = PLAT_REQ;
			perform_substitution( plat_string, "$1", as_str( *iti ) );

			q.add_buffer_task( HOSTNAME, plat_string, 0 );
			q.do_next_task( status, err_req );
			if ( status != sf::Http::Response::Ok )
			{
				std::cout << " * Unable to get platform information. Status code: "
					<< status << " (" << err_req << ")" << std::endl;
				continue;
			}

			body.clear();
			q.pop_completed_task( temp, body );

			FeGameDBArt my_art;
			FeGameDBPlatformParser gdbpp( my_art );
			gdbpp.parse( body );

			std::string hostn = HOSTNAME;
			std::string base_req = "banners/";
			get_url_components( my_art.base,
				hostn, base_req );

			std::string path = base_path + FLYER;
			if ( m_scrape_flyers && ( !my_art.flyer.empty() )
				&& ( !art_exists( path, emu_name ) ))
			{
				confirm_directory( base_path, FLYER );
				q.add_file_task( hostn, base_req + my_art.flyer,
					path + emu_name );
			}

			path = base_path + WHEEL;
			if ( m_scrape_wheels && ( !my_art.wheel.empty() )
				&& ( !art_exists( path, emu_name ) ))
			{
				confirm_directory( base_path, WHEEL );
				q.add_file_task( hostn, base_req + my_art.wheel,
					path + emu_name );
			}

			path = base_path + MARQUEE;
			if ( m_scrape_marquees && ( !my_art.marquee.empty() )
				&& ( !art_exists( path, emu_name ) ))
			{
				confirm_directory( base_path, MARQUEE );
				q.add_file_task( hostn, base_req + my_art.marquee,
					path + emu_name );
			}

			if ( m_scrape_fanart && !my_art.fanart.empty() )
			{
				std::string path_base = base_path + FANART + emu_name + "/";
				confirm_directory( base_path, "" );
				confirm_directory( base_path + FANART, emu_name );

				for ( std::vector<std::string>::iterator itr = my_art.fanart.begin();
							itr != my_art.fanart.end(); ++itr )
				{
					size_t start_pos = (*itr).find_last_of( "/\\" );
					size_t end_pos = (*itr).find_last_of( '.' );

					if (( start_pos != std::string::npos )
						&& ( !file_exists( path_base + (*itr).substr( start_pos+1 ) ) ))
					{
						if (( end_pos != std::string::npos ) && ( end_pos > start_pos ))
						{
							q.add_file_task( hostn,
								base_req + (*itr),
								path_base + (*itr).substr( start_pos+1,
											end_pos-start_pos-1 ) );
						}
					}
				}
			}
		}
	}

	bool prefer_alt_filename = c.emulator.is_mess();

	//
	// Build a map for looking up parents
	//
	ParentMapType parent_map;
	build_parent_map( parent_map, c.romlist, prefer_alt_filename );

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

	//
	// Create worker threads to process the queue, adding new tasks to download
	// artwork files where appropriate
	//
	FeNetWorker worker1( q ), worker2( q ), worker3( q ), worker4( q );
	std::string aux;

	//
	// Process the output queue from our worker threads
	//
	while ( !( q.input_done() && q.output_done() ) )
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
					if ( pos != std::string::npos )
					{
						pos = result.find_last_of( "\\/", pos-1 );
						if ( pos != std::string::npos )
							aux = result.substr( pos );
					}
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
					get_url_components( my_art.base,
						hostn, base_req );

					const FeRomInfo &rom = *(worklist[id]);

					if ( m_scrape_flyers && ( !my_art.flyer.empty() ) && (!has_artwork( rom, "flyer" )) )
					{
						std::string fname = base_path + FLYER;

						const std::string &altname = rom.get_info( FeRomInfo::AltRomname );
						if ( prefer_alt_filename && !altname.empty() )
							fname += rom.get_info( FeRomInfo::AltRomname );
						else
							fname += rom.get_info( FeRomInfo::Romname );

						confirm_directory( base_path, FLYER );
						q.add_file_task( hostn, base_req + my_art.flyer, fname );
					}
					else
						done_count++;

					if ( m_scrape_wheels && ( !my_art.wheel.empty() ) && (!has_artwork( rom, "wheel" )) )
					{
						std::string fname = base_path + WHEEL;

						const std::string &altname = rom.get_info( FeRomInfo::AltRomname );
						if ( prefer_alt_filename && !altname.empty() )
							fname += rom.get_info( FeRomInfo::AltRomname );
						else
							fname += rom.get_info( FeRomInfo::Romname );

						confirm_directory( base_path, WHEEL );
						q.add_file_task( hostn, base_req + my_art.wheel, fname );
					}
					else
						done_count++;

					if ( m_scrape_marquees && (!my_art.marquee.empty() ) && (!has_artwork( rom, "marquee" )) )
					{
						std::string fname = base_path + MARQUEE;

						const std::string &altname = rom.get_info( FeRomInfo::AltRomname );
						if ( prefer_alt_filename && !altname.empty() )
							fname += rom.get_info( FeRomInfo::AltRomname );
						else
							fname += rom.get_info( FeRomInfo::Romname );

						confirm_directory( base_path, MARQUEE );
						q.add_file_task( hostn, base_req + my_art.marquee, fname );
					}
					else
						done_count++;

					if ( m_scrape_snaps && (!my_art.snap.empty() ) && (!has_image_artwork( rom, "snap" )) )
					{
						std::string fname = base_path + SNAP;

						const std::string &altname = rom.get_info( FeRomInfo::AltRomname );
						if ( prefer_alt_filename && !altname.empty() )
							fname += rom.get_info( FeRomInfo::AltRomname );
						else
							fname += rom.get_info( FeRomInfo::Romname );

						confirm_directory( base_path, SNAP );
						q.add_file_task( hostn, base_req + my_art.snap, fname );
					}
					else
						done_count++;

					if ( m_scrape_fanart && !my_art.fanart.empty() )
					{
						std::string fname_base = base_path + FANART;

						confirm_directory( base_path, "" );

						const std::string &altname = rom.get_info( FeRomInfo::AltRomname );
						if ( prefer_alt_filename && !altname.empty() )
						{
							fname_base += rom.get_info( FeRomInfo::AltRomname );
							confirm_directory( base_path + FANART,
								rom.get_info( FeRomInfo::AltRomname ) );
						}
						else
						{
							fname_base += rom.get_info( FeRomInfo::Romname );
							confirm_directory( base_path + FANART,
								rom.get_info( FeRomInfo::Romname ) );
						}

						fname_base += "/";

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

			if (( c.uiupdate ) && !worklist.empty() )
			{
				int p = c.progress_past + done_count * c.progress_range / ( NUM_ARTS * worklist.size() );
				if ( c.uiupdate( c.uiupdatedata, p, aux ) == false )
					return false;
			}
		}
		else if ( q.output_done() )
		{
			sf::Http::Response::Status status;
			std::string err_req;
			q.do_next_task( status, err_req );
			if ( status != sf::Http::Response::Ok )
			{
				std::cout << " * Error processing request. Status code: "
					<< status << " (" << err_req << ")" << std::endl;
			}
		}
		else
			sf::sleep( sf::milliseconds( 10 ) );
	}
#endif
	return true;
}
