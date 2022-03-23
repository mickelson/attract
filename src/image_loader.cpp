/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2020 Andrew Mickelson
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

#include <SFML/System/InputStream.hpp>

#include <list>
#include <map>
#include <queue>
#include <string>
#include <mutex>
#include <thread>
#include <chrono>

#include "fe_base.hpp" // logging
#include "fe_file.hpp"
#include "zip.hpp"

#ifndef NO_MOVIE
#include "media.hpp"
#endif

#include "image_loader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
	// stb_image callbacks that operate on a sf::InputStream
	int read(void* user, char* data, int size)
	{
		sf::InputStream* stream = static_cast<sf::InputStream*>(user);
		return static_cast<int>(stream->read(data, size));
	}
	void skip(void* user, int size)
	{
		sf::InputStream* stream = static_cast<sf::InputStream*>(user);
		stream->seek(stream->tell() + size);
	}
	int eof(void* user)
	{
		sf::InputStream* stream = static_cast<sf::InputStream*>(user);
		return stream->tell() >= stream->getSize();
	}
	std::recursive_mutex g_mutex;

#ifdef FE_DEBUG
	int g_entry_count=0;

	class EntryCountReporter
	{
	public:
		~EntryCountReporter()
		{
			FeLog() << "Image Loader Entry Count on exit: " << g_entry_count << std::endl;
		}
	};

	EntryCountReporter g_e_rep;
#endif
}

class FeImageLRUCache
{
public:
	typedef std::pair< std::string, FeImageLoaderEntry * > kvp_t;
	typedef std::list<kvp_t>::iterator list_iterator_t;
	typedef std::map< std::string, list_iterator_t >::iterator map_iterator_t;

	FeImageLRUCache( size_t max_bytes )
		: m_max_bytes( max_bytes ),
		m_current_bytes( 0 )
	{
	}

	~FeImageLRUCache()
	{
		while ( !m_items.empty() )
		{
			list_iterator_t last = m_items.end();
			--last;

			{
				std::lock_guard<std::recursive_mutex> l( g_mutex );
				if ( last->second->dec_ref() )
					delete last->second;
			}

			m_items_map.erase( last->first );
			m_items.pop_back();
		}
	}

	void put( const std::string &key, FeImageLoaderEntry *value )
	{
		m_items.push_front( kvp_t( key, value ) );
		value->add_ref();

		m_items_map[key] = m_items.begin();

		m_current_bytes += value->get_bytes();
		prune();
	}

	bool get( const std::string &key, FeImageLoaderEntry **val )
	{
		map_iterator_t it = m_items_map.find( key );
		if ( it != m_items_map.end() )
		{
			// promote
			m_items.splice( m_items.begin(), m_items, it->second );
			*val = it->second->second;
			return true;
		}

		return false;
	}

	void resize( size_t new_size )
	{
		m_max_bytes = new_size;
		prune();
	}

	size_t get_max_size() { return m_max_bytes; };
	size_t get_size() { return m_current_bytes; };
	size_t get_count() { return m_items.size(); };

	const char *get_name_at( int pos )
	{
		std::list< kvp_t >::iterator it = m_items.begin();
		std::advance( it, pos );

		return it->first.c_str();
	}

	int get_size_at( int pos )
	{
		std::list< kvp_t >::iterator it = m_items.begin();
		std::advance( it, pos );

		return it->second->get_bytes();
	}

private:
	void prune()
	{
		while ( !m_items.empty() && ( m_current_bytes > m_max_bytes ) )
		{
			list_iterator_t last = m_items.end();
			--last;

			size_t lsize = last->second->get_bytes();

			if ( lsize >= m_current_bytes )
				m_current_bytes = 0;
			else
				m_current_bytes -= lsize;

			{
				std::lock_guard<std::recursive_mutex> l( g_mutex );

				if ( last->second->dec_ref() )
					delete last->second;
			}

			m_items_map.erase( last->first );
			m_items.pop_back();
		}
	}

	std::list< kvp_t > m_items;
	std::map< std::string, list_iterator_t > m_items_map;
	size_t m_max_bytes;
	size_t m_current_bytes;
};



class FeImageLoaderThread
{
public:
	FeImageLoaderThread()
		: m_thread( &FeImageLoaderThread::run_thread, this ),
		m_run( true )
	{
	};

	~FeImageLoaderThread()
	{
		m_run=false;

		if ( m_thread.joinable() )
			m_thread.join();

		while ( !m_in.empty() )
		{
			std::lock_guard<std::recursive_mutex> l( g_mutex );
			if ( m_in.front().second->dec_ref() )
				delete m_in.front().second;

			m_in.pop();
		}
#ifndef NO_MOVIE
		while ( !m_vid.empty() )
		{
			delete m_vid.front();
			m_vid.pop();
		}
#endif
	}

