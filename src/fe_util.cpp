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
#include "nowide/fstream.hpp"
#include "nowide/cstdio.hpp"
#include "nowide/cstdlib.hpp"
#include "nowide/convert.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <SFML/Config.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Clock.hpp>

#ifdef USE_LIBARCHIVE
#include <zlib.h>
#endif

#ifdef SFML_SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <tlhelp32.h> // for CreateToolhelp32Snapshot()
#include <psapi.h> // for GetModuleFileNameEx()
#else
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <wordexp.h>
#endif

#ifdef SFML_SYSTEM_MACOS
#include "fe_util_osx.hpp"
#endif

#ifdef SFML_SYSTEM_ANDROID
#include "fe_util_android.hpp"
#endif

#ifdef USE_XLIB
#include <X11/Xlib.h>
 #ifdef USE_XINERAMA
 #include <X11/extensions/Xinerama.h>
 #endif
#endif


namespace {

	void str_from_c( std::string &s, const char *c )
	{
		if ( c )
			s += c;
	}

	void wstr_from_c( std::basic_string<wchar_t> &s, const wchar_t *c )
	{
		if ( c )
			s += c;
	}

	std::string narrow( const std::basic_string<wchar_t> &s )
	{
		try
		{
			return nowide::narrow( s );
		}
		catch ( ... )
		{
			FeLog() << "Error converting string to UTF-8."<< std::endl;
			return "";
		}
	}

	std::basic_string<wchar_t> widen( const std::string &s )
	{
		try
		{
			return nowide::widen( s );
		}
		catch ( ... )
		{
			FeLog() << "Error converting string from UTF-8: " << s << std::endl;
			return L"";
		}
	}


#if defined(SFML_SYSTEM_WINDOWS)

	std::string get_home_dir()
	{
		std::string retval;
		str_from_c( retval, nowide::getenv( "HOMEDRIVE" ) );
		str_from_c( retval, nowide::getenv( "HOMEPATH" ) );

		return retval;
	}

#elif !defined(SFML_SYSTEM_ANDROID)

