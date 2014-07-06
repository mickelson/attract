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
#include "fe_image.hpp"
#include "fe_text.hpp"
#include "fe_listbox.hpp"
#include "fe_input.hpp"
#include "fe_util_sq.hpp"

#include <sqrat.h>

#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>

#include <iostream>
#include <stdio.h>
#include <ctime>
#include <stdarg.h>

void FeFontContainer::set_font( const std::string &n )
{
	m_name = n;
	m_font.loadFromFile( m_name );
}

const char *FePresent::transitionTypeStrings[] =
{
		"StartLayout",
		"EndLayout",
		"ToNewSelection",
		"FromOldSelection",
		"ToGame",
		"FromGame",
		"ToNewList",
		NULL
};

FePresent::FePresent( FeSettings *fesettings, FeFontContainer &defaultfont )
	: m_feSettings( fesettings ),
	m_currentScriptConfig( NULL ),
	m_currentFont( &defaultfont ),
	m_defaultFont( defaultfont ),
	m_moveState( MoveNone ),
	m_baseRotation( FeSettings::RotateNone ),
	m_toggleRotation( FeSettings::RotateNone ),
	m_playMovies( true ),
	m_screenSaverActive( false ),
	m_listBox( NULL ),
	m_emptyShader( NULL )
{
	Sqrat::DefaultVM::Set( NULL );

	sf::VideoMode vm = sf::VideoMode::getDesktopMode();
	m_outputSize.x = vm.width;
	m_outputSize.y = vm.height;

	m_layoutFontName = m_feSettings->get_info( FeSettings::DefaultFont );

	srand( time( NULL ) );
}

FePresent::~FePresent()
{
	clear();
	vm_close();
}

void FePresent::clear()
{
	//
	// keep toggle rotation state and mute state through clear
	//
	m_listBox=NULL; // listbox gets deleted with the m_elements below
	m_moveState = MoveNone;
	m_baseRotation = FeSettings::RotateNone;
	m_transform = sf::Transform();
	m_currentFont = &m_defaultFont;
	m_layoutFontName = m_feSettings->get_info( FeSettings::DefaultFont );
	m_ticksList.clear();
	m_transitionList.clear();

	while ( !m_elements.empty() )
	{
		FeBasePresentable *p = m_elements.back();
		m_elements.pop_back();
		delete p;
	}

	while ( !m_texturePool.empty() )
	{
		FeBaseTextureContainer *t = m_texturePool.back();
		m_texturePool.pop_back();
		delete t;
	}

	while ( !m_scriptSounds.empty() )
	{
		FeScriptSound *s = m_scriptSounds.back();
		m_scriptSounds.pop_back();
		delete s;
	}

	while ( !m_scriptShaders.empty() )
	{
		FeShader *sh = m_scriptShaders.back();
		m_scriptShaders.pop_back();
		delete sh;
	}

	if ( m_emptyShader )
	{
		delete m_emptyShader;
		m_emptyShader = NULL;
	}

	while ( !m_fontPool.empty() )
	{
		FeFontContainer *f = m_fontPool.back();
		m_fontPool.pop_back();
		delete f;
	}

	m_layoutSize = m_outputSize;
	m_layoutScale.x = 1.0;
	m_layoutScale.y = 1.0;
}

void FePresent::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	states.transform = m_transform;

	std::vector<FeBasePresentable *>::const_iterator itl;
	for ( itl=m_elements.begin(); itl != m_elements.end(); ++itl )
	{
		if ( (*itl)->get_visible() )
			target.draw( (*itl)->drawable(), states );
	}
}

FeImage *FePresent::add_image( bool is_artwork, const std::string &n, int x, int y, int w, int h,
										std::vector<FeBasePresentable *> &l )
{
	std::string name;
	if ( is_artwork )
		name = n;
	else
	{
		// If it is a relative path we assume it is in the layout directory
		//
		if ( is_relative_path( n ) )
			name = m_feSettings->get_current_layout_dir() + n;
		else
			name = n;
	}

	FeTextureContainer *new_tex = new FeTextureContainer( is_artwork, name );
	FeImage *new_image = new FeImage( new_tex, x, y, w, h );

	// if this is a non-artwork (i.e. static image/video) then load it now
	//
	if ( !is_artwork )
		new_tex->load_static( name );

	m_redrawTriggered = true;
	m_texturePool.push_back( new_tex );
	l.push_back( new_image );

	return new_image;
}

FeImage *FePresent::add_clone( FeImage *o,
			std::vector<FeBasePresentable *> &l )
{
	FeImage *new_image = new FeImage( o );
	m_redrawTriggered = true;
	l.push_back( new_image );
	return new_image;
}

FeText *FePresent::add_text( const std::string &n, int x, int y, int w, int h,
			std::vector<FeBasePresentable *> &l )
{
	FeText *new_text = new FeText( n, x, y, w, h );

	ASSERT( m_currentFont );
	new_text->setFont( m_currentFont->get_font() );

	m_redrawTriggered = true;
	l.push_back( new_text );
	return new_text;
}

FeListBox *FePresent::add_listbox( int x, int y, int w, int h,
			std::vector<FeBasePresentable *> &l )
{
	FeListBox *new_lb = new FeListBox( x, y, w, h );

	ASSERT( m_currentFont );
	new_lb->setFont( m_currentFont->get_font() );

	m_redrawTriggered = true;
	m_listBox = new_lb;
	l.push_back( new_lb );
	return new_lb;
}

FeImage *FePresent::add_surface( int w, int h, std::vector<FeBasePresentable *> &l )
{
	FeSurfaceTextureContainer *new_surface = new FeSurfaceTextureContainer( w, h );

	//
	// Set the default sprite size to the same as the texture itself
	//
	FeImage *new_image = new FeImage( new_surface, 0, 0, w, h );

	new_image->texture_changed();

	m_redrawTriggered = true;
	l.push_back( new_image );
	m_texturePool.push_back( new_surface );
	return new_image;
}

FeScriptSound *FePresent::add_sound( const std::string &n )
{
	std::string filename = m_feSettings->get_current_layout_dir();
	filename += n;

	FeScriptSound *new_sound = new FeScriptSound();
	new_sound->load( filename );
	new_sound->set_volume(
		m_feSettings->get_play_volume( FeSoundInfo::Sound ) );

	m_scriptSounds.push_back( new_sound );
	return new_sound;
}

FeShader *FePresent::add_shader( FeShader::Type type, const char *shader1, const char *shader2 )
{
	std::string path = m_feSettings->get_current_layout_dir();

	switch ( type )
	{
		case FeShader::VertexAndFragment:
			m_scriptShaders.push_back(
						new FeShader( type, (shader1 ? path + shader1 : ""),
										(shader2 ? path + shader2 : "") ) );
			break;

		case FeShader::Vertex:
			m_scriptShaders.push_back(
						new FeShader( type, (shader1 ? path + shader1 : ""), "" ) );
			break;

		case FeShader::Fragment:
			m_scriptShaders.push_back(
						new FeShader( type, "", (shader1 ? path + shader1 : "") ) );
			break;

		case FeShader::Empty:
		default:
			m_scriptShaders.push_back(
						new FeShader( type, "", "" ) );
			break;
	}

	return m_scriptShaders.back();
}

void FePresent::add_ticks_callback( const std::string &n )
{
	m_ticksList.push_back( n );
}

