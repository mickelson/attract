/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2015 Andrew Mickelson
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

#include "zip.hpp"
#include "fe_util.hpp"
#include "fe_base.hpp"
#include <iostream>
#include <cstring>
#include <mutex>

typedef void *(*FE_ZIP_ALLOC_CALLBACK) ( size_t );

namespace
{
	//
	// We keep a cache of zip file content results, with the cache
	// growing up to CONTENT_CACHE_SIZE entries max
	//
	const int CONTENT_CACHE_SIZE = 8;
	std::vector < std::pair < std::string, std::vector < std::string > > > g_ccache;
	std::recursive_mutex g_ccache_mutex;

	bool check_content_cache( const std::string &archive,
			std::vector < std::string > &contents )
	{
		size_t i=0;
		for ( i=0; i < g_ccache.size(); i++ )
		{
			if ( g_ccache[i].first.compare( archive ) == 0 )
			{
				// Cache hit
				//
				contents = g_ccache[i].second; // vector copy

				//
				// Promote hit to the top of the cache
				//
				for ( int j=i; j>0; j-- )
				{
					g_ccache[j].first.swap( g_ccache[j-1].first );
					g_ccache[j].second.swap( g_ccache[j-1].second );
				}

				return true;
			}
		}
		return false;
	}

	void add_to_content_cache( const std::string &archive,
			std::vector < std::string > &contents )
	{
		std::pair< std::string, std::vector< std::string > > temp;
		temp.first = archive;
		temp.second = contents; // vector copy

		if ( g_ccache.size() < CONTENT_CACHE_SIZE )
			g_ccache.push_back(
				std::pair< std::string, std::vector< std::string > >() );

		g_ccache.back().first.swap( temp.first );
		g_ccache.back().second.swap( temp.second );

		for ( size_t i=g_ccache.size()-1; i > 0; i-- )
		{
			g_ccache[i].first.swap( g_ccache[i-1].first );
			g_ccache[i].second.swap( g_ccache[i-1].second );
		}

		//
		// Result is the new entry at the front of the cache
		//
	}

	// Count the number of path separators ('/' or '\') in path
	//
	int count_path_seps( const std::string &path )
	{
		int count( 0 );
		size_t pos( 0 );

		while ( pos != std::string::npos )
		{
			pos = path.find_first_of( "/\\", pos );
			if ( pos != std::string::npos )
			{
				count++;
				pos++;
			}
		}

		return count;
	}

	// Return the string position that is immediately after the last
	// path separator in "path", after skipping "extra_seps" path
	// separators.
	//
	// returns 0 if there aren't enough separators in the string
	//
	size_t get_pos_from_back( const std::string &path, int extra_seps )
	{
		int i=0;
		size_t pos( std::string::npos );

		while ( i < extra_seps+1 )
		{
			pos = path.find_last_of( "/\\", pos );
			if ( pos == std::string::npos )
				return 0;

			i++;
			pos--;
		}

		return pos+2;
	}
};

#ifdef USE_LIBARCHIVE

#include "archive.h"
#include "archive_entry.h"

namespace
{
	struct archive *my_archive_init()
	{
		struct archive *a = archive_read_new();

		archive_read_support_filter_gzip( a );
		archive_read_support_filter_bzip2( a );
		archive_read_support_format_7zip( a );
		archive_read_support_format_rar( a );
		archive_read_support_format_tar( a );
		archive_read_support_format_zip( a );

		return a;
	}
};

bool fe_zip_open_to_buff(
	const char *arch,
	const char *filename,
	std::vector< char > &buff )
{
	struct archive *a = my_archive_init();
	int r = archive_read_open_filename( a, arch, 8192 );

	if ( r != ARCHIVE_OK )
	{
		FeLog() << "Error opening archive: "
			<< arch << std::endl;
		archive_read_free( a );
		return false;
	}

	struct archive_entry *ae;

	std::string fn = filename;
	while ( archive_read_next_header( a, &ae ) == ARCHIVE_OK )
	{
		if ( fn.compare( archive_entry_pathname( ae ) ) == 0 )
		{
			size_t total = archive_entry_size( ae );

			buff.resize( total );
			archive_read_data( a, &(buff[0]), buff.size() );
			archive_read_free( a );
			return true;
		}
	}

	archive_read_free( a );
	return false;
}

bool fe_zip_get_dir(
	const char *archive,
	std::vector<std::string> &result )
{
	std::lock_guard<std::recursive_mutex> l( g_ccache_mutex );
	if ( check_content_cache( archive, result ) )
		return true;

	struct archive *a = my_archive_init();
	int r = archive_read_open_filename( a, archive, 8192 );

	if ( r != ARCHIVE_OK )
	{
		FeLog() << "Error opening archive: "
			<< archive << std::endl;
		archive_read_free( a );
		return false;
	}

	struct archive_entry *ae;

	while ( archive_read_next_header( a, &ae ) == ARCHIVE_OK )
		result.push_back( archive_entry_pathname( ae ) );

	archive_read_free( a );

	add_to_content_cache( archive, result );
	return true;
}

#else

#include "miniz.c"

