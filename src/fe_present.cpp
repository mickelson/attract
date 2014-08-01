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

#include <iostream>

void FeFontContainer::set_font( const std::string &n )
{
	m_name = n;
	m_font.loadFromFile( m_name );
}

FePresent::FePresent( FeSettings *fesettings, FeFontContainer &defaultfont )
	: m_feSettings( fesettings ),
	m_vm( NULL ),
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
	sf::VideoMode vm = sf::VideoMode::getDesktopMode();
	m_outputSize.x = vm.width;
	m_outputSize.y = vm.height;

	m_layoutFontName = m_feSettings->get_info( FeSettings::DefaultFont );
}

FePresent::~FePresent()
{
	clear();
	m_vm->vm_close();
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

	m_vm->clear();

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

	m_vm->flag_redraw();
	m_texturePool.push_back( new_tex );
	l.push_back( new_image );

	return new_image;
}

FeImage *FePresent::add_clone( FeImage *o,
			std::vector<FeBasePresentable *> &l )
{
	FeImage *new_image = new FeImage( o );
	m_vm->flag_redraw();
	l.push_back( new_image );
	return new_image;
}

FeText *FePresent::add_text( const std::string &n, int x, int y, int w, int h,
			std::vector<FeBasePresentable *> &l )
{
	FeText *new_text = new FeText( n, x, y, w, h );

	ASSERT( m_currentFont );
	new_text->setFont( m_currentFont->get_font() );

	m_vm->flag_redraw();
	l.push_back( new_text );
	return new_text;
}

FeListBox *FePresent::add_listbox( int x, int y, int w, int h,
			std::vector<FeBasePresentable *> &l )
{
	FeListBox *new_lb = new FeListBox( x, y, w, h );

	ASSERT( m_currentFont );
	new_lb->setFont( m_currentFont->get_font() );

	m_vm->flag_redraw();
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

	m_vm->flag_redraw();
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
	m_vm->flag_redraw();
}

void FePresent::set_layout_height( int h )
{
	m_layoutSize.y = h;
	m_layoutScale.y = (float) m_outputSize.y / h;
	set_transforms();
	m_vm->flag_redraw();
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
		m_vm->flag_redraw();
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
	m_vm->flag_redraw();
}

int FePresent::get_base_rotation() const
{
	return m_baseRotation;
}

