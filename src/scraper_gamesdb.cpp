/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-19 Andrew Mickelson
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

#include <string>
#include <map>

#include "nowide/fstream.hpp"
#include "rapidjson/document.h"

#include "fe_settings.hpp"
#include "fe_util.hpp"
#include "fe_net.hpp"
#include "fe_base.hpp" // For FeLog()
#include "scraper_base.hpp"

#ifdef USE_LIBCURL

const char *FE_IDDB_EXT = ".txt";

//
// Database of game names -> gamesdb ids for a given sys_name
//
class FeIDDb
{
	private:
		std::map< std::string, int > m_name_map;
		std::string m_filename;
		bool m_changed;
		int m_plat_id;

		void reset();
		bool save();

	public:
		FeIDDb();
		FeIDDb( const FeIDDb & );
		~FeIDDb();

		FeIDDb &operator=( const FeIDDb & );

		bool load( const std::string &, const std::string &, int );

		void set_name( const std::string &, int );
		int get_id_from_name( const std::string & );

		int get_plat_id() { return m_plat_id; };
};

FeIDDb::FeIDDb()
	: m_changed( false ), m_plat_id( -1 )
{
}

FeIDDb::FeIDDb( const FeIDDb &o )
{
	*this = o;
}

FeIDDb::~FeIDDb()
{
	if ( m_changed )
		save();
}

FeIDDb &FeIDDb::operator=( const FeIDDb &o )
{
	m_name_map = o.m_name_map;
	m_filename = o.m_filename;
	m_changed = o.m_changed;
	m_plat_id = o.m_plat_id;

	return *this;
}

bool FeIDDb::load( const std::string &sys_name, const std::string &path, int plat_id )
{
	if ( m_changed )
	{
		// save current if loading a different system
		save();
		reset();
	}

	confirm_directory( path, "" );
	m_filename = path;

	m_filename += sys_name;
	m_filename += FE_IDDB_EXT;

	m_plat_id = plat_id;

	nowide::ifstream myfile( m_filename.c_str() );

	if ( !myfile.is_open() )
		return false;


	while ( myfile.good() )
	{
		std::string line;
		getline( myfile, line );

		std::string n,v;
		size_t pos=0;

		token_helper( line, pos, n, ";" );
		token_helper( line, pos, v, ";" );

		int val = as_int( v );
		if (( val > 0 ) || ( v.compare( "0" ) == 0 ))
		{
			FeDebug() << m_filename << ": " << n << " => " << val << std::endl;
			m_name_map[ n ] = val;
		}
	}

	myfile.close();
	return true;
}

void FeIDDb::reset()
{
	m_name_map.clear();
	m_filename.clear();
	m_changed = false;
	m_plat_id=-1;
}

bool FeIDDb::save()
{
	nowide::ofstream outfile( m_filename.c_str() );

	if ( !outfile.is_open() )
	{
		FeLog() << "Error opening ID database " << m_filename << " for writing" << std::endl;
		return false;
	}

	FeDebug() << "Saving ID database to: " << m_filename << std::endl;

	std::map< std::string, int >::const_iterator it;
	for ( it=m_name_map.begin(); it != m_name_map.end(); ++it )
		outfile << it->first << ";" << it->second << "\n";

	outfile.close();
	m_changed = false;
	return true;
}

void FeIDDb::set_name( const std::string &n, int id )
{
	std::map< std::string, int >::iterator it;
	it = m_name_map.find( n );

	if (( it == m_name_map.end() ) || ( it->second != id ))
		m_changed = true;

	FeDebug() << m_filename << ": set value: " << n << " => " << id << std::endl;
	m_name_map[ n ] = id;
}

int FeIDDb::get_id_from_name( const std::string &n )
{
	std::map< std::string, int >::iterator it;

	it = m_name_map.find( n );

	if ( it == m_name_map.end() )
		return -1;

	return (*it).second;
}

bool load_from_local(
	const std::string &fname,
	std::vector< std::pair < std::string, int > > &l )
{
	if ( !file_exists( fname ) )
		return false;

	nowide::ifstream myfile( fname.c_str() );
	if ( !myfile.is_open() )
		return false;

	while ( myfile.good() )
	{
		std::string line;
		getline( myfile, line );

		std::string n,v;
		size_t pos=0;

		token_helper( line, pos, n, ";" );
		token_helper( line, pos, v, ";" );

		if ( !n.empty() )
		{
			int vi = as_int( v );
			l.push_back( std::pair < std::string, int >( n, vi ) );
		}
	}

	myfile.close();
	FeDebug() << "Loaded info from local file: " << fname << std::endl;
	return true;
}