void FePresent::add_transition_callback( const std::string &n )
{
	m_transitionList.push_back( n );
}

int FePresent::get_layout_width() const
{
	return m_layoutSize.x;
}

int FePresent::get_layout_height() const
{
	return m_layoutSize.y;
}

float FePresent::get_layout_scale_x() const
{
	return m_layoutScale.x;
}

float FePresent::get_layout_scale_y() const
{
	return m_layoutScale.y;
}

void FePresent::set_layout_width( int w )
{
	m_layoutSize.x = w;
	m_layoutScale.x = (float) m_outputSize.x / w;
	set_transforms();
	m_redrawTriggered = true;
}

void FePresent::set_layout_height( int h )
{
	m_layoutSize.y = h;
	m_layoutScale.y = (float) m_outputSize.y / h;
	set_transforms();
	m_redrawTriggered = true;
}

const FeFontContainer *FePresent::get_pooled_font( const std::string &n )
{
	std::string fullpath;
	if ( !m_feSettings->get_font_file( fullpath, n ) )
		return NULL;

	// First check if this matches the default font
	//
	if ( fullpath.compare( m_defaultFont.get_name() ) == 0 )
		return &m_defaultFont;

	// Next check if this font is already loaded in our pool
	//
	for ( std::vector<FeFontContainer *>::iterator itr=m_fontPool.begin();
		itr != m_fontPool.end(); ++itr )
	{
		if ( fullpath.compare( (*itr)->get_name() ) == 0 )
			return *itr;
	}

	// No match, so load this font and add it to the pool
	//
	m_fontPool.push_back( new FeFontContainer() );
	m_fontPool.back()->set_font( fullpath );

	return m_fontPool.back();
}

void FePresent::set_layout_font( const char *n )
{
	const FeFontContainer *font = get_pooled_font( n );

	if ( font )
	{
		m_layoutFontName = n;
		m_currentFont = font;
		m_redrawTriggered = true;
	}
}

const char *FePresent::get_layout_font() const
{
	return m_layoutFontName.c_str();
}

void FePresent::set_base_rotation( int r )
{
	m_baseRotation = (FeSettings::RotationState)r;
	set_transforms();
	m_redrawTriggered = true;
}

int FePresent::get_base_rotation() const
{
	return m_baseRotation;
}

void FePresent::set_toggle_rotation( int r )
{
	m_toggleRotation = (FeSettings::RotationState)r;
	set_transforms();
	m_redrawTriggered = true;
}

int FePresent::get_toggle_rotation() const
{
	return m_toggleRotation;
}

const char *FePresent::get_list_name() const
{
	return m_feSettings->get_current_list_title().c_str();
}

const char *FePresent::get_filter_name() const
{
	return m_feSettings->get_current_filter_name().c_str();
}

int FePresent::get_list_size() const
{
	return m_feSettings->get_current_list_size();
}

int FePresent::get_list_index() const
{
	return m_feSettings->get_rom_index();
}

void FePresent::set_list_index( int index )
{
	int new_offset = index - get_list_index();
	if ( new_offset != 0 )
	{
		m_feSettings->change_rom( index - get_list_index() );
		update( false );
	}
}

bool FePresent::reset_screen_saver( sf::RenderWindow *wnd )
{
	if ( m_screenSaverActive )
	{
		// Reset from screen saver
		//
		load_layout( wnd );
		return true;
	}

	m_lastInput=m_layoutTimer.getElapsedTime();
	return false;
}

bool FePresent::handle_event( FeInputMap::Command c,
	const sf::Event &ev,
	sf::RenderWindow *wnd )
{
	m_moveState=MoveNone;

	if ( reset_screen_saver( wnd ) )
		return true;

	switch( c )
	{
	case FeInputMap::Down:
		if ( m_moveState == MoveNone )
		{
			m_moveTimer.restart();
			m_moveState=MoveDown;
			m_moveEvent = ev;
			vm_on_transition( ToNewSelection, 1, wnd );

			m_feSettings->change_rom( 1 );
			update( false );

			vm_on_transition( FromOldSelection, -1, wnd );
		}
		break;

	case FeInputMap::Up:
		if ( m_moveState == MoveNone )
		{
			m_moveTimer.restart();
			m_moveState=MoveUp;
			m_moveEvent = ev;
			vm_on_transition( ToNewSelection, -1, wnd );

			m_feSettings->change_rom( -1 );
			update( false );

			vm_on_transition( FromOldSelection, 1, wnd );
		}
		break;

	case FeInputMap::PageDown:
		if ( m_moveState == MoveNone )
		{
			m_moveTimer.restart();
			m_moveState=MovePageDown;
			m_moveEvent = ev;

			int step = get_no_wrap_step( get_page_size() );
			if ( step != 0 )
			{
				vm_on_transition( ToNewSelection, step, wnd );

				m_feSettings->change_rom( step );
				update( false );

				vm_on_transition( FromOldSelection, -step, wnd );
			}
		}
		break;

	case FeInputMap::PageUp:
		if ( m_moveState == MoveNone )
		{
			m_moveTimer.restart();
			m_moveState=MovePageUp;
			m_moveEvent = ev;
			int step = get_no_wrap_step( -get_page_size() );
			if ( step != 0 )
			{
				vm_on_transition( ToNewSelection, step, wnd );

				m_feSettings->change_rom( step );
				update( false );

				vm_on_transition( FromOldSelection, -step, wnd );
			}
		}
		break;

	case FeInputMap::RandomGame:
		{
			int step = rand() % m_feSettings->get_current_list_size();
			if ( step != 0 )
			{
				vm_on_transition( ToNewSelection, step, wnd );

				m_feSettings->change_rom( step );
				update( false );

				vm_on_transition( FromOldSelection, -step, wnd );
			}
		}
		break;

	case FeInputMap::ToggleRotateRight:
		toggle_rotate( FeSettings::RotateRight );
		break;

	case FeInputMap::ToggleFlip:
		toggle_rotate( FeSettings::RotateFlip );
		break;

	case FeInputMap::ToggleRotateLeft:
		toggle_rotate( FeSettings::RotateLeft );
		break;

	case FeInputMap::ToggleMovie:
		toggle_movie();
		break;

	case FeInputMap::NextList:
		// next_list returns true if the layout changes with the new list
		//
		if ( m_feSettings->next_list() )
			load_layout( wnd );
		else
			update_to_new_list( wnd );

		break;

	case FeInputMap::PrevList:
		// prev_list returns true if the layout changes with the new list
		//
		if ( m_feSettings->prev_list() )
			load_layout( wnd );
		else
			update_to_new_list( wnd );

		break;

	case FeInputMap::NextFilter:
		m_feSettings->set_filter( m_feSettings->get_current_filter_index() + 1 );
		update_to_new_list( wnd );
		break;

	case FeInputMap::PrevFilter:
		m_feSettings->set_filter( m_feSettings->get_current_filter_index() - 1 );
		update_to_new_list( wnd );
		break;

	case FeInputMap::ToggleLayout:
		m_feSettings->toggle_layout();
		load_layout( wnd );
		break;

	case FeInputMap::PrevFavourite:
	case FeInputMap::NextFavourite:
	case FeInputMap::PrevLetter:
	case FeInputMap::NextLetter:
		{
			int step( 0 );
			switch ( c )
			{
				case FeInputMap::PrevFavourite:
					step = m_feSettings->get_prev_fav_offset();
					break;

				case FeInputMap::NextFavourite:
					step = m_feSettings->get_next_fav_offset();
					break;

				case FeInputMap::PrevLetter:
					step = m_feSettings->get_next_letter_offset( -1 );
					break;

				case FeInputMap::NextLetter:
					step = m_feSettings->get_next_letter_offset( 1 );
					break;

				default:
					break;
			}

			if ( step != 0 )
			{
				vm_on_transition( ToNewSelection, step, wnd );

				m_feSettings->change_rom( step );
				update( false );

				vm_on_transition( FromOldSelection, -step, wnd );
			}
		}
		break;

	case FeInputMap::ScreenSaver:
		load_screensaver( wnd );
		break;

	case FeInputMap::LAST_COMMAND:
	default:
		// Not handled by us, return false so calling function knows
		//
		return false;
	}

	return true;
}

