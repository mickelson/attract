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

#include "fe_util.hpp"
#include "fe_base.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <cassert>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef SFML_SYSTEM_WINDOWS
#include <windows.h>
#else
#include <pwd.h>
#endif

#include <dirent.h>
#include <cstring>

namespace {

	void str_from_c( std::string &s, const char *c )
	{
		if ( c )
			s += c;
	}

#ifdef SFML_SYSTEM_WINDOWS

	std::string get_home_dir()
	{
		std::string retval;
		str_from_c( retval, getenv( "HOMEDRIVE" ) );
		str_from_c( retval, getenv( "HOMEPATH" ) );

		return retval;
	}

#else

	std::string get_home_dir()
	{
		std::string retval;
		str_from_c( retval, getenv( "HOME" ));
		if ( retval.empty() )
		{
			struct passwd *pw = getpwuid(getuid());
			str_from_c( retval, pw->pw_dir );
		}
		return retval;
	}

#endif

	bool split_out_path( const std::string &src, std::string &path )
	{
		size_t pos = src.find_last_of( "/\\" );
		if ( pos != std::string::npos )
		{
			path = src.substr( 0, pos );
			return true;
		}
		return false;
	}
} // end namespace

//
// Case insensitive check if filename has the specified extension/ending.
//
bool tail_compare(
			const std::string &filename,
			const char *extension )
{
	unsigned int extlen = strlen( extension );

	if ( extlen >= filename.size() )
		return false;

	for ( unsigned int i=0; i < extlen; i++ )
	{
		if ( std::tolower( filename[ filename.size() - extlen + i ] )
					!= std::tolower( extension[i] ) )
			return false;
	}

	return true;
}

bool file_exists( const std::string &file )
{
	return (( access( file.c_str(), 0 ) == -1 ) ? false : true);
}

std::string clean_path( const std::string &path, bool require_trailing_slash )
{
	std::string retval = path;

	if ( retval.empty() )
		return retval;

#ifdef SFML_SYSTEM_WINDOWS
	// substitute systemroot leading %SYSTEMROOT%
	if (( retval.size() > 12 )
		&& ( retval.compare( 0, 12, "%SYSTEMROOT%" ) == 0 ))
	{
		std::string end = retval.substr( 12 );
		retval.clear();
		str_from_c( retval, getenv( "SystemRoot" ) );
		retval += end;
	}
#endif

	// substitute home dir for leading ~
	if (( retval.size() > 1 ) && ( retval.compare( 0, 1, "~" ) == 0 ))
	{
		std::string end = retval.substr( 1 );
		retval = get_home_dir();
		retval += end;
	}

	// substitute home dir for leading $HOME
	if (( retval.size() > 5 ) && ( retval.compare( 0, 5, "$HOME" ) == 0 ))
	{
		std::string end = retval.substr( 5 );
		retval = get_home_dir();
		retval += end;
	}

	if (( require_trailing_slash )
#ifdef SFML_SYSTEM_WINDOWS
			&& (retval[retval.size()-1] != '\\')
#endif
			&& (retval[retval.size()-1] != '/'))
		retval += '/';

	return retval;
}

bool search_for_file( const std::string &base_path,
						const std::string &base_name,
						const char **valid_exts,
						std::string &result )
{
	std::vector<std::string> result_list;
	std::vector<std::string> bn;
	bn.push_back( base_name );
	if ( get_filename_from_base(
					result_list, base_path, bn, valid_exts, false )  )
	{
		result = result_list.front();
		return true;
	}

#ifdef _DIRENT_HAVE_D_TYPE

	DIR *dir;
	struct dirent *ent;

	if ( (dir = opendir( base_path.c_str() )) != NULL )
	{
		while ((ent = readdir( dir )) != NULL )
		{
			if (( ent->d_type == DT_DIR )
					&& ( strcmp( ent->d_name, "." ) != 0 )
					&& ( strcmp( ent->d_name, ".." ) != 0 ))
			{
				std::string subdir(base_path);
				str_from_c( subdir, ent->d_name );
				subdir += "/";

				if ( search_for_file( subdir, base_name, valid_exts, result ) )
					return true;
			}
		}
		closedir( dir );
	}

#endif
	return false;
}

void get_subdirectories(
			std::vector<std::string> &list,
			const std::string &path )
{
#ifdef _DIRENT_HAVE_D_TYPE
	DIR *dir;
	struct dirent *ent;
	if ( (dir = opendir( path.c_str() )) != NULL )
	{
		while ((ent = readdir( dir )) != NULL )
		{
			if (( ent->d_type == DT_DIR )
					&& ( strcmp( ent->d_name, "." ) != 0 )
					&& ( strcmp( ent->d_name, ".." ) != 0 ))
			{
				std::string subdir;
				str_from_c( subdir, ent->d_name );
				list.push_back( subdir );
			}
		}
	}
#endif
}

