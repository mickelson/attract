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
#include <list>
#include <map>
#include <cstring>

#ifdef USE_LIBCURL
#include "fe_net.hpp"
#endif // USE_LIBCURL

extern const char *FE_ROMLIST_SUBDIR;

#ifdef USE_LIBCURL
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
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

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

class URLBuilder : public URLBuilderBase
{
private:
	std::string m_host;
	std::string m_pre_req;
	std::string m_post_req;

public:
	URLBuilder( const char *host,
		const char *pre_req,
		const char *post_req )
		: URLBuilderBase(),
		m_host( host ),
		m_pre_req( pre_req ),
		m_post_req( post_req )
	{
	};

	const std::string get_url( const std::string &rname )
	{
		std::string url = m_host;

		url += m_pre_req;
		url += rname;
		url += m_post_req;

		return url;
	};
};

//
// Custom url builder for the internet archives mame video snaps at:
// https://archive.org/download/mame-videos-snaps/
//
class IAMameSnapURLBuilder : public URLBuilderBase
{
	const std::string get_url( const std::string &rname )
	{
		if ( rname.empty() )
			return "";

		std::string url =
			"https://archive.org/download/mame-videos-snaps/";

		if ( std::isdigit( rname[0] ) )
			url += "%23.zip/0";
		else
		{
			char ch = toupper( rname[0] );
			url += ch;
			url += ".zip/";
			url += ch;
		}

		url += "%2F";
		url += rname;
		url += ".mp4";

		return url;
	};
};

bool FeSettings::simple_scraper( FeImporterContext &c,
		URLBuilderBase &urlb,
		const char *art_label,
		bool is_vid )
{
#ifdef USE_LIBCURL
	//
	// Build a map for looking up parents
	//
	ParentMapType parent_map;
	build_parent_map( parent_map, c.romlist, false );

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

			const std::string url = urlb.get_url( rname );

			size_t pos = url.find_last_of( "." );
			if ( pos != std::string::npos )
				fname += url.substr( pos );
			else
				fname += ".png";

			q.add_file_task( url, fname );
			taskc++;
		}
	}

	return process_q_simple( q, c, taskc );
#else
	FeLog() << " - Unable to scrape network, frontend was built without libcurl enabled" << std::endl;
	return true;
#endif
}

bool FeSettings::general_mame_scraper( FeImporterContext &c )
{
	if ( !c.emulator.is_mame() )
		return true;

	const char *ADB = "http://adb.arcadeitalia.net";
	const char *PNG = ".png";

	if ( m_scrape_marquees )
	{
		FeLog() << " - Scraping " << ADB << " [marquee]" << std::endl;
		URLBuilder ub( ADB, "/media/mame.current/marquees/", PNG );
		if ( !simple_scraper( c, ub, "marquee" ) )
			return false;
	}

	if ( m_scrape_snaps )
	{
		FeLog() << " - Scraping " << ADB << " [snap]" << std::endl;
		URLBuilder ub( ADB, "/media/mame.current/ingames/", PNG );
		if ( !simple_scraper( c, ub, "snap" ) )
			return false;
	}

	if ( m_scrape_flyers )
	{
		FeLog() << " - Scraping " << ADB << " [flyer]" << std::endl;
		URLBuilder ub( ADB, "/media/mame.current/flyers/", PNG );
		if ( !simple_scraper( c, ub, "flyer" ) )
			return false;
	}

	if ( m_scrape_wheels )
	{
		FeLog() << " - Scraping " << ADB << " [wheel]" << std::endl;
		URLBuilder ub( ADB, "/media/mame.current/decals/", PNG );
		if ( !simple_scraper( c, ub, "wheel" ) )
			return false;
	}


	if ( m_scrape_vids )
	{
		FeLog() << " - Scraping archive.org [MAME video snap]" << std::endl;
		IAMameSnapURLBuilder ub;
		if ( !simple_scraper( c, ub, "snap", true ) )
			return false;
	}

	return true;
}