int FePresent::update( bool new_list )
{
	std::vector<FeBaseTextureContainer *>::iterator itc;
	std::vector<FeBasePresentable *>::iterator itl;
	if ( new_list )
	{
		for ( itc=m_texturePool.begin(); itc != m_texturePool.end(); ++itc )
			(*itc)->on_new_list( m_feSettings,
				m_layoutScale.x,
				m_layoutScale.y );

		for ( itl=m_elements.begin(); itl != m_elements.end(); ++itl )
			(*itl)->on_new_list( m_feSettings,
				m_layoutScale.x,
				m_layoutScale.y );
	}

	for ( itc=m_texturePool.begin(); itc != m_texturePool.end(); ++itc )
		(*itc)->on_new_selection( m_feSettings, m_screenSaverActive );

	for ( itl=m_elements.begin(); itl != m_elements.end(); ++itl )
		(*itl)->on_new_selection( m_feSettings );

	return 0;
}

void FePresent::load_screensaver( sf::RenderWindow *wnd )
{
	vm_on_transition( EndLayout, FromToScreenSaver, wnd );
	clear();
	set_transforms();
	m_screenSaverActive=true;

	//
	// Run the script which actually sets up the screensaver
	//
	m_layoutTimer.restart();
	std::string path, filename;
	m_feSettings->get_screensaver_file( path, filename );

	vm_on_new_layout( path, filename, m_feSettings->get_screensaver_config() );

	//
	// if there is no screen saver script then do a blank screen
	//
	update( true );
	vm_on_transition( StartLayout, FromToNoValue, wnd );
}

void FePresent::load_layout( sf::RenderWindow *wnd, bool initial_load )
{
	int var = ( m_screenSaverActive ) ? FromToScreenSaver : FromToNoValue;

	if ( !initial_load )
		vm_on_transition( EndLayout, FromToNoValue, wnd );
	else
		var = FromToFrontend;

	clear();
	set_transforms();
	m_screenSaverActive=false;

	if ( m_feSettings->lists_count() < 1 )
		return;

	//
	// Run the script which actually sets up the layout
	//
	m_layoutTimer.restart();
	vm_on_new_layout(
		m_feSettings->get_current_layout_dir(),
		m_feSettings->get_current_layout_file(),
		m_feSettings->get_current_layout_config() );

	// make things usable if the layout is empty
	//
	if ( m_elements.empty() )
	{
		//
		// Nothing loaded, default to a full screen list with the
		// configured movie artwork as the background
		//
		FeImage *img = cb_add_artwork( "", 0, 0,
			m_layoutSize.x, m_layoutSize.y );

		img->setColor( sf::Color( 100, 100, 100, 180 ) );
		cb_add_listbox( 0, 0, m_layoutSize.x, m_layoutSize.y );
	}

	update( true );

	vm_on_transition( ToNewList, FromToNoValue, wnd );
	vm_on_transition( StartLayout, var, wnd );
}

void FePresent::update_to_new_list( sf::RenderWindow *wnd )
{
	update( true );
	vm_on_transition( ToNewList, FromToNoValue, wnd );
}

bool FePresent::tick( sf::RenderWindow *wnd )
{
	bool ret_val = false;
	if ( m_moveState != MoveNone )
	{
		bool cont=false;

		switch ( m_moveEvent.type )
		{
		case sf::Event::KeyPressed:
			if ( sf::Keyboard::isKeyPressed( m_moveEvent.key.code ) )
				cont=true;
			break;

		case sf::Event::MouseButtonPressed:
			if ( sf::Mouse::isButtonPressed( m_moveEvent.mouseButton.button ) )
				cont=true;
			break;

		case sf::Event::JoystickButtonPressed:
			if ( sf::Joystick::isButtonPressed(
					m_moveEvent.joystickButton.joystickId,
					m_moveEvent.joystickButton.button ) )
				cont=true;
			break;

		case sf::Event::JoystickMoved:
			{
				sf::Joystick::update();

				float pos = sf::Joystick::getAxisPosition(
						m_moveEvent.joystickMove.joystickId,
						m_moveEvent.joystickMove.axis );
				if ( abs( pos ) > m_feSettings->get_joy_thresh() )
					cont=true;
			}
			break;

		default:
			break;
		}

		if ( cont )
		{
			int t = m_moveTimer.getElapsedTime().asMilliseconds();
			if ( t > 500 )
			{
				// As the button is held down, the advancement accelerates
				int shift = ( t / 500 ) - 1;
				if ( shift > 7 ) // don't go above a maximum advance of 2^7 (128)
					shift = 7;

				int step = 1 << ( shift );

				switch ( m_moveState )
				{
					case MoveUp: step = -step; break;
					case MoveDown: break; // do nothing
					case MovePageUp: step *= -get_page_size(); break;
					case MovePageDown: step *= get_page_size(); break;
					default: break;
				}

				int real_step = get_no_wrap_step( step );
				if ( real_step != 0 )
				{
					vm_on_transition( ToNewSelection, real_step, wnd );

					m_feSettings->change_rom( real_step );
					ret_val=true;
					update( false );

					vm_on_transition( FromOldSelection, -real_step, wnd );
				}
			}
		}
		else
		{
			m_moveState = MoveNone;
		}
	}

	if ( vm_on_tick())
		ret_val = true;

	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
			itm != m_texturePool.end(); ++itm )
	{
		if ( (*itm)->tick( m_feSettings, m_playMovies, true ) )
			ret_val=true;
	}

	return ret_val;
}

bool FePresent::saver_activation_check(  sf::RenderWindow *wnd )
{
	ASSERT( wnd );

	int saver_timeout = m_feSettings->get_screen_saver_timeout();
	if (( !m_screenSaverActive ) && ( saver_timeout > 0 ))
	{
		if ( ( m_layoutTimer.getElapsedTime() - m_lastInput )
				> sf::seconds( saver_timeout ) )
		{
			load_screensaver( wnd );
			return true;
		}
	}
	return false;
}

int FePresent::get_no_wrap_step( int step )
{
	int curr_sel = m_feSettings->get_rom_index( 0 );
	if ( ( curr_sel + step ) < 0 )
		return -curr_sel;

	int list_size = m_feSettings->get_current_list_size();
	if ( ( curr_sel + step ) >= list_size )
		return list_size - curr_sel - 1;

	return step;
}

int FePresent::get_page_size() const
{
	if ( m_listBox )
		return m_listBox->getRowCount();
	else
		return 5;
}

void FePresent::on_stop_frontend( sf::RenderWindow *wnd )
{
	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_play_state( false );

	vm_on_transition( EndLayout, FromToFrontend, wnd );
}

