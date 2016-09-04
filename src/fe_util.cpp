/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-16 Andrew Mickelson
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
#include "fe_input.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <cstring>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <SFML/Config.hpp>
#include <SFML/System/Sleep.hpp>

#ifdef USE_LIBARCHIVE
#include <zlib.h>
#endif

#ifdef SFML_SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <sys/wait.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#endif

#ifdef SFML_SYSTEM_MACOS
#include "fe_util_osx.hpp"
#endif

#ifdef USE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

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
bool c_tail_compare(
	const char *filename,
	size_t filelen,
	const char *extension,
	size_t extlen )
{
	if ( extlen > filelen )
		return false;

	for ( unsigned int i=0; i < extlen; i++ )
	{
		if ( std::tolower( filename[ filelen - extlen + i ] )
					!= std::tolower( extension[i] ) )
			return false;
	}

	return true;
}

bool tail_compare(
	const std::string &filename,
	const std::string &extension )
{
	return c_tail_compare( filename.c_str(),
		filename.size(),
		extension.c_str(),
		extension.size() );
}

bool tail_compare(
	const std::string &filename,
	const std::vector<std::string> &ext_list )
{
	for ( std::vector<std::string>::const_iterator itr=ext_list.begin();
			itr != ext_list.end(); ++itr )
	{
		if ( c_tail_compare( filename.c_str(),
				filename.size(),
				(*itr).c_str(),
				(*itr).size() ) )
			return true;

	}

	return false;
}

int icompare(
	const std::string &one,
	const std::string &two )
{
	unsigned int one_len = one.size();

	if ( one_len != two.size() )
		return ( one_len - two.size() );

	for ( unsigned int i=0; i < one_len; i++ )
	{
		if ( std::tolower( one[i] ) != std::tolower( two[i] ) )
			return ( one[i] - two[i] );
	}

	return 0;
}

bool file_exists( const std::string &file )
{
	return (( access( file.c_str(), 0 ) == -1 ) ? false : true);
}

bool directory_exists( const std::string &file )
{
	if (( file.empty() )
			|| ( file[ file.size()-1 ] == '/' )
			|| ( file[ file.size()-1 ] == '\\' ))
		return file_exists( file );
	else
		return file_exists( file + '/' );
}

bool is_relative_path( const std::string &name )
{
#ifdef SFML_SYSTEM_WINDOWS
	if (( name.size() > 2 )
			&& ( isalpha( name[0] ) )
			&& ( name[1] == ':' )
			&& (( name[2] == '\\' ) || ( name[2] == '/' )))
		return false;

	// Windows UNC i.e. \\computer\share
	if (( name.size() > 1 )
			&& ( name[0] == '\\' )
			&& ( name[1] == '\\' ))
		return false;
#endif
	return (( !name.empty() ) && ( name[0] != '/' ));
}

std::string clean_path( const std::string &path, bool add_trailing_slash )
{
	std::string retval = path;

	if ( retval.empty() )
		return retval;

#ifdef SFML_SYSTEM_WINDOWS
	// substitute systemroot leading %SYSTEMROOT%
	if (( retval.size() >= 12 )
		&& ( retval.compare( 0, 12, "%SYSTEMROOT%" ) == 0 ))
	{
		std::string sysroot;
		str_from_c( sysroot, getenv( "SystemRoot" ) );
		retval.replace( 0, 12, sysroot );
	}
	else if (( retval.size() >= 14 )
		&& ( retval.compare( 0, 14, "%PROGRAMFILES%" ) == 0 ))
	{
		std::string pf;
		str_from_c( pf, getenv( "ProgramFiles" ) );
		retval.replace( 0, 14, pf );
	}

#endif

	// substitute home dir for leading ~
	if (( retval.size() >= 1 ) && ( retval.compare( 0, 1, "~" ) == 0 ))
		retval.replace( 0, 1, get_home_dir() );

	// substitute home dir for leading $HOME
	if (( retval.size() >= 5 ) && ( retval.compare( 0, 5, "$HOME" ) == 0 ))
		retval.replace( 0, 5, get_home_dir() );

	if (( add_trailing_slash )
#ifdef SFML_SYSTEM_WINDOWS
			&& (retval[retval.size()-1] != '\\')
#endif
			&& (retval[retval.size()-1] != '/')
			&& (directory_exists(retval)) )
		retval += '/';

	return retval;
}