bool fe_zip_open_to_buff(
	const char *archive,
	const char *filename,
	std::vector< char > &buff )
{
	mz_zip_archive zip;
	memset( &zip, 0, sizeof( zip ) );

	if ( !mz_zip_reader_init_file( &zip, archive, 0 ) )
	{
		FeLog() << "Error initializing zip.  zip: "
			<< archive << std::endl;
		return false;
	}

	int index = mz_zip_reader_locate_file( &zip,
		filename, NULL, 0 );
	if ( index < 0 )
	{
		mz_zip_reader_end( &zip );
		return false;
	}

	mz_zip_archive_file_stat file_stat;
	if ( !mz_zip_reader_file_stat(&zip, index, &file_stat) )
	{
		FeLog() << "Error reading filestats. zip: "
			<< archive << ", file: " << filename << std::endl;
		mz_zip_reader_end( &zip );
		return false;
	}

	buff.resize( file_stat.m_uncomp_size );

	if ( !mz_zip_reader_extract_to_mem( &zip,
		index, &(buff[0]), buff.size(), 0 ) )
	{
		FeLog() << "Error extracting to buffer. zip: "
			<< archive << ", file: " << filename << std::endl;
		mz_zip_reader_end( &zip );
		return false;
	}

	mz_zip_reader_end( &zip );
	return true;
}

bool fe_zip_get_dir(
	const char *archive,
	std::vector<std::string> &result )
{
	std::lock_guard<std::recursive_mutex> l( g_ccache_mutex );
	if ( check_content_cache( archive, result ) )
		return true;

	mz_zip_archive zip;
	memset( &zip, 0, sizeof( zip ) );

	if ( !mz_zip_reader_init_file( &zip, archive, 0 ) )
	{
		FeLog() << "Error initializing zip: "
			<< archive << std::endl;
		return false;
	}

	for ( int i=0; i<(int)mz_zip_reader_get_num_files(&zip); i++)
	{
		mz_zip_archive_file_stat file_stat;
		if ( mz_zip_reader_file_stat(&zip, i, &file_stat) )
			result.push_back( file_stat.m_filename );
	}

	mz_zip_reader_end( &zip );

	add_to_content_cache( archive, result );
	return true;
}

#endif // USE_LIBARCHIVE

const char *FE_ARCHIVE_EXT[] =
{
	".zip",
#ifdef USE_LIBARCHIVE
	".rar",
	".7z",
	".tar.gz",
	".tgz",
	".tar.bz2",
	".tbz2",
	".tar",
#endif
	NULL
};

bool is_supported_archive( const std::string &fname )
{
	return tail_compare( fname, FE_ARCHIVE_EXT );
}

void *zip_stream_alloc_cb( size_t s )
{
	return (void *)(new char[s]);
}

FeZipStream::FeZipStream()
	: m_pos( 0 )
{
}

FeZipStream::FeZipStream( const std::string &archive )
	: m_archive( archive ),
	m_pos( 0 )
{
}

FeZipStream::~FeZipStream()
{
	clear();
}

void FeZipStream::clear()
{
	m_data.resize( 0 );
	m_pos = 0;
}

bool FeZipStream::open( const std::string &filename )
{
	clear();

	return fe_zip_open_to_buff(
		m_archive.c_str(),
		filename.c_str(),
		m_data );
}

sf::Int64 FeZipStream::read( void *data, sf::Int64 size )
{
	if ( m_data.empty() )
		return -1;

	sf::Int64 end_pos = m_pos + size;
	size_t count = ( end_pos <= (sf::Int64)m_data.size() )
		? size : ( m_data.size() - m_pos );

	if ( count > 0 )
	{
		memcpy( data, &(m_data[m_pos]), count );
		m_pos += count;
	}

	return count;
}

sf::Int64 FeZipStream::seek( sf::Int64 position )
{
	if ( m_data.empty() )
		return -1;

	m_pos = ( position < (sf::Int64)m_data.size() ) ? position : m_data.size();
	return m_pos;
}

sf::Int64 FeZipStream::tell()
{
	if ( m_data.empty() )
		return -1;

	return m_pos;
}

sf::Int64 FeZipStream::getSize()
{
	if ( m_data.empty() )
		return -1;

	return m_data.size();
}

void FeZipStream::setArchive( const std::string &archive )
{
	clear();
	m_archive = archive;
}

char *FeZipStream::getData()
{
	return &(m_data[0]);
}

void gather_archive_filenames_with_base(
	std::vector < std::string > &in_list,
	std::vector < std::string > &out_list,
	const std::string &archive,
	const std::string &basename,
	const char **exts )
{
	// Need to support basenames that contain path separators.
	int sep_count = count_path_seps( basename );

	std::vector<std::string> wl;
	fe_zip_get_dir( archive.c_str(), wl );
	for ( std::vector<std::string>::iterator itr=wl.begin();
		itr!=wl.end(); ++itr )
	{
		size_t pos = get_pos_from_back( *itr, sep_count );

		if ( icompare( (*itr).substr( pos, basename.size() ),
				basename ) == 0 )
		{
			if ( !exts )
				in_list.push_back( *itr );
			else
			{
				if ( tail_compare( *itr, exts ) )
					in_list.push_back( *itr );
				else
					out_list.push_back( *itr );
			}

		}

	}
}

bool get_archive_filename_with_base(
	std::string &filename,
	const std::string &archive,
	const std::string &basename,
	const char **exts )
{
	std::vector < std::string > t1;
	std::vector < std::string > t2;

	gather_archive_filenames_with_base( t1, t2, archive, basename, exts );

	if ( !t1.empty() )
	{
		filename = t1.front();
		return true;
	}

	return false;
}
