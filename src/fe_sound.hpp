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

#ifndef FE_SOUND_HPP
#define FE_SOUND_HPP

#ifdef NO_MOVIE
#include <SFML/Audio/Music.hpp>
#else
#include "media.hpp"
#endif

#include <string>
#include "fe_input.hpp"

class FeSettings;

class FeSoundSystem
{
private:
	FeSoundSystem( const FeSoundSystem & );
	FeSoundSystem &operator=( const FeSoundSystem & );

#ifdef NO_MOVIE
	sf::Music m_music;
	sf::Music m_sound;
#else
	FeMedia m_music;
	FeMedia m_sound;
#endif
	FeSettings *m_fes;

public:
	FeSoundSystem( FeSettings * );
	~FeSoundSystem();

	void sound_event( FeInputMap::Command );
	void play_ambient();
	void update_volumes();
	void stop();
	void tick();
};

class FeScriptSound
{
private:
	FeScriptSound( const FeScriptSound & );
	FeScriptSound &operator=( const FeScriptSound & );

#ifdef NO_MOVIE
	sf::Music m_sound;
#else
	FeMedia m_sound;
#endif

public:
	FeScriptSound();

	bool load( const std::string & );
	void play();
	void set_volume( int );

	bool is_playing();
	float get_pitch();
	void set_pitch( float );

	float get_x();
	float get_y();
	float get_z();
	void set_x( float );
	void set_y( float );
	void set_z( float );
};

#endif