bool get_basename_from_extension(
			std::vector<std::string> &list,
			const std::string &path,
			const std::string &extension,
			bool strip_extension )
{
	DIR *dir;
	struct dirent *ent;

	if ( (dir = opendir( path.c_str() )) != NULL )
	{
		while ((ent = readdir( dir )) != NULL )
		{
			std::string what;
			str_from_c( what, ent->d_name );
			if ( ( what.compare( "." ) != 0 ) && ( what.compare( ".." ) != 0 )
				&& ( tail_compare( what, extension.c_str() ) ) )
			{
				if ( strip_extension && ( what.size() > extension.size() ))
					list.push_back( what.substr( 0, what.size() - extension.size() ) );
				else
					list.push_back( what );
			}
		}
		closedir( dir );
	}
	else
	{
		std::cout << "Error opening directory: " << path << std::endl;
	}

	std::sort( list.begin(), list.end() );
	return !(list.empty());
}

bool get_filename_from_base( std::vector<std::string> &list,
            const std::string &path,
				const std::vector <std::string> &base_list,
				const char **filter, bool filter_excludes )
{
	DIR *dir;
	struct dirent *ent;

	if ( base_list.empty() )
		return false;

	if ( (dir = opendir( path.c_str() )) != NULL )
	{
		for ( std::vector<std::string>::const_iterator itr = base_list.begin();
				itr < base_list.end(); ++itr )
		{
			rewinddir( dir );
			while ((ent = readdir( dir )) != NULL )
			{
				std::string what;
				str_from_c( what, ent->d_name );
				if (( what.compare( "." ) == 0 ) || ( what.compare( ".." ) == 0 ))
					continue;

				if (( what.size() >= (*itr).size() ) &&
					( what.compare( 0, (*itr).size(), (*itr) ) == 0 ))
				{
					bool add=true;
					if ( filter )
					{
						if ( !filter_excludes )
							add=false;
						int i=0;
						while ( filter[i] != NULL )
						{
							if ( tail_compare( what, filter[i] ) )
							{
								if ( !filter_excludes )
									add=true;
								else
									add=false;
								break;
							}
							i++;
						}
					}

					if ( add )
					{
						std::string new_name = path;
						str_from_c( new_name, ent->d_name );
						list.push_back( new_name );
					}
				}
			}
		}
		closedir( dir );
	}
	else
	{
		std::cout << "Error opening directory: " << path << std::endl;
	}

	return !(list.empty());
}


bool token_helper( const std::string &from,
	size_t &pos, std::string &token, const char *sep )
{
	std::string to;
	bool retval( false ), in_quotes( false ), escaped( false );
	size_t end;

	if ( from[pos] == '"' )
	{
		in_quotes = true;
		pos++;

		//
		// Find the next quote character that is not preceded
		// by a backslash
		//
		end = from.find_first_of( '"', pos );
		while (( end != std::string::npos ) &&
				( from[ end - 1 ] == '\\' ))
		{
			escaped = true;
			end = from.find_first_of( '"', end + 1 );
		}
	}
	else
	{
		end = from.find_first_of( sep, pos );
	}


	if ( end == std::string::npos )
	{
		to = from.substr( pos );
		pos = from.size();
	}
	else
	{
		to = from.substr( pos, end - pos );

		if ( in_quotes )
			pos = end + 2; // skip over quote and sep
		else
			pos = end + 1; // skip over sep

		retval = true;
	}

	// clean out leading and trailing whitespace from token
	//
	size_t f= to.find_first_not_of( FE_WHITESPACE );
	if ( f == std::string::npos )
	{
		token.clear();
	}
	else
	{
		size_t l = to.find_last_not_of( FE_WHITESPACE );
		token = to.substr( f, l-f+1 );
	}

	if ( escaped )
		perform_substitution( token, "\\\"", "\"" );

	return retval;
}

sf::Color colour_helper( const std::string &value )
{
	// format: R,G,B,A
	// values from 0 to 255
	sf::Color c;
	c.a = 255;

	size_t pos=0;
	std::string val;

	token_helper( value, pos, val, "," );

	if ( pos >= value.size() )
		return c;

	c.r = as_int( val );

	token_helper( value, pos, val, "," );
	c.g = as_int( val );

	if ( pos >= value.size() )
		return c;

	token_helper( value, pos, val, "," );
	c.b = as_int( val );

	if ( pos >= value.size() )
		return c;

	token_helper( value, pos, val, "," );
	c.a = as_int( val );

	return c;
}