	void run_thread()
	{
		while ( m_run )
		{
			std::pair< std::string, FeImageLoaderEntry * > e = get_next();
			if ( e.second )
			{
				int ignored;

				// Load image pixel data
				stbi_io_callbacks cb;
				cb.read = &read;
				cb.skip = &skip;
				cb.eof = &eof;

				unsigned char *data = stbi_load_from_callbacks( &cb, e.second->m_stream,
					&(e.second->m_width), &(e.second->m_height), &ignored, STBI_rgb_alpha );

				if ( !data )
					FeLog() << "Error loading image: " << e.first << " - " << stbi_failure_reason() << std::endl;

				{
					std::lock_guard<std::recursive_mutex> l( g_mutex );
					e.second->m_data = data;
					e.second->m_loaded = true;

					if ( e.second->dec_ref() )
						delete e.second;
				}
			}
			else
			{
#ifndef NO_MOVIE
				FeMedia *vid = get_vid_to_reap();
				if ( vid )
					delete vid;
				else
#endif
					std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
			}
		}
	}

	void add( const std::string &n, FeImageLoaderEntry *e )
	{
		std::lock_guard<std::recursive_mutex> l( g_mutex );
		e->add_ref(); // Add ref while we are loading it
		m_in.push( std::pair< std::string, FeImageLoaderEntry * >( n, e ) );
	}

#ifndef NO_MOVIE
	void reap_video( FeMedia *vid )
	{
		std::lock_guard<std::recursive_mutex> l( g_mutex );
		m_vid.push( vid );
	}
#endif

private:
	std::pair < std::string, FeImageLoaderEntry * > get_next()
	{
		std::lock_guard<std::recursive_mutex> l( g_mutex );
		if ( !m_in.empty() )
		{
			std::pair < std::string, FeImageLoaderEntry * > retval = m_in.front();
			m_in.pop();

			return retval;
		}
		return std::pair < std::string, FeImageLoaderEntry *>( "", NULL );
	}

#ifndef NO_MOVIE
	FeMedia *get_vid_to_reap()
	{
		std::lock_guard<std::recursive_mutex> l( g_mutex );
		if ( !m_vid.empty() )
		{
			FeMedia *retval = m_vid.front();
			m_vid.pop();
			return retval;
		}
		return NULL;
	}
#endif

	std::thread m_thread;
	bool m_run;

	std::queue< std::pair < std::string, FeImageLoaderEntry * > > m_in;
#ifndef NO_MOVIE
	std::queue< FeMedia * > m_vid;
#endif
};

class FeImageLoaderImp
{
public:
	FeImageLoaderImp()
		: m_cache( NULL ),
		m_load_images_in_bg( false )
	{
	};

	~FeImageLoaderImp()
	{
		if ( m_cache )
			delete m_cache;
	}

	FeImageLRUCache *m_cache;
	FeImageLoaderThread m_bg_loader;
	bool m_load_images_in_bg;
};

FeImageLoaderEntry::FeImageLoaderEntry( sf::InputStream *s )
		: m_stream( s ),
		m_ref_count( 0 ),
		m_width( 0 ),
		m_height( 0 ),
		m_data( NULL ),
		m_loaded( false )
{
#ifdef FE_DEBUG
	g_entry_count++;
#endif
}

FeImageLoaderEntry::~FeImageLoaderEntry()
{
#ifdef FE_DEBUG
	g_entry_count--;
#endif

	if ( m_data )
		stbi_image_free( m_data );

	if ( m_stream )
		delete m_stream;
}

size_t FeImageLoaderEntry::get_bytes()
{
	return m_width * m_height * 4;
}

unsigned char *FeImageLoaderEntry::get_data()
{
	return m_data;
}

int FeImageLoaderEntry::get_width()
{
	return m_width;
}

int FeImageLoaderEntry::get_height()
{
	return m_height;
}

void FeImageLoaderEntry::add_ref()
{
	m_ref_count++;
}

bool FeImageLoaderEntry::dec_ref()
{
	m_ref_count--;
	return ( m_ref_count == 0 );
}

FeImageLoader::FeImageLoader()
	: m_imp( NULL )
{
}

FeImageLoader::~FeImageLoader()
{
	if ( m_imp )
		delete m_imp;
}

bool FeImageLoader::load_image_from_file( const std::string &fn, FeImageLoaderEntry **e )
{
	sf::InputStream *fs = new FeFileInputStream( fn );
	return internal_load_image( fn, fs, e );
}

bool FeImageLoader::load_image_from_archive( const std::string &arch, const std::string &fn, FeImageLoaderEntry **e )
{
	FeZipStream *zs = new FeZipStream( arch );
	zs->open( fn );

	std::string key = arch + "|" + fn;
	return internal_load_image( key, zs, e );
}