std::string absolute_path( const std::string &path )
{
#ifdef SFML_SYSTEM_WINDOWS

	const int BUFF_SIZE = 512;
	char buff[ BUFF_SIZE + 1 ];
	buff[BUFF_SIZE] = 0;

	if ( GetFullPathNameA( path.c_str(), BUFF_SIZE, buff, NULL ))
		return std::string( buff );
#else
	char buff[PATH_MAX+1];

	if ( realpath( path.c_str(), buff ) )
	{
		std::string retval = buff;
		if (( retval.size() > 0 )
				&& ( retval[ retval.size()-1 ] != '/' )
				&& directory_exists( retval ))
			retval += "/";

		return retval;
	}
#endif // SFML_SYSTEM_WINDOWS

	return path;
}

bool search_for_file( const std::string &base_path,
						const std::string &base_name,
						const char **valid_exts,
						std::string &result )
{
	std::vector<std::string> result_list;
	std::vector<std::string> ignore_list;

	if ( get_filename_from_base(
		result_list, ignore_list, base_path, base_name, valid_exts )  )
	{
		result = result_list.front();
		return true;
	}

	std::vector<std::string> subs;
	get_subdirectories( subs, base_path );

	std::vector<std::string>::iterator itr;
	for ( itr = subs.begin(); itr != subs.end(); ++itr )
	{

		if ( search_for_file(
				base_path + (*itr) + "/",
				base_name,
				valid_exts,
				result ) )
		{
			return true;
		}
	}

	return false;
}

bool get_subdirectories(
			std::vector<std::string> &list,
			const std::string &path )
{
#ifdef SFML_SYSTEM_WINDOWS
	std::string temp = path + "*";

	struct _finddata_t t;
	intptr_t srch = _findfirst( temp.c_str(), &t );

	if  ( srch < 0 )
		return false;

	do
	{
		std::string what;
		str_from_c( what, t.name );

		if ( ( what.compare( "." ) == 0 ) || ( what.compare( ".." ) == 0 ) )
			continue;

		if ( t.attrib & _A_SUBDIR )
			list.push_back( what );

	} while ( _findnext( srch, &t ) == 0 );
	_findclose( srch );

#else

	DIR *dir;
	struct dirent *ent;
	if ( (dir = opendir( path.c_str() )) == NULL )
		return false;

	while ((ent = readdir( dir )) != NULL )
	{
		std::string name;
		str_from_c( name, ent->d_name );

		if (( name.compare( "." ) == 0 )
				|| ( name.compare( ".." ) == 0 ))
			continue;

		struct stat st;
		stat( (path + name).c_str(), &st );

		if ( S_ISDIR( st.st_mode ) )
			list.push_back( name );
	}
	closedir( dir );

#endif

	return true;
}

bool get_basename_from_extension(
			std::vector<std::string> &list,
			const std::string &path,
			const std::string &extension,
			bool strip_extension )
{
#ifdef SFML_SYSTEM_WINDOWS
	std::string temp = path;
	if ( !path.empty()
			&& ( path[path.size()-1] != '/' )
			&& ( path[path.size()-1] != '\\' ))
		temp += "/";

	temp += "*" + extension;

	struct _finddata_t t;
	intptr_t srch = _findfirst( temp.c_str(), &t );

	if  ( srch < 0 )
		return false;

	do
	{
		std::string what;
		str_from_c( what, t.name );

		// I don't know why but the search filespec we are using
		// "path/*.ext"seems to also match "path/*.ext*"... so we
		// do the tail comparison below on purpose to catch this...
#else
	DIR *dir;
	struct dirent *ent;

	if ( (dir = opendir( path.c_str() )) == NULL )
		return false;

	while ((ent = readdir( dir )) != NULL )
	{
		std::string what;
		str_from_c( what, ent->d_name );
#endif

		if ( ( what.compare( "." ) == 0 ) || ( what.compare( ".." ) == 0 ) )
			continue;

		if ( tail_compare( what, extension ) )
		{
			if ( strip_extension && ( what.size() > extension.size() ))
			{
				std::string bname = what.substr( 0,
					what.size() - extension.size() );

				// don't add duplicates if we are stripping extension
				// example: if there is both foo.zip and foo.7z
				//
				if ( list.empty() || ( bname.compare( list.back() ) != 0 ))
					list.push_back( bname );
			}
			else
				list.push_back( what );

		}
#ifdef SFML_SYSTEM_WINDOWS
	} while ( _findnext( srch, &t ) == 0 );
	_findclose( srch );
#else
	}
	closedir( dir );
#endif

	return !(list.empty());
}