	std::string get_home_dir()
	{
		std::string retval;
		str_from_c( retval, nowide::getenv( "HOME" ));
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

bool tail_compare(
	const std::string &filename,
	const char **ext_list )
{
	for ( int i=0; ext_list[i] != NULL; i++ )
	{
		if ( c_tail_compare( filename.c_str(),
				filename.size(),
				ext_list[i],
				strlen( ext_list[i] ) ) )
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
#ifdef SFML_SYSTEM_WINDOWS
	return ( _waccess( widen( file ).c_str(), 0 ) != -1 );
#else
	return ( access( file.c_str(), 0 ) != -1 );
#endif
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

bool is_relative_path( const std::string &n )
{
	std::string name = clean_path( n );

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
	struct subs_struct
	{
		const char *comp;
		const char *env;
	} subs[] =
	{
		{ "%SYSTEMROOT%", "SystemRoot" },
		{ "%PROGRAMFILES%", "ProgramFiles" },
		{ "%PROGRAMFILESx86%", "ProgramFiles(x86)" },
		{ "%APPDATA%", "APPDATA" },
		{ "%SYSTEMDRIVE%", "SystemDrive" },
		{ NULL, NULL }
	};

	for ( int i=0; subs[i].comp != NULL; i++ )
	{
		int l = strlen( subs[i].comp );
		if (( retval.size() >= l ) && ( retval.compare( 0, l, subs[i].comp ) == 0 ))
		{
			std::string temp;
			str_from_c( temp, nowide::getenv( subs[i].env ) );
			retval.replace( 0, l, temp );
			break;
		}
	}
#endif

	// substitute home dir for leading ~
	if (( retval.size() >= 1 ) && ( retval.compare( 0, 1, "~" ) == 0 ))
		retval.replace( 0, 1, get_home_dir() );

	// substitute home dir for leading $HOME
	if (( retval.size() >= 5 ) && ( retval.compare( 0, 5, "$HOME" ) == 0 ))
		retval.replace( 0, 5, get_home_dir() );

	// substitute program dir for leading $PROGDIR
	if (( retval.size() >= 8 ) && ( retval.compare( 0, 8, "$PROGDIR" ) == 0 ))
		retval.replace( 0, 8, get_program_path() );

	if (( add_trailing_slash )
#ifdef SFML_SYSTEM_WINDOWS
			&& (retval[retval.size()-1] != '\\')
#endif
			&& (retval[retval.size()-1] != '/'))
		retval += '/';

	return retval;
}

std::string get_program_path()
{
	std::string path;
#ifdef SFML_SYSTEM_WINDOWS
	char result[ MAX_PATH ];
	path = std::string( result, GetModuleFileName( NULL, result, MAX_PATH ) );
#else
	char result[ PATH_MAX ];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	path = std::string( result, (count > 0) ? count : 0 );
#endif
	size_t found = path.find_last_of("/\\");
	return( path.substr(0, found) );
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

	struct _wfinddata_t t;
	intptr_t srch = _wfindfirst( widen( temp ).c_str(), &t );

	if  ( srch < 0 )
		return false;

	do
	{
		std::basic_string<wchar_t> what;
		wstr_from_c( what, t.name );

		if ( ( what.compare( L"." ) == 0 ) || ( what.compare( L".." ) == 0 ) )
			continue;

		if ( t.attrib & _A_SUBDIR )
			list.push_back( narrow( what ) );

	} while ( _wfindnext( srch, &t ) == 0 );
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

	struct _wfinddata_t t;
	intptr_t srch = _wfindfirst( widen( temp ).c_str(), &t );

	if  ( srch < 0 )
		return false;

	do
	{
		std::string what = narrow( t.name );

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
	} while ( _wfindnext( srch, &t ) == 0 );
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

	struct _wfinddata_t t;
	intptr_t srch = _wfindfirst( widen( temp ).c_str(), &t );

	if  ( srch < 0 )
		return false;

	do
	{
		std::string whatstr = narrow( t.name );
		const char *what = whatstr.c_str();
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
	} while ( _wfindnext( srch, &t ) == 0 );
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
	nowide::remove( file.c_str() );
}

bool confirm_directory( const std::string &base, const std::string &sub )
{
	bool retval=false;

	if ( !directory_exists( base ) )
	{
#ifdef SFML_SYSTEM_WINDOWS
		_wmkdir( widen( base ).c_str() );
#else
		mkdir( base.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH  );
#endif // SFML_SYSTEM_WINDOWS
		retval=true;
	}

	if ( (!sub.empty()) && (!directory_exists( base + sub )) )
	{
#ifdef SFML_SYSTEM_WINDOWS
		_wmkdir( widen(base + sub).c_str() );
#else
		mkdir( (base + sub).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH  );
#endif // SFML_SYSTEM_WINDOWS
		retval=true;
	}

	return retval;
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

namespace
{
	bool process_check_for_hotkey(
		const run_program_options_class *opt,
		const FeInputMapEntry &hotkey )

	{
		// Check for key down
		//
		if ( !hotkey.get_current_state( opt->joy_thresh ) )
			return false;

		// If down, wait for key release
		//
		while ( hotkey.get_current_state( opt->joy_thresh ) )
		{
			if ( opt->wait_cb )
				opt->wait_cb( opt->launch_opaque );

			sf::sleep( sf::milliseconds( 10 ) );
		}

		return true;
	}
};

run_program_options_class::run_program_options_class()
	: joy_thresh( 0 ),
	launch_cb( NULL ),
	wait_cb( NULL ),
	launch_opaque( NULL ),
	running_pid( 0 ),
	running_wnd( NULL )
{
}

const int POLL_FOR_EXIT_MS=20;

#if defined ( SFML_SYSTEM_WINDOWS )

typedef LONG (NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle );
typedef LONG (NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle );

void windows_wait_process(
	const HANDLE &hProcess,
	const DWORD &dwProcessId,
	run_program_options_class *opt )
{
	DWORD timeout = ( opt->exit_hotkey.empty() && opt->pause_hotkey.empty() )
		? INFINITE : POLL_FOR_EXIT_MS;

	FeInputMapEntry exit_is( opt->exit_hotkey );
	FeInputMapEntry pause_is( opt->pause_hotkey );

	bool keep_wait=true;
	while (keep_wait)
	{
		switch (MsgWaitForMultipleObjects(1, &hProcess,
						FALSE, timeout, 0))
		{
		case WAIT_OBJECT_0:
			keep_wait=false;
			break;

		case WAIT_OBJECT_0 + 1:
			//
			// See: https://blogs.msdn.microsoft.com/oldnewthing/20050217-00/?p=36423
			//
			if ( opt->wait_cb )
				opt->wait_cb( opt->launch_opaque );

			break;

		case WAIT_TIMEOUT:
			// We should only ever get here if an exit_hotkey was provided
			//
			if ( process_check_for_hotkey( opt, exit_is ) )
			{
				FeLog() << " - Exit Hotkey pressed, terminating process: " << (int)dwProcessId << std::endl;
				TerminateProcess( hProcess, 0 );

				keep_wait=false;
			}
			else if ( process_check_for_hotkey( opt, pause_is ) )
			{
				FeLog() << " - Pause Hotkey pressed, pausing process: " << (int)dwProcessId << std::endl;

				HWND fgw = GetForegroundWindow();
				ShowWindow( fgw, SW_FORCEMINIMIZE );
				FeDebug() << "Force minimize window: " << fgw << std::endl;

				// Sleep to let the other process deal with the hiding of its window
				// before we suspend it
				sf::sleep( sf::milliseconds( 600 ) );

				// Undocumented windows system call
				NtSuspendProcess pfn_NtSuspendProcess = (NtSuspendProcess)GetProcAddress(
					GetModuleHandle( "ntdll" ), "NtSuspendProcess" );

				pfn_NtSuspendProcess( hProcess );

				opt->running_wnd = (void *)fgw;
				opt->running_pid = dwProcessId;

				keep_wait=false;
			}
			break;

		default:
			FeLog() << "Unexpected failure waiting on process" << std::endl;
			keep_wait=false;
			break;
		}
	}
}

#else

void unix_wait_process( unsigned int pid, run_program_options_class *opt )

{
	int status;
	int k = ( opt->exit_hotkey.empty() && opt->pause_hotkey.empty() )
		? 0 : WNOHANG; // option for waitpid.  0= wait for process to complete, WNOHANG=return right away

	FeInputMapEntry exit_is( opt->exit_hotkey );
	FeInputMapEntry pause_is( opt->pause_hotkey );

	while (1)
	{
		pid_t w = waitpid( pid, &status, k );
		if ( w == 0 )
		{
			// waitpid should only return 0 if WNOHANG is used and the child is still running, so we
			// should only ever get here if there is an hotkey provided
			//
			if ( process_check_for_hotkey( opt, exit_is ) )
			{
				// Where the user has configured the "exit hotkey" in Attract-Mode to the same key as the emulator
				// uses to exit, we often have a problem of losing focus.  Delaying a bit and testing to make sure
				// the emulator process is still running before sending the kill signal seems to help...
				//
				sf::sleep( sf::milliseconds( 100 ) );
				if ( kill( pid, 0 ) == 0 )
				{
					FeLog() << " - Exit Hotkey pressed, sending SIGTERM signal to process " << pid << std::endl;
					kill( pid, SIGTERM );

					//
					// Give the process TERM_TIMEOUT ms to respond to sig term
					//
					const int TERM_TIMEOUT = 1500;
					sf::Clock term_clock;

					while (( term_clock.getElapsedTime().asMilliseconds() < TERM_TIMEOUT )
						&& ( waitpid( pid, &status, WNOHANG ) == 0 ))
					{
						sf::sleep( sf::milliseconds( POLL_FOR_EXIT_MS ) );
					}

					//
					// Do the more abrupt SIGKILL if process is still running at this point
					//
					if ( kill( pid, 0 ) == 0 )
					{
						FeLog() << " - Timeout on SIGTERM after " << TERM_TIMEOUT
							<< " ms, sending SIGKILL signal to process " << pid << std::endl;

						kill( pid, SIGKILL );
					}

					// reap
					waitpid( pid, &status, 0 );
				}

				break; // leave do/while loop
			}
			else if ( process_check_for_hotkey( opt, pause_is ) )
			{
				FeLog() << " - Pause Hotkey pressed, sending SIGSTOP signal to process " << pid << std::endl;

				// TODO: OS X - implement finding and hiding of foreground window
#if defined( USE_XLIB )
				Window wnd( 0 );
				int revert;

				::Display *xdisp = XOpenDisplay( NULL );
				XGetInputFocus( xdisp, &wnd, &revert );
				opt->running_wnd = (void *)wnd;

				XUnmapWindow( xdisp, wnd );
				XFlush( xdisp );
				FeDebug() << "Unmapped window: " << wnd << std::endl;

				// Sleep to let the other process deal with the unmapping of its window
				// before we SIGSTOP it
				sf::sleep( sf::milliseconds( 600 ) );
#endif
				opt->running_pid = pid;
				kill( pid, SIGSTOP );
				break; // leave do/while loop
			}

			sf::sleep( sf::milliseconds( POLL_FOR_EXIT_MS ) );
		}
		else
		{
			if ( w < 0 )
				FeLog() << " ! error returned by wait_pid(): "
					<< strerror( errno ) << std::endl;

			break; // leave do/while loop
		}
	}
}
#endif

bool run_program( const std::string &prog,
	const std::string &args,
	const std::string &cwork_dir,
	output_callback_fn callback,
	void *opaque,
	bool block,
	run_program_options_class *opt )
{

	std::string work_dir = cwork_dir;
	if ( work_dir.empty() )
	{
		// If no working directory provided, try to divine one from the program name
		//
		size_t pos = prog.find_last_of( "/\\" );
		if ( pos != std::string::npos )
			work_dir = prog.substr( 0, pos );
	}

	run_program_options_class default_opt;
	if ( !opt )
		opt = &default_opt;

#ifdef SFML_SYSTEM_WINDOWS
	HANDLE child_output_read=NULL;
	HANDLE child_output_write=NULL;

	SECURITY_ATTRIBUTES satts;
	satts.nLength = sizeof( SECURITY_ATTRIBUTES );
	satts.bInheritHandle = TRUE;
	satts.lpSecurityDescriptor = NULL;

	STARTUPINFOW si;
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

	std::basic_string<wchar_t> comstr( widen( prog ) );
	comstr += L" ";
	comstr += widen( args );

	LPWSTR cmdline = new wchar_t[ comstr.length() + 1 ];
	wcsncpy( cmdline, comstr.c_str(), comstr.length() + 1 );

	DWORD current_wd_len = GetCurrentDirectoryW( 0, NULL );
	LPWSTR current_wd = new wchar_t[ current_wd_len ];
	GetCurrentDirectoryW( current_wd_len, current_wd );
	SetCurrentDirectoryW( widen( work_dir ).c_str() );

	bool ret = CreateProcessW( NULL,
		cmdline,
		NULL,
		NULL,
		( NULL == callback ) ? FALSE : TRUE,
		CREATE_NO_WINDOW,
		NULL,
		NULL, // use current directory (set above) as working directory for the process
		&si,
		&pi );

	SetCurrentDirectoryW( current_wd );
	delete [] current_wd;

	// Parent process - close the child write handle after child created
	if ( child_output_write )
		CloseHandle( child_output_write );

	//
	// Cleanup our allocated values now
	//
	delete [] cmdline;

	if ( ret == false )
	{
		FeLog() << "Error executing command: '" << narrow( comstr ) << "'" << std::endl;
		return false;
	}

	if (( NULL != callback ) && ( block ))
	{
		const int BUFF_SIZE = 2048;
		char buffer[ BUFF_SIZE*2 ];
		char *ibuf=buffer;
		DWORD bytes_read;

		while ( block && ( ReadFile( child_output_read, ibuf, BUFF_SIZE-1, &bytes_read, NULL ) != 0 ))
		{
			ibuf[bytes_read]=0;

			// call the callback - do this line by line for consistency w/ linux handling
			// newline character at end of string is preserved (see `man getline`)
			//
			char *obuf = buffer;
			char *c = strchr( obuf, '\n' );

			while ( c || ( strlen( obuf ) > BUFF_SIZE-1 ) )
			{
				int olen = BUFF_SIZE-1;
				if ( c )
					olen = c - obuf + 1;

				char rep = obuf[olen];
				obuf[olen] = 0;

				if ( callback( obuf, opaque ) == false )
				{
					TerminateProcess( pi.hProcess, 0 );
					block=false;
					break;
				}

				obuf[olen] = rep;
				obuf += olen;
				c = strchr( obuf, '\n' );
			}

			// deal with leftovers
			if ( strlen( obuf ) > 0 )
			{
				int sl = strlen( obuf );
				strncpy( buffer, obuf, sl );
				buffer[ sl ] =0;
				ibuf = buffer + sl;
			}
			else
				ibuf = buffer;
		}

		// handle last bit of data if not terminated with a line ending
		if ( block && ( ibuf != buffer ) )
		{
			if ( callback( buffer, opaque ) == false )
				TerminateProcess( pi.hProcess, 0 );
		}
	}

	if ( child_output_read )
		CloseHandle( child_output_read );

	if ( opt->launch_cb )
		opt->launch_cb( opt->launch_opaque );

	if ( block )
		windows_wait_process( pi.hProcess, pi.dwProcessId, opt );

	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

#else
	wordexp_t we;

	int mypipe[2] = { 0, 0 }; // mypipe[0] = read end, mypipe[1] = write end
	if (( NULL != callback ) && block && ( pipe( mypipe ) ))
		FeLog() << "Error, pipe() failed" << std::endl;

	pid_t pid = fork();
	switch (pid)
	{
	case -1:
		FeLog() << "Error, fork() failed" << std::endl;
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

		if ( !work_dir.empty() && ( chdir( work_dir.c_str() ) != 0 ) )
			FeLog() << "Warning, chdir(" << work_dir << ") failed." << std::endl;

		wordexp( prog.c_str(), &we, 0 );

		// according to the manual page, we.we_wordv is always null terminated, so we can feed it
		// right into execvp
		if ( wordexp( args.c_str(), &we, WRDE_APPEND ) != 0 )
			FeLog() << "Parameter word expansion failed. [" << args << "]." << std::endl;

		// Provide some debugging output for what we are doing
		if ( callback == NULL )
		{
			FeDebug() << "execvp(): file='" << prog.c_str() << "', argv=";
			for ( int r=0; we.we_wordv[r]; r++ )
				FeDebug() << "[" << we.we_wordv[r] << "]";
			FeDebug() << std::endl;
		}

		execvp( prog.c_str(), we.we_wordv );

		// execvp doesn't return unless there is an error.
		FeLog() << "Error executing: " << prog << " " << args << std::endl;

		wordfree( &we );

		_exit(127);

	default: // parent process

		if ( mypipe[0] )
		{
			FILE *fp = fdopen( mypipe[0], "r" );
			close( mypipe[1] );

			const int BUFF_SIZE = 2048;
			char buffer[ BUFF_SIZE ];

			while( fgets( buffer, BUFF_SIZE, fp ) != NULL )
			{
				if ( callback( buffer, opaque ) == false )
				{
					// User cancelled
					kill_program( pid );
					block=false;
					break;
				}
			}

			fclose( fp );
			close( mypipe[0] );
		}

		if ( opt->launch_cb )
			opt->launch_cb( opt->launch_opaque );

		if ( block )
			unix_wait_process( pid, opt );
	}
#endif // SFML_SYSTEM_WINDOWS
	return true;
}

void resume_program(
	unsigned int pid,
	void *wnd,
	run_program_options_class *opt )
{
#if defined( SFML_SYSTEM_WINDOWS )
	HANDLE hp = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );

	// Undocumented Windows system call
	NtResumeProcess pfn_NtResumeProcess = (NtResumeProcess)GetProcAddress(
		GetModuleHandle( "ntdll" ), "NtResumeProcess" );

	pfn_NtResumeProcess( hp );

	sf::sleep( sf::milliseconds( 600 ) );

	HWND w = (HWND)wnd;
	ShowWindow( w, SW_RESTORE );
	SetForegroundWindow( w );

	// We do this to process messages on the frontend's window
	// (allowing it to adjust to the foreground window change ??)
	//
	// In any event, this seems to be necessary...
	if ( opt->wait_cb )
		opt->wait_cb( opt->launch_opaque );

	windows_wait_process( hp, (DWORD)pid, opt );
	CloseHandle( hp );
#else
	// TODO: OS X - implement setting of foreground window
#if defined ( USE_XLIB )
	set_x11_foreground_window( (unsigned long)wnd );
#endif

	kill( pid, SIGCONT );

	if ( opt->launch_cb )
		opt->launch_cb( opt->launch_opaque );

	unix_wait_process( pid, opt );
#endif
}

void kill_program( unsigned int pid )
{
#if defined( SFML_SYSTEM_WINDOWS )
	HANDLE hp = OpenProcess( PROCESS_TERMINATE, FALSE, pid );

	if ( hp )
		TerminateProcess( hp, 0 );

	CloseHandle( hp );
#else
	kill( pid, SIGKILL );

	// reap
	int status;
	waitpid( pid, &status, 0 );
#endif
}

bool process_exists(
	unsigned int pid )
{
#if defined( SFML_SYSTEM_WINDOWS )
	HANDLE pss = CreateToolhelp32Snapshot( TH32CS_SNAPALL, 0 );
	PROCESSENTRY32 pe = {0};
	pe.dwSize = sizeof( pe );

	if ( Process32First( pss, &pe ) )
	{
		do
		{
			if ( pe.th32ProcessID == pid )
				return true;
		}
		while ( Process32Next( pss, &pe ) );
	}

	CloseHandle( pss );
	return false;
#else
	return ( kill( pid, 0 ) == 0 );
#endif
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

#if defined(USE_XLIB)
//
// We use this in fe_window but implement it here because the XWindows
// namespace (Window, etc) clashes with the SFML namespace used in fe_window
// (sf::Window)
//
void get_x11_geometry( bool multimon, int &x, int &y, int &width, int &height )
{
	x=0;
	y=0;
	width=0;
	height=0;

	::Display *xdisp = XOpenDisplay( NULL );
	int num=0;

#ifdef USE_XINERAMA
	XineramaScreenInfo *si=XineramaQueryScreens( xdisp, &num );

	if ( multimon )
	{
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
	}
	else
	{
		width = si[0].width;
		height = si[0].height;
	}

	XFree( si );
#else
	::Screen *xscreen = XDefaultScreenOfDisplay( xdisp );
	width = XWidthOfScreen( xscreen );
	height = XHeightOfScreen( xscreen );
#endif

	XCloseDisplay( xdisp );
}

void set_x11_foreground_window( unsigned long w )
{
	::Display *xdisp = XOpenDisplay( NULL );
	Window wnd = (Window)w;

	// Note: order seems to matter here!
	// (if 'set input focus' is before 'raise window' the window doesn't actually get raised !?)
	// We used XMapRaised() because in some cases we have unmapped the window being raised
	XMapRaised( xdisp, wnd );

	XSetInputFocus( xdisp, wnd, RevertToParent, CurrentTime );
	XFlush( xdisp );

	FeDebug() << "Raised and changed window input focus to: " << (unsigned long)wnd << std::endl;
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

std::string newline_escape( const std::string &raw )
{
	std::string temp;
	size_t pos1=0, pos=0;

	while ( ( pos = raw.find( "\n", pos1 ) ) != std::string::npos )
	{
		temp += raw.substr( pos1, pos-pos1 );
		temp += "\\n";
		pos1 = pos+1;
	}
	temp += raw.substr( pos1 );

	return temp;
}

void remove_trailing_spaces( std::string &str )
{
	str.erase( str.find_last_not_of( " \n\r\t" )+1 );
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

bool get_console_stdin( std::string &str )
{
//
// TODO: Implement non-blocking console input read on Windows
// PeekNamedPipe() and ReadFile() ??
//
#ifndef SFML_SYSTEM_WINDOWS
	int count=0;
	ioctl( fileno(stdin), FIONREAD, &count );

	while ( count > 0 )
	{
		char buf[count+1];
		if ( read( fileno(stdin), buf, count ) < 0 )
			break;

		buf[count]=0;
		str += buf;

		ioctl( fileno(stdin), FIONREAD, &count );
	}
#endif

	return ( !str.empty() );
}

std::string get_focus_process()
{
	std::string retval( "Unknown" );

#if defined( SFML_SYSTEM_WINDOWS )
	HWND focus_wnd = GetForegroundWindow();
	if ( !focus_wnd )
		return "None";

	DWORD focus_pid( 0 );
	GetWindowThreadProcessId( focus_wnd, &focus_pid );

	HANDLE snap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if ( snap == INVALID_HANDLE_VALUE )
	{
		FeLog() << "Could not get process snapshot." << std::endl;
		return retval;
	}

	PROCESSENTRY32 pei;
	pei.dwSize = sizeof( pei );
	BOOL np = Process32First( snap, &pei );
	while ( np )
	{
		if ( pei.th32ProcessID == focus_pid )
		{
			retval = pei.szExeFile;
			break;
		}
		np = Process32Next( snap, &pei );
	}

	CloseHandle( snap );
	retval += " (" + as_str( (int)focus_pid ) + ")";

#elif defined( USE_XLIB )

	::Display *xdisp = XOpenDisplay( NULL );
	Atom _NET_WM_PID = XInternAtom( xdisp, "_NET_WM_PID", True );

	Window wnd( 0 );
	int revert;
	XGetInputFocus( xdisp, &wnd, &revert );
	if ( wnd == PointerRoot )
		return "Pointer Root";
	else if ( wnd == None )
		return "None";

	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	unsigned char *prop = NULL;

	if ( XGetWindowProperty( xdisp,
				wnd,
				_NET_WM_PID,
				0,
				1024,
				False,
				AnyPropertyType,
				&actual_type,
				&actual_format,
				&nitems,
				&bytes_after,
				&prop ) != Success )
	{
		FeDebug() << "Could not get window property." << std::endl;
		return retval;
	}

	if ( !prop )
	{
		FeDebug() << "Empty window property." << std::endl;
		return retval;
	}

	int pid = prop[1] * 256;
	pid += prop[0];
	XFree( prop );

	// Try to get the actual name for the process id
	//
	// Note: this is Linux specific
	//
	std::string fn = "/proc/" + as_str( pid ) + "/cmdline";
	nowide::ifstream myfile( fn.c_str() );
	if ( myfile.good() )
	{
		getline( myfile, retval );
		myfile.close();
	}

	retval += " (" + as_str( pid ) + ")";

#endif

	return retval;
}