void FePresent::pre_run( sf::RenderWindow *wnd )
{
	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_play_state( false );

	vm_on_transition( ToGame, FromToNoValue, wnd );
}

void FePresent::post_run( sf::RenderWindow *wnd )
{
	perform_autorotate();
	vm_on_transition( FromGame, FromToNoValue, wnd );

	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_play_state( m_playMovies );

	reset_screen_saver( wnd );
}

void FePresent::toggle_movie()
{
	m_playMovies = !m_playMovies;

	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_play_state( m_playMovies );
}

void FePresent::toggle_mute()
{
	int movie_vol = m_feSettings->get_play_volume( FeSoundInfo::Movie );
	int sound_vol = m_feSettings->get_play_volume( FeSoundInfo::Sound );

	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_vol( movie_vol );

	for ( std::vector<FeScriptSound *>::iterator its=m_scriptSounds.begin();
				its != m_scriptSounds.end(); ++its )
		(*its)->set_volume( sound_vol );
}

const sf::Transform &FePresent::get_transform() const
{
	return m_transform;
}

const sf::Font *FePresent::get_font() const
{
	ASSERT( m_currentFont );
	return &(m_currentFont->get_font());
}

void FePresent::toggle_rotate( FeSettings::RotationState r )
{
	if ( m_toggleRotation != FeSettings::RotateNone )
		m_toggleRotation = FeSettings::RotateNone;
	else
		m_toggleRotation = r;

	set_transforms();
}

void FePresent::set_transforms()
{
	m_transform = sf::Transform();

	FeSettings::RotationState actualRotation
		= (FeSettings::RotationState)(( m_baseRotation + m_toggleRotation ) % 4);

	switch ( actualRotation )
	{
	case FeSettings::RotateNone:
		// do nothing
		break;
	case FeSettings::RotateRight:
		m_transform.translate( m_outputSize.x, 0 );
		m_transform.scale( (float) m_outputSize.x / m_outputSize.y,
												(float) m_outputSize.y / m_outputSize.x );
		m_transform.rotate(90);
		break;
	case FeSettings::RotateFlip:
		m_transform.translate( m_outputSize.x, m_outputSize.y );
		m_transform.rotate(180);
		break;
	case FeSettings::RotateLeft:
		m_transform.translate( 0, m_outputSize.y );
		m_transform.scale( (float) m_outputSize.x / m_outputSize.y,
											(float) m_outputSize.y / m_outputSize.x );
		m_transform.rotate(270);
		break;
	}

	m_transform.scale( m_layoutScale.x, m_layoutScale.y );
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

	set_transforms();
}

//
// Squirrel callback functions
//
void printFunc(HSQUIRRELVM v, const SQChar *s, ...)
{
	va_list vl;
	va_start(vl, s);
	vprintf(s, vl);
	va_end(vl);
}

FeImage* FePresent::cb_add_image(const char *n, int x, int y, int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	FeImage *ret = fep->add_image( false, n, x, y, w, h, fep->m_elements );

	// Add the image to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeImage* FePresent::cb_add_image(const char *n, int x, int y )
{
	return cb_add_image( n, x, y, 0, 0 );
}

FeImage* FePresent::cb_add_image(const char *n )
{
	return cb_add_image( n, 0, 0, 0, 0 );
}

FeImage* FePresent::cb_add_artwork(const char *n, int x, int y, int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	FeImage *ret = fep->add_image( true, n, x, y, w, h, fep->m_elements );

	// Add the image to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeImage* FePresent::cb_add_artwork(const char *n, int x, int y )
{
	return cb_add_artwork( n, x, y, 0, 0 );
}

FeImage* FePresent::cb_add_artwork(const char *n )
{
	return cb_add_artwork( n, 0, 0, 0, 0 );
}

FeImage* FePresent::cb_add_clone( FeImage *o )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	FeImage *ret = fep->add_clone( o, fep->m_elements );

	// Add the image to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeText* FePresent::cb_add_text(const char *n, int x, int y, int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	FeText *ret = fep->add_text( n, x, y, w, h, fep->m_elements );

	// Add the text to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeListBox* FePresent::cb_add_listbox(int x, int y, int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	FeListBox *ret = fep->add_listbox( x, y, w, h, fep->m_elements );

	// Add the listbox to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe ( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeImage* FePresent::cb_add_surface( int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	FeImage *ret = fep->add_surface( w, h, fep->m_elements );

	// Add the surface to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe ( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeScriptSound* FePresent::cb_add_sound( const char *s )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	return fep->add_sound( s );
	//
	// We assume the script will keep a reference to the sound
	//
}

FeShader* FePresent::cb_add_shader( int type, const char *shader1, const char *shader2 )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	return fep->add_shader( (FeShader::Type)type, shader1, shader2 );
	//
	// We assume the script will keep a reference to the shader
	//
}

FeShader* FePresent::cb_add_shader( int type, const char *shader1 )
{
	return FePresent::cb_add_shader( type, shader1, NULL );
}

FeShader* FePresent::cb_add_shader( int type )
{
	return FePresent::cb_add_shader( type, NULL, NULL );
}

void FePresent::cb_add_ticks_callback( const char *n )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	fep->add_ticks_callback( n );
}

void FePresent::cb_add_transition_callback( const char *n )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	fep->add_transition_callback( n );
}

bool FePresent::cb_is_keypressed( int k )
{
	return sf::Keyboard::isKeyPressed( (sf::Keyboard::Key)k );
}

bool FePresent::cb_is_joybuttonpressed( int num, int b )
{
	return sf::Joystick::isButtonPressed( num, b );
}

float FePresent::cb_get_joyaxispos( int num, int a )
{
	sf::Joystick::update();
	return sf::Joystick::getAxisPosition( num, (sf::Joystick::Axis)a );
}

bool FePresent::cb_get_input_state( const char *input )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );
	FeSettings *fes = fep->get_fes();

	return FeInputSource( input ).get_current_state( fes->get_joy_thresh() );
}

int FePresent::cb_get_input_pos( const char *input )
{
	return FeInputSource( input ).get_current_pos();
}

// return false if file not found
bool FePresent::internal_do_nut( const std::string &work_dir,
			const std::string &script_file )
{
	std::string path;

	if ( is_relative_path( script_file) )
	{
		path = work_dir;
		path += script_file;
	}
	else
		path = script_file;


	if ( !file_exists( path ) )
		return false;

	try
	{
		Sqrat::Script sc;
		sc.CompileFile( path );
		sc.Run();
	}
	catch( Sqrat::Exception e )
	{
		std::cout << "Script Error in " << path
			<< " - " << e.Message() << std::endl;
	}

	return true;
}

void FePresent::do_nut( const char *script_file )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );
	FeSettings *fes = fep->get_fes();

	internal_do_nut( fes->get_current_layout_dir(), script_file );
}

bool FePresent::load_module( const char *module_file )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );
	FeSettings *fes = fep->get_fes();

	std::string fixed_file = module_file;
	if ( !tail_compare( fixed_file, FE_LAYOUT_FILE_EXTENSION ) )
		fixed_file += FE_LAYOUT_FILE_EXTENSION;

	std::string temp = fes->get_module_dir( fixed_file );
	size_t len = temp.find_last_of( "/\\" );
	ASSERT( len != std::string::npos );

	std::string path = temp.substr( 0, len + 1 );
	return internal_do_nut( path, fixed_file );
}

