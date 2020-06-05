/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-16 Andrew Mickelson
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

#include "fe_input.hpp"
#include "fe_util.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <set>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/System/Vector2.hpp>

#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 )) // touch support sfml >= 2.2
#include <SFML/Window/Touch.hpp>
#endif

namespace
{
#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 )) // touch support sfml >= 2.2
	// globals to track when last touch event "began"
	//
	sf::Event g_last_touch;
	bool g_touch_moved=false;
#endif

	static std::vector< int > g_joyfemap( sf::Joystick::Count, 0 );

	int joymap2feid( int raw_id )
	{
		return g_joyfemap[ raw_id ];
	}

	int joymap2raw( int config_id )
	{
		for ( size_t i=0; i<sf::Joystick::Count; i++ )
		{
			if ( g_joyfemap[i] == config_id )
				return i;
		}

		return 0;
	}

	void joy_raw_map_init( const std::vector < std::pair < int, std::string > > &joy_config )
	{
//
// sf::Joystick::getIdentification() only available if SFML version is >= 2.2
//
#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 ))

		size_t i=0;
		std::vector < std::pair < int, std::string > >::iterator itr;
		std::vector < std::pair < int, std::string  > > wl = joy_config;
		bool mapped[ sf::Joystick::Count ];

		// clear map structure
		for ( i=0; i< sf::Joystick::Count; i++ )
		{
			g_joyfemap[i] = sf::Joystick::Count-1;
			mapped[i] = false;
		}

		// initialize structure telling us whether a slot is already mapped by the user
		for ( itr = wl.begin(); itr != wl.end(); ++itr )
			mapped[ (*itr).first ] = true;

		sf::Joystick::update();

		// update the map structure using supplied joy_config
		//
		for ( i=0; i< sf::Joystick::Count; i++ )
		{
			if ( sf::Joystick::isConnected( i ) )
			{
				std::string name = sf::Joystick::getIdentification( i ).name.toAnsiString();

				for ( itr = wl.begin(); itr != wl.end(); ++itr )
				{
					if ( name.compare( (*itr).second ) == 0 )
					{
						g_joyfemap[i] = (*itr).first;
						wl.erase( itr );
						break;
					}
				}
			}
		}

		// Put any remaining unmapped joysticks in the lowest available "Default" slot
		for ( i=0; i< sf::Joystick::Count; i++ )
		{
			if (( g_joyfemap[i] == sf::Joystick::Count-1 ) && ( sf::Joystick::isConnected( i ) ))
			{
				for ( size_t j=0; j< sf::Joystick::Count; j++ )
				{
					if ( !mapped[j] )
					{
						g_joyfemap[i] = j;
						mapped[j] = true;
						break;
					}
				}
			}
		}

		FeDebug() << "Joysticks after mapping: " << std::endl;
		for ( i=0; i<sf::Joystick::Count; i++ )
			FeDebug() << "ID: " << i << " => Joy" << g_joyfemap[i] << "("
				<< sf::Joystick::getIdentification(i).name.toAnsiString()
				<< ")" << std::endl;
#else // sfml < 2.2
		for ( size_t i=0; i< sf::Joystick::Count; i++ )
			g_joyfemap[i] = i;
#endif
	}
};

// Needs to stay aligned with sf::Keyboard
//
const char *FeInputSingle::keyStrings[] =
{
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"Num0",
	"Num1",
	"Num2",
	"Num3",
	"Num4",
	"Num5",
	"Num6",
	"Num7",
	"Num8",
	"Num9",
	"Escape",
	"LControl",
	"LShift",
	"LAlt",
	"LSystem",
	"RControl",
	"RShift",
	"RAlt",
	"RSystem",
	"Menu",
	"LBracket",
	"RBracket",
	"Semicolon",
	"Comma",
	"Period",
	"Quote",
	"Slash",
	"Backslash",
	"Tilde",
	"Equal",
	"Dash",
	"Space",
	"Return",
	"Backspace",
	"Tab",
	"PageUp",
	"PageDown",
	"End",
	"Home",
	"Insert",
	"Delete",
	"Add",
	"Subtract",
	"Multiply",
	"Divide",
	"Left",
	"Right",
	"Up",
	"Down",
	"Numpad0",
	"Numpad1",
	"Numpad2",
	"Numpad3",
	"Numpad4",
	"Numpad5",
	"Numpad6",
	"Numpad7",
	"Numpad8",
	"Numpad9",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"Pause",
	NULL // needs to be last
};

const char *FeInputSingle::mouseStrings[] =
{
	"Up",
	"Down",
	"Left",
	"Right",
	"WheelUp",
	"WheelDown",
	"LeftButton",
	"RightButton",
	"MiddleButton",
	"ExtraButton1",
	"ExtraButton2",
	NULL
};

const char *FeInputSingle::touchStrings[] =
{
	"Up",
	"Down",
	"Left",
	"Right",
	"Tap",
	NULL
};

const char *FeInputSingle::joyStrings[] =
{
	"Zpos",
	"Zneg",
	"Rpos",
	"Rneg",
	"Upos",
	"Uneg",
	"Vpos",
	"Vneg",
	"PovXpos",
	"PovXneg",
	"PovYpos",
	"PovYneg",
	"Up",
	"Down",
	"Left",
	"Right",
	"Button",
	NULL
};

FeInputSingle::FeInputSingle()
	: m_type( Unsupported ),
	m_code( 0 )
{
}

FeInputSingle::FeInputSingle( Type t, int c )
	: m_type( t ),
	m_code( c )
{
}