bool get_filename_from_base(
	std::vector<std::string> &in_list,
	std::vector<std::string> &out_list,
	const std::string &path,
	const std::string &base_name,
	const char **filter )
{
#ifdef SFML_SYSTEM_WINDOWS
	std::string temp = path + base_name + "*";

	struct _finddata_t t;
	intptr_t srch = _findfirst( temp.c_str(), &t );

	if  ( srch < 0 )
		return false;

	do
	{
		const char *what = t.name;
		size_t what_len = strlen( what );

		if (( strcmp( what, "." ) != 0 )
				&& ( strcmp( what, ".." ) != 0 ))
		{
#else

	DIR *dir;
	struct dirent *ent;

	if ( (dir = opendir( path.c_str() )) == NULL )
		return false;

	while ((ent = readdir( dir )) != NULL )
	{
		const char *what = ent->d_name;
		size_t what_len = strlen( what );
		size_t base_len = base_name.size();

		if (( strcmp( what, "." ) != 0 )
				&& ( strcmp( what, ".." ) != 0 )
				&& ( what_len >= base_len )
				&& ( strncasecmp( what, base_name.c_str(), base_len ) == 0 ))
		{
#endif // SFML_SYSTEM_WINDOWS
			if ( filter )
			{
				bool add=false;
				int i=0;
				while ( filter[i] != NULL )
				{
					if ( c_tail_compare( what,
						what_len,
						filter[i],
						strlen( filter[i] ) ) )
					{
						add=true;
						break;
					}
					i++;
				}

				if ( add )
					in_list.push_back( path + what );
				else
					out_list.push_back( path + what );
			}
			else
				in_list.push_back( path + what );
#ifdef SFML_SYSTEM_WINDOWS
		}
	} while ( _findnext( srch, &t ) == 0 );
	_findclose( srch );
#else
		}
	}
	closedir( dir );
#endif // SFML_SYSTEM_WINDOWS

	return !(in_list.empty());
}