bool my_callback( const char *buffer, void *opaque )
{
	Sqrat::Function func( Sqrat::RootTable(), (const char *)opaque );

	if ( !func.IsNull() )
		func.Execute( buffer );

	return true; // return false to cancel callbacks
}

bool FePresent::cb_plugin_command( const char *command,
		const char *args,
		const char *output_callback )
{
	return run_program( clean_path( command ),
		args, my_callback, (void *)output_callback );
}

bool FePresent::cb_plugin_command( const char *command, const char *args )
{
	return run_program( clean_path( command ), args );
}

bool FePresent::cb_plugin_command_bg( const char *command, const char *args )
{
	return run_program( clean_path( command ), args, NULL, NULL, false );
}

const char *FePresent::cb_path_expand( const char *path )
{
		static std::string internal_str;

		internal_str = clean_path( path );
		return internal_str.c_str();
}

const char *FePresent::cb_game_info( int index, int offset )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );
	FeSettings *fes = fep->get_fes();

	return (fes->get_rom_info( offset, (FeRomInfo::Index)index )).c_str();
}

const char *FePresent::cb_game_info( int index )
{
	return cb_game_info( index, 0 );
}

Sqrat::Table FePresent::cb_get_config()
{
	Sqrat::Object uConfig = Sqrat::RootTable().GetSlot( "UserConfig" );
	if ( uConfig.IsNull() )
		return NULL;

	Sqrat::Table retval;
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FePresent *fep = (FePresent *)sq_getforeignptr( vm );

	Sqrat::Object::iterator it;
	while ( uConfig.Next( it ) )
	{
		std::string key, value;
		fe_get_object_string( vm, it.getKey(), key );

		// use the default value from the script if a value has
		// not already been configured
		//
		if (( !fep->m_currentScriptConfig )
				|| ( !fep->m_currentScriptConfig->get_param( key, value ) ))
		{
			fe_get_object_string( vm, it.getValue(), value );
		}

		retval.SetValue( key.c_str(), value.c_str() );
	}

	return retval;
}

void FePresent::flag_redraw()
{
	m_redrawTriggered=true;
}

void FePresent::vm_close()
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	if ( vm )
	{
		sq_close( vm );
		Sqrat::DefaultVM::Set( NULL );
	}
}

void FePresent::vm_init()
{
	vm_close();
	HSQUIRRELVM vm = sq_open( 1024 );
	sq_setprintfunc( vm, printFunc, printFunc );
	sq_pushroottable( vm );
	sq_setforeignptr( vm, this );

	sqstd_register_bloblib( vm );
	sqstd_register_iolib( vm );
	sqstd_register_mathlib( vm );
	sqstd_register_stringlib( vm );
	sqstd_register_systemlib( vm );
	sqstd_seterrorhandlers( vm );

	Sqrat::DefaultVM::Set( vm );
}