FeInputSingle::FeInputSingle( const sf::Event &e, const sf::IntRect &mc_rect, const int joy_thresh )
	: m_type( Unsupported ),
	m_code( 0 )
{
	switch ( e.type )
	{
		case sf::Event::KeyPressed:
			if ( e.key.code != sf::Keyboard::Unknown )
			{
				m_type = Keyboard;
				m_code = e.key.code;
			}
			break;

		case sf::Event::JoystickButtonPressed:
			m_type = (Type)(Joystick0 + joymap2feid( e.joystickButton.joystickId ));
			m_code = JoyButton0 + e.joystickButton.button;
			break;

		case sf::Event::JoystickMoved:
			if ( std::abs( e.joystickMove.position ) > joy_thresh )
			{
				switch ( e.joystickMove.axis )
				{
					case sf::Joystick::X:
						m_code = ( e.joystickMove.position > 0 ) ? JoyRight : JoyLeft;
						break;

					case sf::Joystick::Y:
						m_code = ( e.joystickMove.position > 0 ) ? JoyDown : JoyUp;
						break;

#ifdef SFML_SYSTEM_LINUX
					//
					// On Linux, SFML's Z and R axes are mapped to Throttle and Rudder controls
					// They seem to rest at -100 and go up to 100
					//
					case sf::Joystick::Z:
					case sf::Joystick::R:
						if ( e.joystickMove.position < 0 )
							return;

						m_code = ( e.joystickMove.axis == sf::Joystick::Z )
							? JoyZPos : JoyRPos;
						break;
#else
					case sf::Joystick::Z:
						m_code = ( e.joystickMove.position > 0 ) ? JoyZPos : JoyZNeg;
						break;

					case sf::Joystick::R:
						m_code = ( e.joystickMove.position > 0 ) ? JoyRPos : JoyRNeg;
						break;
#endif

					case sf::Joystick::U:
						m_code = ( e.joystickMove.position > 0 ) ? JoyUPos : JoyUNeg;
						break;

					case sf::Joystick::V:
						m_code = ( e.joystickMove.position > 0 ) ? JoyVPos : JoyVNeg;
						break;

					case sf::Joystick::PovX:
						m_code = ( e.joystickMove.position > 0 ) ? JoyPOVXPos : JoyPOVXNeg;
						break;

					case sf::Joystick::PovY:
						m_code = ( e.joystickMove.position > 0 ) ? JoyPOVYPos : JoyPOVYNeg;
						break;

					default:
						ASSERT( 0 ); // unhandled joystick axis encountered
						return;
				}

				m_type = (Type)(Joystick0 + joymap2feid( e.joystickMove.joystickId ) );
			}
			break;

		case sf::Event::MouseMoved:
			if ( e.mouseMove.x < mc_rect.left )
			{
				m_type = Mouse;
				m_code = MouseLeft;
			}
			else if ( e.mouseMove.y < mc_rect.top )
			{
				m_type = Mouse;
				m_code = MouseUp;
			}
			else if ( e.mouseMove.x >= mc_rect.left + mc_rect.width )
			{
				m_type = Mouse;
				m_code = MouseRight;
			}
			else if ( e.mouseMove.y >= mc_rect.top + mc_rect.height )
			{
				m_type = Mouse;
				m_code = MouseDown;
			}
			break;

		case sf::Event::MouseWheelMoved:
			m_type = Mouse;
			if ( e.mouseWheel.delta > 0 )
				m_code=MouseWheelUp;
			else
				m_code=MouseWheelDown;
			break;

		case sf::Event::MouseButtonPressed:
			switch ( e.mouseButton.button )
			{
				case sf::Mouse::Left: m_code=MouseBLeft; break;
				case sf::Mouse::Right: m_code=MouseBRight; break;
				case sf::Mouse::Middle: m_code=MouseBMiddle; break;
				case sf::Mouse::XButton1: m_code=MouseBX1; break;
				case sf::Mouse::XButton2: m_code=MouseBX2; break;
				default:
					ASSERT( 0 ); // unhandled mouse button encountered
					return;
			}
			m_type = Mouse;
			break;

#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 )) // touch support sfml >= 2.2
		case sf::Event::TouchMoved:
			{
				const int THRESH = 100;
				int diff_x = e.touch.x - g_last_touch.touch.x;
				int diff_y = e.touch.y - g_last_touch.touch.y;

				if ( abs( diff_y ) > THRESH )
				{
					if ( diff_y < 0 )
						m_code = SwipeUp;
					else
						m_code = SwipeDown;

					m_type = Touch;

					g_touch_moved = true;
					g_last_touch = e;
				}
				else if ( abs( diff_x ) > THRESH )
				{
					if ( diff_x < 0 )
						m_code = SwipeRight;
					else
						m_code = SwipeLeft;

					m_type = Touch;

					g_touch_moved = true;
					g_last_touch = e;
				}
			}
			break;

		case sf::Event::TouchBegan:
			g_touch_moved = false;
			g_last_touch = e;
			break;

		case sf::Event::TouchEnded:
			if ( !g_touch_moved )
			{
				m_code = Tap;
				m_type = Touch;
			}
			break;
#endif

		default:
			break;
	}
}

FeInputSingle::FeInputSingle( const std::string &str )
	: m_type( Unsupported ),
	m_code( 0 )
{
	if ( str.empty() )
		return;

	size_t pos=0;
	std::string val;

	token_helper( str, pos, val, FE_WHITESPACE );
	int i=0;

	if ( val.compare( "Mouse" ) == 0 )
	{
		m_type = Mouse;

		token_helper( str, pos, val, FE_WHITESPACE );
		while ( mouseStrings[i] != NULL )
		{
			if ( val.compare( mouseStrings[i] ) == 0 )
			{
				m_code = i;
				break;
			}
			i++;
		}
	}
	else if ( val.compare( "Touch" ) == 0 )
	{
		m_type = Touch;

		token_helper( str, pos, val, FE_WHITESPACE );
		while ( touchStrings[i] != NULL )
		{
			if ( val.compare( touchStrings[i] ) == 0 )
			{
				m_code = i;
				break;
			}
			i++;
		}
	}
	else if ( val.compare( 0, 3, "Joy" ) == 0 )
	{
		int num = as_int( val.substr( 3 ) );
		m_type = (Type)(Joystick0 + num);

		token_helper( str, pos, val, FE_WHITESPACE );
		while ( joyStrings[i] != NULL )
		{
			if ( val.compare( 0, strlen(joyStrings[i]), joyStrings[i] ) == 0 )
			{
				if ( i == JoyButton0 )
				{
					int temp = as_int( val.substr( strlen( joyStrings[i] ) ) );
					m_code = i + temp;
				}
				else
				{
					m_code = i;
				}
				break;
			}
			i++;
		}
	}
	else
	{
		// key
		while ( keyStrings[i] != NULL )
		{
			if ( val.compare( keyStrings[i] ) == 0 )
			{
				m_type = Keyboard;
				m_code = i;
				return;
			}
			i++;
		}
	}
}

