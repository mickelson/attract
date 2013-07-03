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

#include "fe_present.hpp"
#include "fe_util.hpp"
#include <iostream>

FePresent::FePresent( FeSettings *fesettings, sf::Font &defaultfont )
	: m_feSettings( fesettings ), 
	m_currentFont( NULL ), 
	m_defaultFont( defaultfont ), 
	m_moveState( MoveNone ), 
	m_baseRotation( FeSettings::RotateNone ), 
	m_toggleRotation( FeSettings::RotateNone ), 
	m_play_movies( true ), 
	m_currentConfigObject( NULL ), 
	m_listBox( NULL ) 
{
}

FePresent::~FePresent() 
{
	clear();
}

void FePresent::clear()
{
	//
	// keep toggle rotation state and mute state through clear
	//
	m_currentConfigObject=NULL;
	m_listBox=NULL; // listbox gets deleted with the m_elements below
	m_moveState = MoveNone;
	m_baseRotation = FeSettings::RotateNone;
	m_scaleTransform = sf::Transform();
	m_currentFont = &m_defaultFont;

	m_movies.clear();

	while ( !m_elements.empty() )
	{
		FeBasePresentable *p = m_elements.back();
		m_elements.pop_back();
		delete p;
	}
}

void FePresent::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	states = m_rotationTransform * m_scaleTransform;

	for ( std::vector<FeBasePresentable *>::const_iterator itl=m_elements.begin();
               itl != m_elements.end(); ++itl )
	{
		target.draw( (*itl)->drawable(), states );
	}
}

int FePresent::process_setting( const std::string &setting,
                        const std::string &value,
								const std::string &fn )
{
	const char *stokens[] =
	{
		"layout_size",
		"font",
		"image",
		"artwork",
		"text",
		"list",
		"movie",
		"animation",
		"layout_rotation",
		NULL
	};

   if ( setting.compare( stokens[0] ) == 0 ) // layout_size
	{
   	size_t pos=0;
   	std::string val;

		// size is WW,HH
		token_helper( value, pos, val, ",x" );
		int width = as_int( val );
		token_helper( value, pos, val );
		int height = as_int( val );

		sf::VideoMode vm = sf::VideoMode::getDesktopMode();

		m_scaleTransform.scale( (float) vm.width / width, 
						(float) vm.height / height );
	}
   else if ( setting.compare( stokens[1] ) == 0 ) // font
	{
		std::string filename;
		if ( m_feSettings->get_font_file( filename, value ) )
		{
			if ( m_layoutFont.loadFromFile( filename ) )
				m_currentFont = &m_layoutFont;
		}
	}
   else if ( setting.compare( stokens[2] ) == 0 ) // image
   {
		FeImage *new_image = new FeImage();
		std::string filename = m_feSettings->get_current_layout_dir();
		filename += value;
		new_image->loadFromFile( filename );

		m_currentConfigObject = new_image;
		m_elements.push_back( new_image );
   }
	else if ( setting.compare( stokens[3] ) == 0 ) // artwork
	{
		FeArtwork *ae = new FeArtwork( value );

		m_currentConfigObject = ae;
		m_elements.push_back( ae );

		// print out a message if the artwork label 
		//  is not defined for the current emulator
		if ( m_feSettings->confirm_artwork( value ) == false )
		{
			std::cout << "Note: artwork \"" << value
				<< "\" is used by layout but not configured for current emulator."
				<< std::endl;
		}
	}
	else if ( setting.compare( stokens[4] ) == 0 ) // text
	{
		FeText *te = new FeText( value );

		if ( m_currentFont )
			te->setFont( *m_currentFont );

		m_currentConfigObject = te;
		m_elements.push_back( te );
	}
	else if ( setting.compare( stokens[5] ) == 0 ) // list
	{
		FeListBox *lb = new FeListBox();

		if ( m_currentFont )
			lb->setFont( *m_currentFont );

		m_currentConfigObject = m_listBox = lb;
		m_elements.push_back( lb );
	}
	else if ( setting.compare( stokens[6] ) == 0 ) // movie
	{
#ifdef NO_MOVIE
		FeArtwork *me = new FeArtwork( m_feSettings->get_movie_artwork() );
#else
		FeMovie *me = new FeMovie( m_feSettings->get_movie_artwork() );
#endif
		m_currentConfigObject = me;
		m_elements.push_back( me );
		m_movies.push_back( me );
	}
	else if ( setting.compare( stokens[7] ) == 0 ) // animation
	{
		std::string filename = m_feSettings->get_current_layout_dir();
		filename += value;

		FeAnimate *ani = new FeAnimate( filename );
		m_elements.push_back( ani );
		m_movies.push_back( ani );
		m_currentConfigObject = ani;
	}
	else if ( setting.compare( stokens[8] ) == 0 ) // layout_rotation
	{
		int i=0;
		while ( FeSettings::rotationTokens[i] != NULL )
		{
			if ( value.compare( FeSettings::rotationTokens[i] ) == 0 )
			{
				m_baseRotation = (FeSettings::RotationState)i;
				break;
			}
			i++;
		}

		if ( FeSettings::rotationTokens[i] == NULL )
		{
			invalid_setting( fn, stokens[8], setting, 
				FeSettings::rotationTokens, NULL, "value" );
			return 1;
		}
	}
	else if ( m_currentConfigObject != NULL )
	{
		m_currentConfigObject->process_setting( setting, value, fn );
	}
	else
	{
		invalid_setting( fn, "layout", setting, stokens );
		return 1;
	}
	return 0;
}