void FePresent::vm_on_new_layout( const std::string &path,
			const std::string &filename, const FeLayoutInfo &layout_params )
{
	using namespace Sqrat;

	vm_close();

	// Squirrel VM gets reinitialized on each layout
	//
	vm_init();

	// Set fe-related constants
	//
	ConstTable()
		.Const( _SC("FeVersion"), FE_VERSION)
		.Const( _SC("FeVersionNum"), FE_VERSION_NUM)
		.Const( _SC("ScreenWidth"), (int)m_outputSize.x )
		.Const( _SC("ScreenHeight"), (int)m_outputSize.y )
		.Const( _SC("ScreenSaverActive"), m_screenSaverActive )
		.Const( _SC("OS"), get_OS_string() )
		.Const( _SC("ShadersAvailable"), sf::Shader::isAvailable() )
		.Const( _SC("FeConfigDirectory"), m_feSettings->get_config_dir().c_str() )

		.Enum( _SC("Style"), Enumeration()
			.Const( _SC("Regular"), sf::Text::Regular )
			.Const( _SC("Bold"), sf::Text::Bold )
			.Const( _SC("Italic"), sf::Text::Italic )
			.Const( _SC("Underlined"), sf::Text::Underlined )
			)
		.Enum( _SC("Align"), Enumeration()
			.Const( _SC("Left"), FeTextPrimative::Left )
			.Const( _SC("Centre"), FeTextPrimative::Centre )
			.Const( _SC("Right"), FeTextPrimative::Right )
			)
		.Enum( _SC("RotateScreen"), Enumeration()
			.Const( _SC("None"), FeSettings::RotateNone )
			.Const( _SC("Right"), FeSettings::RotateRight )
			.Const( _SC("Flip"), FeSettings::RotateFlip )
			.Const( _SC("Left"), FeSettings::RotateLeft )
			)
		// The "Axis" enum is deprecated along with fe.get_joyaxispos() as of version 1.2
		.Enum( _SC("Axis"), Enumeration()
			.Const( _SC("X"), sf::Joystick::X )
			.Const( _SC("Y"), sf::Joystick::Y )
			.Const( _SC("Z"), sf::Joystick::Z )
			.Const( _SC("R"), sf::Joystick::R )
			.Const( _SC("U"), sf::Joystick::U )
			.Const( _SC("V"), sf::Joystick::V )
			.Const( _SC("PovX"), sf::Joystick::PovX )
			.Const( _SC("PovY"), sf::Joystick::PovY )
			)
		.Enum( _SC("FromTo"), Enumeration()
			.Const( _SC("NoValue"), FromToNoValue )
			.Const( _SC("ScreenSaver"), FromToScreenSaver )
			.Const( _SC("Frontend"), FromToFrontend )
			)
		.Enum( _SC("Shader"), Enumeration()
			.Const( _SC("VertexAndFragment"), FeShader::VertexAndFragment )
			.Const( _SC("Vertex"), FeShader::Vertex )
			.Const( _SC("Fragment"), FeShader::Fragment )
			.Const( _SC("Empty"), FeShader::Empty )
			)
		.Enum( _SC("Vid"), Enumeration()
			.Const( _SC("Default"), VF_Normal )
			.Const( _SC("ImagesOnly"), VF_DisableVideo )
			.Const( _SC("NoAudio"), VF_NoAudio )
			.Const( _SC("NoAutoStart"), VF_NoAutoStart )
			.Const( _SC("NoLoop"), VF_NoLoop )
			)
		;

	// The "Key" enum is deprecated along with fe.get_keypressed() as of version 1.2
	Enumeration keys;
	int i=0;
	while ( FeInputSource::keyStrings[i] != NULL )
	{
		keys.Const( FeInputSource::keyStrings[i], i );
		i++;
	}

	ConstTable().Enum( _SC("Key"), keys);

	Enumeration info;
	i=0;
	while ( FeRomInfo::indexStrings[i] != NULL )
	{
		info.Const( FeRomInfo::indexStrings[i], i );
		i++;
	}
	ConstTable().Enum( _SC("Info"), info);

	Enumeration transition;
	i=0;
	while ( transitionTypeStrings[i] != NULL )
	{
		transition.Const( transitionTypeStrings[i], i );
		i++;
	}
	ConstTable().Enum( _SC("Transition"), transition );

	// All frontend functionality is in the "fe" table in Squirrel
	//
	Table fe;

	//
	// Define classes for fe objects that get exposed to Squirrel
	//

	// Base Presentable Object Class
	//
	fe.Bind( _SC("Presentable"),
		Class<FeBasePresentable, NoConstructor>()
		.Prop(_SC("visible"),
			&FeBasePresentable::get_visible, &FeBasePresentable::set_visible )
		.Prop(_SC("x"), &FeBasePresentable::get_x, &FeBasePresentable::set_x )
		.Prop(_SC("y"), &FeBasePresentable::get_y, &FeBasePresentable::set_y )
		.Prop(_SC("width"),
			&FeBasePresentable::get_width, &FeBasePresentable::set_width )
		.Prop(_SC("height"),
			&FeBasePresentable::get_height, &FeBasePresentable::set_height )
		.Prop(_SC("rotation"),
			&FeBasePresentable::getRotation, &FeBasePresentable::setRotation )
		.Prop(_SC("red"), &FeBasePresentable::get_r, &FeBasePresentable::set_r )
		.Prop(_SC("green"), &FeBasePresentable::get_g, &FeBasePresentable::set_g )
		.Prop(_SC("blue"), &FeBasePresentable::get_b, &FeBasePresentable::set_b )
		.Prop(_SC("alpha"), &FeBasePresentable::get_a, &FeBasePresentable::set_a )
		.Prop(_SC("index_offset"), &FeBasePresentable::getIndexOffset, &FeBasePresentable::setIndexOffset )
		.Prop(_SC("shader"), &FeBasePresentable::script_get_shader, &FeBasePresentable::script_set_shader )
		.Func( _SC("set_rgb"), &FeBasePresentable::set_rgb )
		.Overload<void (FeBasePresentable::*)(float, float)>(_SC("set_pos"), &FeBasePresentable::set_pos)
		.Overload<void (FeBasePresentable::*)(float, float, float, float)>(_SC("set_pos"), &FeBasePresentable::set_pos)
	);

	fe.Bind( _SC("Image"),
		DerivedClass<FeImage, FeBasePresentable, NoConstructor>()

		// Misnamed: shear_x and shear_y deprecated as of version 1.3, more accurately named
		// skew_x and skew_y instead
		.Prop(_SC("shear_x"), &FeImage::get_skew_x, &FeImage::set_skew_x )
		.Prop(_SC("shear_y"), &FeImage::get_skew_y, &FeImage::set_skew_y )

		.Prop(_SC("skew_x"), &FeImage::get_skew_x, &FeImage::set_skew_x )
		.Prop(_SC("skew_y"), &FeImage::get_skew_y, &FeImage::set_skew_y )
		.Prop(_SC("pinch_x"), &FeImage::get_pinch_x, &FeImage::set_pinch_x )
		.Prop(_SC("pinch_y"), &FeImage::get_pinch_y, &FeImage::set_pinch_y )
		.Prop(_SC("texture_width"), &FeImage::get_texture_width )
		.Prop(_SC("texture_height"), &FeImage::get_texture_height )
		.Prop(_SC("subimg_x"), &FeImage::get_subimg_x, &FeImage::set_subimg_x )
		.Prop(_SC("subimg_y"), &FeImage::get_subimg_y, &FeImage::set_subimg_y )
		.Prop(_SC("subimg_width"), &FeImage::get_subimg_width, &FeImage::set_subimg_width )
		.Prop(_SC("subimg_height"), &FeImage::get_subimg_height, &FeImage::set_subimg_height )
		// "movie_enabled" deprecated as of version 1.3, use the video_flags property instead:
		.Prop(_SC("movie_enabled"), &FeImage::getMovieEnabled, &FeImage::setMovieEnabled )
		.Prop(_SC("video_flags"), &FeImage::getVideoFlags, &FeImage::setVideoFlags )
		.Prop(_SC("video_playing"), &FeImage::getVideoPlaying, &FeImage::setVideoPlaying )
		.Prop(_SC("video_duration"), &FeImage::getVideoDuration )
		.Prop(_SC("video_time"), &FeImage::getVideoTime )
		.Prop(_SC("preserve_aspect_ratio"), &FeImage::get_preserve_aspect_ratio,
				&FeImage::set_preserve_aspect_ratio )
		.Prop(_SC("file_name"), &FeImage::getFileName, &FeImage::setFileName )

		//
		// Surface-specific functionality:
		//
		.Overload<FeImage * (FeImage::*)(const char *, int, int, int, int)>(_SC("add_image"), &FeImage::add_image)
		.Overload<FeImage * (FeImage::*)(const char *, int, int)>(_SC("add_image"), &FeImage::add_image)
		.Overload<FeImage * (FeImage::*)(const char *)>(_SC("add_image"), &FeImage::add_image)
		.Overload<FeImage * (FeImage::*)(const char *, int, int, int, int)>(_SC("add_artwork"), &FeImage::add_artwork)
		.Overload<FeImage * (FeImage::*)(const char *, int, int)>(_SC("add_artwork"), &FeImage::add_artwork)
		.Overload<FeImage * (FeImage::*)(const char *)>(_SC("add_artwork"), &FeImage::add_artwork)
		.Func( _SC("add_clone"), &FeImage::add_clone )
		.Func( _SC("add_text"), &FeImage::add_text )
		.Func( _SC("add_listbox"), &FeImage::add_listbox )
		.Func( _SC("add_surface"), &FeImage::add_surface )
	);

	fe.Bind( _SC("Text"),
		DerivedClass<FeText, FeBasePresentable, NoConstructor>()
		.Prop(_SC("msg"), &FeText::get_string, &FeText::set_string )
		.Prop(_SC("bg_red"), &FeText::get_bgr, &FeText::set_bgr )
		.Prop(_SC("bg_green"), &FeText::get_bgg, &FeText::set_bgg )
		.Prop(_SC("bg_blue"), &FeText::get_bgb, &FeText::set_bgb )
		.Prop(_SC("bg_alpha"), &FeText::get_bga, &FeText::set_bga )
		.Prop(_SC("charsize"), &FeText::get_charsize, &FeText::set_charsize )
		.Prop(_SC("style"), &FeText::get_style, &FeText::set_style )
		.Prop(_SC("align"), &FeText::get_align, &FeText::set_align )
		.Prop(_SC("word_wrap"), &FeText::get_word_wrap, &FeText::set_word_wrap )
		.Prop(_SC("font"), &FeText::get_font, &FeText::set_font )
		.Func( _SC("set_bg_rgb"), &FeText::set_bg_rgb )
	);

	fe.Bind( _SC("ListBox"),
		DerivedClass<FeListBox, FeBasePresentable, NoConstructor>()
		.Prop(_SC("bg_red"), &FeListBox::get_bgr, &FeListBox::set_bgr )
		.Prop(_SC("bg_green"), &FeListBox::get_bgg, &FeListBox::set_bgg )
		.Prop(_SC("bg_blue"), &FeListBox::get_bgb, &FeListBox::set_bgb )
		.Prop(_SC("bg_alpha"), &FeListBox::get_bga, &FeListBox::set_bga )
		.Prop(_SC("sel_red"), &FeListBox::get_selr, &FeListBox::set_selr )
		.Prop(_SC("sel_green"), &FeListBox::get_selg, &FeListBox::set_selg )
		.Prop(_SC("sel_blue"), &FeListBox::get_selb, &FeListBox::set_selb )
		.Prop(_SC("sel_alpha"), &FeListBox::get_sela, &FeListBox::set_sela )
		.Prop(_SC("selbg_red"), &FeListBox::get_selbgr, &FeListBox::set_selbgr )
		.Prop(_SC("selbg_green"), &FeListBox::get_selbgg, &FeListBox::set_selbgg )
		.Prop(_SC("selbg_blue"), &FeListBox::get_selbgb, &FeListBox::set_selbgb )
		.Prop(_SC("selbg_alpha"), &FeListBox::get_selbga, &FeListBox::set_selbga )
		.Prop(_SC("rows"), &FeListBox::get_rows, &FeListBox::set_rows )
		.Prop(_SC("charsize"), &FeListBox::get_charsize, &FeListBox::set_charsize )
		.Prop(_SC("style"), &FeListBox::get_style, &FeListBox::set_style )
		.Prop(_SC("align"), &FeListBox::get_align, &FeListBox::set_align )
		.Prop(_SC("sel_style"), &FeListBox::getSelStyle, &FeListBox::setSelStyle )
		.Prop(_SC("font"), &FeListBox::get_font, &FeListBox::set_font )
		.Func( _SC("set_bg_rgb"), &FeListBox::set_bg_rgb )
		.Func( _SC("set_sel_rgb"), &FeListBox::set_sel_rgb )
		.Func( _SC("set_selbg_rgb"), &FeListBox::set_selbg_rgb )
	);

	fe.Bind( _SC("LayoutGlobals"), Class <FePresent, NoConstructor>()
		.Prop( _SC("width"), &FePresent::get_layout_width, &FePresent::set_layout_width )
		.Prop( _SC("height"), &FePresent::get_layout_height, &FePresent::set_layout_height )
		.Prop( _SC("font"), &FePresent::get_layout_font, &FePresent::set_layout_font )
		// orient property deprecated as of 1.3.2, use "base_rotation" instead
		.Prop( _SC("orient"), &FePresent::get_base_rotation, &FePresent::set_base_rotation )
		.Prop( _SC("base_rotation"), &FePresent::get_base_rotation, &FePresent::set_base_rotation )
		.Prop( _SC("toggle_rotation"), &FePresent::get_toggle_rotation, &FePresent::set_toggle_rotation )
	);

	fe.Bind( _SC("CurrentList"), Class <FePresent, NoConstructor>()
		.Prop( _SC("name"), &FePresent::get_list_name )
		.Prop( _SC("filter"), &FePresent::get_filter_name )
		.Prop( _SC("size"), &FePresent::get_list_size )
		.Prop( _SC("index"), &FePresent::get_list_index, &FePresent::set_list_index )
	);

	fe.Bind( _SC("Sound"), Class <FeScriptSound, NoConstructor>()
		.Func( _SC("play"), &FeScriptSound::play )
		.Prop( _SC("is_playing"), &FeScriptSound::is_playing )
		.Prop( _SC("pitch"), &FeScriptSound::get_pitch, &FeScriptSound::set_pitch )
		.Prop( _SC("x"), &FeScriptSound::get_x, &FeScriptSound::set_x )
		.Prop( _SC("y"), &FeScriptSound::get_y, &FeScriptSound::set_y )
		.Prop( _SC("z"), &FeScriptSound::get_z, &FeScriptSound::set_z )
	);

	fe.Bind( _SC("Shader"), Class <FeShader, NoConstructor>()
		.Prop( _SC("type"), &FeShader::get_type )
		.Overload<void (FeShader::*)(const char *, float)>(_SC("set_param"), &FeShader::set_param)
		.Overload<void (FeShader::*)(const char *, float, float)>(_SC("set_param"), &FeShader::set_param)
		.Overload<void (FeShader::*)(const char *, float, float, float)>(_SC("set_param"), &FeShader::set_param)
		.Overload<void (FeShader::*)(const char *, float, float, float, float)>(_SC("set_param"), &FeShader::set_param)
		.Overload<void (FeShader::*)(const char *)>( _SC("set_texture_param"), &FeShader::set_texture_param )
		.Overload<void (FeShader::*)(const char *, FeImage *)>( _SC("set_texture_param"), &FeShader::set_texture_param )
	);

	//
	// Define functions that get exposed to Squirrel
	//

	fe.Overload<FeImage* (*)(const char *, int, int, int, int)>(_SC("add_image"), &FePresent::cb_add_image);
	fe.Overload<FeImage* (*)(const char *, int, int)>(_SC("add_image"), &FePresent::cb_add_image);
	fe.Overload<FeImage* (*)(const char *)>(_SC("add_image"), &FePresent::cb_add_image);

	fe.Overload<FeImage* (*)(const char *, int, int, int, int)>(_SC("add_artwork"), &FePresent::cb_add_artwork);
	fe.Overload<FeImage* (*)(const char *, int, int)>(_SC("add_artwork"), &FePresent::cb_add_artwork);
	fe.Overload<FeImage* (*)(const char *)>(_SC("add_artwork"), &FePresent::cb_add_artwork);

	fe.Func<FeImage* (*)(FeImage *)>(_SC("add_clone"), &FePresent::cb_add_clone);

	fe.Overload<FeText* (*)(const char *, int, int, int, int)>(_SC("add_text"), &FePresent::cb_add_text);
	fe.Func<FeListBox* (*)(int, int, int, int)>(_SC("add_listbox"), &FePresent::cb_add_listbox);
	fe.Func<FeImage* (*)(int, int)>(_SC("add_surface"), &FePresent::cb_add_surface);
	fe.Func<FeScriptSound* (*)(const char *)>(_SC("add_sound"), &FePresent::cb_add_sound);
	fe.Overload<FeShader* (*)(int, const char *, const char *)>(_SC("add_shader"), &FePresent::cb_add_shader);
	fe.Overload<FeShader* (*)(int, const char *)>(_SC("add_shader"), &FePresent::cb_add_shader);
	fe.Overload<FeShader* (*)(int)>(_SC("add_shader"), &FePresent::cb_add_shader);
	fe.Func<void (*)(const char *)>(_SC("add_ticks_callback"), &FePresent::cb_add_ticks_callback);
	fe.Func<void (*)(const char *)>(_SC("add_transition_callback"), &FePresent::cb_add_transition_callback);
	fe.Func<bool (*)(const char *)>(_SC("get_input_state"), &FePresent::cb_get_input_state);
	fe.Func<int (*)(const char *)>(_SC("get_input_pos"), &FePresent::cb_get_input_pos);
	fe.Func<void (*)(const char *)>(_SC("do_nut"), &FePresent::do_nut);
	fe.Func<bool (*)(const char *)>(_SC("load_module"), &FePresent::load_module);
	fe.Overload<const char* (*)(int)>(_SC("game_info"), &FePresent::cb_game_info);
	fe.Overload<const char* (*)(int, int)>(_SC("game_info"), &FePresent::cb_game_info);
	fe.Overload<bool (*)(const char *, const char *, const char *)>(_SC("plugin_command"), &FePresent::cb_plugin_command);
	fe.Overload<bool (*)(const char *, const char *)>(_SC("plugin_command"), &FePresent::cb_plugin_command);
	fe.Func<bool (*)(const char *, const char *)>(_SC("plugin_command_bg"), &FePresent::cb_plugin_command_bg);
	fe.Func<const char* (*)(const char *)>(_SC("path_expand"), &FePresent::cb_path_expand);
	fe.Func<Sqrat::Table (*)()>(_SC("get_config"), &FePresent::cb_get_config);

	// BEGIN deprecated functions
	// is_keypressed() and is_joybuttonpressed() deprecated as of version 1.2.  Use get_input_state() instead
	fe.Func<bool (*)(int)>(_SC("is_keypressed"), &FePresent::cb_is_keypressed);
	fe.Func<bool (*)(int, int)>(_SC("is_joybuttonpressed"), &FePresent::cb_is_joybuttonpressed);
	// get_joyaxispos() deprecated as of version 1.2.  Use get_input_pos() instead
	fe.Func<float (*)(int, int)>(_SC("get_joyaxispos"), &FePresent::cb_get_joyaxispos);
	// END deprecated functions

	//
	// Define variables that get exposed to Squirrel
	//

	fe.SetInstance( _SC("layout"), this );
	fe.SetInstance( _SC("list"), this );

	// Each presentation object gets an instance in the
	// "obj" table available in Squirrel
	//
	Table obj;
	fe.Bind( _SC("obj"), obj );
	RootTable().Bind( _SC("fe"),  fe );

	//
	// Run the layout script
	//
	if ( file_exists( path + filename ) )
	{
		fe.SetValue( _SC("script_dir"), path );
		fe.SetValue( _SC("script_file"), filename );
		m_currentScriptConfig = &layout_params;

		try
		{
			Script sc;
			sc.CompileFile( path + filename );
			sc.Run();
		}
		catch( Exception e )
		{
			std::cerr << "Script Error in " << path + filename
				<< " - " << e.Message() << std::endl;
		}
	}
	else
	{
		std::cerr << "Script file not found: " << path + filename << std::endl;
	}

	//
	// Now run any plugin script(s)
	//
	const std::vector< FePlugInfo > &plugins = m_feSettings->get_plugins();

	for ( std::vector< FePlugInfo >::const_iterator itr= plugins.begin();
		itr != plugins.end(); ++itr )
	{
		// Don't run disabled plugins...
		if ( (*itr).get_enabled() == false )
			continue;

		std::string plug_path, plug_name;
		m_feSettings->get_plugin_full_path(
				(*itr).get_name(),
				plug_path,
				plug_name );

		if ( !plug_name.empty() )
		{
			fe.SetValue( _SC("script_dir"), plug_path );
			fe.SetValue( _SC("script_file"), plug_name );
			m_currentScriptConfig = &(*itr);

			try
			{
				Script sc;
				sc.CompileFile( plug_path + plug_name );
				sc.Run();
			}
			catch( Exception e )
			{
				std::cout << "Script Error in " << plug_path + plug_name
					<< " - " << e.Message() << std::endl;
			}
		}
	}

	fe.SetValue( _SC("script_dir"), "" );
	fe.SetValue( _SC("script_file"), "" );
	m_currentScriptConfig = NULL;
}