std::string FeInputSingle::as_string() const
{
	std::string temp;

	if ( m_type == Keyboard )
	{
		temp = keyStrings[ m_code ];
	}
	else if ( m_type == Mouse )
	{
		temp = "Mouse ";
		temp += mouseStrings[ m_code ];
	}
	else if ( m_type == Touch )
	{
		temp = "Touch ";
		temp += touchStrings[ m_code ];
	}
	else if ( m_type >= Joystick0 )
	{
		temp = "Joy";
		temp += as_str( m_type - Joystick0 );
		temp += " ";
		if ( m_code >= JoyButton0 )
		{
			temp += joyStrings[ JoyButton0 ];
			temp += as_str( m_code - JoyButton0 );
		}
		else
		{
			temp += joyStrings[ m_code ];
		}
	}

	return temp;
}

bool FeInputSingle::is_mouse_move() const
{
	return (( m_type == Mouse )
		&& ( m_code <= MouseRight )
		&& ( m_code >= MouseUp ));
}

bool FeInputSingle::operator< ( const FeInputSingle &o ) const
{
	if ( m_type == o.m_type )
		return ( m_code < o.m_code );

	return ( m_type < o.m_type );
}

bool FeInputSingle::get_current_state( int joy_thresh ) const
{
	if ( m_type == Unsupported )
		return false;
	else if ( m_type == Keyboard )
		return sf::Keyboard::isKeyPressed( (sf::Keyboard::Key)m_code );
	else if ( m_type == Mouse )
	{
		switch ( m_code )
		{
		case MouseBLeft: return sf::Mouse::isButtonPressed( sf::Mouse::Left );
		case MouseBRight: return sf::Mouse::isButtonPressed( sf::Mouse::Right );
		case MouseBMiddle: return sf::Mouse::isButtonPressed( sf::Mouse::Middle );
		case MouseBX1: return sf::Mouse::isButtonPressed( sf::Mouse::XButton1 );
		case MouseBX2: return sf::Mouse::isButtonPressed( sf::Mouse::XButton2 );
		default: return false; // mouse moves and wheels are not supported
		}
	}
#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 )) // touch support sfml >= 2.2
	else if ( m_type == Touch )
		return sf::Touch::isDown( 0 );
#endif
	else // Joysticks
	{
		sf::Joystick::update();

		int id = joymap2raw( m_type - Joystick0 );

		if ( m_code < JoyButton0 )
		{
			switch ( m_code )
			{
				case JoyLeft: return ( -sf::Joystick::getAxisPosition( id, sf::Joystick::X ) > joy_thresh );
				case JoyRight: return ( sf::Joystick::getAxisPosition( id, sf::Joystick::X ) > joy_thresh );
				case JoyUp: return ( -sf::Joystick::getAxisPosition( id, sf::Joystick::Y ) > joy_thresh );
				case JoyDown: return ( sf::Joystick::getAxisPosition( id, sf::Joystick::Y ) > joy_thresh );
				case JoyZPos: return ( sf::Joystick::getAxisPosition( id, sf::Joystick::Z ) > joy_thresh );
				case JoyZNeg: return ( -sf::Joystick::getAxisPosition( id, sf::Joystick::Z ) > joy_thresh );
				case JoyRPos: return ( sf::Joystick::getAxisPosition( id, sf::Joystick::R ) > joy_thresh );
				case JoyRNeg: return ( -sf::Joystick::getAxisPosition( id, sf::Joystick::R ) > joy_thresh );
				case JoyUPos: return ( sf::Joystick::getAxisPosition( id, sf::Joystick::U ) > joy_thresh );
				case JoyUNeg: return ( -sf::Joystick::getAxisPosition( id, sf::Joystick::U ) > joy_thresh );
				case JoyVPos: return ( sf::Joystick::getAxisPosition( id, sf::Joystick::V ) > joy_thresh );
				case JoyVNeg: return ( -sf::Joystick::getAxisPosition( id, sf::Joystick::V ) > joy_thresh );
				case JoyPOVXPos: return ( sf::Joystick::getAxisPosition( id, sf::Joystick::PovX ) > joy_thresh );
				case JoyPOVXNeg: return ( -sf::Joystick::getAxisPosition( id, sf::Joystick::PovX ) > joy_thresh );
				case JoyPOVYPos: return ( sf::Joystick::getAxisPosition( id, sf::Joystick::PovY ) > joy_thresh );
				case JoyPOVYNeg: return ( -sf::Joystick::getAxisPosition( id, sf::Joystick::PovY ) > joy_thresh );
				default: return false;
			}
		}
		else
			return sf::Joystick::isButtonPressed( id, m_code - JoyButton0 );
	}
}