bool FeImageLoader::internal_load_image( const std::string &key, sf::InputStream *stream, FeImageLoaderEntry **e )
{
	FeImageLoaderEntry *temp_e( NULL );

	// check if we already have it in the cache
	if ( m_imp->m_cache && m_imp->m_cache->get( key, &temp_e ) )
	{
		FeDebug() << "Image cache hit: " << key << std::endl;
		delete stream;

		std::lock_guard<std::recursive_mutex> l( g_mutex );
		temp_e->add_ref();
		*e = temp_e;
		return temp_e->m_loaded;
	}

	temp_e = new FeImageLoaderEntry( stream );

	// load image dimensions now
	stbi_io_callbacks cb;
	cb.read = &read;
	cb.skip = &skip;
	cb.eof = &eof;

	int retval=false;
	int ignored;
	bool err=false;
	if ( !m_imp->m_load_images_in_bg )
	{
		temp_e->m_data = stbi_load_from_callbacks( &cb, temp_e->m_stream,
			&(temp_e->m_width), &(temp_e->m_height), &ignored, STBI_rgb_alpha );

		temp_e->m_loaded = true;

		if ( !temp_e->m_data )
		{
			FeLog() << "Error loading image: " << key << " - " << stbi_failure_reason() << std::endl;
			err = true;
			retval = false;
		}
		else
			retval = true;
	}
	else
	{
		stbi_info_from_callbacks( &cb, temp_e->m_stream, &(temp_e->m_width), &(temp_e->m_height), &ignored );

		// reset to beginning of stream
		stream->seek( 0 );

		// send to background thread to load pixel data
		m_imp->m_bg_loader.add( key, temp_e );
	}

	// add to cache
	if ( !err && m_imp->m_cache )
		m_imp->m_cache->put( key, temp_e );

	{
		std::lock_guard<std::recursive_mutex> l( g_mutex );
		*e = temp_e;
		(*e)->add_ref();
	}

	return retval;
}

void FeImageLoader::release_entry( FeImageLoaderEntry **e )
{
	if ( e )
	{
		std::lock_guard<std::recursive_mutex> l( g_mutex );
		if ( *e && (*e)->dec_ref() )
			delete *e;

		*e = NULL;
	}
}

bool FeImageLoader::check_loaded( FeImageLoaderEntry *e )
{
	std::lock_guard<std::recursive_mutex> l( g_mutex );
	return ( e && e->m_loaded );
}

#ifndef NO_MOVIE
void FeImageLoader::reap_video( FeMedia *vid )
{
	vid->signal_stop();

	FeImageLoader &il = get_ref();
	if ( il.m_imp )
		il.m_imp->m_bg_loader.reap_video( vid );
}
#endif

FeImageLoader &FeImageLoader::get_ref()
{
	static FeImageLoader g_image_loader;

	if ( !g_image_loader.m_imp )
		g_image_loader.m_imp = new FeImageLoaderImp();

	return g_image_loader;
}

void FeImageLoader::set_cache_size( size_t s )
{
	FeImageLoader &il = get_ref();

	if ( s == 0 )
	{
		if ( il.m_imp->m_cache )
		{
			delete il.m_imp->m_cache;
			il.m_imp->m_cache = NULL;
		}

		return;
	}

	if ( !il.m_imp->m_cache )
		il.m_imp->m_cache = new FeImageLRUCache( s );
	else
		il.m_imp->m_cache->resize( s );
}

void FeImageLoader::set_background_loading( bool flag )
{
	FeImageLoader &il = get_ref();
	il.m_imp->m_load_images_in_bg = flag;

	FeDebug() << "Set background image loading: " << flag << std::endl;
}

bool FeImageLoader::get_background_loading()
{
	FeImageLoader &il = get_ref();
	return il.m_imp->m_load_images_in_bg;
}

//
//
void FeImageLoader::cache_image( const char *fn )
{
	FeImageLoaderEntry *e;

	std::string arch;
	std::string filename = fn;

	// Test for image in an archive
	// format of filename is "<archivename>|<filename>"
	//
	size_t pos = filename.find( "|" );
	if ( pos != std::string::npos )
	{
		arch = filename.substr( 0, pos );
		filename = filename.substr( pos+1 );

		load_image_from_archive( arch, filename, &e );
	}
	else
		load_image_from_file( filename, &e );

	release_entry( &e );
}

int FeImageLoader::cache_max()
{
	if ( !m_imp->m_cache )
		return 0;

	return m_imp->m_cache->get_max_size();
}

int FeImageLoader::cache_size()
{
	if ( !m_imp->m_cache )
		return 0;

	return m_imp->m_cache->get_size();
}

int FeImageLoader::cache_count()
{
	if ( !m_imp->m_cache )
		return 0;

	return m_imp->m_cache->get_count();
}

const char *FeImageLoader::cache_get_name_at( int pos )
{
	if ( !m_imp->m_cache || ( pos < 0 ) || ( pos >= (int)m_imp->m_cache->get_count() ) )
		return "";

	return m_imp->m_cache->get_name_at( pos );
}

int FeImageLoader::cache_get_size_at( int pos )
{
	if ( !m_imp->m_cache || ( pos < 0 ) || ( pos >= (int)m_imp->m_cache->get_count() ) )
		return 0;

	return m_imp->m_cache->get_size_at( pos );
}