bool FePresent::vm_on_tick()
{
	using namespace Sqrat;
	m_redrawTriggered = false;

	for ( std::vector<std::string>::iterator itr = m_ticksList.begin();
		itr != m_ticksList.end(); )
	{
		// Assumption: Ticks list is empty if no vm is active
		//
		ASSERT( Sqrat::DefaultVM::Get() );

		bool remove=false;
		try
		{
			Function func( RootTable(), (*itr).c_str() );

			if ( !func.IsNull() )
				func.Execute( m_layoutTimer.getElapsedTime().asMilliseconds() );
		}
		catch( Exception e )
		{
			std::cout << "Script Error in " << (*itr)
				<< "(): " << e.Message() << std::endl;

			// Knock out this entry.   If it causes a script error, we don't
			// want to call it anymore
			//
			remove=true;
		}

		if ( remove )
			itr = m_ticksList.erase( itr );
		else
			itr++;
	}

	return m_redrawTriggered;
}

bool FePresent::vm_on_transition(
	FeTransitionType t,
	int var,
	sf::RenderWindow *wnd )
{
	using namespace Sqrat;

#ifdef FE_DEBUG
	std::cout << "[Transition] type=" << transitionTypeStrings[t] << ", var=" << var << std::endl;
#endif // FE_DEBUG

	sf::Time tstart = m_layoutTimer.getElapsedTime();
	m_redrawTriggered = false;

	std::vector<const char *> worklist( m_transitionList.size() );
	for ( unsigned int i=0; i< m_transitionList.size(); i++ )
		worklist[i] = m_transitionList[i].c_str();

	//
	// A registered transition callback stays in the worklist for as long
	// as it keeps returning true.
	//
	while ( !worklist.empty() )
	{
		// Assumption: Transition list is empty if no vm is active
		//
		ASSERT( Sqrat::DefaultVM::Get() );

		//
		// Call each remaining transition callback on each pass through
		// the worklist
		//
		for ( std::vector<const char *>::iterator itr=worklist.begin();
			itr != worklist.end(); )
		{
			bool keep=false;
			try
			{
				Function func( RootTable(), (*itr) );
				if ( !func.IsNull() )
				{
					sf::Time ttime = m_layoutTimer.getElapsedTime() - tstart;
					keep = func.Evaluate<bool>(
						(int)t,
						var,
						ttime.asMilliseconds() );
				}
			}
			catch( Exception e )
			{
				std::cout << "Script Error in " << (*itr)
					<< "(): " << e.Message() << std::endl;
			}

			if ( !keep )
				itr = worklist.erase( itr );
			else
				itr++;
		}

		// redraw now if we are doing another pass...
		//
		if (( !worklist.empty() ) && ( wnd ))
		{
			for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
					itm != m_texturePool.end(); ++itm )
			{
				// don't start a video during transition, but keep playing one if it is already playing
				(*itm)->tick( m_feSettings, m_playMovies, false );
			}

			wnd->clear();
			wnd->draw( *this );
			wnd->display();
			m_redrawTriggered = false; // clear redraw flag
		}
	}

	return m_redrawTriggered;
}

