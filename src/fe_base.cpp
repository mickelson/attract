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

#include "fe_base.hpp"
#include "fe_util.hpp"
#include <iostream>
#include <fstream>

#define FE_NAME_D					"Attract-Mode"
#define FE_VERSION_D 			"1.0.1"
const int FE_VERSION_NUM		= 101;

const char *FE_NAME				= FE_NAME_D;
const char *FE_COPYRIGHT		= FE_NAME_D " " FE_VERSION_D \
	" Copyright (c) 2013 Andrew Mickelson";
const char *FE_VERSION 			= FE_VERSION_D;

const char *FE_WHITESPACE=" \t\r";

void FeBaseConfigurable::invalid_setting( 
					const std::string & fn,
					const char *base, 
					const std::string &setting, 
					const char **valid1,
					const char **valid2,
					const char *label )
{
	std::cout << "Unrecognized \"" << base << "\" " << label 
					<< " of \"" << setting << "\" in file: " << fn << ".";
	int i=0;
	if (valid1[i]) 
		std::cout << "  Valid " << label <<"s are: " << valid1[i++];

	while (valid1[i])
		std::cout << ", " << valid1[i++];

	if ( valid2 != NULL )
	{
		i=0;
		while (valid2[i])
			std::cout << ", " << valid2[i++];
	}

	std::cout << std::endl;
}

bool FeFileConfigurable::load_from_file( const std::string &filename,
									const char *sep )
{
   std::ifstream myfile( filename.c_str() );

   if ( !myfile.is_open() )
		return false;

	while ( myfile.good() )
	{
		size_t pos=0, end=0;
		std::string line, setting, value;
		getline( myfile, line );

		// skip opening whitespace on line
		pos = line.find_first_not_of( FE_WHITESPACE, pos );
		if ( pos != std::string::npos )
		{
			end = line.find_first_of( sep, pos );

			if ( end == std::string::npos )
				setting = line.substr( pos );
			else
				setting = line.substr( pos, end - pos );

			// skip comments
			if (( setting.size() > 0 ) && (setting[0] != '#' ))
			{
				pos = line.find_first_not_of( FE_WHITESPACE, end + 1 );
				end = line.find_last_not_of( FE_WHITESPACE );
				if ( pos != std::string::npos )
					value = line.substr( pos, end - pos + 1 );

				 process_setting( setting, value, filename );
			}
		}
	}

	myfile.close();
	return true;
}