bool token_helper( const std::string &from,
	size_t &pos, std::string &token, const char *sep )
{
	bool retval( false ), in_quotes( false ), escaped( false );
	size_t end;

	//
	// Skip leading whitespace
	//
	pos = from.find_first_not_of( FE_WHITESPACE, pos );
	if ( pos == std::string::npos )
	{
		token.clear();
		pos = from.size();
		return false;
	}

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

	size_t old_pos = pos;
	if ( end == std::string::npos )
	{
		pos = from.size();
	}
	else
	{
		if ( in_quotes )
			pos = end + 2; // skip over quote and sep
		else
			pos = end + 1; // skip over sep

		retval = true;
	}

	// clean out leading and trailing whitespace from token
	//
	size_t f = from.find_first_not_of( FE_WHITESPACE, old_pos );

	if (( f == std::string::npos ) || ( f == end ))
	{
		token.clear();
	}
	else
	{
		size_t l = from.find_last_not_of( FE_WHITESPACE, end-1 );
		token = from.substr( f, l-f+1 );
	}

	if ( escaped )
		perform_substitution( token, "\\\"", "\"" );

	return retval;
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
	if ( !directory_exists( base ) )
#ifdef SFML_SYSTEM_WINDOWS
		mkdir( base.c_str() );
#else
		mkdir( base.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH  );
#endif // SFML_SYSTEM_WINDOWS

	if ( (!sub.empty()) && (!directory_exists( base + sub )) )
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

std::string as_str( float f, int decimals )
{
	std::ostringstream ss;
	ss << std::setprecision( decimals ) << std::fixed << f;
	return ss.str();
}


int as_int( const std::string &s )
{
	return atoi( s.c_str() );
}

bool config_str_to_bool( const std::string &s )
{
	if (( s.compare( "yes" ) == 0 ) || ( s.compare( "true" ) == 0 ))
		return true;
	else
		return false;
}

const char *get_OS_string()
{
#if defined(SFML_SYSTEM_WINDOWS)
	return "Windows";
#elif defined(SFML_SYSTEM_MACOS)
	return "OSX";
#elif defined(SFML_SYSTEM_FREEBSD)
	return "FreeBSD";
#elif defined(SFML_SYSTEM_LINUX)
	return "Linux";
#elif defined(SFML_SYSTEM_ANDROID)
	return "Android";
#else
	return "Unknown";
#endif
}


bool run_program( const std::string &prog,
	const::std::string &args,
	output_callback_fn callback,
	void *opaque,
	bool block,
	const std::string &exit_hotkey,
	int joy_thresh )
{
	const int POLL_FOR_EXIT_MS=50;

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

	if (( NULL != callback )
		&& CreatePipe( &child_output_read, &child_output_write, &satts, 0 ))
	{
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
	{
		std::cerr << "Error executing command: '" << comstr << "'" << std::endl;
		return false;
	}

	if (( NULL != callback ) && ( block ))
	{
		const int BUFF_SIZE = 2048;
		char buffer[ BUFF_SIZE+1 ];
		buffer[BUFF_SIZE]=0;
		DWORD bytes_read;
		while ( ReadFile( child_output_read, buffer, BUFF_SIZE, &bytes_read, NULL ) != 0 )
		{
			buffer[bytes_read]=0;
			if ( callback( buffer, opaque ) == false )
			{
				TerminateProcess( pi.hProcess, 0 );
				block=false;
				break;
			}
		}
	}

	if ( child_output_read )
		CloseHandle( child_output_read );

	DWORD timeout = ( callback || exit_hotkey.empty() )
		? INFINITE : POLL_FOR_EXIT_MS;

	FeInputMapEntry exit_is( exit_hotkey );

	bool keep_wait=block;
	while (keep_wait)
	{
		switch (MsgWaitForMultipleObjects(1, &pi.hProcess,
						FALSE, timeout, 0))
		{
		case WAIT_OBJECT_0:
			keep_wait=false;
			break;

		case WAIT_OBJECT_0 + 1:
			//
			// See: https://blogs.msdn.microsoft.com/oldnewthing/20050217-00/?p=36423
			//
			// we have a message - peek and dispatch it
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			break;

		case WAIT_TIMEOUT:
			// We should only ever get here if an exit_hotkey was provided
			//
			if ( exit_is.get_current_state( joy_thresh ) )
			{
				TerminateProcess( pi.hProcess, 0 );

				keep_wait=false;
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
	size_t pos=0;

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

		{
			size_t pos = prog.find_last_of( "/" );
			if ( pos != std::string::npos )
				if (chdir( prog.substr( 0, pos ).c_str() ) != 0)
					std::cerr << "Warning, chdir(" << prog.substr( 0, pos ) << ") failed.";
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
			int opt = exit_hotkey.empty() ? 0 : WNOHANG; // option for waitpid.  0= wait for process to complete, WNOHANG=return right away

			FeInputMapEntry exit_is( exit_hotkey );

			while (1)
			{
				pid_t w = waitpid( pid, &status, opt );
				if ( w == 0 )
				{
					// waitpid should only return 0 if WNOHANG is used and the child is still running, so we
					// should only ever get here if there is an exit_hotkey provided
					//
					if ( exit_is.get_current_state( joy_thresh ) )
					{
						// Where the user has configured the "exit hotkey" in Attract-Mode to the same key as the emulator
						// uses to exit, we often have a problem of losing focus.  Delaying a bit and testing to make sure
						// the emulator process is still running before sending the kill signal seems to help...
						//
						sf::sleep( sf::milliseconds( 100 ) );
						if ( kill( pid, 0 ) == 0 )
							kill( pid, SIGTERM );

						break; // leave do/while loop
					}
					sf::sleep( sf::milliseconds( POLL_FOR_EXIT_MS ) );
				}
				else
				{
					if ( w < 0 )
						std::cerr << " ! error returned by wait_pid(): "
							<< strerror( errno ) << std::endl;

					break; // leave do/while loop
				}
			}
		}
	}
#endif // SFML_SYSTEM_WINDOWS
	return true;
}

std::string name_with_brackets_stripped( const std::string &name )
{
	size_t pos = name.find_first_of( "(/[" );

	if (( pos == std::string::npos ) || ( pos == 0 ))
		return name;

	if ( name.at( pos-1 ) == ' ' )
		pos = name.find_last_not_of( FE_WHITESPACE, pos-1 ) + 1;

	return name.substr( 0, pos );
}


std::basic_string<sf::Uint32> clipboard_get_content()
{
	std::basic_string<sf::Uint32> retval;

#ifdef SFML_SYSTEM_MACOS
	retval = osx_clipboard_get_content();
#endif

#ifdef SFML_SYSTEM_WINDOWS
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
		return retval;

	if (!OpenClipboard(NULL))
		return retval;

	HGLOBAL hglob = GetClipboardData(CF_UNICODETEXT);
	if (hglob != NULL)
	{
		LPWSTR lptstr = (LPWSTR)GlobalLock(hglob);
		if (lptstr != NULL)
		{
			std::wstring str = lptstr;

			for ( std::wstring::iterator itr=str.begin(); itr!=str.end(); ++itr )
			{
				if ( *itr >= 32 ) // strip control characters such as line feeds, etc
					retval += *itr;
			}

			GlobalUnlock(hglob);
		}
	}

	CloseClipboard();
#endif // if WINDOWS

	return retval;
}

#ifdef USE_XINERAMA
//
// We use this in fe_window but implement it here because the XWindows
// namespace (Window, etc) clashes with the SFML namespace used in fe_window
// (sf::Window)
//
void get_xinerama_geometry( int &x, int &y, int &width, int &height )
{
	x=0;
	y=0;
	width=0;
	height=0;

	::Display *xdisp = XOpenDisplay( NULL );
	int num=0;

	XineramaScreenInfo *si=XineramaQueryScreens( xdisp, &num );

	if ( num > 1 )
	{
		x=-si[0].x_org;
		y=-si[0].y_org;
	}

	for ( int i=0; i<num; i++ )
	{
		width = std::max( width, si[i].x_org + si[i].width );
		height = std::max( height, si[i].y_org + si[i].height );
	}

	XFree( si );
	XCloseDisplay( xdisp );
}
#endif

#ifdef SFML_SYSTEM_WINDOWS
void hide_console()
{
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	GetStartupInfo(&si);
	if ( si.dwFlags && !(si.dwFlags & STARTF_USESTDHANDLES ) )
	{
		HWND handle = GetConsoleWindow();
		ShowWindow(handle, SW_HIDE);
	}
}
#endif

std::string url_escape( const std::string &raw )
{
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = raw.begin(); i != raw.end(); ++i)
	{
		const unsigned char c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum( c ) || c == '-' || c == '_' || c == '.' || c == '~')
		{
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << '%' << std::setw(2) << int(c);
	}

	return escaped.str();
}

void get_url_components( const std::string &url,
	std::string &host,
	std::string &req )
{
	size_t pos=0;

	for ( int i=0; i<3 && ( pos != std::string::npos ); i++ )
		pos = url.find_first_of( '/', pos+1 );

	if (( pos != std::string::npos ) && ( pos < url.size()-1 ) )
	{
		host = url.substr( 0, pos+1 );
		req = url.substr( pos+1 );
	}
}

std::string get_crc32( char *buff, int size )
{
#ifdef USE_LIBARCHIVE
	uLong crc = crc32( 0L, Z_NULL, 0 );
	crc = crc32( crc, (Bytef *)buff, size );

	std::ostringstream ss;
	ss.fill('0');
	ss << std::hex << std::setw(8) << crc;
	return ss.str();
#else
	return "";
#endif
}

void string_to_vector( const std::string &input,
	std::vector< std::string > &vec, bool allow_empty )
{
	size_t pos=0;
	do
	{
		std::string val;
		token_helper( input, pos, val );

		if ( ( !val.empty() ) || allow_empty )
			vec.push_back( val );

	} while ( pos < input.size() );
}

bool line_to_setting_and_value( const std::string &line,
	std::string &setting,
	std::string &value,
	const char *sep )
{
	// skip opening whitespace on line
	size_t pos = line.find_first_not_of( FE_WHITESPACE );
	if ( pos != std::string::npos )
	{
		size_t end = line.find_first_of( sep, pos );
		std::string tmp_setting;

		if ( end == std::string::npos )
			tmp_setting = line.substr( pos );
		else
			tmp_setting = line.substr( pos, end - pos );

		// skip comments
		if (( tmp_setting.size() > 0 ) && (tmp_setting[0] != '#' ))
		{
			pos = line.find_first_not_of( FE_WHITESPACE, end + 1 );
			end = line.find_last_not_of( FE_WHITESPACE );
			if ( pos != std::string::npos )
				value = line.substr( pos, end - pos + 1 );

			setting.swap( tmp_setting );
			return true;
		}
	}
	return false;
}