bool FePresent::handle_event( FeInputMap::Command c, sf::Event ev )
{
	m_moveState=MoveNone;
	bool retval=false;

	switch( c )
	{
	case FeInputMap::Down:
		m_moveTimer.restart();
		m_moveState=MoveDown;
		m_moveEvent = ev;
		m_feSettings->change_rom( 1 );
		update( false );
		retval=true;
		break;

	case FeInputMap::Up:
		m_moveTimer.restart();
		m_moveState=MoveUp;
		m_moveEvent = ev;
		m_feSettings->change_rom( -1 );
		update( false );
		retval=true;
		break;

	case FeInputMap::PageDown:
		m_moveTimer.restart();
		m_moveState=MovePageDown;
		m_moveEvent = ev;
		m_feSettings->change_rom( get_page_size(), false );
		update( false );
		retval=true;
		break;

	case FeInputMap::PageUp:
		m_moveTimer.restart();
		m_moveState=MovePageUp;
		m_moveEvent = ev;
		m_feSettings->change_rom( -1 * get_page_size(), false );
		update( false );
		retval=true;
		break;

	case FeInputMap::ToggleRotateRight:
		toggle_rotate( FeSettings::RotateRight );
		retval=true;
		break;

	case FeInputMap::ToggleFlip:
		toggle_rotate( FeSettings::RotateFlip );
		retval=true;
		break;

	case FeInputMap::ToggleRotateLeft:
		toggle_rotate( FeSettings::RotateLeft );
		retval=true;
		break;

	case FeInputMap::ToggleMovie:
		toggle_movie();
		retval=true;
		break;

	case FeInputMap::Select:
	case FeInputMap::ExitMenu:
	default:
		// dealt with elsewhere
		break;
	}

	return retval;
}

int FePresent::update( bool new_list ) 
{
	std::vector<FeBasePresentable *>::iterator itl;
	if ( new_list )
	{
		for ( itl=m_elements.begin(); itl != m_elements.end(); ++itl )
			(*itl)->on_new_list( m_feSettings );
	}

	for ( itl=m_elements.begin(); itl != m_elements.end(); ++itl )
		(*itl)->on_new_selection( m_feSettings );

	m_movieStartTimer.restart();

	return 0;
}


int FePresent::load_layout() 
{
	clear();

	if ( m_feSettings->lists_count() < 1 )
	{
		set_to_no_lists_message();
		update( true );
		return 0;
	}

	std::string layout_file = m_feSettings->get_current_layout_file();

	if ( !layout_file.empty() )
	{
		if ( load_from_file( layout_file ) == false )
	      std::cout << "Error, file not found: " << layout_file << std::endl;
	}

	// make things usable if the layout is empty
	if ( m_elements.empty() )
	{
		// Nothing loaded, default to a full screen list
		//
		sf::VideoMode vm = sf::VideoMode::getDesktopMode();

		// Use the first artwork (if any are defined)
		//
		FeArtwork *art = new FeArtwork( "" ); 
		art->setPosition( 0, 0 );
		art->setSize( vm.width, vm.height );
		art->setColor( sf::Color( 100, 100, 100, 180 ) );
		m_elements.push_back( art );

		FeListBox *lb = new FeListBox();
		if ( m_currentFont ) lb->setFont( *m_currentFont );

		lb->setPosition( sf::Vector2f( 0, 0 ) );
		lb->setSize( sf::Vector2f( vm.width, vm.height ) );
		m_elements.push_back( lb );
		m_listBox = lb;
	}

	set_rotation_transform();
	update( true );
	return 0;
}

