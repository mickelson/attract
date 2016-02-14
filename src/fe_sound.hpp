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
#include "zip.hpp"
#else
#include "media.hpp"
#endif

#include <string>
#include "fe_input.hpp"

class FeSettings;

class FeSound
{
private:
	FeSound( const FeSound & );
	FeSound &operator=( const FeSound & );

#ifdef NO_MOVIE
	FeZipStream m_zip; // needs to be declared before sf::Music m_sound !
	sf::Music m_sound;
#else
	FeMedia m_sound;
#endif
	std::string m_file_name;
	bool m_play_state;

public:
	FeSound( bool loop=false );

	void load( const std::string &path, const std::string &fn );
	void tick();

	void load_from_archive( const char *, const char * );
	void set_file_name( const char * );
	const char *get_file_name();

	void set_volume( int );

	bool get_playing();
	void set_playing( bool );

	float get_pitch();
	void set_pitch( float );

	bool get_loop();
	void set_loop( bool );

	float get_x();
	float get_y();
	float get_z();
	void set_x( float );
	void set_y( float );
	void set_z( float );

	int get_duration();
	int get_time();
	const char *get_metadata( const char * );

	void release_audio( bool );
};

class FeSoundSystem
{
private:
	FeSoundSystem( const FeSoundSystem & );
	FeSoundSystem &operator=( const FeSoundSystem & );

	FeSound m_sound;
	FeSound m_music;
	FeSettings *m_fes;
	FeInputMap::Command m_current_sound;

public:
	FeSoundSystem( FeSettings * );
	~FeSoundSystem();

	FeSound &get_ambient_sound();

	void sound_event( FeInputMap::Command );
	bool is_sound_event_playing( FeInputMap::Command );

	void play_ambient();
	void update_volumes();
	void stop();
	void tick();

	void release_audio( bool );
};

#endif
