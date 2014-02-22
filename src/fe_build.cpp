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
#include <iostream>
#include <fstream>
#include <list>
#include <map>

extern const char *FE_ROMLIST_SUBDIR;

namespace {

void build_basic_romlist( const FeEmulatorInfo &emulator,
				std::list<FeRomInfo> &romlist )
{
	std::string rom_path = clean_path( emulator.get_info(
					FeEmulatorInfo::Rom_path ) );

	const std::vector<std::string> &extensions = emulator.get_extensions();

	std::vector<std::string> base_list;
	get_basename_from_extension( base_list, rom_path, extensions );

	for ( std::vector<std::string>::iterator its=base_list.begin();
				its != base_list.end(); ++its )
	{
		FeRomInfo new_rom;
		new_rom.set_info( FeRomInfo::Romname, (*its) );
		new_rom.set_info( FeRomInfo::Title, (*its) );
		new_rom.set_info( FeRomInfo::Emulator, emulator.get_info(
															FeEmulatorInfo::Name ));

		romlist.push_back( new_rom );
	}

	std::cout << "Found " << romlist.size()
		<< " unique files with rom extension(s):";

	for ( std::vector<std::string>::const_iterator itr=extensions.begin();
			itr != extensions.end(); ++itr )
		std::cout << " " << (*itr);

	std::cout << ".  Directory: " << rom_path << std::endl;
}

void apply_listxml( const FeEmulatorInfo &emulator,
				std::list<FeRomInfo> &romlist,
				FeXMLParser::UiUpdate uiupdate=NULL, void *uiupdatedata=NULL )
{
	std::string listxml = emulator.get_info(
				FeEmulatorInfo::Listxml );

	if ( listxml.empty() )
		return;

	std::string base_command = clean_path( emulator.get_info(
				FeEmulatorInfo::Executable ) );

	if ( listxml.compare( 0, 4, "mame" ) == 0 )
	{
		std::cout << "Obtaining -listxml info...";
		FeMameXMLParser mamep( romlist, uiupdate, uiupdatedata );
		mamep.parse( base_command );
		std::cout << std::endl;
	}
	else if ( listxml.compare( 0, 4, "mess" ) == 0 )
	{
		std::string args;
		size_t pos=4;
		token_helper( listxml, pos, args );

		if ( args.empty() )
		{
			std::cout << "Note: No system provided in \""
				<< "listxml mess\" entry for emulator: "
				<< emulator.get_info( FeEmulatorInfo::Name )
				<< ", unable to obtain -listsoftware info."
				<< std::endl;
			return;
		}

		args += " -listsoftware";

		std::cout << "Obtaining -listsoftware info...";
		FeMessXMLParser messp( romlist, uiupdate, uiupdatedata );
		messp.parse( base_command, args );
		std::cout << std::endl;
	}
	else
	{
		std::cout << "Unrecognized listxml setting: " << listxml
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
		while ( FeRomInfo::indexStrings[i] != NULL )
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

void ini_import( const std::string &filename,
				std::list<FeRomInfo> &romlist,
				FeRomInfo::Index index,
				const std::string &init_tag )
{
	std::map <std::string, std::string> my_map;

	// create entries in the map for each name we want to find
	for ( std::list<FeRomInfo>::iterator itr=romlist.begin();
			itr!=romlist.end(); ++itr )
	{
		my_map[ (*itr).get_info( FeRomInfo::Romname ) ] = "";

		std::string key = (*itr).get_info( FeRomInfo::Cloneof );
		if ( !key.empty() )
			my_map[ key ] = "";
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

			std::map<std::string, std::string>::iterator itr;
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
			val = my_map[ (*itr).get_info( FeRomInfo::Cloneof ) ];

		if ( !val.empty() )
		{
			count++;
			(*itr).set_info( index, val );
		}
	}

	std::cout << "Found info for " << count << " entries.  File: "
					<< filename << std::endl;
}

void apply_import_extras( const FeEmulatorInfo &emulator,
				std::list<FeRomInfo> &romlist )
{
	std::string iextras = emulator.get_info( FeEmulatorInfo::Import_extras );

	if ( iextras.empty() )
		return;

	size_t pos=0;
	do
	{
		std::string val, path;
		token_helper( iextras, pos, val );
		path = clean_path( val );

		if ( tail_compare( path, "catver.ini" ) )
		{
			ini_import( path, romlist, FeRomInfo::Category, "[Category]" );
		}
		else if ( tail_compare( val, "nplayers.ini" ) )
		{
			ini_import( path, romlist, FeRomInfo::Players, "[NPlayers]" );
		}
		else
		{
			std::cout << "Unsupported import_extras file: " << path << std::endl;
		}

	} while ( pos < iextras.size() );
}

}; // end namespace

bool FeSettings::build_romlist( const std::vector <std::string> &emu_names )
{
	std::list<FeRomInfo> total_romlist;
   std::string name, list_name, path;

	for ( std::vector<std::string>::const_iterator itr=emu_names.begin();
			itr < emu_names.end(); ++itr )
	{
		FeEmulatorInfo *emu = get_emulator( *itr );
		if ( emu == NULL )
		{
			std::cout << "Error: Invalid -build-rom-list target: " <<  (*itr)
				<< std::endl;
		}
		else
		{
			std::list<FeRomInfo> romlist;
			name = emu->get_info( FeEmulatorInfo::Name );

			build_basic_romlist( *emu, romlist );

			apply_listxml( *emu, romlist );
			apply_import_extras( *emu, romlist );

			total_romlist.splice( total_romlist.end(), romlist );
		}
	}

	std::string rex_str;
	get_resource( "_sort_regexp", rex_str );
	FeRomListCompare::init_rex( rex_str );
	total_romlist.sort( FeRomListCompare::cmp );
	FeRomListCompare::close_rex();

	if ( emu_names.size() > 1 )
		name = "multi";

	path = get_config_dir();
	confirm_directory( path, FE_ROMLIST_SUBDIR );

	path += FE_ROMLIST_SUBDIR;
	get_available_filename( path, name, FE_ROMLIST_FILE_EXTENSION, list_name );
	write_romlist( list_name, total_romlist );

	return true;
}

bool FeSettings::build_romlist( const std::string &emu_name, UiUpdate uiu, void *uid, int &size )
{
	FeEmulatorInfo *emu = get_emulator( emu_name );
	if ( emu == NULL )
		return false;

	std::list<FeRomInfo> romlist;
	build_basic_romlist( *emu, romlist );

	apply_listxml( *emu, romlist, uiu, uid );
	apply_import_extras( *emu, romlist );

	std::string rex_str;
	get_resource( "_sort_regexp", rex_str );
	FeRomListCompare::init_rex( rex_str );
	romlist.sort( FeRomListCompare::cmp );
	FeRomListCompare::close_rex();

	size = romlist.size();

	std::string filename = get_config_dir();
	confirm_directory( filename, FE_ROMLIST_SUBDIR );

	filename += FE_ROMLIST_SUBDIR;
	filename += emu_name;
	filename += FE_ROMLIST_FILE_EXTENSION;
	write_romlist( filename, romlist );

	return true;
}