bool write_local_if_needed(
	const std::string &fname,
	std::vector< std::pair < std::string, int > > &l )
{
	if ( fname.empty() )
		return false;

	nowide::ofstream outfile( fname.c_str() );
	if ( !outfile.is_open() )
	{
		FeLog() << "Error opening file for writing: " << fname << std::endl;
		return false;
	}

	std::vector< std::pair< std::string, int > >::iterator it;
	for ( it = l.begin(); it != l.end(); ++it )
		outfile << (*it).first << ";" << (*it).second << std::endl;

	outfile.close();

	FeLog() << "Wrote file: " << fname << std::endl;
	return true;
}

const char *HOSTNAME = "https://api.thegamesdb.net/v1";
const char *PLATFORM_REQ = "/Platforms?apikey=$1";
const char *GENRES_REQ = "/Genres?apikey=$1";
const char *PUBLISHERS_REQ = "/Publishers?apikey=$1";

const char *DATA_E = "data";
const char *CODE_E = "code";
const char *STATUS_E = "status";
const char *PLATFORMS_E = "platforms";
const char *GENRES_E = "genres";
const char *PUBLISHERS_E = "publishers";
const char *GAMES_E = "games";
const char *GAME_TITLE_E = "game_title";
const char *RELEASE_E = "release";
const char *RATING_E = "rating";
const char *PLAYERS_E = "players";
const char *OVERVIEW_E = "overview";
const char *PLATFORM_E = "platform";
const char *BASEURL_E = "base_url";
const char *ORIGINAL_E = "original";
const char *IMAGES_E = "images";
const char *TYPE_E = "type";
const char *FILENAME_E = "filename";
const char *NAME_E = "name";
const char *ID_E = "id";
const char *RMA_E = "remaining_monthly_allowance";
const char *EXTRA_E = "extra_allowance";

bool parse_confirm_data( const std::string &t, rapidjson::Document &doc, int &remaining_allowance )
{
	if ( doc.Parse( t.c_str() ).HasParseError() )
	{
		FeLog() << "Error parsing json, text: " << t << std::endl;
		return false;
	}

	// check for error from thegamesdb.net api
	if ( !doc.HasMember( CODE_E ) && !doc[CODE_E].IsInt() )
	{
		FeLog() << "Error, unable to get response code, text: " << t << std::endl;
		return false;
	}

	int code = doc[CODE_E].GetInt();
	if ( code != 200 )
	{
		std::string status;
		if ( doc.HasMember( STATUS_E ) && doc[STATUS_E].IsString() )
			status = doc[STATUS_E].GetString();

		FeLog() << "Thegamesdb.net API returned an error, code: " << code << ", status: " << status << std::endl;
		return false;
	}

	// confirm data element exists
	if ( !doc.HasMember( DATA_E ) || !doc[DATA_E].IsObject() )
	{
		FeLog() << "Error, unable to get data element, text: " << t << std::endl;
		return false;
	}

	if ( doc.HasMember( RMA_E ) && doc[RMA_E].IsInt() && doc.HasMember( EXTRA_E ) && doc[EXTRA_E].IsInt() )
	{
		remaining_allowance = doc[RMA_E].GetInt() + doc[EXTRA_E].GetInt();
		FeDebug() << "remaining allowance=" << remaining_allowance << std::endl;
	}

	return true;
}

bool get_json_doc( const std::string &api_key, rapidjson::Document &doc, const char *req, int &remaining_allowance )
{
	std::string my_url = HOSTNAME;
	my_url += req;
	perform_substitution( my_url, "$1", api_key );

	FeNetTask my_task( my_url, 0 );
	my_task.do_task();
	FeDebug() << " - db query: " << my_url << std::endl;

	std::string body;
	int ignored;
	my_task.grab_result( ignored, body );

	return parse_confirm_data( body, doc, remaining_allowance );
}

bool get_tgdb_platform_list(
	const std::string &api_key,
	const std::string &local_path,
	std::vector< std::pair < std::string, int > > &plats,
	int &remaining_allowance )
{
	std::string local_file = local_path + PLATFORMS_E + FE_IDDB_EXT;

	if ( load_from_local( local_file, plats ) )
		return true;

	rapidjson::Document d;
	if ( get_json_doc( api_key, d, PLATFORM_REQ, remaining_allowance ) )
	{
		const rapidjson::Value &e = d[DATA_E];
		if ( !e.HasMember( PLATFORMS_E ) || !e[PLATFORMS_E].IsObject() )
		{
			FeLog() << "thegamedb.net - unable to get platforms element" << std::endl;
			return false;
		}

		const rapidjson::Value &p = e[PLATFORMS_E];

		rapidjson::Value::ConstMemberIterator itr;
		for ( itr=p.MemberBegin(); itr != p.MemberEnd(); ++itr )
		{
			plats.push_back( std::pair< std::string, int >(
				itr->value[ NAME_E ].GetString(),
				itr->value[ ID_E ].GetInt() ) );
		}
	}

	write_local_if_needed( local_file, plats );
	return true;
}