int perform_substitution( std::string &target,
					const std::string &from,
					const std::string &to )
{
	int count=0;

	if ( from.empty() )
		return count;

	size_t loc=0;
	while ( (loc = target.find( from, loc )) != std::string::npos )
	{
		target.replace( loc, from.size(), to );
		loc += to.size();
		count++;
	}

	return count;
}

std::string get_available_filename(
			const std::string &path,
			const std::string &base,
			const std::string &extension,
			std::string &result )
{
	std::string base_name = base;
	std::string test_name = base_name;
	test_name += extension;

	int i=0;
	while ( file_exists( path + test_name ) )
	{
		std::ostringstream ss;
		ss << base << ++i;
		test_name = base_name = ss.str();
		test_name += extension;
	}

	result = path;
	result += test_name;

	return base_name;
}

//
// Delete named file
//
void delete_file( const std::string &file )
{
	remove( file.c_str() );
}

bool confirm_directory( const std::string &base, const std::string &sub )
{
	if ( !file_exists( base ) )
#ifdef SFML_SYSTEM_WINDOWS
		mkdir( base.c_str() );
#else
		mkdir( base.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH  );
#endif // SFML_SYSTEM_WINDOWS

	if ( (!sub.empty()) && (!file_exists( base + sub )) )
#ifdef SFML_SYSTEM_WINDOWS
		mkdir( (base + sub).c_str() );
#else
		mkdir( (base + sub).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH  );
#endif // SFML_SYSTEM_WINDOWS

	return true;
}

std::string as_str( int i )
{
	std::ostringstream ss;
	ss << i;
	return ss.str();
}

int as_int( const std::string &s )
{
	return atoi( s.c_str() );
}

bool run_program( const std::string &prog, const::std::string &args )
{
	std::string comstr( prog );
	comstr += " ";
	comstr += args;

	std::cout << "Running: " << comstr << std::endl;

#ifdef SFML_SYSTEM_WINDOWS
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	MSG msg;

	ZeroMemory( &si, sizeof(si) );
	ZeroMemory( &pi, sizeof(pi) );
	si.cb = sizeof(si);

	LPSTR ugh = const_cast<char *>(comstr.c_str());
	LPSTR current_dir( NULL );

	std::string path;
	if ( split_out_path( comstr, path ))
		current_dir = const_cast<char *>(path.c_str());

	if ( !CreateProcess( NULL, ugh, NULL, NULL, FALSE, 0, NULL, current_dir, &si, &pi ))
		return false;

	bool keep_wait=true;
	while (keep_wait)
	{
		switch (MsgWaitForMultipleObjects(1, &pi.hProcess,
						FALSE, INFINITE, QS_ALLINPUT))
		{
		case WAIT_OBJECT_0:
			keep_wait=false;
			break;

		case WAIT_OBJECT_0 + 1:
			// we have a message - peek and dispatch it
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			break;

		default:
			std::cout << "Unexpected failure waiting on process" << std::endl;
			keep_wait=false;
			break;
		}
	}
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	return true;
#else

#if 1
	return ( system( comstr.c_str() ) == 0 ) ? true : false;

#else

	std::vector < std::string > string_list;
	unsigned int pos=0;

	while ( pos < args.size() )
	{
		std::string val;
		token_helper( args, pos, val, FE_WHITESPACE );
		string_list.push_back( val );
	}

	std::cout << prog << " ";
	char *arg_list[string_list.size() + 2];
	arg_list[0] = (char *)prog.c_str();
	for ( unsigned int i=0; i < string_list.size(); i++ )
	{
		arg_list[i+1] = (char *)string_list[i].c_str();
		std::cout << "[" << arg_list[i+1] << "] ";
	}
	arg_list[string_list.size() + 1] = NULL;
	std::cout << std::endl;

	pid_t pid = fork();

	switch (pid)
	{
	case -1:
		std::cout << "Error, fork() failed" << std::endl;
		return false;

	case 0: // child process
		execv( prog.c_str(), arg_list );

		// execl doesn't return unless there is an error.
		std::cout << "Error, execl() failed" << std::endl;
		exit(1);

	default: // parent process

		int status;
		while ( !WIFEXITED( status ) )
		{
			waitpid( pid, &status, 0 ); // wait for child process to complete
		}
		return true;
	}
#endif

#endif // SFML_SYSTEM_WINDOWS
}

