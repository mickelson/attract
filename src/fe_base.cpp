/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-2018 Andrew Mickelson
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

#ifndef NO_MOVIE
extern "C"
{
#include <libavutil/log.h>
}
#endif

#ifndef NO_SWF
#include "gameswf/gameswf.h"
#endif

#include <iomanip>
#include "nowide/fstream.hpp"
#include "nowide/iostream.hpp"

#define FE_NAME_D			"Attract-Mode"

const char *FE_NAME			= FE_NAME_D;
const char *FE_COPYRIGHT		= FE_NAME_D " " FE_VERSION_D \
	" Copyright (c) 2013-2018 Andrew Mickelson";
const char *FE_VERSION 			= FE_VERSION_D;

const char *FE_WHITESPACE=" \t\r";
const char *FE_DIR_TOKEN				= "<DIR>";

const char *FE_DEFAULT_ARTWORK		= "snap";

const char *FE_EMULATOR_SUBDIR		= "emulators/";
const char *FE_EMULATOR_TEMPLATES_SUBDIR	= "emulators/templates/";
const char *FE_EMULATOR_FILE_EXTENSION	= ".cfg";
const char *FE_EMULATOR_DEFAULT		= "default-emulator.cfg";

namespace {
	nowide::ofstream g_logfile;
#ifdef SFML_SYSTEM_WINDOWS
	nowide::ofstream g_nullstream( "NUL" );
	double fe_os_version;
#else
	nowide::ofstream g_nullstream( "/dev/null" );
#endif
	enum FeLogLevel g_log_level=FeLog_Info;

#ifndef NO_MOVIE
	void ffmpeg_log_callback( void *ptr, int level, const char *fmt, va_list vargs )
	{
		if ( level <= av_log_get_level() )
		{
			char buff[256];
			vsnprintf( buff, 256, fmt, vargs );
			FeLog() << "FFmpeg: " << buff;
		}
	}
#endif

	void gs_log_callback( bool error, const char *message )
	{
		FeLog() << "gameswf: " << message;
	}
};

std::ostream &FeDebug()
{
	if ( g_log_level == FeLog_Debug )
		return FeLog();
	else
		return g_nullstream;
}

std::ostream &FeLog()
{
	if ( g_log_level == FeLog_Silent )
		return g_nullstream;

	if ( g_logfile.is_open() )
		return g_logfile;
	else
		return nowide::cout;
}

void fe_set_log_file( const std::string &fn )
{
	if ( fn.empty() )
		g_logfile.close();
	else
		g_logfile.open( fn.c_str() );
}

void fe_set_log_level( enum FeLogLevel f )
{
	g_log_level = f;


#ifndef NO_MOVIE
	if ( f == FeLog_Silent )
		av_log_set_callback( NULL );
	else
	{
		av_log_set_callback( ffmpeg_log_callback );
		av_log_set_level( ( f == FeLog_Debug ) ? AV_LOG_VERBOSE : AV_LOG_ERROR );
	}
#endif
#ifndef NO_SWF
	if ( f == FeLog_Silent )
		gameswf::register_log_callback( NULL );
	else
	{
		gameswf::register_log_callback( gs_log_callback );

		gameswf::set_verbose_action( f == FeLog_Debug );
	}
#endif
}

void fe_print_version()
{
#ifdef SFML_SYSTEM_WINDOWS
	OSVERSIONINFOEX info;
	ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFO)&info);
	fe_os_version = info.dwMajorVersion 
			+ ( info.dwMinorVersion / 10.0 );

	FeDebug() << "Windows version: " << fe_os_version << std::endl;
#endif

	FeLog() << FE_NAME << " " << FE_VERSION << " ("
		<< get_OS_string()
		<< ", SFML " << SFML_VERSION_MAJOR << '.' << SFML_VERSION_MINOR
#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 ) )
		<< "." << SFML_VERSION_PATCH
#endif
#ifdef USE_FONTCONFIG
		<< " +FontConfig"
#endif
#ifdef USE_XINERAMA
		<< " +Xinerama"
#endif
#ifdef USE_GLES
		<< " +GLES"
#endif
#ifndef NO_SWF
		<< " +SWF"
#endif
#ifdef USE_LIBARCHIVE
		<< " +7z"
#endif
#ifdef USE_LIBCURL
		<< " +Curl"
#endif
		<< ") " << std::endl;
#ifdef NO_MOVIE
	FeLog() << "No Video, using SFML for Audio." << std::endl;
#else
	print_ffmpeg_version_info();
#endif

}

#ifdef SFML_SYSTEM_WINDOWS
bool fe_is_dwmapi_available()
{
	// Return true if we are running on
	// Windows Vista or above.
	// This function is used to decide
	// to whether DwmFlush() or not
	if ( fe_os_version >= 6.0 )
		return true;
	else
		return false;
}
#endif

void FeBaseConfigurable::invalid_setting(
	const std::string & fn,
	const char *base,
	const std::string &setting,
	const char **valid1,
	const char **valid2,
	const char *label )
{
	FeLog() << "Unrecognized \"" << base << "\" " << label
		<< " of \"" << setting << "\"";

	if ( !fn.empty() )
		FeLog() << " in file: " << fn;

	FeLog() << ".";

	int i=0;
	if ( valid1 && valid1[i] )
	{
		FeLog() << "  Valid " << label <<"s are: " << valid1[i++];

		while (valid1[i])
			FeLog() << ", " << valid1[i++];
	}

	if ( valid2 )
	{
		i=0;
		while (valid2[i])
			FeLog() << ", " << valid2[i++];
	}

	FeLog() << std::endl;
}

bool FeBaseConfigurable::load_from_file( const std::string &filename,
	const char *sep )
{
   nowide::ifstream myfile( filename.c_str() );

   if ( !myfile.is_open() )
		return false;

	const int DEBUG_MAX_LINES=200;
	int count=0;

	while ( myfile.good() )
	{
		std::string line, setting, value;
		getline( myfile, line );

		if ( line_to_setting_and_value( line, setting, value, sep ) )
		{
			if (( g_log_level == FeLog_Debug ) && ( count <= DEBUG_MAX_LINES ))
			{
				FeDebug() << "[" << filename <<  "] " << std::setw(15) << std::left << setting
					<< " = " << value << std::endl;

				if ( count == DEBUG_MAX_LINES )
					FeDebug() << "[" << filename <<  "] DEBUG_MAX_LINES exceeded, truncating further debug output from this file." << std::endl;

				count++;
			}

			process_setting( setting, value, filename );
		}
	}

	myfile.close();
	return true;
}
