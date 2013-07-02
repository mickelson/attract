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

#ifndef FE_UTIL_HPP
#define FE_UTIL_HPP

#include <vector>
#include <string>
#include <SFML/Graphics/Color.hpp>

#ifdef FE_DEBUG
#include <cassert>
#endif

//
// Utility functions for processing config files:
//
bool token_helper( const std::string &from,
		size_t &pos, std::string &token, const char *sep=";" );

sf::Color colour_helper( const std::string &value );

//
// Substitute all occurrences of "from" that appear in "target" with
// the text from "to"
//
// returns the number of substitutions made
//
int perform_substitution( std::string &target,
                           const std::string &from,
                           const std::string &to );

//
// Run the specified program, blocks while program running
//
bool run_program( const std::string &prog, const::std::string &args );

//
// Utility functions for file processing:
//
// return true if file exists
bool file_exists( const std::string &file );

// clean the path string for usage.  Performs substitution of $HOME etc...
std::string clean_path( const std::string &path, 
		bool require_trailing_slash = false );

//
// Search "base_path"'s dir structure for a file with the given "base_name".
// Valid extensions for the "result" file are in the NULL terminated list 
// "valid_extensions".
//
// put the full path and name in "result" and return true if found
//
bool search_for_file( const std::string &base_path,
                  const std::string &file,
						const char **valid_extensions,
                  std::string &result );

//       
// Return "list" of the base filenames in "path" where the file extension 
// is "ext"
//
// "list" is sorted alphabetically
//
bool get_basename_from_extension( 
			std::vector<std::string> &list, 
			const std::string &path, 
			const std::string &ext,
			bool strip_extension = true );

//       
// Return "list" of filenames in "path" where the file basename is in "baselist"
// Ordering of returned "list" corresponds to ordering of baselist
//
// if filter_excludes is false and ext_filter is non-NULL: Only include 
// files with an extension in the "ext_filter" NULL terminated list. 
//
// if filter_excludes is true and ext_filter is non-NULL: Don't include 
// files with an extension in the "ext_filter" NULL terminated list. 
//
bool get_filename_from_base( std::vector<std::string> &list, 
				const std::string &path, 
				const std::vector<std::string> &base_list,
				const char **ext_filter=NULL,
				bool filter_excludes=false );

//
// Get a filename that does not currently exist.  
// The resulting filename must start with "base" and end with "extension"
//
// Results:
// "result" is the resulting filename with the full path and extension.
// the returned string is the resulting basename only (no path or extension)
//
std::string get_available_filename(
         const std::string &path,
         const std::string &base,
         const std::string &extension,
         std::string &result );

//
// Create "base" directory if it doesn't exist
// Create "sub" folder in "base" if it doesn't already exist
//
bool confirm_directory( const std::string &base, const std::string &sub );

//
// Delete named file
//
void delete_file( const std::string &file );

//
// Return integer as a string
//
std::string as_str( int i );

//
// Return string as integer
//
int as_int( const std::string &s );

#ifdef FE_DEBUG
#define ASSERT(a) assert(a)
#else
#define ASSERT(a)
#endif

#endif