FeShader *FePresent::get_empty_shader()
{
	if ( !m_emptyShader )
		m_emptyShader = new FeShader( FeShader::Empty, "", "" );

	return m_emptyShader;
}

FePresent *helper_get_fep()
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();

	if ( !vm )
		return NULL;

	return (FePresent *)sq_getforeignptr( vm );
}

void script_do_update( FeBasePresentable *bp )
{
	FePresent *fep = helper_get_fep();

	if ( fep )
	{
		bp->on_new_list( fep->get_fes(),
			fep->get_layout_scale_x(),
			fep->get_layout_scale_y() );

		bp->on_new_selection( fep->get_fes() );

		fep->flag_redraw();
	}
}

void script_do_update( FeBaseTextureContainer *tc )
{
	FePresent *fep = helper_get_fep();

	if ( fep )
	{
		tc->on_new_selection( fep->get_fes(), fep->get_screensaver_active() );
		fep->flag_redraw();
	}
}

void script_flag_redraw()
{
	FePresent *fep = helper_get_fep();

	if ( fep )
		fep->flag_redraw();
}

const sf::Font *script_get_font( const std::string &name )
{
	FePresent *fep = helper_get_fep();
	if ( fep )
	{
		const FeFontContainer *font = fep->get_pooled_font( name );

		if ( font )
			return &(font->get_font());
	}

	return NULL;
}

FeShader *script_get_empty_shader()
{
	FePresent *fep = helper_get_fep();

	if ( fep )
		return fep->get_empty_shader();

	return NULL;
}