void FePresent::set_toggle_rotation( int r )
{
	m_toggleRotation = (FeSettings::RotationState)r;
	set_transforms();
	m_vm->flag_redraw();
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

bool FePresent::reset_screen_saver()
{
	if ( m_screenSaverActive )
	{
		// Reset from screen saver
		//
		load_layout();
		return true;
	}

	m_lastInput=m_layoutTimer.getElapsedTime();
	return false;
}

bool FePresent::handle_event( FeInputMap::Command c,
	const sf::Event &ev )
{
	m_moveState=MoveNone;

	if ( reset_screen_saver() )
		return true;

	switch( c )
	{
	case FeInputMap::Down:
		if ( m_moveState == MoveNone )
		{
			m_moveTimer.restart();
			m_moveState=MoveDown;
			m_moveEvent = ev;
			m_vm->on_transition( ToNewSelection, 1 );

			m_feSettings->change_rom( 1 );
			update( false );

			m_vm->on_transition( FromOldSelection, -1 );
		}
		break;

	case FeInputMap::Up:
		if ( m_moveState == MoveNone )
		{
			m_moveTimer.restart();
			m_moveState=MoveUp;
			m_moveEvent = ev;
			m_vm->on_transition( ToNewSelection, -1 );

			m_feSettings->change_rom( -1 );
			update( false );

			m_vm->on_transition( FromOldSelection, 1 );
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
				m_vm->on_transition( ToNewSelection, step );

				m_feSettings->change_rom( step );
				update( false );

				m_vm->on_transition( FromOldSelection, -step );
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
				m_vm->on_transition( ToNewSelection, step );

				m_feSettings->change_rom( step );
				update( false );

				m_vm->on_transition( FromOldSelection, -step );
			}
		}
		break;

	case FeInputMap::RandomGame:
		{
			int step = rand() % m_feSettings->get_current_list_size();
			if ( step != 0 )
			{
				m_vm->on_transition( ToNewSelection, step );

				m_feSettings->change_rom( step );
				update( false );

				m_vm->on_transition( FromOldSelection, -step );
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
			load_layout();
		else
			update_to_new_list();

		break;

	case FeInputMap::PrevList:
		// prev_list returns true if the layout changes with the new list
		//
		if ( m_feSettings->prev_list() )
			load_layout();
		else
			update_to_new_list();

		break;

	case FeInputMap::NextFilter:
		m_feSettings->set_filter( m_feSettings->get_current_filter_index() + 1 );
		update_to_new_list();
		break;

	case FeInputMap::PrevFilter:
		m_feSettings->set_filter( m_feSettings->get_current_filter_index() - 1 );
		update_to_new_list();
		break;

	case FeInputMap::ToggleLayout:
		m_feSettings->toggle_layout();
		load_layout();
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
				m_vm->on_transition( ToNewSelection, step );

				m_feSettings->change_rom( step );
				update( false );

				m_vm->on_transition( FromOldSelection, -step );
			}
		}
		break;

	case FeInputMap::ScreenSaver:
		load_screensaver();
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

void FePresent::load_screensaver()
{
	m_vm->on_transition( EndLayout, FromToScreenSaver );
	clear();
	set_transforms();
	m_screenSaverActive=true;

	//
	// Run the script which actually sets up the screensaver
	//
	m_layoutTimer.restart();
	std::string path, filename;
	m_feSettings->get_screensaver_file( path, filename );

	m_vm->on_new_layout( path, filename, m_feSettings->get_screensaver_config() );

	//
	// if there is no screen saver script then do a blank screen
	//
	update( true );
	m_vm->on_transition( StartLayout, FromToNoValue );
}

void FePresent::load_layout( bool initial_load )
{
	int var = ( m_screenSaverActive ) ? FromToScreenSaver : FromToNoValue;

	if ( !initial_load )
		m_vm->on_transition( EndLayout, FromToNoValue );
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
	m_vm->on_new_layout(
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
		FeImage *img = FeVM::cb_add_artwork( "", 0, 0,
			m_layoutSize.x, m_layoutSize.y );

		img->setColor( sf::Color( 100, 100, 100, 180 ) );
		FeVM::cb_add_listbox( 0, 0, m_layoutSize.x, m_layoutSize.y );
	}

	update( true );

	m_vm->on_transition( ToNewList, FromToNoValue );
	m_vm->on_transition( StartLayout, var );
}

void FePresent::update_to_new_list()
{
	update( true );
	m_vm->on_transition( ToNewList, FromToNoValue );
}

bool FePresent::tick()
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
			const int TRIG_MS = 350;
			int t = m_moveTimer.getElapsedTime().asMilliseconds();
			if ( t > TRIG_MS )
			{
				// As the button is held down, the advancement accelerates
				int shift = ( t / TRIG_MS ) - 3;
				if ( shift < 0 )
					shift = 0;
				else if ( shift > 7 ) // don't go above a maximum advance of 2^7 (128)
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
					m_vm->on_transition( ToNewSelection, real_step );

					m_feSettings->change_rom( real_step );
					ret_val=true;
					update( false );

					m_vm->on_transition( FromOldSelection, -real_step );
				}
			}
		}
		else
		{
			m_moveState = MoveNone;
		}
	}

	if ( m_vm->on_tick())
		ret_val = true;

	if ( video_tick() )
		ret_val = true;

	return ret_val;
}

bool FePresent::video_tick()
{
	bool ret_val=false;

	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
			itm != m_texturePool.end(); ++itm )
	{
		if ( (*itm)->tick( m_feSettings, m_playMovies, true ) )
			ret_val=true;
	}

	return ret_val;
}

bool FePresent::saver_activation_check()
{
	int saver_timeout = m_feSettings->get_screen_saver_timeout();
	if (( !m_screenSaverActive ) && ( saver_timeout > 0 ))
	{
		if ( ( m_layoutTimer.getElapsedTime() - m_lastInput )
				> sf::seconds( saver_timeout ) )
		{
			load_screensaver();
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

void FePresent::on_stop_frontend()
{
	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_play_state( false );

	m_vm->on_transition( EndLayout, FromToFrontend );
}

void FePresent::pre_run()
{
	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_play_state( false );

	m_vm->on_transition( ToGame, FromToNoValue );
}

void FePresent::post_run()
{
	m_vm->on_transition( FromGame, FromToNoValue );

	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_play_state( m_playMovies );

	reset_screen_saver();
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

FeShader *FePresent::get_empty_shader()
{
	if ( !m_emptyShader )
		m_emptyShader = new FeShader( FeShader::Empty, "", "" );

	return m_emptyShader;
}