bool get_tgdb_matching_platform_list(
	FeSettings &fes,
	FeImporterContext &c,
	const std::string &api_key,
	const std::string &local_path,
	std::vector< std::pair < std::string, int > > &plats,
	int &remaining_allowance )
{
	std::vector< std::pair < std::string, int > > all_plats;

	if ( !get_tgdb_platform_list( api_key, local_path, all_plats, remaining_allowance ) )
		return false;

	// fuzz the all platform list
	std::vector< std::string > all_plats_fuzz;

	int arcade_id = -1;
	int pc_id = -1;

	std::vector< std::pair < std::string, int > >::iterator itr;
	for ( itr = all_plats.begin(); itr != all_plats.end(); ++itr )
	{
		all_plats_fuzz.push_back( get_fuzzy( (*itr).first ) );

		// Special case: gather the ids for Arcade and PC
		// so we can fallback to them if necessary
		//
		if ( (*itr).first.compare( "Arcade" ) == 0 )
			arcade_id = (*itr).second;
		else if ( (*itr).first.compare( "PC" ) == 0 )
			pc_id = (*itr).second;
	}

	const std::vector<std::string> &sl_temp = c.emulator.get_systems();
	std::vector<std::string>::const_iterator sit;
	for ( sit = sl_temp.begin(); sit != sl_temp.end(); ++sit )
	{
		std::string fuz = get_fuzzy( *sit );

		for ( size_t i=0; i<all_plats_fuzz.size(); i++ )
		{
			if ( all_plats_fuzz[i].compare( fuz ) == 0 )
			{
				// match
				plats.push_back( all_plats[ i ] );
				break;
			}
		}
	}

	if ( plats.empty() )
	{
		// Correct if we can based on the configured info source,
		// otherwise we error out
		switch( c.emulator.get_info_source() )
		{
		case FeEmulatorInfo::Listxml:
			plats.push_back( std::pair < std::string, int > ("Arcade", arcade_id) ); break;
		case FeEmulatorInfo::Steam:
		case FeEmulatorInfo::Scummvm:
			plats.push_back( std::pair < std::string, int > ("PC", pc_id) ); break;
		default:
			fes.get_resource( "Error: None of the configured system identifier(s) are recognized by thegamesdb.net.",
				c.user_message );
			FeLog() << " ! " << c.user_message << std::endl;
			return false;
		}
	}
	return true;
}

bool get_tgdb_genres_map(
	const std::string &api_key,
	const std::string &local_path,
	std::map< int, std::string > &genres,
	int &remaining_allowance )
{
	std::vector< std::pair < std::string, int > > temp;

	std::string local_file = local_path + GENRES_E + FE_IDDB_EXT;


	if ( !load_from_local( local_file, temp ) )
	{
		rapidjson::Document d;
		if ( get_json_doc( api_key, d, GENRES_REQ, remaining_allowance ) )
		{
			const rapidjson::Value &e = d[DATA_E];
			if ( !e.HasMember( GENRES_E ) || !e[GENRES_E].IsObject() )
			{
				FeLog() << "Error, unable to get genres element" << std::endl;
				return false;
			}

			// get the "data" -> "genres" node
			const rapidjson::Value &p = d[DATA_E][GENRES_E];

			rapidjson::Value::ConstMemberIterator itr;
			for ( itr=p.MemberBegin(); itr != p.MemberEnd(); ++itr )
			{
				temp.push_back( std::pair< std::string, int >(
					itr->value[ NAME_E ].GetString(),
					itr->value[ ID_E ].GetInt() ) );
			}
		}

		write_local_if_needed( local_file, temp );
	}

	std::vector< std::pair < std::string, int > >::iterator itr;

	for ( itr = temp.begin(); itr != temp.end(); ++itr )
		genres[ (*itr).second ] = (*itr).first;

	return true;
}