int FeInputSingle::get_current_pos() const
{
	if ( m_type == Mouse )
	{
		if (( m_code == MouseUp ) || ( m_code == MouseDown ))
			return sf::Mouse::getPosition().y;
		else if (( m_code == MouseLeft ) || ( m_code == MouseRight ))
			return sf::Mouse::getPosition().x;
	}
	else if (( m_type >= Joystick0 ) && ( m_code < JoyButton0 ))
	{
		// return the joystick position on the specified axis
		sf::Joystick::update();

		int temp = 0;
		int id = joymap2raw( m_type - Joystick0 );

		switch ( m_code )
		{
			case JoyLeft: temp = -sf::Joystick::getAxisPosition( id, sf::Joystick::X ); break;
			case JoyRight: temp = sf::Joystick::getAxisPosition( id, sf::Joystick::X ); break;
			case JoyUp: temp = -sf::Joystick::getAxisPosition( id, sf::Joystick::Y ); break;
			case JoyDown: temp = sf::Joystick::getAxisPosition( id, sf::Joystick::Y ); break;
			case JoyZPos: temp = sf::Joystick::getAxisPosition( id, sf::Joystick::Z ); break;
			case JoyZNeg: temp = -sf::Joystick::getAxisPosition( id, sf::Joystick::Z ); break;
			case JoyRPos: temp = sf::Joystick::getAxisPosition( id, sf::Joystick::R ); break;
			case JoyRNeg: temp = -sf::Joystick::getAxisPosition( id, sf::Joystick::R ); break;
			case JoyUPos: temp = sf::Joystick::getAxisPosition( id, sf::Joystick::U ); break;
			case JoyUNeg: temp = -sf::Joystick::getAxisPosition( id, sf::Joystick::U ); break;
			case JoyVPos: temp = sf::Joystick::getAxisPosition( id, sf::Joystick::V ); break;
			case JoyVNeg: temp = -sf::Joystick::getAxisPosition( id, sf::Joystick::V ); break;
			case JoyPOVXPos: temp = sf::Joystick::getAxisPosition( id, sf::Joystick::PovX ); break;
			case JoyPOVXNeg: temp = -sf::Joystick::getAxisPosition( id, sf::Joystick::PovX ); break;
			case JoyPOVYPos: temp = sf::Joystick::getAxisPosition( id, sf::Joystick::PovY ); break;
			case JoyPOVYNeg: temp = -sf::Joystick::getAxisPosition( id, sf::Joystick::PovY ); break;
			default: break;
		}

		return ( temp < 0 ) ? 0 : temp;
	}

	return 0;
}

std::string FeInputSingle::get_joy_name() const
{
#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 ))
	if ( m_type >= Joystick0 )
	{
		int raw_id = joymap2raw( m_type - Joystick0 );

		if ( sf::Joystick::isConnected( raw_id ) )
			return sf::Joystick::getIdentification( raw_id ).name.toAnsiString();
	}
#endif
	return "";
}

bool FeInputSingle::operator==( const FeInputSingle &o ) const
{
	return (( m_type == o.m_type ) && ( m_code == o.m_code ));
}

bool FeInputSingle::operator!=( const FeInputSingle &o ) const
{
	return (( m_type != o.m_type ) || ( m_code != o.m_code ));
}

FeInputMapEntry::FeInputMapEntry()
	: command( FeInputMap::LAST_COMMAND )
{
}

FeInputMapEntry::FeInputMapEntry( FeInputSingle::Type t, int code, FeInputMap::Command c )
	: command( c )
{
	inputs.insert( FeInputSingle( t, code ) );
}

FeInputMapEntry::FeInputMapEntry( const std::string &s, FeInputMap::Command c )
	: command( c )
{
	size_t pos=0;

	do
	{
		std::string tok;
		token_helper( s, pos, tok, "+" );
		if ( !tok.empty() )
			inputs.insert( FeInputSingle( tok ) );

	} while ( pos < s.size() );
}

bool FeInputMapEntry::get_current_state( int joy_thresh, const FeInputSingle &trigger ) const
{
	if ( inputs.empty() )
		return false;

	for ( std::set < FeInputSingle >::const_iterator it=inputs.begin(); it != inputs.end(); ++it )
	{
		if (( (*it) != trigger ) && ( !(*it).get_current_state( joy_thresh ) ))
			return false;
	}

	return true;
}

std::string FeInputMapEntry::as_string() const
{
	std::string retval;

	for ( std::set < FeInputSingle >::const_iterator it=inputs.begin(); it != inputs.end(); ++it )
	{
		if ( !retval.empty() )
			retval += "+";

		retval += (*it).as_string();
	}

	return retval;
}

bool FeInputMapEntry::has_mouse_move() const
{
	for ( std::set < FeInputSingle >::const_iterator it=inputs.begin(); it != inputs.end(); ++it )
	{
		if ( (*it).is_mouse_move() )
			return true;
	}

	return false;
}

FeMapping::FeMapping( FeInputMap::Command cmd )
	: command( cmd )
{
}

bool FeMapping::operator< ( const FeMapping &o ) const
{
	return ( command < o.command );
}

// NOTE: This needs to be kept in alignment with enum FeInputMap::Command
//
const char *FeInputMap::commandStrings[] =
{
	"back",
	"up",
	"down",
	"left",
	"right",
	"select",
	"prev_game",
	"next_game",
	"prev_page",    // was page_up
	"next_page",    // was page_down
	"prev_display",	// was prev_list
	"next_display",	// was next_list
	"displays_menu",// was lists_menu
	"prev_filter",
	"next_filter",
	"filters_menu",
	"toggle_layout",
	"toggle_movie",
	"toggle_mute",
	"toggle_rotate_right",
	"toggle_flip",
	"toggle_rotate_left",
	"exit",
	"exit_to_desktop",
	"screenshot",
	"configure",
	"random_game",
	"replay_last_game",
	"add_favourite",
	"prev_favourite",
	"next_favourite",
	"add_tags",
	"screen_saver",
	"prev_letter",
	"next_letter",
	"intro",
	"insert_game",
	"edit_game",
	"layout_options",
	"custom1",
	"custom2",
	"custom3",
	"custom4",
	"custom5",
	"custom6",
	NULL, // LAST_COMMAND... NULL required here
	"ambient",
	"startup",
	"game_return",
	NULL
};

