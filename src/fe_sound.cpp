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

#include "fe_sound.hpp"
#include "fe_settings.hpp"
#include "fe_present.hpp"
#include "fe_util.hpp"
#include "fe_file.hpp"
#include "zip.hpp"
#include <iostream>
#include <cstring>

FeSoundSystem::FeSoundSystem( FeSettings *fes )
	: m_sound( false ),
	m_music( true ),
	m_fes( fes ),
	m_current_sound( FeInputMap::LAST_COMMAND )
{
}

FeSoundSystem::~FeSoundSystem()
{
}

FeSound &FeSoundSystem::get_ambient_sound()
{
	return m_music;
}

void FeSoundSystem::sound_event( FeInputMap::Command c )
{
	if ( m_fes->get_play_volume( FeSoundInfo::Sound ) <= 0 )
		return;

	std::string sound;
	if ( !m_fes->get_sound_file( c, sound ) )
		return;

	if ( sound.compare( m_sound.get_file_name() ) != 0 )
		m_sound.load( "", sound );

	m_current_sound = c;
	m_sound.set_playing( true );
}

bool FeSoundSystem::is_sound_event_playing( FeInputMap::Command c )
{
	return (( m_current_sound == c ) && m_sound.get_playing() );
}

void FeSoundSystem::play_ambient()
{
	if ( m_fes->get_play_volume( FeSoundInfo::Ambient ) <= 0 )
		return;

	std::string sound;
	if ( !m_fes->get_sound_file( FeInputMap::AmbientSound, sound ) )
		return;

	if ( sound.compare( m_music.get_file_name() ) != 0 )
		m_music.load( "", sound );

	m_music.set_playing( true );
}

void FeSoundSystem::stop()
{
	m_music.set_playing( false );
}

void FeSoundSystem::tick()
{
	m_music.tick();
	m_sound.tick();
}

void FeSoundSystem::update_volumes()
{
	m_music.set_volume( m_fes->get_play_volume( FeSoundInfo::Ambient ) );
	m_sound.set_volume( m_fes->get_play_volume( FeSoundInfo::Sound ) );
}

void FeSoundSystem::release_audio( bool state )
{
	m_music.release_audio( state );
	m_sound.release_audio( state );
}

FeSound::FeSound( bool loop )
	: m_stream( NULL ),
#ifdef NO_MOVIE
	m_sound(),
#else
	m_sound( FeMedia::Audio ),
#endif
	m_play_state( false )
{
	// default to no looping for script sounds
	m_sound.setLoop( loop );
}

FeSound::~FeSound()
{
	if ( m_stream )
		delete m_stream;
}

void FeSound::release_audio( bool state )
{
	// fix our state if sound is being stopped...
	if ( state )
		set_playing( false );

#ifndef NO_MOVIE
	m_sound.release_audio( state );
#endif
}

void FeSound::tick()
{
#ifndef NO_MOVIE
	if ( m_play_state )
		m_sound.tick();
#endif
}

void FeSound::load( const std::string &path, const std::string &fn )
{
	if ( m_stream )
	{
		delete m_stream;
		m_stream = NULL;
	}

	if ( is_supported_archive( path ) )
	{
#ifndef NO_MOVIE
		if ( !m_sound.open( path, fn ) )
		{
			FeLog() << "Error loading sound file from archive: "
				<< path << " (" << fn << ")" << std::endl;
			m_file_name = "";
			return;
		}
#else
		FeZipStream *zip = new FeZipStream( path );
		m_stream = zip;

		if ( !zip->open( fn ) )
		{
			FeLog() << "Error loading sound file from archive: "
				<< path << " (" << fn << ")" << std::endl;
			m_file_name = "";
			return;
		}

		if ( !m_sound.openFromStream( *m_stream ) )
		{
			FeLog() << "Error loading sound file: " << fn
				<< std::endl;
			m_file_name = "";
			return;
		}
#endif

		m_file_name = path + "|" + fn;
	}
	else
	{
		std::string file_to_load = path + fn;

#ifndef NO_MOVIE
		if ( !m_sound.open( "", file_to_load ) )
		{
			FeLog() << "Error loading sound file: " << file_to_load << std::endl;
			m_file_name = "";
			return;
		}
#else
		FeFileInputStream *fs = new FeFileInputStream( file_to_load );
		m_stream = fs;

		if ( !m_sound.openFromStream( *m_stream ) )
		{
			FeLog() << "Error loading sound file: " << file_to_load << std::endl;
			m_file_name = "";
			return;
		}
#endif

		m_file_name = file_to_load;
	}
}

void FeSound::set_file_name( const char *n )
{
	std::string path;
	std::string filename = n;

	// Test for sound from an archive
	// format of filename is "<archivename>|<filename>"
	//
	size_t pos = filename.find( "|" );
	if ( pos != std::string::npos )
	{
		path = filename.substr( 0, pos );
		filename = filename.substr( pos+1 );
	}


	load_from_archive( path.c_str(), filename.c_str() );
}

void FeSound::load_from_archive( const char *a, const char *n )
{
	std::string fn = clean_path( n );
	if ( fn.empty() )
	{
		m_file_name = "";
		return;
	}

	std::string path;

	if ( a && ( strlen( a ) > 0 ) )
		path = clean_path( a );
	else if ( is_relative_path( fn ) )
		path = FePresent::script_get_base_path();

	load( path, fn );
}

const char *FeSound::get_file_name()
{
	return m_file_name.c_str();
}

void FeSound::set_volume( int v )
{
	m_sound.setVolume( v );
}

void FeSound::set_playing( bool flag )
{
	m_play_state = flag;

	// calling stop will reset to the beginning (if sound has previously been played)
	//
	m_sound.stop();

	if ( m_play_state == true )
		m_sound.play();
}

bool FeSound::get_playing()
{
	return ( m_sound.getStatus() == sf::SoundSource::Playing ) ? true : false;
}

float FeSound::get_pitch()
{
	return m_sound.getPitch();
}

void FeSound::set_pitch( float p )
{
	m_sound.setPitch( p );
}

bool FeSound::get_loop()
{
	return m_sound.getLoop();
}

void FeSound::set_loop( bool loop )
{
	m_sound.setLoop( loop );
}

float FeSound::get_x()
{
	return m_sound.getPosition().x;
}

float FeSound::get_y()
{
	return m_sound.getPosition().y;
}

float FeSound::get_z()
{
	return m_sound.getPosition().z;
}

void FeSound::set_x( float v )
{
	m_sound.setPosition( sf::Vector3f( v, get_y(), get_z() ) );
}

void FeSound::set_y( float v )
{
	m_sound.setPosition( sf::Vector3f( get_x(), v, get_z() ) );
}

void FeSound::set_z( float v )
{
	m_sound.setPosition( sf::Vector3f( get_x(), get_y(), v ) );
}

int FeSound::get_duration()
{
#ifndef NO_MOVIE
	return m_sound.get_duration().asMilliseconds();
#else
	return 0;
#endif
}

int FeSound::get_time()
{
#ifndef NO_MOVIE
	return m_sound.get_video_time().asMilliseconds();
#else
	return 0;
#endif
}

const char *FeSound::get_metadata( const char *tag )
{
#ifndef NO_MOVIE
	return m_sound.get_metadata( tag );
#else
	return "";
#endif
}