bool get_tgdb_publishers_map(
	const std::string &api_key,
	const std::string &local_path,
	std::map< int, std::string > &pubs,
	int &remaining_allowance )
{
	std::vector< std::pair < std::string, int > > temp;
	std::string local_file = local_path + PUBLISHERS_E + FE_IDDB_EXT;

	if ( !load_from_local( local_file, temp ) )
	{
		rapidjson::Document d;
		if ( get_json_doc( api_key, d, PUBLISHERS_REQ, remaining_allowance ) )
		{
			const rapidjson::Value &e = d[DATA_E];
			if ( !e.HasMember( PUBLISHERS_E ) || !e[PUBLISHERS_E].IsObject() )
			{
				FeLog() << "Error, unable to get publishers element" << std::endl;
				return false;
			}

			// get the "data" -> "publishers" node
			const rapidjson::Value &p = d[DATA_E][PUBLISHERS_E];

			rapidjson::Value::ConstMemberIterator itr;
			for ( itr=p.MemberBegin(); itr != p.MemberEnd(); ++itr )
			{
				temp.push_back( std::pair< std::string, int >(
					itr->value[ NAME_E ].GetString(),
					itr->value[ ID_E ].GetInt() ) );
			}
		}

		write_local_if_needed( local_file, temp );
	}

	std::vector< std::pair < std::string, int > >::iterator itr;

	for ( itr = temp.begin(); itr != temp.end(); ++itr )
		pubs[ (*itr).second ] = (*itr).first;

	return true;
}

void create_ft(
		FeNetQueue &q,
		const rapidjson::Value &node,
		const char *local_label,
		const char *url_label,
		const std::string &base_url,
		const std::string &base_path,
		const std::string &name,
		int &q_total )
{
	std::vector < std::string > fnames;

	// Special case: we only want the front facing boxart images
	bool front_only = ( strcmp( url_label, "boxart" ) == 0 );

	for ( rapidjson::SizeType i=0; i< node.Size(); i++ )
	{
		if ( node[i].HasMember( TYPE_E ) && node[i][TYPE_E].IsString()
			&& node[i].HasMember( FILENAME_E ) && node[i][FILENAME_E].IsString() )
		{
			std::string t = node[i][TYPE_E].GetString();

			if ( t.compare( url_label ) == 0 )
			{
				// Special case: we only want the front facing boxart images
				if ( front_only && node[i].HasMember( "side" )
						&& node[i]["side"].IsString()
						&& ( strcmp( node[i]["side"].GetString(), "front" ) != 0 ) )
					continue;

				fnames.push_back( node[i][FILENAME_E].GetString() );
			}
		}
	}

	if ( fnames.empty() )
	{
		FeDebug() << "No " << local_label << " images found for: " << name << std::endl;
		return;
	}

	std::string local_sub = local_label;
	local_sub += "/";

	confirm_directory( base_path, local_sub );

	std::string local_path = base_path + local_sub;

	if ( fnames.size() > 1 )
	{
		std::string temp = name + "/";
		confirm_directory( local_path, temp );
		local_path += temp;

		for ( size_t i=0; i<fnames.size(); i++ )
		{
			std::string stub = fnames[i];
			size_t pos = fnames[i].find_last_of("\\/");
			if ( pos != std::string::npos )
				stub = fnames[i].substr( pos+1 );

			std::string fn = local_path + stub;

			if ( !file_exists( fn ) )
			{
				FeDebug() << "Adding task to get file: " << fn
					<< " (" << base_url << fnames[i] << ")" << std::endl;

				q.add_file_task( base_url + fnames[i], local_path + stub );
				q_total++;
			}
		}
	}
	else
	{
		std::string ext=".jpg";

		size_t pos = fnames[0].find_last_of(".");
		if ( pos != std::string::npos )
			ext = fnames[0].substr( pos );

		std::string fn = local_path + name + ext;

		if ( !file_exists( fn ) )
		{
			FeDebug() << "Adding task to get file: " << fn
				<< " (" << base_url << fnames[0] << ")" << std::endl;
			q.add_file_task( base_url + fnames[0], local_path + name + ext );
			q_total++;
		}
	}
}

#define INFO_QUERY 16
#define IMAGE_QUERY 420

void create_single_id_query(
		std::vector < std::pair< int, FeRomInfo * > > &id_worklist,
		std::map< int, FeRomInfo * > &id_map,
		bool scrape_art,
		const std::string &api_key,
		FeNetQueue &q,
		int &q_total )
{
	const int QCAP = 20; // thegamesdb seems to cap what it will handle in one query at 20
	int count=0;

	std::string id_str;

	while ( !id_worklist.empty() && count < QCAP )
	{
		std::pair< int, FeRomInfo * > temp = id_worklist.back();

		id_map[ temp.first ] = temp.second;

		if ( id_str.size() > 0 )
			id_str += "%2C";

		id_str += as_str( temp.first );

		id_worklist.pop_back();
		count++;
	}

	std::string my_req = HOSTNAME;
	int id=0;

	if ( !scrape_art )
	{
		my_req += "/Games/ByGameID?apikey=$1&id=$2&fields=players%2Cpublishers%2Cgenres%2Coverview%2Crating";
		id = INFO_QUERY;
	}
	else
	{
		my_req += "/Games/Images?apikey=$1&games_id=$2";
		id = IMAGE_QUERY;
	}

	perform_substitution( my_req, "$1", api_key );
	perform_substitution( my_req, "$2", id_str );

	FeDebug() << " - db query: " << my_req << std::endl;
	q.add_buffer_task( my_req, id );
	q_total++;
}

