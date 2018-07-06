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
#include <cstring>

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

#ifndef NO_NET
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
	while ( !q.all_done() )
	{
		int id;
		std::string result;

		if ( q.pop_completed_task( id, result ) )
		{
			if ( id < 0 )
			{
				if ( id == -1 )
				{
					FeLog() << " + Downloaded: " << result << std::endl;
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
#endif

}; // end namespace

bool FeSettings::simple_scraper( FeImporterContext &c,
		const char *host,
		const char *pre_req,
		const char *post_req,
		const char *art_label,
		bool is_vid )
{
#ifndef NO_NET
	//
	// Build a map for looking up parents
	//
	ParentMapType parent_map;
	build_parent_map( parent_map, c.romlist, false );

	FeLog() << " - Scraping " << host << " [" << art_label << "]" << std::endl;

	std::string emu_name = c.emulator.get_info( FeEmulatorInfo::Name );
	std::string base_path = get_config_dir() + FE_SCRAPER_SUBDIR;
	base_path += emu_name + "/";

	FeNetQueue q;
	int taskc( 0 );

	bool is_snap = ( strcmp( art_label, "snap" ) == 0 );

	for ( FeRomInfoListType::iterator itr=c.romlist.begin(); itr!=c.romlist.end(); ++itr )
	{
		// ugh, this must be set for has_artwork() to correctly function
		(*itr).set_info( FeRomInfo::Emulator, emu_name );

		// Don't scrape for a clone if its parent has the same name
		//
		if ( has_same_name_as_parent( *itr, parent_map ) )
			continue;

		bool do_scrape = false;
		if ( !is_snap )
			do_scrape = !has_artwork( *itr, art_label );
		else if ( is_vid ) // vid snap
			do_scrape = !has_video_artwork( *itr, art_label );
		else // image snap
			do_scrape = !has_image_artwork( *itr, art_label );

		if ( do_scrape )
		{
			const std::string &rname = (*itr).get_info( FeRomInfo::Romname );
			std::string fname = base_path + art_label + "/" + rname;

			std::string al_sub = art_label;
			al_sub += "/";
			confirm_directory( base_path, al_sub );

			q.add_file_task( host, pre_req + rname + post_req, fname );
			taskc++;
		}
	}

	return process_q_simple( q, c, taskc );
#else
	return true;
#endif
}

bool FeSettings::general_mame_scraper( FeImporterContext &c )
{
	if ( !c.emulator.is_mame() )
		return true;

	const char *MAMEDB = "http://mamedb.blu-ferret.co.uk";
	const char *ADB = "http://adb.arcadeitalia.net";
	const char *PNG = ".png";

	if ( m_scrape_marquees )
	{
		if ( !simple_scraper( c, MAMEDB,
				"/marquees/",
				PNG,
				"marquee" ) )
			return false;

		if ( !simple_scraper( c, ADB,
				"/media/mame.current/marquees/",
				PNG,
				"marquee" ) )
			return false;
	}

	if ( m_scrape_snaps )
	{
		if ( !simple_scraper( c, MAMEDB,
				"/snap/",
				PNG,
				"snap" ) )
			return false;

		if ( !simple_scraper( c, ADB,
				"/media/mame.current/ingames/",
				PNG,
				"snap" ) )
			return false;
	}

	if ( m_scrape_flyers )
		if ( !simple_scraper( c, ADB,
				"/media/mame.current/flyers/",
				PNG,
				"flyer" ) )
			return false;

	if ( m_scrape_wheels )
		if ( !simple_scraper( c, ADB,
				"/media/mame.current/decals/",
				PNG,
				"wheel" ) )
			return false;


	if ( m_scrape_vids )
		if ( !simple_scraper( c,
				"http://www.progettosnaps.net",
				"/videosnaps/mp4/",
				".mp4",
				"snap",
				true ) )
			return false;

	return true;
}

#ifndef NO_NET
namespace
{
//
// Functions and constants used by thegamesdb_scraper()
//
const char *HOSTNAME = "http://legacy.thegamesdb.net";
const char *PLATFORM_LIST_REQ = "api/GetPlatformsList.php";
const char *PLAT_REQ = "api/GetPlatform.php?id=$1";
const char *GAME_REQ = "api/GetGame.php?name=$1";

const char *flyer_sub = "flyer/";
const char *wheel_sub = "wheel/";
const char *marquee_sub = "marquee/";
const char *snap_sub = "snap/";
const char *fanart_sub = "fanart/";

//
// Get a list of valid platforms
//
bool get_system_list( FeImporterContext &c,
	FeSettings &fes,
	std::vector<std::string> &system_list,
	std::vector<int> &system_ids )
{
	FeNetQueue q;
	q.add_buffer_task( HOSTNAME, PLATFORM_LIST_REQ, 0 );
	sf::Http::Response::Status status;
	std::string err_req;

	q.do_next_task( status, err_req );

	if ( status != sf::Http::Response::Ok )
	{
		fes.get_resource( "Error getting platform list from thegamesdb.net.  Code: $1",
			as_str( status ), c.user_message );

		FeLog() << " ! " << c.user_message << " (" << err_req << ")" << std::endl;
		return false;
	}

	int ignored;
	std::string body;
	q.pop_completed_task( ignored, body );

	FeGameDBPlatformListParser gdbplp;
	gdbplp.parse( body );

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
			fes.get_resource( "Error: None of the configured system identifier(s) are recognized by thegamesdb.net.",
				c.user_message );

			FeLog() << " ! " << c.user_message << std::endl;
			return false;
		}
	}

	return true;
}

//
// helper function for creating a "file download" task
//
bool create_ft(
	FeNetQueue &q,
	const std::string &source_base,
	const std::string &source_name,
	const std::string &out_base,
	const char *out_sub,
	const std::string &out_name )
{
	if ( source_name.empty() )
		return false;

	std::string fname = out_base;
	fname += out_sub;

	if ( art_exists( fname, out_name ) )
		return false;

	fname += out_name;

	confirm_directory( out_base, out_sub );

	std::string hostn = HOSTNAME;
	std::string base_req = "banners/";
	get_url_components( source_base, hostn, base_req );


	q.add_file_task( hostn, base_req + source_name, fname );
	return true;
}

bool create_fanart_ft(
	FeNetQueue &q,
	const std::string &source_base,
	const std::vector < std::string > &source_names,
	const std::string &out_base,
	const std::string &out_name,
	bool done_first=true )
{
	if ( source_names.empty() )
		return false;

	std::string fname = out_base;
	fname += fanart_sub;
	fname += out_name;
	fname += "/";

	confirm_directory( out_base, "" );
	confirm_directory( out_base + fanart_sub, out_name );

	std::string hostn = HOSTNAME;
	std::string base_req = "banners/";
	get_url_components( source_base, hostn, base_req );

	for ( std::vector<std::string>::const_iterator itr = source_names.begin();
			itr != source_names.end(); ++itr )
	{
		size_t start_pos = (*itr).find_last_of( "/\\" );

		if (( start_pos != std::string::npos )
			&& ( !file_exists( fname + (*itr).substr( start_pos+1 ) ) ))
		{
			size_t end_pos = (*itr).find_last_of( '.' );
			if (( end_pos != std::string::npos ) && ( end_pos > start_pos ))
			{
				q.add_file_task( hostn,
					base_req + (*itr),
					fname + (*itr).substr( start_pos+1, end_pos-start_pos-1 ),
					done_first );

				done_first=true;
			}
		}
	}

	return true;
}

};
#endif

bool FeSettings::thegamesdb_scraper( FeImporterContext &c )
{
#ifndef NO_NET
	std::vector<std::string> system_list;
	std::vector<int> system_ids;

	FeLog() << " - scraping thegamesdb.net..." << std::endl;

	//
	// Get a list of valid platforms
	//
	if ( !get_system_list( c, *this, system_list, system_ids ) )
		return true;

	FeNetQueue q;

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
			sf::Http::Response::Status status;
			std::string err_req;

			std::string plat_string = PLAT_REQ;
			perform_substitution( plat_string, "$1", as_str( *iti ) );

			q.add_buffer_task( HOSTNAME, plat_string, 0 );
			q.do_next_task( status, err_req );
			if ( status != sf::Http::Response::Ok )
			{
				FeLog() << " * Unable to get platform information. Status code: "
					<< status << " (" << err_req << ")" << std::endl;
				continue;
			}

			int ignored;
			std::string body;
			q.pop_completed_task( ignored, body );

			FeGameDBArt my_art;
			FeGameDBPlatformParser gdbpp( my_art );
			gdbpp.parse( body );

#define CHECK_AND_CREATE_FT(LABEL) \
	do { if ( m_scrape_##LABEL##s  ) \
		create_ft( q, my_art.base, my_art.LABEL, base_path, LABEL##_sub, emu_name ); \
	} while(0)

			CHECK_AND_CREATE_FT(flyer);
			CHECK_AND_CREATE_FT(wheel);
			CHECK_AND_CREATE_FT(marquee);
			CHECK_AND_CREATE_FT(snap);
#undef CHECK_AND_CREATE_FT

			if ( m_scrape_fanart && !my_art.fanart.empty() )
				create_fanart_ft( q, my_art.base, my_art.fanart, base_path, emu_name );
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
	while ( !q.all_done() )
	{
		int id;
		std::string result;

		if ( q.pop_completed_task( id, result ) )
		{
			if ( id < 0 )
			{
				if (( id == FeNetTask::FileTask ) || ( id == FeNetTask::SpecialFileTask ))
				{
					FeLog() << " + Downloaded: " << result << std::endl;
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
					const FeRomInfo &rom = *(worklist[id]);

					bool use_alt = ( prefer_alt_filename
						&& !rom.get_info( FeRomInfo::AltRomname ).empty() );

					const std::string &rn = use_alt ? rom.get_info( FeRomInfo::AltRomname )
						: rom.get_info( FeRomInfo::Romname );

#define CHECK_AND_CREATE_FT(LABEL) \
	do { if ( m_scrape_##LABEL##s && ( !my_art.LABEL.empty() ) \
			&& (!has_artwork( rom, #LABEL )) ) \
		create_ft( q, my_art.base, my_art.LABEL, base_path, LABEL##_sub, rn ); \
	else \
		done_count++; } \
	while(0)

					CHECK_AND_CREATE_FT(flyer);
					CHECK_AND_CREATE_FT(wheel);
					CHECK_AND_CREATE_FT(marquee);
					CHECK_AND_CREATE_FT(snap);
#undef CHECK_AND_CREATE_FT

					if ( m_scrape_fanart && !my_art.fanart.empty() )
						create_fanart_ft( q, my_art.base, my_art.fanart, base_path, rn, false );
					else
						done_count++;
				}
				else
				{
					std::string overview;
					if( m_scrape_overview && gdbp.get_overview( overview ) )
					{
						//
						// Save the overview now...
						//
						std::map<GameExtra,std::string> extras;
						const std::string &rn = (worklist[id])->get_info( FeRomInfo::Romname );

						load_game_extras( c.out_name, rn, extras );
						extras[ Overview ] = overview;
						save_game_extras( c.out_name, rn, extras );
					}

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
				FeLog() << " * Error processing request. Status code: "
					<< status << " (" << err_req << ")" << std::endl;
			}
		}
		else
			sf::sleep( sf::milliseconds( 10 ) );
	}
#endif
	return true;
}
