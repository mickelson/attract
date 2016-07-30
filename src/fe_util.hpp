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

#ifndef FE_UTIL_HPP
#define FE_UTIL_HPP

#include <vector>
#include <string>
#include <SFML/Config.hpp>

#ifdef FE_DEBUG
#include <cassert>
#endif

extern const char *FE_WHITESPACE;

//
// Utility functions for processing config files:
//
bool token_helper( const std::string &from,
		size_t &pos, std::string &token, const char *sep=";" );

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
//
std::string name_with_brackets_stripped( const std::string &name );

//
// Case insensitive check if filename has the specified extension/ending.
//
bool tail_compare(
         const std::string &filename,
         const std::string &extension );

bool tail_compare(
         const std::string &filename,
         const std::vector<std::string> &ext_list );

//
// Case insensitive compare of one and two
// returns 0 if equal
//
int icompare(
			const std::string &one,
			const std::string &two );

typedef bool (*output_callback_fn)( const char *, void * );
//
// Run the specified program, blocks while program running
//
// Arguments:
//		"prog" - program to run
//		"args" - space separated args. An arg can be wrapped in double quotes
//					if it contains spaces.
//		"cb" - callback function to call with stdout output from program
//		"opaque" - opaque ptr to pass to the callback function.  run_program()
//					doesn't care what this is.
//		"block" - if true, don't return until program is done execution
//					if false, "cb" and "opaque" are ignored
//		"exit_hotkey" - hotkey that when pressed will force exit of program
//					(use NULL for no hotkey checking)
//		"joy_thresh" - joystick threshold, only used if exit_hotkey is mapped to
//					a joystick
//
//	Returns true if program ran successfully
//
bool run_program( const std::string &prog,
	const::std::string &args,
	output_callback_fn cb = NULL,
	void *opaque=NULL,
	bool block=true,
	const std::string &exit_hotkey="",
	int joy_thresh=0 );

//
// Utility functions for file processing:
//
// return true if file exists (file or directory)
bool file_exists( const std::string &file );

// return true if specified path is an existing directory
bool directory_exists( const std::string &file );

// return true if the specified path is a relative path
bool is_relative_path( const std::string &file );

// clean the path string for usage.  Performs substitution of $HOME etc...
// if add_trailing_slash is true, this function adds a trailing '/' if
// path is to a directory
std::string clean_path( const std::string &path,
		bool add_trailing_slash = false );

// return path as an absolute path
std::string absolute_path( const std::string &path );

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
// Return list of subdirectories in path
//
bool get_subdirectories(
			std::vector<std::string> &list,
			const std::string &path );
//
// Return "list" of the base filenames in "path" where the file extension
// is in the vector "extensions"
//
// "list" is sorted alphabetically
//
bool get_basename_from_extension(
			std::vector<std::string> &list,
			const std::string &path,
			const std::string &extension,
			bool strip_extension = true );

//
// Return "in_list" of filenames in "path" where the base filename is "base_name"
//
// if filter is non-NULL: Only include files with an extension in the "ext_filter"
// NULL terminated list in "in_list".  All others go in "out_list".
//
bool get_filename_from_base( std::vector<std::string> &in_list,
				std::vector<std::string> &out_list,
				const std::string &path,
				const std::string &base_name,
				const char **ext_filter=NULL );

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
// Return float as a string
//
std::string as_str( float f, int decimals=3 );

//
// Return string as integer
//
int as_int( const std::string &s );

//
// Return config string (i.e. "yes", "true", "no", "false" as bool
//
bool config_str_to_bool( const std::string &s );

//
// Return the name of the operating system.
//
const char *get_OS_string();

//
// return the contents of the clipboard (if implemented for OS)
//
std::basic_string<sf::Uint32> clipboard_get_content();

//
// We use this in fe_window but implement it here because the XWindows
// namespace (Window, etc) clashes with the SFML namespace used in fe_window
// (sf::Window)
//
#ifdef USE_XINERAMA
void get_xinerama_geometry( int &, int &, int &, int & );
#endif

#ifndef NO_MOVIE
//
// Print FFmpeg version information to stdout.
//
void print_ffmpeg_version_info();
#endif

std::string url_escape( const std::string &raw );

void get_url_components( const std::string &url,
	std::string &host,
	std::string &req );

std::string get_crc32( char *buff, int size );

void string_to_vector( const std::string &input,
	std::vector< std::string > &vec, bool allow_empty=false );

//
// Take config file line and output setting and value pairs
// return true if setting and value found, false otherwise (for example if comment line encountered)
//
bool line_to_setting_and_value( const std::string &line,
	std::string &setting,
	std::string &value,
	const char *sep=FE_WHITESPACE );

//
// Windows systems: Hide the console window if not launched from the command line
//
#ifdef SFML_SYSTEM_WINDOWS
void hide_console();
#endif

#ifdef FE_DEBUG
#define ASSERT(a) assert(a)
#else
#define ASSERT(a)
#endif

#endif
