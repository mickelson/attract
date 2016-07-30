/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-2016 Andrew Mickelson
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

#include "fe_base.hpp"
#include "fe_util.hpp"
#include <iostream>
#include <fstream>

#define FE_NAME_D			"Attract-Mode"

const char *FE_NAME			= FE_NAME_D;
const char *FE_COPYRIGHT		= FE_NAME_D " " FE_VERSION_D \
	" Copyright (c) 2013-2016 Andrew Mickelson";
const char *FE_VERSION 			= FE_VERSION_D;

const char *FE_WHITESPACE=" \t\r";
const char *FE_DIR_TOKEN				= "<DIR>";

const char *FE_DEFAULT_ARTWORK		= "snap";

const char *FE_EMULATOR_SUBDIR		= "emulators/";
const char *FE_EMULATOR_FILE_EXTENSION	= ".cfg";
const char *FE_EMULATOR_DEFAULT		= "default-emulator.cfg";

void FeBaseConfigurable::invalid_setting(
	const std::string & fn,
	const char *base,
	const std::string &setting,
	const char **valid1,
	const char **valid2,
	const char *label )
{
	std::cerr << "Unrecognized \"" << base << "\" " << label
					<< " of \"" << setting << "\"";

	if ( !fn.empty() )
		std::cerr << " in file: " << fn;

	std::cerr << ".";

	int i=0;
	if (valid1[i])
		std::cerr << "  Valid " << label <<"s are: " << valid1[i++];

	while (valid1[i])
		std::cerr << ", " << valid1[i++];

	if ( valid2 != NULL )
	{
		i=0;
		while (valid2[i])
			std::cerr << ", " << valid2[i++];
	}

	std::cerr << std::endl;
}

bool FeBaseConfigurable::load_from_file( const std::string &filename,
	const char *sep )
{
   std::ifstream myfile( filename.c_str() );

   if ( !myfile.is_open() )
		return false;

	while ( myfile.good() )
	{
		std::string line, setting, value;
		getline( myfile, line );

		if ( line_to_setting_and_value( line, setting, value, sep ) )
			process_setting( setting, value, filename );
	}

	myfile.close();
	return true;
}
