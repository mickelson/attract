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
#include <iostream>

typedef void *(*FE_ZIP_ALLOC_CALLBACK) ( size_t );

#ifdef USE_LIBARCHIVE

#include "archive.h"
#include "archive_entry.h"
#include <cstring>

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
	const char *archive,
	const char *filename,
	FE_ZIP_ALLOC_CALLBACK callback,
	void **buff,
	size_t *buff_size )
{
	ASSERT( buff != NULL );

	struct archive *a = my_archive_init();
	int r = archive_read_open_filename( a, archive, 8192 );

	if ( r != ARCHIVE_OK )
	{
		std::cerr << "Error opening archive: "
			<< archive << std::endl;
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

			void *tb = callback( total );

			if ( tb == NULL )
			{
				std::cerr << "Error allocating archive buffer: "
					<< archive << ", file: " << filename << std::endl;
				break;
			}

			archive_read_data( a, tb, total );

			if ( buff_size )
				*buff_size = total;

			*buff = tb;

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
	struct archive *a = my_archive_init();
	int r = archive_read_open_filename( a, archive, 8192 );

	if ( r != ARCHIVE_OK )
	{
		std::cerr << "Error opening archive: "
			<< archive << std::endl;
		archive_read_free( a );
		return false;
	}

	struct archive_entry *ae;

	while ( archive_read_next_header( a, &ae ) == ARCHIVE_OK )
		result.push_back( archive_entry_pathname( ae ) );

	archive_read_free( a );
	return true;
}

#else

#include "miniz.c"

bool fe_zip_open_to_buff(
	const char *archive,
	const char *filename,
	FE_ZIP_ALLOC_CALLBACK callback,
	void **buff,
	size_t *buff_size )
{
	ASSERT( buff != NULL );

	mz_zip_archive zip;
	memset( &zip, 0, sizeof( zip ) );

	if ( !mz_zip_reader_init_file( &zip, archive, 0 ) )
	{
		std::cerr << "Error initializing zip.  zip: "
			<< archive << std::endl;
		return false;
	}

	int index = mz_zip_reader_locate_file( &zip,
		filename, NULL, 0 );
	if ( index < 0 )
	{
		std::cerr << "Error locating file. zip: "
			<< archive << ", file: " << filename << std::endl;
		mz_zip_reader_end( &zip );
		return false;
	}

	mz_zip_archive_file_stat file_stat;
	if ( !mz_zip_reader_file_stat(&zip, index, &file_stat) )
	{
		std::cerr << "Error reading filestats. zip: "
			<< archive << ", file: " << filename << std::endl;
		mz_zip_reader_end( &zip );
		return false;
	}

	void *tb = callback( file_stat.m_uncomp_size );

	if ( tb == NULL )
	{
		std::cerr << "Error allocating zip buffer. zip: "
			<< archive << ", file: " << filename << std::endl;
		mz_zip_reader_end( &zip );
		return false;
	}

	if ( !mz_zip_reader_extract_to_mem( &zip,
		index, tb, file_stat.m_uncomp_size, 0 ) )
	{
		std::cerr << "Error extracting to buffer. zip: "
			<< archive << ", file: " << filename << std::endl;
		mz_zip_reader_end( &zip );

		delete (char *)tb;
		return false;
	}

	mz_zip_reader_end( &zip );

	*buff = tb;

	if ( buff_size )
		*buff_size = file_stat.m_uncomp_size;

	return true;
}

bool fe_zip_get_dir(
	const char *archive,
	std::vector<std::string> &result )
{
	mz_zip_archive zip;
	memset( &zip, 0, sizeof( zip ) );

	if ( !mz_zip_reader_init_file( &zip, archive, 0 ) )
	{
		std::cerr << "Error initializing zip: "
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
	return true;
}

#endif // USE_LIBARCHIVE

void *zip_stream_alloc_cb( size_t s )
{
	return (void *)(new char[s]);
}

FeZipStream::FeZipStream()
	: m_data( NULL ),
	m_pos( 0 ),
	m_size( 0 )
{
}

FeZipStream::FeZipStream( const std::string &archive )
	: m_archive( archive ),
	m_data( NULL ),
	m_pos( 0 ),
	m_size( 0 )
{
}

FeZipStream::~FeZipStream()
{
	clear();
}

void FeZipStream::clear()
{
	if ( m_data )
		delete m_data;

	m_data = NULL;
	m_pos = 0;
	m_size = 0;
}

bool FeZipStream::open( const std::string &filename )
{
	clear();

	void *ptr=NULL;
	size_t sz=0;

	if ( fe_zip_open_to_buff(
		m_archive.c_str(),
		filename.c_str(),
		zip_stream_alloc_cb,
		&ptr,
		&sz ) )
	{
		m_data = (char *)ptr;
		m_size = sz;
		return true;
	}
	return false;
}

sf::Int64 FeZipStream::read( void *data, sf::Int64 size )
{
	if ( !m_data )
		return -1;

	sf::Int64 end_pos = m_pos + size;
	size_t count = ( end_pos <= m_size ) ? size : ( m_size - m_pos );

	if ( count > 0 )
	{
		memcpy( data, m_data + m_pos, count );
		m_pos += count;
	}

	return count;
}

sf::Int64 FeZipStream::seek( sf::Int64 position )
{
	if ( !m_data )
		return -1;

	m_pos = ( position < m_size ) ? position : m_size;
	return m_pos;
}

sf::Int64 FeZipStream::tell()
{
	if ( !m_data )
		return -1;

	return m_pos;
}

sf::Int64 FeZipStream::getSize()
{
	if ( !m_data )
		return -1;

	return m_size;
}

void FeZipStream::setArchive( const std::string &archive )
{
	clear();
	m_archive = archive;
}

char *FeZipStream::getData()
{
	return m_data;
}
