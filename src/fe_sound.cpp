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
#include <iostream>

FeSoundSystem::FeSoundSystem( FeSettings *fes )
#ifdef NO_MOVIE
	: m_music(),
	m_sound(),
#else
	: m_music( FeMedia::Audio ),
	m_sound( FeMedia::Audio ),
#endif
	m_fes( fes )
{
	m_music.setLoop( true );
	m_sound.setLoop( false );
}

FeSoundSystem::~FeSoundSystem()
{
}

void FeSoundSystem::sound_event( FeInputMap::Command c )
{
	int volume = m_fes->get_play_volume( FeSoundInfo::Sound );

	if ( volume <= 0 )
		return;

	std::string sound;
	if ( !m_fes->get_sound_file( c, sound ) )
		return;

	if ( !m_sound.openFromFile( sound ) )
		return;

	m_sound.setVolume( volume );
	m_sound.play();
}

void FeSoundSystem::play_ambient()
{
	int volume = m_fes->get_play_volume( FeSoundInfo::Ambient );

	if ( volume <= 0 )
		return;

	std::string sound;
	if ( !m_fes->get_sound_file( FeInputMap::AmbientSound, sound ) )
		return;

	if ( !m_music.openFromFile( sound ) )
		return;

	m_music.setVolume( volume );
	m_music.play();
}

void FeSoundSystem::stop()
{
#ifdef NO_MOVIE
	m_music.stop();
#else
	m_music.close();
#endif
}

void FeSoundSystem::tick()
{
#ifndef NO_MOVIE
	m_music.tick();
	m_sound.tick();
#endif
}

void FeSoundSystem::update_volumes()
{
	m_music.setVolume( m_fes->get_play_volume( FeSoundInfo::Ambient ) );
	m_sound.setVolume( m_fes->get_play_volume( FeSoundInfo::Sound ) );
}

FeScriptSound::FeScriptSound()
#ifdef NO_MOVIE
	: m_sound()
#else
	: m_sound( FeMedia::Audio )
#endif
{
}

bool FeScriptSound::load( const std::string &n )
{
	if ( !m_sound.openFromFile( n ) )
	{
		std::cout << "Error loading sound file: " << n << std::endl;
		return false;
	}

	m_sound.setLoop( false );
	return true;
}

void FeScriptSound::play()
{
	// call stop to reset to the beginning (if sound has previously been played)
	//
	m_sound.stop();
	m_sound.play();
}

void FeScriptSound::set_volume( int v )
{
	m_sound.setVolume( v );
}

bool FeScriptSound::is_playing()
{
	return ( m_sound.getStatus() == sf::SoundSource::Playing ) ? true : false;
}

float FeScriptSound::get_pitch()
{
	return m_sound.getPitch();
}

void FeScriptSound::set_pitch( float p )
{
	m_sound.setPitch( p );
}

float FeScriptSound::get_x()
{
	return m_sound.getPosition().x;
}

float FeScriptSound::get_y()
{
	return m_sound.getPosition().y;
}

float FeScriptSound::get_z()
{
	return m_sound.getPosition().z;
}

void FeScriptSound::set_x( float v )
{
	m_sound.setPosition( sf::Vector3f( v, get_y(), get_z() ) );
}

void FeScriptSound::set_y( float v )
{
	m_sound.setPosition( sf::Vector3f( get_x(), v, get_z() ) );
}

void FeScriptSound::set_z( float v )
{
	m_sound.setPosition( sf::Vector3f( get_x(), get_y(), v ) );
}
