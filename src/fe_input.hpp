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

#ifndef FE_INPUT_HPP
#define FE_INPUT_HPP

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Rect.hpp>
#include "fe_base.hpp"
#include <vector>
#include <map>

class FeMapping;

class FeInputSource
{
public:
	enum Type
	{
		Unsupported=-1,
		Keyboard=0,
		Mouse=1,
		Joystick0=2 // up to sf::Joystick::Count joysticks supported starting from Joystick0
	};

	enum MouseCode
	{
		MouseUp, MouseDown, MouseLeft, MouseRight,
		MouseWheelUp, MouseWheelDown, MouseBLeft, MouseBRight, MouseBMiddle, MouseBX1, MouseBX2
	};

	enum JoyCode
	{
		JoyUp, JoyDown, JoyLeft, JoyRight, JoyZPos, JoyZNeg,
		JoyRPos, JoyRNeg, JoyUPos, JoyUNeg, JoyVPos, JoyVNeg,
		JoyPOVXPos, JoyPOVXNeg, JoyPOVYPos, JoyPOVYNeg,
		JoyButton0 // up to sf::Joystick::ButtonCount buttons supported starting from JoyButton0
	};

	static const char *keyStrings[];

	FeInputSource();

	// Construct from a known type and code
	FeInputSource( Type t, int code );

	// Construct from an SFML event
	FeInputSource( const sf::Event &ev, const sf::IntRect &mc_rect, const int joy_thresh );

	// Construct from a config string
	FeInputSource( const std::string &str );

	// Output as a config string
	std::string as_string() const;

	Type get_type() const { return m_type; }
	bool is_mouse_move() const;

	bool operator< ( const FeInputSource &o ) const;

	// test the current state of the input that this object represents and return true if it is depressed,
	// false otherwise.  Works for keys, buttons and joystick axes.  Does not work for mouse moves or wheels.
	bool get_current_state( int joy_thresh ) const;

	// Return the current position of the input that this object represents.
	// Works for joystick axes
	int get_current_pos() const;

private:
	static const char *mouseStrings[];
	static const char *joyStrings[];

	Type m_type;
	int m_code;
};

class FeInputMap : public FeBaseConfigurable
{
	friend class FeMapping;
public:

	//
	// Input actions supported by Attract-Mode:
	//
	// This enum needs to be kept in sync with commandStrings[] and
	// commandDispStrings[] below.
	//
	enum Command {
		Select=0,
		Up,
		Down,
		PageUp,
		PageDown,
		PrevDisplay,
		NextDisplay,
		DisplaysMenu,
		PrevFilter,
		NextFilter,
		FiltersMenu,
		ToggleLayout,
		ToggleMovie,
		ToggleMute,
		ToggleRotateRight,
		ToggleFlip,
		ToggleRotateLeft,
		ExitMenu,
		ExitNoMenu,
		ScreenShot,
		Configure,
		RandomGame,
		ReplayLastGame,
		ToggleFavourite,
		PrevFavourite,
		NextFavourite,
		ToggleTags,
		ScreenSaver,
		PrevLetter,
		NextLetter,
		Custom1,
		Custom2,
		LAST_COMMAND,

		//
		// The following are not input actions per se, but rather are here
		// as events to which sounds can be mapped:
		//
		AmbientSound,
		EventStartup,
		EventGameReturn,
		LAST_EVENT
	};

	static const char *commandStrings[];
	static const char *commandDispStrings[];

	FeInputMap();

	Command map_input( const sf::Event &, const sf::IntRect &mc_rect, const int joy_thresh );

	//
	// Test if any of the inputs mapped to command c are pressed
	//
	bool get_current_state( FeInputMap::Command c, int joy_thresh ) const;

	void get_mappings( std::vector< FeMapping > &mappings ) const;
	void set_mapping( const FeMapping &mapping );
	void default_mappings();

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &fn );

	void save( std::ofstream & ) const;
	bool has_mouse_moves() const { return ( m_mmove_count > 0 ); };

	static Command string_to_command( const std::string &s );

private:
	std::map< FeInputSource, Command > m_map;
	int m_mmove_count; // counter of whether mouse moves are mapped
};


//
// Container class used in mapping configuration
//
class FeMapping
{
public:
	FeInputMap::Command command;
	std::vector< std::string > input_list;

	FeMapping( FeInputMap::Command cmd );
	bool operator< ( const FeMapping ) const;
};

//
// Class to contain the sound settings
//
class FeSoundInfo : public FeBaseConfigurable
{
public:
	static const char *settingStrings[];
	static const char *settingDispStrings[];

	FeSoundInfo();

	enum SoundType { Sound=0, Ambient, Movie };
	void set_volume( SoundType, const std::string & );
	int get_set_volume( SoundType ) const;
	int get_play_volume( SoundType ) const; // takes mute state into account

	bool get_mute() const;
	void set_mute( bool );

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &fn );

	bool get_sound( FeInputMap::Command c, std::string &name ) const;
	void set_sound( FeInputMap::Command c, const std::string &name );

	void save( std::ofstream & ) const;

private:
	int m_sound_vol;
	int m_ambient_vol;
	int m_movie_vol;
	bool m_mute;

	std::map<FeInputMap::Command, std::string> m_sounds;
};

#endif