bool FeSettings::thegamesdb_scraper( FeImporterContext &c )
{
	FeLog() << " - scraping thegamesdb.net..." << std::endl;

	int remaining_allowance = -1;

	std::string path = get_config_dir();
	confirm_directory( path, FE_SCRAPER_SUBDIR );
	path += FE_SCRAPER_SUBDIR;
	path += "thegamesdb.net/";
	confirm_directory( path, "" );

	// Use public API key assigned to the Attract-Mode project if no specific key configured
	//
	std::string api_key = get_info( FeSettings::ThegamesdbKey );
	if ( !api_key.empty() )
		FeLog() << " - Using custom API key: " << api_key << std::endl;
	else
		api_key = "035b376cf5769eca89325626f698702fb72bff15ef5d0451e1859bb7b6b854be";

	std::vector< std::pair < std::string, int > > plats;
	std::vector< std::pair < std::string, int > >::iterator itr;

	std::vector< FeIDDb > plats_db;
	std::vector< FeIDDb >::iterator pd_itr;

	// Get platform list
	if ( get_tgdb_matching_platform_list( *this, c, api_key, path, plats, remaining_allowance ) )
	{
		for ( itr = plats.begin(); itr != plats.end(); ++itr )
		{
			FeLog() << " - Platform :" << (*itr).first << " (" << (*itr).second << ")" << std::endl;

			plats_db.push_back( FeIDDb() );
			plats_db.back().load( (*itr).first, path, (*itr).second );
		}
	}

	//
	// Sort roms into separate lists based on whether we already have a thegamesdb.net game ID
	// for the game in question
	//
	std::vector<std::pair < int, FeRomInfo * > > id_worklist;	// games where we have an ID
	std::vector<FeRomInfo *> name_worklist;	// games where we don't

	ParentMapType parent_map;
	build_parent_map( parent_map, c.romlist, c.emulator.is_mess() );

	std::string emu_name = c.emulator.get_info( FeEmulatorInfo::Name );

	for ( FeRomInfoListType::iterator itl = c.romlist.begin(); itl != c.romlist.end(); ++itl )
	{
		(*itl).set_info( FeRomInfo::Emulator, emu_name );

		if ( c.scrape_art )
		{
			// Don't scrape art for a clone if its parent has the same name
			if ( has_same_name_as_parent( *itl, parent_map ) )
				continue;

			// Don't query if we already have the art already
			if ( ( !m_scrape_snaps || has_artwork( *itl, "snap" ) )
					&& ( !m_scrape_marquees || has_artwork( *itl, "marquee" ) )
					&& ( !m_scrape_flyers || has_artwork( *itl, "flyer" ) )
					&& ( !m_scrape_wheels || has_artwork( *itl, "wheel" ) )
					&& ( !m_scrape_fanart || has_artwork( *itl, "fanart" ) ) )
				continue;
		}

		int id=-1;
		for ( pd_itr = plats_db.begin(); (( id < 0 ) && ( pd_itr != plats_db.end() )); ++pd_itr )
			id = (*pd_itr).get_id_from_name( name_with_brackets_stripped( (*itl).get_info( FeRomInfo::Title ) ));

		if ( id < 0 )
			name_worklist.push_back( &(*itl) );
		else
			id_worklist.push_back( std::pair<int, FeRomInfo *>( id, &(*itl) ) );
	}

	FeNetQueue q;
	std::map< int, FeRomInfo * > my_id_map;
	std::map< std::string, FeRomInfo * > my_fuzz_map;

	std::map< int, std::string > genres;
	std::map< int, std::string > publishers;

	int q_total=1; // start at 1 to avoid divide by zero scenario
	int q_count=0;

	if ( c.scrape_art && !plats.empty() )
	{

		// Get emulator specific images
		std::string my_req = HOSTNAME;
		my_req += "/Platforms/Images?apikey=$1&platforms_id=$2";
		perform_substitution( my_req, "$1", api_key );
		perform_substitution( my_req, "$2", as_str( plats[0].second ) );

		FeNetTask my_task( my_req, 0 );
		if ( !my_task.do_task() )
		{
			FeLog() << " * Unable to get platform information" << std::endl;
			goto fail_emu_scrape;
		}
		FeDebug() << " - db query: " << my_req << std::endl;

		int ignored;
		std::string body;
		my_task.grab_result( ignored, body );

		rapidjson::Document doc;
		if ( !parse_confirm_data( body, doc, remaining_allowance ) )
			goto fail_emu_scrape;

		const rapidjson::Value &d = doc[DATA_E];
		if ( !d.HasMember( BASEURL_E ) || !d[BASEURL_E].IsObject() )
		{
			FeLog() << "Error, unable to get base_url element: " << body << std::endl;
			goto fail_emu_scrape;
		}

		const rapidjson::Value &e = d[BASEURL_E];
		if ( !e.HasMember( ORIGINAL_E ) || !e[ORIGINAL_E].IsString() )
		{
			FeLog() << "Error, unable to get original element: " << body << std::endl;
			goto fail_emu_scrape;
		}

		std::string base_url = e[ORIGINAL_E].GetString();

		std::string base_path = get_config_dir() + FE_SCRAPER_SUBDIR;
		base_path += emu_name + "/";

		if ( !d.HasMember( IMAGES_E ) || !d[IMAGES_E].IsObject() )
		{
			FeLog() << "Error, unable to get images element: " << body << std::endl;
			goto fail_emu_scrape;
		}

		const rapidjson::Value &imgs = d[IMAGES_E];

		rapidjson::Value::ConstMemberIterator itm = imgs.MemberBegin();

		if ( m_scrape_snaps )
			create_ft( q, itm->value, "snap", "screenshot", base_url, base_path, emu_name, q_total );

		if ( m_scrape_marquees )
			create_ft( q, itm->value, "marquee", "banner", base_url, base_path, emu_name, q_total );

		if ( m_scrape_flyers )
			create_ft( q, itm->value, "flyer", "boxart", base_url, base_path, emu_name, q_total );

		if ( m_scrape_wheels )
			create_ft( q, itm->value, "wheel", "clearlogo", base_url, base_path, emu_name, q_total );

		if ( m_scrape_fanart )
			create_ft( q, itm->value, "fanart", "fanart", base_url, base_path, emu_name, q_total );
	}
fail_emu_scrape:

	//
	// Build requests for games with IDs.
	//
	while ( !id_worklist.empty() )
		create_single_id_query( id_worklist, my_id_map, c.scrape_art, api_key, q, q_total );

	//
	// Build requests for games without IDs (skip if we are scraping art, because we need
	// the IDs in order to get artworks
	//
	std::string plat_id_str;
	for ( pd_itr = plats_db.begin(); pd_itr != plats_db.end(); ++pd_itr )
	{
		if ( !plat_id_str.empty() )
			plat_id_str += "%2C";

		plat_id_str += as_str( (*pd_itr).get_plat_id() );
	}

	while ( !name_worklist.empty() )
	{
		const std::string &temp =  name_worklist.back()->get_info( FeRomInfo::Title );

		std::string my_req = HOSTNAME;
		my_req += "/Games/ByGameName?apikey=$1&name=$2&fields=players%2Cpublishers%2Cgenres%2Coverview&filter%5Bplatform%5D=$3";
		perform_substitution( my_req, "$1", api_key );
		perform_substitution( my_req, "$2", url_escape( name_with_brackets_stripped( temp ) ) );
		perform_substitution( my_req, "$3", plat_id_str );

		my_fuzz_map[ get_fuzzy( temp ) ] = name_worklist.back();
		name_worklist.pop_back();

		FeDebug() << " - db query: " << my_req << std::endl;
		q.add_buffer_task( my_req, INFO_QUERY );
		q_total++;
	}

	if ( !c.scrape_art )
	{
		get_tgdb_genres_map( api_key, path, genres, remaining_allowance );
		get_tgdb_publishers_map( api_key, path, publishers, remaining_allowance );
	}

	//
	// Create work threads to process the queue, adding new tasks to download
	//
	FeNetWorker worker1( q ), worker2( q ), worker3( q ), worker4( q );
	std::string aux;

	bool my_all_done = false;

	while ( !my_all_done )
	{
		if ( q.all_done() )
		{
			while ( !id_worklist.empty() )
				create_single_id_query( id_worklist, my_id_map, c.scrape_art, api_key, q, q_total );

			my_all_done = true;
			continue;
		}

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

					// find the emulator name in the result filename
					// and display the string afterwards to the user
					size_t pos = result.find( emu_name );
					if ( pos != std::string::npos )
					{
						pos += emu_name.size();
						aux = result.substr( pos );
					}
				}

				if ( id == FeNetTask::FileTask ) // we don't increment if id = special file task
					q_count++;

				if ( c.uiupdate )
				{
					int p= q_count * 100 / q_total;
					if ( c.uiupdate( c.uiupdatedata, p, aux ) == false )
						return false;
				}

				continue;
			}

			q_count++;

			rapidjson::Document doc;
			if ( !parse_confirm_data( result, doc, remaining_allowance ) )
				continue;

			const rapidjson::Value &d = doc[DATA_E];

			if ( id == IMAGE_QUERY )
			{
				if ( !d.HasMember( BASEURL_E ) || !d[BASEURL_E].IsObject() )
				{
					FeLog() << "Error, unable to get base_url element: " << result << std::endl;
					continue;
				}

				const rapidjson::Value &e = d[BASEURL_E];
				if ( !e.HasMember( ORIGINAL_E ) || !e[ORIGINAL_E].IsString() )
				{
					FeLog() << "Error, unable to get original element: " << result << std::endl;
					continue;
				}

				std::string base_url = e[ORIGINAL_E].GetString();

				std::string base_path = get_config_dir() + FE_SCRAPER_SUBDIR;
				base_path += emu_name + "/";

				if ( !d.HasMember( IMAGES_E ) || !d[IMAGES_E].IsObject() )
				{
					FeLog() << "Error, unable to get images element: " << result << std::endl;
					continue;
				}

				const rapidjson::Value &imgs = d[IMAGES_E];

				for ( rapidjson::Value::ConstMemberIterator itm = imgs.MemberBegin(); itm != imgs.MemberEnd(); ++itm )
				{
					int id = as_int( itm->name.GetString() );

					std::map<int, FeRomInfo *>::iterator it = my_id_map.find( id );
					if ( it != my_id_map.end() )
					{
						std::string name = (*it).second->get_info( FeRomInfo::Romname );

						if ( m_scrape_snaps && !has_artwork( *(*it).second, "snap" ) )
							create_ft( q, itm->value, "snap", "screenshot", base_url, base_path, name, q_total );

						if ( m_scrape_marquees && !has_artwork( *(*it).second, "marquee" ) )
							create_ft( q, itm->value, "marquee", "banner", base_url, base_path, name, q_total );

						if ( m_scrape_flyers && !has_artwork( *(*it).second, "flyer" ) )
							create_ft( q, itm->value, "flyer", "boxart", base_url, base_path, name, q_total );

						if ( m_scrape_wheels && !has_artwork( *(*it).second, "wheel" ) )
							create_ft( q, itm->value, "wheel", "clearlogo", base_url, base_path, name, q_total );

						if ( m_scrape_fanart )
							create_ft( q, itm->value, "fanart", "fanart", base_url, base_path, name, q_total );
					}
				}

				continue;
			}

			if ( !d.HasMember( GAMES_E ) || !d[GAMES_E].IsArray() )
			{
				FeLog() << "Error, unable to get games element: " << result << std::endl;
				continue;
			}

			const rapidjson::Value &g = d[GAMES_E];

			for ( rapidjson::SizeType i=0; i < g.Size(); i++ )
			{
				const rapidjson::Value &e = g[i];
				if ( !e.HasMember( ID_E ) || !e[ID_E].IsInt() )
				{
					FeLog() << "Error, unable to access id element" << std::endl;
					continue;
				}

				if ( !e.HasMember( GAME_TITLE_E ) || !e[GAME_TITLE_E].IsString() )
				{
					FeLog() << "Error, unable to access game title" << std::endl;
					continue;
				}

				std::string gamedb_title = e[GAME_TITLE_E].GetString();
				FeRomInfo *ptr=NULL;

				std::map<int, FeRomInfo *>::iterator it = my_id_map.find( e[ID_E].GetInt() );
				if ( it == my_id_map.end() )
				{
					std::map<std::string, FeRomInfo *>::iterator it2 = my_fuzz_map.find( get_fuzzy( gamedb_title ) );
					if ( it2 == my_fuzz_map.end() )
					{
						FeDebug() << "Skipping entry from thegamesdb.net, name=" << gamedb_title << std::endl;
						continue;
					}

					// Update our gamesdb ID database with the ID for this game (which we don't have currently)
					//
					if ( !e.HasMember( PLATFORM_E ) || !e[PLATFORM_E].IsInt() )
					{
						FeLog() << "Error, unable to get platform ID" << std::endl;
					}
					else
					{
						int plat_id = e[PLATFORM_E].GetInt();

						for ( pd_itr = plats_db.begin(); pd_itr != plats_db.end(); ++pd_itr )
						{
							if ( (*pd_itr).get_plat_id() == plat_id )
							{
								(*pd_itr).set_name( gamedb_title, e[ID_E].GetInt() );

								// store the "local" title in our DB as well if it isn't an exact match
								// to thegamesdb's title
								//
								std::string local_title = name_with_brackets_stripped(
									(*it2).second->get_info( FeRomInfo::Title ) );

								if ( gamedb_title.compare( local_title ) != 0 )
									(*pd_itr).set_name( local_title, e[ID_E].GetInt() );

								break;
							}
						}

					}

					ptr = (*it2).second;
					my_fuzz_map.erase( it2 );
				}
				else
					ptr = (*it).second;

				FeLog() << " + Matched game: " << ptr->get_info( FeRomInfo::Title )
					<< " => " << gamedb_title << std::endl;

				if ( c.scrape_art )
				{
					// Found a gamesdb.net ID for this game that we now have to do an images query
					// for
					id_worklist.push_back( std::pair<int, FeRomInfo *>( e[ID_E].GetInt(), ptr ) );
				}
				else
				{
					if ( ptr->get_info( FeRomInfo::AltTitle ).empty() )
						ptr->set_info( FeRomInfo::AltTitle, gamedb_title );

					if ( e.HasMember( RELEASE_E ) && e[RELEASE_E].IsString() )
					{
						std::string work = e[RELEASE_E].GetString();
						size_t cut = work.find_last_of( "/" );

						std::string year;
						if ( cut != std::string::npos )
							year = work.substr( cut+1 );
						else
							year = work;

						ptr->set_info( FeRomInfo::Year, year );
					}

					if ( e.HasMember( PLAYERS_E ) && e[PLAYERS_E].IsInt() )
						ptr->set_info( FeRomInfo::Players, as_str( e[PLAYERS_E].GetInt() ) );

					if ( e.HasMember( GENRES_E ) && e[GENRES_E].IsArray() )
					{
						std::string genre_str;
						for ( rapidjson::SizeType j=0; j < e[GENRES_E].Size(); j++ )
						{
							if ( !e[GENRES_E][j].IsInt() )
								FeLog() << "Error, unable to get genre id" << std::endl;
							else
							{
								std::map< int, std::string >::iterator itg;
								itg = genres.find( e[GENRES_E][j].GetInt() );
								if ( itg != genres.end() )
								{
									if (!genre_str.empty() )
										genre_str += ", ";

									genre_str += (*itg).second;
								}
							}
						}
						ptr->set_info( FeRomInfo::Category, genre_str );
					}

					if ( e.HasMember( PUBLISHERS_E ) && e[PUBLISHERS_E].IsArray() )
					{
						std::string manu_str;
						for ( rapidjson::SizeType j=0; j < e[PUBLISHERS_E].Size(); j++ )
						{
							if ( !e[PUBLISHERS_E][j].IsInt() )
								FeLog() << "Error, unable to get publisher id" << std::endl;
							else
							{
								std::map< int, std::string >::iterator itp;
								itp = publishers.find( e[PUBLISHERS_E][j].GetInt() );
								if ( itp != publishers.end() )
								{
									if (!manu_str.empty() )
										manu_str += ", ";

									manu_str += (*itp).second;
								}
							}
						}
						ptr->set_info( FeRomInfo::Manufacturer, manu_str );
					}

					if ( e.HasMember( RATING_E ) && e[RATING_E].IsString() )
					{
						std::string rating = e[RATING_E].GetString();
						ptr->set_info( FeRomInfo::Rating, rating );
					}


					if ( g[i].HasMember( OVERVIEW_E ) && g[i][OVERVIEW_E].IsString() )
					{
						set_game_overview( emu_name,
							ptr->get_info( FeRomInfo::Romname ),
							g[i][OVERVIEW_E].GetString(),
							false );
					}
				}
			}

			if ( c.uiupdate )
			{
				int p= q_count * 100 / q_total;
				if ( c.uiupdate( c.uiupdatedata, p, aux ) == false )
					return false;
			}

		}
		else
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
	}

	if ( !my_fuzz_map.empty() )
	{
		FeLog() << " - " << my_fuzz_map.size() << " games not matched on thegamesdb.net: ";

		bool dc = false;
		std::map<std::string, FeRomInfo *>::iterator it2;
		for ( it2 = my_fuzz_map.begin(); it2 != my_fuzz_map.end(); ++it2 )
		{
			FeLog() << ( dc?",":"" ) << "\"" << (*it2).second->get_info( FeRomInfo::Romname ) << "\"";
			dc=true;
		}

		FeLog() << std::endl;
	}

	FeLog() << " - thegamesdb.net reports a remaining allowance of: "
		<<(( remaining_allowance < 0 ) ? "Unknown" : as_str( remaining_allowance ) )
		<< std::endl;

	return true;
}

#else

bool FeSettings::thegamesdb_scraper( FeImporterContext &c )
{
	c.user_message = "Unable to scrape, frontend was built without libcurl enabled!";
	FeLog() << c.user_message << std::endl;
	return true;
}

#endif