const char *FeInputMap::commandDispStrings[] =
{
	"Back",
	"Up",
	"Down",
	"Left",
	"Right",
	"Select",
	"Previous Game",
	"Next Game",
	"Previous Page",
	"Next Page",
	"Previous Display",
	"Next Display",
	"Displays Menu",
	"Previous Filter",
	"Next Filter",
	"Filters Menu",
	"Toggle Layout",
	"Toggle Movie",
	"Toggle Mute",
	"Toggle Rotate Right",
	"Toggle Flip",
	"Toggle Rotate Left",
	"Exit",
	"Exit to Desktop",
	"Screenshot",
	"Configure",
	"Random Game",
	"Replay Last Game",
	"Add/Remove Favourite",
	"Previous Favourite",
	"Next Favourite",
	"Add/Remove Tags",
	"Screen Saver",
	"Previous Letter",
	"Next Letter",
	"Intro",
	"Insert Game",
	"Edit Game",
	"Layout Options",
	"Custom1",
	"Custom2",
	"Custom3",
	"Custom4",
	"Custom5",
	"Custom6",
	NULL, // LAST_COMMAND... NULL required here
	"Ambient Soundtrack",
	"Startup Sound",
	"Game Return Sound",
	NULL
};

FeInputMap::FeInputMap()
	: m_defaults( (int)Select ),
	m_mmove_count( 0 )
{
	// Set default actions for the "UI" commands (Back, Up, Down, Left, Right)
	//
	m_defaults[ Back ]  = Exit;
	m_defaults[ Up ]    = PrevGame;
	m_defaults[ Down ]  = NextGame;
	m_defaults[ Left ]  = PrevDisplay;
	m_defaults[ Right ] = NextDisplay;
}

bool my_sort_fn( FeInputMapEntry *a, FeInputMapEntry *b )
{
	return ( a->inputs.size() > b->inputs.size() );
}

void FeInputMap::initialize_mappings()
{
	if ( m_inputs.empty() )
	{
		//
		// Set some convenient initial mappings (user can always choose to unmap)...
		//
		struct InitialMappings { const char *label; Command command; } imap[] =
		{
			{ "Tab",                      Configure },
			{ "Up+LControl",              PrevLetter },
			{ "Down+LControl",            NextLetter },
			{ "Left+LControl",            FiltersMenu },
			{ "Right+LControl",           NextFilter },
			{ "Escape+Up",                Configure },
			{ "Escape+Down",              EditGame },
			{ "Escape+LControl",          ToggleFavourite },

			{ "Joy0 Up+Joy0 Button0",     PrevLetter },
			{ "Joy0 Down+Joy0 Button0",   NextLetter },
			{ "Joy0 Left+Joy0 Button0",   FiltersMenu },
			{ "Joy0 Right+Joy0 Button0",  NextFilter },
			{ "Joy0 Up+Joy0 Button1",     Configure },
			{ "Joy0 Down+Joy0 Button1",   EditGame },
			{ "Joy0 Button0+Joy0 Button1",ToggleFavourite },

			{ NULL,                       LAST_COMMAND }
		};

		int i=0;
		while ( imap[i].label != NULL )
		{
			m_inputs.push_back( FeInputMapEntry( imap[i].label, imap[i].command ) );
			i++;
		}
	}

	//
	// Now ensure that the various 'UI' commands and 'Select' are mapped
	//
	std::vector < bool > ui_mapped( (int)Select+1, false );

	std::vector< FeInputMapEntry >::iterator it;
	for ( it = m_inputs.begin(); it != m_inputs.end(); ++it )
	{
		if ( it->command <= Select )
			ui_mapped[ it->command ] = true;
	}

	bool fix = false;
	for ( unsigned int i=0; i<ui_mapped.size(); i++ )
	{
		if ( !ui_mapped[i] )
		{
			fix=true;
			break;
		}
	}

	if ( fix )
	{
		//
		// The default UI command mappings:
		//
		struct DefaultMappings { FeInputSingle::Type type; int code; Command comm; };
		DefaultMappings dmap[] =
		{
			{ FeInputSingle::Keyboard,    sf::Keyboard::Escape,        Back },
			{ FeInputSingle::Joystick0,   FeInputSingle::JoyButton0+1, Back },
			{ FeInputSingle::Keyboard,    sf::Keyboard::Up,            Up },
			{ FeInputSingle::Joystick0,   FeInputSingle::JoyUp,        Up },
			{ FeInputSingle::Keyboard,    sf::Keyboard::Down,          Down },
			{ FeInputSingle::Joystick0,   FeInputSingle::JoyDown,      Down },
			{ FeInputSingle::Keyboard,    sf::Keyboard::Left,          Left },
			{ FeInputSingle::Joystick0,   FeInputSingle::JoyLeft,      Left },
			{ FeInputSingle::Keyboard,    sf::Keyboard::Right,         Right },
			{ FeInputSingle::Joystick0,   FeInputSingle::JoyRight,     Right },
			{ FeInputSingle::Keyboard,    sf::Keyboard::Return,        Select },
			{ FeInputSingle::Keyboard,    sf::Keyboard::LControl,      Select },
			{ FeInputSingle::Joystick0,   FeInputSingle::JoyButton0,   Select },

#ifdef SFML_SYSTEM_ANDROID
			{ FeInputSingle::Touch,       FeInputSingle::SwipeUp,      Up },
			{ FeInputSingle::Touch,       FeInputSingle::SwipeDown,    Down },
			{ FeInputSingle::Touch,       FeInputSingle::SwipeLeft,    Left },
			{ FeInputSingle::Touch,       FeInputSingle::SwipeRight,   Right },
			{ FeInputSingle::Touch,       FeInputSingle::Tap,          Select },
#endif

			{ FeInputSingle::Unsupported, sf::Keyboard::Unknown,     LAST_COMMAND }	// keep as last
		};

		int i=0;
		while ( dmap[i].comm != LAST_COMMAND )
		{
			// This will overwrite any conflicting input mapping
			if ( !ui_mapped[ dmap[i].comm ] )
				m_inputs.push_back( FeInputMapEntry( dmap[i].type, dmap[i].code, dmap[i].comm ) );

			i++;
		}
	}

	//
	// Now setup m_single_map
	//
	std::map< FeInputSingle, std::vector< FeInputMapEntry * > >::iterator itm;
	m_single_map.clear();

	for ( std::vector < FeInputMapEntry >::iterator ite=m_inputs.begin(); ite != m_inputs.end(); ++ite )
	{
		for ( std::set < FeInputSingle >::iterator its=(*ite).inputs.begin(); its != (*ite).inputs.end(); ++its )
		{
			itm = m_single_map.find( *its );
			if ( itm != m_single_map.end() )
				(*itm).second.push_back( &( *ite ) );
			else
			{
				std::vector< FeInputMapEntry *> temp;
				temp.push_back( &( *ite ) );
				m_single_map[ *its ] = temp;
			}
		}
	}

	for ( itm = m_single_map.begin(); itm != m_single_map.end(); ++itm )
		std::sort( (*itm).second.begin(), (*itm).second.end(), my_sort_fn );

	joy_raw_map_init( m_joy_config );
}

