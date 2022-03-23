/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-2017 Andrew Mickelson
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

#include <android/log.h>
#include <android/native_activity.h>
#include <android/asset_manager.h>

#include <SFML/System/NativeActivity.hpp>
#include <unistd.h>
#include <string>
#include <sstream>
#include <thread>

#include "fe_util_android.hpp"
#include "fe_util.hpp"
#include "fe_base.hpp"

namespace {
	//
	// Send std::cout to the android debug log
	//
	class FeAndroidLogger
	{
	protected:
		int m_pfd[2];
		std::thread m_logthread;
		std::string m_line;

		void log_fn()
		{
			size_t rs;
			char buf[256];
			while ( (rs = read( m_pfd[0], buf, sizeof( buf )-1 )) > 0 )
			{
				if ( buf[ rs - 1 ] == '\n' )
				{
					--rs;
					buf[rs] = 0;

					m_line += buf;

					__android_log_write( ANDROID_LOG_DEBUG, "Attract-Mode", m_line.c_str() );
					m_line.clear();
				}
				else
				{
					buf[rs] = 0;
					m_line += buf;
				}

			}

			if ( !m_line.empty() )
				__android_log_write( ANDROID_LOG_DEBUG, "Attract-Mode2", m_line.c_str() );
		}

	public:
		FeAndroidLogger()
			: m_logthread( &FeAndroidLogger::log_fn, this )
		{
			setvbuf( stdout, 0, _IOLBF, 0 ); // stdout line buffered

			pipe( m_pfd );
			dup2( m_pfd[1], STDOUT_FILENO );
		}

		~FeAndroidLogger()
		{
		}

	};

	FeAndroidLogger g_logger;

	bool confirm_subdirectories( const std::string &base, const std::string &entry )
	{
		std::string work_base( base );
		size_t new_pos=0, old_pos=0;

		while ( old_pos != std::string::npos )
		{
			new_pos = entry.find_first_of( '/', old_pos );
			if ( new_pos != std::string::npos )
			{
				new_pos++;
				std::string new_sub = entry.substr( old_pos, new_pos-old_pos );
				if ( confirm_directory( work_base, new_sub ) )
					FeLog() << "Created directory: " << work_base << new_sub << std::endl;

				work_base += new_sub;
			}
			old_pos = new_pos;
		}
	}
};

std::string get_home_dir()
{
	ANativeActivity *na = sf::getNativeActivity();
	return na->internalDataPath;
}

void android_copy_assets()
{
	ANativeActivity *na = sf::getNativeActivity();

	//
	// Copy all assets listed in manifest.txt to our "internal data path" on android
	//
	std::string manifest_data;


	// Fill manifest_data
	//
	AAsset *manifest = AAssetManager_open( na->assetManager, "manifest.txt",
		AASSET_MODE_STREAMING );

	if ( !manifest )
	{
		FeLog() << "Error opening file manifest.  No assets copied." << std::endl;
		return;
	}

	char buf[1024];
	int nb_read = 0;

	while ((nb_read = AAsset_read( manifest, buf, 1024)) > 0 )
		manifest_data.append( buf, nb_read );

	AAsset_close( manifest );

	// Each line of manifest_data specifies an asset to copy
	//
	std::istringstream f( manifest_data );
	std::string entry;

	while ( std::getline( f, entry ) )
	{
		AAsset *asset = AAssetManager_open( na->assetManager, entry.c_str(),
			AASSET_MODE_STREAMING );

		if ( !asset )
			FeLog() << "Unable to open asset '" << entry << "'" << std::endl;
		else
		{
			std::string target = na->internalDataPath;
			target += "/";
			confirm_subdirectories( target, entry );

			target += entry;

			FILE *out = fopen( target.c_str(), "wb" );

			if ( !out )
				FeLog() << "Unable to open file for writing '" << target << "'" << std::endl;
			else
			{
				FeLog() << "Copying asset '" << entry << "' to '" << target << "'" << std::endl;
				while ((nb_read = AAsset_read( asset, buf, 1024)) > 0 )
					fwrite( buf, nb_read, 1, out );

				fclose(out);
			}
		}
	}
}
