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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <Fcntl.h>
#else
#include <sys/wait.h>
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
} // end namespace

//
// Case insensitive check if filename has the specified extension/ending.
//
bool tail_compare(
			const std::string &filename,
			const std::string &extension )
{
	unsigned int extlen = extension.size();

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

	std::vector<std::string> subs;
	get_subdirectories( subs, base_path );

	std::vector<std::string>::iterator itr;
	for ( itr = subs.begin(); itr != subs.end(); ++itr )
	{
		if ( search_for_file( base_path + "/" + (*itr),
				base_name,
				valid_exts,
				result ) )
		{
			return true;
		}
	}

	return false;
}

void get_subdirectories(
			std::vector<std::string> &list,
			const std::string &path )
{
	DIR *dir;
	struct dirent *ent;
	if ( (dir = opendir( path.c_str() )) != NULL )
	{
		while ((ent = readdir( dir )) != NULL )
		{
			if (( strcmp( ent->d_name, "." ) == 0 )
					|| ( strcmp( ent->d_name, ".." ) == 0 ))
				continue;

			std::string name;
			str_from_c( name, ent->d_name );

			struct stat st;
			stat( (path + "/" + name).c_str(), &st );

			if ( S_ISDIR( st.st_mode ) )
				list.push_back( name );
		}
		closedir( dir );
	}
	else
	{
		std::cerr << "Error opening directory: " << path << std::endl;
	}
}

bool get_basename_from_extension(
			std::vector<std::string> &list,
			const std::string &path,
			const std::vector <std::string> &extensions,
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

			if ( ( what.compare( "." ) == 0 ) || ( what.compare( ".." ) == 0 ) ) 
				continue;

			std::vector<std::string>::const_iterator itr;
			for ( itr = extensions.begin(); itr != extensions.end(); ++ itr )
			{
				if ( tail_compare( what, *itr ) )
				{
					if ( strip_extension && ( what.size() > (*itr).size() ))
					{
						std::string bname = what.substr( 0, 
							what.size() - (*itr).size() );

						// don't add duplicates if we are stripping extension
						// example: if there is both foo.zip and foo.7z 
						//
						if ( list.empty() || ( bname.compare( list.back() ) != 0 ))
							list.push_back( bname );
					}
					else
						list.push_back( what );
				}
			}
		}
		closedir( dir );
	}
	else
	{
		std::cerr << "Error opening directory: " << path << std::endl;
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
		std::cerr << "Error opening directory: " << path << std::endl;
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

bool run_program( const std::string &prog,
	const::std::string &args,
	output_callback_fn callback,
	void *opaque,
	bool block )
{
	std::string comstr( prog );
	comstr += " ";
	comstr += args;

#ifdef SFML_SYSTEM_WINDOWS
	HANDLE child_output_read=NULL;
	HANDLE child_output_write=NULL;

	SECURITY_ATTRIBUTES satts;
	satts.nLength = sizeof( SECURITY_ATTRIBUTES );
	satts.bInheritHandle = TRUE;
	satts.lpSecurityDescriptor = NULL;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	MSG msg;

	ZeroMemory( &si, sizeof(si) );
	ZeroMemory( &pi, sizeof(pi) );
	si.cb = sizeof(si);

	if ( NULL != callback )
	{
		CreatePipe( &child_output_read, &child_output_write, &satts, 1024 );
		si.hStdOutput = child_output_write;
		si.dwFlags |= STARTF_USESTDHANDLES;
	}

	LPSTR cmdline = new char[ comstr.length() + 1 ];
	strncpy( cmdline, comstr.c_str(), comstr.length() + 1 );

	LPSTR path = NULL;
	size_t pos = prog.find_last_of( "/\\" );
	if ( pos != std::string::npos )
	{
		path = new char[ pos + 1 ];
		strncpy( path, prog.substr( 0, pos ).c_str(), pos + 1 );
	}

	bool ret = CreateProcess( NULL,
		cmdline,
		NULL,
		NULL,
		( NULL == callback ) ? FALSE : TRUE,
		0,
		NULL,
		path,
		&si,
		&pi );

	// Parent process - close the child write handle after child created
	if ( child_output_write )
		CloseHandle( child_output_write );

	//
	// Cleanup our allocated values now
	//
	if ( path )
		delete [] path;

	delete [] cmdline;

	if ( ret == false )
		return false;

	if (( NULL != callback ) && ( block ))
	{
		int fd = _open_osfhandle( (intptr_t)child_output_read, _O_RDONLY );
		if ( fd == -1 )
		{
			std::cerr << "Error opening osf handle: " << comstr << std::endl;
		}
		else
		{
			FILE *fs = _fdopen( fd, "r" );
			if ( fs == NULL )
			{
				std::cerr << "Error opening output from: " 
					<< comstr << std::endl;
			}
			else
			{
				char buffer[ 1024 ];
				while ( fgets( buffer, 1024, fs ) != NULL )
				{
					if ( callback( buffer, opaque ) == false )
					{
						TerminateProcess( pi.hProcess, 0 );
						block=false;
						break;
					}
				}

				fclose( fs );
			}
			_close( fd ); // _close will close the underlying handle as well, 
							// no need to call CloseHandle()
		}
	}

	bool keep_wait=block;
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
			std::cerr << "Unexpected failure waiting on process" << std::endl;
			keep_wait=false;
			break;
		}
	}

	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

#else

	std::vector < std::string > string_list;
	unsigned int pos=0;

	while ( pos < args.size() )
	{
		std::string val;
		token_helper( args, pos, val, FE_WHITESPACE );
		string_list.push_back( val );
	}

	char *arg_list[string_list.size() + 2];
	arg_list[0] = (char *)prog.c_str();
	for ( unsigned int i=0; i < string_list.size(); i++ )
		arg_list[i+1] = (char *)string_list[i].c_str();

	arg_list[string_list.size() + 1] = NULL;

	int mypipe[2] = { 0, 0 }; // mypipe[0] = read end, mypipe[1] = write end
	if (( NULL != callback ) && block && ( pipe( mypipe ) ))
		std::cerr << "Error, pipe() failed" << std::endl;

	pid_t pid = fork();
	switch (pid)
	{
	case -1:
		std::cerr << "Error, fork() failed" << std::endl;
		if ( mypipe[0] )
		{
			close( mypipe[0] );
			close( mypipe[1] );
		}
		return false;

	case 0: // child process
		if ( mypipe[0] )
		{
			dup2( mypipe[1], fileno(stdout) );
			close( mypipe[1] );
		}
		execvp( prog.c_str(), arg_list );

		// execvp doesn't return unless there is an error.
		std::cerr << "Error executing: " << prog << " " << args << std::endl;
		_exit(127);

	default: // parent process

		if ( mypipe[0] )
		{
			FILE *fp = fdopen( mypipe[0], "r" );
			close( mypipe[1] );

			char buffer[ 1024 ];

			while( fgets( buffer, 1024, fp ) != NULL )
			{
				if ( callback( buffer, opaque ) == false )
				{
					// User cancelled
					kill( pid, SIGTERM );
					block=false;
					break;
				}
			}

			fclose( fp );
			close( mypipe[0] );
		}

		if ( block )
		{
			int status;
			do
			{
				waitpid( pid, &status, 0 ); // wait for child process to complete
			} while ( !WIFEXITED( status ) );
		}
	}
#endif // SFML_SYSTEM_WINDOWS
	return true;
}