void FeInputMap::on_joystick_connect()
{
	joy_raw_map_init( m_joy_config );
}

FeInputMap::Command FeInputMap::get_command_from_tracked_keys( const int joy_thresh ) const
{
	FeInputMap::Command retval = FeInputMap::LAST_COMMAND;
	size_t rank=0; // only replace retval if we found a higher rank

	for ( std::set< FeInputSingle >::iterator itt = m_tracked_keys.begin(); itt != m_tracked_keys.end(); ++itt )
	{
		//
		// Check for trigger on tracked keys
		//
		std::map< FeInputSingle, std::vector< FeInputMapEntry * > >::const_iterator it;
		it = m_single_map.find( *itt );

		if ( it == m_single_map.end() )
		{
			// this should not happen!
			ASSERT( 0 );
			continue;
		}

		std::vector< FeInputMapEntry *>::const_iterator itv;
		for ( itv = (*it).second.begin(); itv != (*it).second.end(); ++itv )
		{
			bool found=true;

			for ( std::set< FeInputSingle >::const_iterator its = (*itv)->inputs.begin();
					its != (*itv)->inputs.end(); ++its )
			{
				if ( m_tracked_keys.find( *its ) == m_tracked_keys.end() )
				{
					found=false;
					break;
				}
			}

			if ( found && ( (*itv)->inputs.size() > rank ) )
			{
				retval = (*itv)->command;
				rank = (*itv)->inputs.size();
			}
		}
	}

	if ( rank > 0 )
		m_tracked_keys.clear();

	return retval;
}

FeInputMap::Command FeInputMap::map_input( const sf::Event &e, const sf::IntRect &mc_rect, const int joy_thresh )
{
	FeInputSingle index( e, mc_rect, joy_thresh );

	switch ( e.type )
	{
	case sf::Event::Closed:
		m_tracked_keys.clear();
		return ExitToDesktop;

	case sf::Event::JoystickMoved:
		{
			//
			// Test that this is a release of a tracked key (joystick axis)
			//
			if (( !index.get_current_state( joy_thresh ) )
				&& ( m_tracked_keys.find( index ) != m_tracked_keys.end() ))
			{
				FeInputMap::Command temp = get_command_from_tracked_keys( joy_thresh );
				if ( temp != LAST_COMMAND )
					return temp;
			}
		}
		break;

	case sf::Event::KeyReleased:
	case sf::Event::JoystickButtonReleased:
	case sf::Event::MouseButtonReleased:
		{
			//
			// Test that this is a release of a tracked key (button)
			//
			sf::Event te = e;
			switch ( e.type )
			{
			case sf::Event::KeyReleased: te.type = sf::Event::KeyPressed; break;
			case sf::Event::JoystickButtonReleased: te.type = sf::Event::JoystickButtonPressed; break;
			case sf::Event::MouseButtonReleased: te.type = sf::Event::MouseButtonPressed; break;
			default: ASSERT( 0 ); break;
			}

			FeInputSingle tt( te, mc_rect, joy_thresh );
			if ( m_tracked_keys.find( tt ) != m_tracked_keys.end() )
			{
				FeInputMap::Command temp = get_command_from_tracked_keys( joy_thresh );
				if ( temp != LAST_COMMAND )
					return temp;
			}
		}
		break;

#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 )) // touch support sfml >= 2.2
	case sf::Event::TouchEnded:
		{
			m_tracked_keys.insert( index );
			FeInputMap::Command temp = get_command_from_tracked_keys( joy_thresh );
			if ( temp != LAST_COMMAND )
				return temp;

			m_tracked_keys.erase( index );
			return LAST_COMMAND;
		}
		break;
#endif

	default:
		break;
	}

	if ( index.get_type() == FeInputSingle::Unsupported )
		return LAST_COMMAND;

	std::map< FeInputSingle, std::vector< FeInputMapEntry * > >::const_iterator it;
	it = m_single_map.find( index );

	if ( it == m_single_map.end() )
		return LAST_COMMAND;

	//
	// Special case: if this is a single button press for a repeatable command
	// then we trigger on the down event.  For everthing else we wait for up
	// to trigger.
	//
	std::vector< FeInputMapEntry *>::const_iterator itv;
	for ( itv = (*it).second.begin(); itv != (*it).second.end(); ++itv )
	{
		FeInputMap::Command c = (*itv)->command;

		if (( is_repeatable_command(c) || (is_ui_command(c) && ( c != Back )))
			&& ( (*itv)->inputs.size() == 1 ) )
		{
			m_tracked_keys.insert( index );

			FeInputMap::Command temp = get_command_from_tracked_keys( joy_thresh );

			if ( temp != LAST_COMMAND )
				return temp;

			return c;
		}
	}

	m_tracked_keys.insert( index );

	return LAST_COMMAND;
}

