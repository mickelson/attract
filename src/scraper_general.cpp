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

extern const char *FE_ROMLIST_SUBDIR;

namespace {

void build_basic_romlist( FeImporterContext &c )
{
	std::vector<std::string> names;
	std::vector<std::string> paths;
	c.emulator.gather_rom_names( names, paths );
	ASSERT( names.size() == paths.size() );

	for ( unsigned int i=0; i<names.size(); i++ )
	{
		FeRomInfo new_rom;
		new_rom.set_info( FeRomInfo::Romname, names[i] );
		new_rom.set_info( FeRomInfo::Title, names[i] );
		new_rom.set_info( FeRomInfo::BuildFullPath, paths[i] );

		c.romlist.push_back( new_rom );
	}

	std::cout << " - Found " << names.size() << " files." << std::endl;
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

void apply_import_extras( FeImporterContext &c, bool skip_xml )
{
	const std::vector< std::string > &extras = c.emulator.get_import_extras();

	for ( std::vector< std::string >::const_iterator itr = extras.begin();
			itr != extras.end(); ++itr )
	{
		std::string path = c.emulator.clean_path_with_wd( *itr );

		if ( tail_compare( path, "catver.ini" ) )
			ini_import( path, c.romlist, FeRomInfo::Category, "[Category]" );
		else if ( tail_compare( path, "nplayers.ini" ) )
			ini_import( path, c.romlist, FeRomInfo::Players, "[NPlayers]" );
		else if ( tail_compare( path, ".xml" ) )
		{
			if ( skip_xml )
			{
				std::cout << " - Skipping import_extras file: "
					<< path << std::endl;
			}
			else
			{
				FeListXMLParser mamep( c );
				mamep.parse_file( path );
			}
		}
		else
			std::cout << " * Unsupported import_extras file: " << path << std::endl;
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
		std::cerr << " ! Error opening file: " << input_filename << std::endl;
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
		std::cout << " * Warning: Unexpected end of file encountered: " << input_filename
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

bool scummvm_cb( const char *buff, void *opaque )
{
	std::string *outp = (std::string *)opaque;
	*outp += buff;
	return true;
}

void scummvm_target( const std::string &base_command,
	const std::string &params,
	const std::string &work_dir,
	std::map< std::string, std::string, myclasscmp > &my_map )
{
	std::string output;
	run_program( base_command, params, work_dir, scummvm_cb, &output );

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

	std::string work_dir = clean_path( c.emulator.get_info(
				FeEmulatorInfo::Working_dir ), true );

	std::map< std::string, std::string, myclasscmp > my_map;

	scummvm_target( base_command, "-z", work_dir, my_map );
	scummvm_target( base_command, "-t", work_dir, my_map );

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

void FeSettings::apply_xml_import( FeImporterContext &c, bool include_gdb )
{
	std::string base_command = clean_path( c.emulator.get_info(
				FeEmulatorInfo::Executable ) );

	std::string work_dir = clean_path( c.emulator.get_info(
				FeEmulatorInfo::Working_dir ), true );

	FeEmulatorInfo::InfoSource is = c.emulator.get_info_source();
	switch ( is )
	{

	case FeEmulatorInfo::Listxml:
	{
		std::cout << " - Obtaining -listxml info...";
		FeListXMLParser mamep( c );
		if ( !mamep.parse_command( base_command, work_dir ) )
			std::cerr << " ! No XML output found, command: "
				<< base_command << " -listxml" << std::endl;
	}
	break;

	case FeEmulatorInfo::Listsoftware:
	case FeEmulatorInfo::Listsoftware_tgdb:
	{
		const std::vector < std::string > &system_names = c.emulator.get_systems();
		if ( system_names.empty() )
		{
			std::cout << " * Note: No system configured for emulator: "
				<< c.emulator.get_info( FeEmulatorInfo::Name )
				<< ", unable to obtain -listsoftware info."
				<< std::endl;
			return;
		}

		FeListSoftwareParser lsp( c );
		lsp.parse( base_command, work_dir, system_names );

		if ( include_gdb && ( is == FeEmulatorInfo::Listsoftware_tgdb ) )
			thegamesdb_scraper( c );
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
				std::cerr << " ! Error opening file: " << fname << std::endl;
		}

		if ( include_gdb )
			thegamesdb_scraper( c );
	}
	break;

	case FeEmulatorInfo::Thegamesdb:
		if ( include_gdb )
			thegamesdb_scraper( c );
		break;

	case FeEmulatorInfo::Scummvm:
		scummvm_lookup( c );
		if ( include_gdb )
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
			std::cout << "*** Generating Collection/Rom List: "
				<< (*itr).emulator_name << std::endl;

			FeEmulatorInfo *emu = m_rl.get_emulator( (*itr).emulator_name );
			if ( emu == NULL )
			{
				std::cerr << " ! Error: Invalid --build-rom-list target: "
					<<  (*itr).emulator_name << std::endl;
			}
			else
			{
				FeRomInfoListType romlist;

				best_name = emu->get_info( FeEmulatorInfo::Name );

				FeImporterContext ctx( *emu, romlist );
				ctx.full = full;
				ctx.out_name = output_name;

				build_basic_romlist( ctx );

				apply_xml_import( ctx, true );
				apply_import_extras( ctx, emu->is_mame() );

				apply_emulator_name( best_name, romlist );
				total_romlist.splice( total_romlist.end(), romlist );
			}
		}
		else if ( (*itr).task_type == FeImportTask::ImportRomlist )
		{
			// import romlist from file task
			std::cout << "*** Importing Collection/Rom List: "
				<< (*itr).file_name << std::endl;

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
				FeEmulatorInfo temp_emu( emu_name );
				FeImporterContext ctx( temp_emu, romlist );
				ctx.full = true; // Flag that all xml entries go into romlist

				FeListXMLParser mamep( ctx );
				if ( mamep.parse_file( (*itr).file_name ) )
					apply_emulator_name( emu_name, romlist );
			}
			else
			{
				std::cerr << " ! Error: Unsupported --import-rom-list file: "
					<<  (*itr).file_name << std::endl;
			}

			std::cout << "[Import " << (*itr).file_name << "] - Imported "
				<< romlist.size() << " entries." << std::endl;

			FeEmulatorInfo *emu = m_rl.get_emulator( emu_name );
			if ( emu == NULL )
			{
				std::cout << " * Warning: The emulator specified with --import-rom-list was not found: "
					<<  emu_name << std::endl;
			}
			else
			{
				FeImporterContext ctx( *emu, romlist );
				apply_import_extras( ctx, true );
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
				apply_xml_import( ctx, false );
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

bool FeSettings::build_romlist( const std::vector<std::string> &emu_list, const std::string &out_name,
	UiUpdate uiu, void *uid, std::string &msg )
{
	//
	// Put up the "building romlist" message at 0 percent while we get going...
	//
	if ( uiu )
		uiu( uid, 0, "" );

	FeRomInfoListType total_romlist;
	bool retval = false;
	std::string user_message;

	for ( std::vector<std::string>::const_iterator itr = emu_list.begin(); itr != emu_list.end(); ++itr )
	{
		FeEmulatorInfo *emu = m_rl.get_emulator( *itr );
		if ( emu == NULL )
			continue;

		retval = true;

		std::cout << "*** Generating Collection/Rom List: "
			<< emu->get_info( FeEmulatorInfo::Name ) << std::endl;

		FeRomInfoListType romlist;

		FeImporterContext ctx( *emu, romlist );
		ctx.uiupdate = uiu;
		ctx.uiupdatedata = uid;
		ctx.out_name = out_name;

		build_basic_romlist( ctx );
		apply_xml_import( ctx, true );
		apply_import_extras( ctx, emu->is_mame() );
		apply_emulator_name( *itr, romlist );

		total_romlist.splice( total_romlist.end(), romlist );

		if ( !ctx.user_message.empty() )
			user_message = ctx.user_message;
	}

	if ( !retval )
		return false;

	total_romlist.sort( FeRomListSorter() );

	// strip duplicate entries
	std::cout << " - Removing any duplicate entries..." << std::endl;
	total_romlist.unique();

	std::string filename = get_config_dir();
	confirm_directory( filename, FE_ROMLIST_SUBDIR );

	if ( uiu )
		uiu( uid, 100, "" );

	filename += FE_ROMLIST_SUBDIR;
	filename += out_name;
	filename += FE_ROMLIST_FILE_EXTENSION;

	write_romlist( filename, total_romlist );

	if ( user_message.empty() )
		get_resource( "Wrote $1 entries to Collection/Rom List",
			as_str( (int)total_romlist.size() ), msg );
	else
		msg = user_message;

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

		apply_xml_import( ctx, false );
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
