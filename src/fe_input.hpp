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
#include "fe_base.hpp"
#include <vector>
#include <map>

//
// FeMouseCapture - A container for information used if we are capturing
// mouse input.  The mouse gets captured if the user maps any mouse moves 
// (i.e. MouseUp, MouseDown, MouseLeft, MouseRight) as an input.
//
class FeMouseCapture
{
public:
	FeMouseCapture( int );

	int capture_count;
	int top_bound;
	int bottom_bound;
	int left_bound;
	int right_bound;
	int reset_x;
	int reset_y;
};

class FeMapping;

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
		PrevList,
		NextList,
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
	static const char *inputStrings[];
	static const int JOY_THRESH=90;

	FeInputMap( bool disable_mousecap );
	Command map( sf::Event );

	void get_mappings( std::vector< FeMapping > &mappings );
	void set_mapping( const FeMapping &mapping );
	void init_config_map_input();
	bool config_map_input( sf::Event e, std::string &s, Command &conflict );
	void default_mappings();

	int process_setting( const std::string &setting, 
								const std::string &value,
								const std::string &fn );

	void save( std::ofstream & );

	static Command string_to_command( const std::string &s );

private:
	enum InputType
	{ 
		JoyUp, JoyDown, JoyLeft, JoyRight, 
		JoyB1, JoyB2, JoyB3, JoyB4, JoyB5, JoyB6, JoyB7, JoyB8, 
		MouseUp, MouseDown, MouseLeft, MouseRight, 
		MouseWheelUp, MouseWheelDown, 
		MouseBLeft, MouseBRight, MouseBMiddle, MouseBX1, MouseBX2, 
		Keyboard, // need to keep "Keyboard" right before LAST_INPUT
		LAST_INPUT
	};

	struct KeyLookup { const char *label; sf::Keyboard::Key key; };
	static const KeyLookup keyTable[];

	std::map< std::pair< int, InputType >, Command> m_map;
	FeMouseCapture m_cap;
	bool m_disable_mousecap;

	static bool is_joystick( InputType i ) { return ( i <= JoyB8 ); }
	static bool is_mouse_move( InputType i );
	std::pair< int, InputType > get_map_index( sf::Event e, bool config=false );
	void string_to_index( const std::string &s,
            std::pair< int, InputType > &index );
};

//
// Container class used in mapping configuration
//
// TODO: Mapping to specific joystick # is current not supported in 
// config interface
//
class FeMapping
{
public:
	FeInputMap::Command command;
	std::vector< std::string > input_list;

	FeMapping( FeInputMap::Command cmd );
   void add_input( const std::pair< int, FeInputMap::InputType > & );
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
	int get_set_volume( SoundType );
	int get_play_volume( SoundType ); // takes mute state into account

	bool get_mute();
	void set_mute( bool );

	int process_setting( const std::string &setting, 
								const std::string &value,
								const std::string &fn );

	bool get_sound( FeInputMap::Command c, std::string &name );
	void set_sound( FeInputMap::Command c, const std::string &name );

	void save( std::ofstream & );

private:
	int m_sound_vol;
	int m_ambient_vol;
	int m_movie_vol;
	bool m_mute;

	std::map<FeInputMap::Command, std::string> m_sounds;
};

#endif