FeInputMap::Command FeInputMap::input_conflict_check( const FeInputMapEntry &e )
{
	if ( e.inputs.empty() )
		return FeInputMap::LAST_COMMAND;

	std::vector< FeInputMapEntry >::iterator ite;
	for ( ite=m_inputs.begin(); ite!=m_inputs.end(); ++ite )
	{
		if ( (*ite).inputs.size() == e.inputs.size() )
		{
			bool match=true;
			std::set< FeInputSingle >::iterator it;
			for ( it=(*ite).inputs.begin(); it!=(*ite).inputs.end(); ++it )
			{
				if ( e.inputs.find( (*it) ) == e.inputs.end() )
				{
					match=false;
					break;
				}
			}

			if ( match )
				return (*ite).command;
		}
	}

	return FeInputMap::LAST_COMMAND;
}

FeInputMap::Command FeInputMap::get_default_command( FeInputMap::Command c )
{
	if (( c < 0 ) || ( c >= Select ))
	{
		ASSERT( 0 ); // this shouldn't be happening
		return LAST_COMMAND;
	}

	return m_defaults[ c ];
}

void FeInputMap::set_default_command( FeInputMap::Command c, FeInputMap::Command v )
{
	m_defaults[ c ] = v;
}

bool FeInputMap::get_current_state( FeInputMap::Command c, int joy_thresh ) const
{
	std::vector< FeInputMapEntry >::const_iterator it;
	for ( it=m_inputs.begin(); it!=m_inputs.end(); ++it )
	{
		if (( (*it).command == c ) && (*it).get_current_state( joy_thresh ) )
			return true;
	}

	return false;
}

void FeInputMap::get_mappings( std::vector< FeMapping > &mappings ) const
{
	//
	// Make a mappings entry for each possible command mapping
	//
	mappings.clear();
	mappings.reserve( LAST_COMMAND );
	for ( int i=0; i< LAST_COMMAND; i++ )
		mappings.push_back( FeMapping( (FeInputMap::Command)i ) );

	//
	// Now populate the mappings vector with the various input mappings
	//
	std::vector< FeInputMapEntry >::const_iterator it;

	for ( it=m_inputs.begin(); it!=m_inputs.end(); ++it )
		mappings[ (*it).command ].input_list.push_back( (*it).as_string() );
}

void FeInputMap::set_mapping( const FeMapping &mapping )
{
	Command cmd = mapping.command;

	if ( cmd == LAST_COMMAND )
		return;

	std::vector< std::string >::const_iterator iti;

	for ( int i=m_inputs.size()-1; i>=0; i-- )
	{
		// Erase existing mappings to this command
		//
		if ( m_inputs[i].command == cmd )
		{
			if ( m_inputs[i].has_mouse_move() )
				m_mmove_count--;

			m_inputs.erase( m_inputs.begin() + i );
			continue;
		}

		// Erase conflicting inputs
		//
		std::string temp_str = m_inputs[i].as_string();
		for ( iti = mapping.input_list.begin(); iti != mapping.input_list.end(); ++iti )
		{
			if ( (*iti).compare( temp_str ) == 0 )
			{
				m_inputs.erase( m_inputs.begin() + i );
				continue;
			}
		}
	}

	//
	// Now update our map with the inputs from this mapping
	//
	for ( iti=mapping.input_list.begin();
			iti!=mapping.input_list.end(); ++iti )
	{
		FeInputMapEntry new_entry( *iti, cmd );

		if (!new_entry.inputs.empty())
		{
			m_inputs.push_back( new_entry );

			if ( new_entry.has_mouse_move() )
				m_mmove_count++;
		}
	}

	initialize_mappings();
}

int FeInputMap::process_setting( const std::string &setting,
	const std::string &value,
	const std::string &fn )
{
	if ( setting.compare( "default" ) == 0 )
	{
		// value: "<command> <command>"
		size_t pos=0;
		std::string from, to;
		Command fc, tc=LAST_COMMAND;

		token_helper( value, pos, from, FE_WHITESPACE );
		token_helper( value, pos, to, FE_WHITESPACE );

		fc = string_to_command( from );

		if (( fc < Select ) && ( !to.empty() ))
			tc = string_to_command( to );

		m_defaults[fc]=tc;
		return 0;
	}
	else if ( setting.compare( "map" ) == 0 )
	{
		size_t pos=0;
		std::string joy_num;

		token_helper( value, pos, joy_num, FE_WHITESPACE );

		if ( joy_num.compare( 0, 3, "Joy" ) == 0 )
		{
			int num = as_int( value.substr( 3 ) );

			std::string joy_name;
			token_helper( value, pos, joy_name );

			if (( num >= 0 ) && ( num <= sf::Joystick::Count ))
				m_joy_config.push_back( std::pair<int, std::string>( num, joy_name ) );
		}

		return 0;
	}

	Command cmd = string_to_command( setting );
	if ( cmd == LAST_COMMAND )
	{
		invalid_setting( fn, "input_map", setting, commandStrings, NULL, "command" );
		return 1;
	}

	FeInputMapEntry new_entry( value, cmd );
	if ( new_entry.inputs.empty() )
	{
		FeLog() << "Unrecognized input type: " << value << " in file: " << fn << std::endl;
		return 1;
	}

	m_inputs.push_back( new_entry );

	if ( new_entry.has_mouse_move() )
		m_mmove_count++;

	return 0;
}