bool FePresent::tick()
{
	bool ret_val = false;
	if ( m_moveState != MoveNone )
	{
		sf::Time t = m_moveTimer.getElapsedTime();
		if ( t.asMilliseconds() > 500 )
		{
			bool cont=false;

			if ( m_moveEvent.type == sf::Event::KeyPressed )
			{
				if ( sf::Keyboard::isKeyPressed( m_moveEvent.key.code ) )
					cont=true;
			}
			else if ( m_moveEvent.type == sf::Event::JoystickButtonPressed )
			{
				if ( sf::Joystick::isButtonPressed( 
									m_moveEvent.joystickButton.joystickId,
									m_moveEvent.joystickButton.button ) )
					cont=true;
			}
			else if ( m_moveEvent.type == sf::Event::JoystickMoved )
			{
				float pos = sf::Joystick::getAxisPosition( 
									m_moveEvent.joystickMove.joystickId,
									m_moveEvent.joystickMove.axis );
				if ( abs( pos ) > FeInputMap::JOY_THRESH )
					cont=true;
			}

			if ( cont )
			{
				switch ( m_moveState )
				{
					case MoveUp: m_feSettings->change_rom( -1, false ); break;
					case MoveDown: m_feSettings->change_rom( 1, false ); break;
					case MovePageUp: 
						m_feSettings->change_rom( -1 * get_page_size(), false ); 
						break;
					case MovePageDown: 
						m_feSettings->change_rom( get_page_size(), false ); 
						break;
					default:
						break;
				}

				ret_val=true;
				update( false );
			}
			else
			{
				m_moveState = MoveNone;
			}
		}
	}

	//
	// Start movies after a small delay
	//
	if (( m_movieStartTimer.getElapsedTime().asMilliseconds() > 500 )
			&& ( m_play_movies ))
	{
		for ( std::vector<FeBasePresentable *>::iterator itm=m_movies.begin();
				itm != m_movies.end(); ++itm )
		{
			if ( (*itm)->tick( m_feSettings ) )
				ret_val=true;
		}
	}

	return ret_val;
}

int FePresent::get_page_size()
{
	if ( m_listBox )
		return m_listBox->getRowCount();
	else
		return 5;
}

void FePresent::play( bool play_state )
{
	for ( std::vector<FeBasePresentable *>::iterator itm=m_movies.begin();
				itm != m_movies.end(); ++itm )
		(*itm)->set_play_state( play_state );
}

void FePresent::toggle_movie()
{
	m_play_movies = !m_play_movies;
	play( m_play_movies );
}

void FePresent::toggle_mute()
{
	for ( std::vector<FeBasePresentable *>::iterator itm=m_movies.begin();
				itm != m_movies.end(); ++itm )
		(*itm)->set_vol( m_feSettings->get_play_volume( FeSoundInfo::Movie ) );
}

const sf::Transform &FePresent::get_rotation_transform() const
{
	return m_rotationTransform;
}

const sf::Font *FePresent::get_font() const
{
	return m_currentFont;
}

void FePresent::set_default_font( sf::Font &f )
{
	m_defaultFont = f;
}

void FePresent::toggle_rotate( FeSettings::RotationState r )
{
	if ( m_toggleRotation != FeSettings::RotateNone )
		m_toggleRotation = FeSettings::RotateNone;
	else
		m_toggleRotation = r;

	set_rotation_transform();
}

void FePresent::set_rotation_transform()
{
	sf::VideoMode vm = sf::VideoMode::getDesktopMode();
	m_rotationTransform = sf::Transform();

	FeSettings::RotationState actualRotation 
		= (FeSettings::RotationState)(( m_baseRotation + m_toggleRotation ) % 4);

	switch ( actualRotation )
	{
	case FeSettings::RotateNone:
		// do nothing
		break;
	case FeSettings::RotateRight:
		m_rotationTransform.translate( vm.width, 0 );
		m_rotationTransform.scale( (float) vm.width / vm.height,
												(float) vm.height / vm.width );
		m_rotationTransform.rotate(90);
		break;
	case FeSettings::RotateFlip:
		m_rotationTransform.translate( vm.width, vm.height );
		m_rotationTransform.rotate(180);
		break;
	case FeSettings::RotateLeft:
		m_rotationTransform.translate( 0, vm.height );
		m_rotationTransform.scale( (float) vm.width / vm.height,
											(float) vm.height / vm.width );
		m_rotationTransform.rotate(270);
		break;
	}
}

void FePresent::perform_autorotate()
{
	FeSettings::RotationState autorotate = m_feSettings->get_autorotate();

	if ( autorotate == FeSettings::RotateNone )
		return;

	std::string rom_rot = m_feSettings->get_rom_info( 0, 
													FeRomInfo::Rotation );

	m_toggleRotation = FeSettings::RotateNone;

	switch ( m_baseRotation )
	{
	case FeSettings::RotateLeft:
	case FeSettings::RotateRight:
		if (( rom_rot.compare( "0" ) == 0 ) || ( rom_rot.compare( "180" ) == 0 ))
			m_toggleRotation = autorotate;
		break;
	case FeSettings::RotateNone:
	case FeSettings::RotateFlip:
	default:
		if (( rom_rot.compare( "90" ) == 0 ) || ( rom_rot.compare( "270" ) == 0 ))
			m_toggleRotation = autorotate;
	}

	set_rotation_transform();
}

void FePresent::set_to_no_lists_message()
{
	sf::VideoMode vm = sf::VideoMode::getDesktopMode();

	std::string msg;
	m_feSettings->get_resource( "No lists configured.", msg );

	FeText *te = new FeText( msg );
	te->setPosition( sf::Vector2f( 0, 0 ) );
	te->setSize( sf::Vector2f( vm.width, vm.height ) );

	if ( m_currentFont )
		te->setFont( *m_currentFont );

	m_elements.push_back( te );
}