void FeInputMap::save( nowide::ofstream &f ) const
{
	std::vector< FeInputMapEntry >::const_iterator it;

	for ( it = m_inputs.begin(); it != m_inputs.end(); ++it )
	{
		f << '\t' << std::setw(20) << std::left
			<< commandStrings[ (*it).command ] << ' '
			<< (*it).as_string() << std::endl;
	}

	for ( int i=0; i < (int)Select; i++ )
	{
		std::string def_str = ( m_defaults[i] == LAST_COMMAND ) ? "" : commandStrings[ m_defaults[i] ];
		f << '\t' << std::setw(20) << std::left << "default "
			<< commandStrings[ i ] << '\t' << def_str << std::endl;
	}

	for ( size_t i=0; i < m_joy_config.size(); i++ )
	{
		f << '\t' << std::setw(20) << std::left << "map "
			<< "Joy" << m_joy_config[i].first << '\t' << m_joy_config[i].second << std::endl;
	}
}

FeInputMap::Command FeInputMap::string_to_command( const std::string &s )
{
	int i=0;

	while ( FeInputMap::commandStrings[i] != NULL )
	{
		if ( s.compare( commandStrings[i] ) == 0 )
			return (Command)i;

		i++;
	}

	//
	// For backward compatability... with 1.5 the "*_list" was switched to "*_display"
	//
	if ( s.compare( "prev_list" ) == 0 )
		return PrevDisplay;
	else if ( s.compare( "next_list" ) == 0 )
		return NextDisplay;
	else if ( s.compare( "page_up" ) == 0 ) // after 2.0.0, page_up/down became prev/next_page
		return PrevPage;
	else if ( s.compare( "page_down" ) == 0 )
		return NextPage;
	else if ( s.compare( "exit_no_menu" ) == 0 ) // after 2.2.1, exit_no_menu became exit_to_desktop
		return ExitToDesktop;

	return LAST_COMMAND;
}

// these are the commands that are repeatable if the input key is held down
//
bool FeInputMap::is_repeatable_command( FeInputMap::Command c )
{
	return (( c == PrevGame ) || ( c == NextGame )
		|| ( c == PrevPage ) || ( c == NextPage )
		|| ( c == NextLetter ) || ( c == PrevLetter )
		|| ( c == NextFavourite ) || ( c == PrevFavourite ));
}

// return true if c is the "Up", "Down", "Left", "Right", or "Back" command
//
bool FeInputMap::is_ui_command( FeInputMap::Command c )
{
	return ( c < Select );
}

// Note the alignment of settingStrings matters in fe_config.cpp
// (FeSoundMenu::get_options)
//
const char *FeSoundInfo::settingStrings[] =
{
	"sound_volume",
	"ambient_volume",
	"movie_volume",
	NULL
};
const char *FeSoundInfo::settingDispStrings[] =
{
	"Sound Volume",
	"Ambient Volume",
	"Movie Volume",
	NULL
};

FeSoundInfo::FeSoundInfo()
	: m_sound_vol( 100 ),
	m_ambient_vol( 100 ),
	m_movie_vol( 100 ),
	m_mute( false )
{
}

void FeSoundInfo::set_volume( SoundType t, const std::string &str )
{
	int v = as_int( str );

	if ( v < 0 )
		v = 0;
	else if ( v > 100 )
		v = 100;

	switch ( t )
	{
	case Movie:
		m_movie_vol = v;
		break;

	case Ambient:
		m_ambient_vol = v;
		break;

	case Sound:
	default:
		m_sound_vol = v;
	}
}

int FeSoundInfo::get_set_volume( SoundType t ) const
{
	switch ( t )
	{
	case Movie:
		return m_movie_vol;
	case Ambient:
		return m_ambient_vol;
	case Sound:
	default:
		return m_sound_vol;
	}
}

int FeSoundInfo::get_play_volume( SoundType t ) const
{
	if ( m_mute )
		return 0;

	return get_set_volume( t );
}

bool FeSoundInfo::get_mute() const
{
	return m_mute;
}

void FeSoundInfo::set_mute( bool m )
{
	m_mute=m;
}

int FeSoundInfo::process_setting( const std::string &setting,
	const std::string &value,
	const std::string &fn )
{
	if ( setting.compare( settingStrings[0] ) == 0 ) // sound_vol
		set_volume( Sound, value );
	else if ( setting.compare( settingStrings[1] ) == 0 ) // ambient_vol
		set_volume( Ambient, value );
	else if ( setting.compare( settingStrings[2] ) == 0 ) // movie_vol
		set_volume( Movie, value );
	else
	{
		bool found=false;
		for ( int i=0; i < FeInputMap::LAST_EVENT; i++ )
		{
			if ( ( FeInputMap::commandStrings[i] )
				&& ( setting.compare( FeInputMap::commandStrings[i] ) == 0 ))
			{
				found=true;
				m_sounds[ (FeInputMap::Command)i ] = value;
				break;
			}
		}

		if (!found)
		{
			invalid_setting( fn, "sound", setting, settingStrings, FeInputMap::commandStrings );
			return 1;
		}
	}

	return 0;
}

bool FeSoundInfo::get_sound( FeInputMap::Command c, std::string &name ) const
{
	if (( !m_mute ) && ( m_sound_vol > 0 ))
	{
		std::map<FeInputMap::Command, std::string>::const_iterator it;

   	it = m_sounds.find( c );

   	if ( it == m_sounds.end() )
     		return false;

		name = (*it).second;
	}
	return true;
}

void FeSoundInfo::set_sound( FeInputMap::Command c, const std::string &name )
{
	if ( name.empty() )
		m_sounds.erase( c );
	else
		m_sounds[ c ] = name;
}

void FeSoundInfo::save( nowide::ofstream &f ) const
{
	std::map<FeInputMap::Command, std::string>::const_iterator it;

	for ( int i=0; i< 3; i++ )
		f << '\t' << std::setw(20) << std::left << settingStrings[i] << ' '
			<< get_set_volume( (SoundType)i ) << std::endl;

	for ( it=m_sounds.begin(); it!=m_sounds.end(); ++it )
		f << '\t' << std::setw(20) << std::left
			<< FeInputMap::commandStrings[ (*it).first ]
			<< ' ' << (*it).second << std::endl;
}
