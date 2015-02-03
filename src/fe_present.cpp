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
	m_currentFont( &defaultfont ),
	m_defaultFont( defaultfont ),
	m_baseRotation( FeSettings::RotateNone ),
	m_toggleRotation( FeSettings::RotateNone ),
	m_playMovies( true ),
	m_screenSaverActive( false ),
	m_user_page_size( -1 ),
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
}

void FePresent::clear()
{
	//
	// keep toggle rotation, base rotation and mute state through clear
	//
	// Activating the screen saver keeps the previous base rotation, while
	// mute and toggle rotation are kept whenever the layout is changed
	//
	m_listBox=NULL; // listbox gets deleted with the m_elements below
	m_transform = sf::Transform();
	m_currentFont = &m_defaultFont;
	m_layoutFontName = m_feSettings->get_info( FeSettings::DefaultFont );
	m_user_page_size = -1;

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

	while ( !m_sounds.empty() )
	{
		FeSound *s = m_sounds.back();
		m_sounds.pop_back();
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

	flag_redraw();
	m_texturePool.push_back( new_tex );
	l.push_back( new_image );

	return new_image;
}

FeImage *FePresent::add_clone( FeImage *o,
			std::vector<FeBasePresentable *> &l )
{
	FeImage *new_image = new FeImage( o );
	flag_redraw();
	l.push_back( new_image );
	return new_image;
}

FeText *FePresent::add_text( const std::string &n, int x, int y, int w, int h,
			std::vector<FeBasePresentable *> &l )
{
	FeText *new_text = new FeText( n, x, y, w, h );

	ASSERT( m_currentFont );
	new_text->setFont( m_currentFont->get_font() );

	flag_redraw();
	l.push_back( new_text );
	return new_text;
}

FeListBox *FePresent::add_listbox( int x, int y, int w, int h,
			std::vector<FeBasePresentable *> &l )
{
	FeListBox *new_lb = new FeListBox( x, y, w, h );

	ASSERT( m_currentFont );
	new_lb->setFont( m_currentFont->get_font() );

	flag_redraw();
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

	flag_redraw();
	l.push_back( new_image );
	m_texturePool.push_back( new_surface );
	return new_image;
}

FeSound *FePresent::add_sound( const char *n )
{
	FeSound *new_sound = new FeSound();
	new_sound->load( n );
	new_sound->set_volume(
		m_feSettings->get_play_volume( FeSoundInfo::Sound ) );

	m_sounds.push_back( new_sound );
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
	flag_redraw();
}

void FePresent::set_layout_height( int h )
{
	m_layoutSize.y = h;
	m_layoutScale.y = (float) m_outputSize.y / h;
	set_transforms();
	flag_redraw();
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
		flag_redraw();
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
	flag_redraw();
}

int FePresent::get_base_rotation() const
{
	return m_baseRotation;
}

void FePresent::set_toggle_rotation( int r )
{
	m_toggleRotation = (FeSettings::RotationState)r;
	set_transforms();
	flag_redraw();
}

int FePresent::get_toggle_rotation() const
{
	return m_toggleRotation;
}

const char *FePresent::get_display_name() const
{
	return m_feSettings->get_current_display_title().c_str();
}

const char *FePresent::get_filter_name() const
{
	return m_feSettings->get_filter_name( m_feSettings->get_current_filter_index() ).c_str();
}

int FePresent::get_filter_index() const
{
	return m_feSettings->get_current_filter_index();
}

void FePresent::set_filter_index( int idx )
{
	int new_offset = idx - get_filter_index();
	if ( new_offset != 0 )
	{
		if ( m_feSettings->navigate_filter( new_offset ) )
			load_layout();
		else
			update_to_new_list( new_offset );
	}
}

int FePresent::get_list_size() const
{
	return m_feSettings->get_filter_size( m_feSettings->get_current_filter_index() );
}

int FePresent::get_selection_index() const
{
	return m_feSettings->get_rom_index( m_feSettings->get_current_filter_index(), 0 );
}

int FePresent::get_sort_by() const
{
	FeRomInfo::Index idx;
	bool rev;
	int limit;

	m_feSettings->get_current_sort( idx, rev, limit );
	return idx;
}

bool FePresent::get_reverse_order() const
{
	FeRomInfo::Index idx;
	bool rev;
	int limit;

	m_feSettings->get_current_sort( idx, rev, limit );
	return rev;
}

int FePresent::get_list_limit() const
{
	FeRomInfo::Index idx;
	bool rev;
	int limit;

	m_feSettings->get_current_sort( idx, rev, limit );
	return limit;
}

void FePresent::set_selection_index( int index )
{
	int new_offset = index - get_selection_index();
	if ( new_offset != 0 )
	{
		m_feSettings->step_current_selection( new_offset );
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

bool FePresent::handle_event( FeInputMap::Command c )
{
	if ( reset_screen_saver() )
		return true;

	switch( c )
	{
	case FeInputMap::Down:
		on_transition( ToNewSelection, 1 );

		m_feSettings->step_current_selection( 1 );
		update( false );

		on_transition( FromOldSelection, -1 );
		break;

	case FeInputMap::Up:
		on_transition( ToNewSelection, -1 );

		m_feSettings->step_current_selection( -1 );
		update( false );

		on_transition( FromOldSelection, 1 );
		break;

	case FeInputMap::PageDown:
		{
			int step = get_page_size();
			on_transition( ToNewSelection, step );

			m_feSettings->step_current_selection( step );
			update( false );

			on_transition( FromOldSelection, -step );
		}
		break;

	case FeInputMap::PageUp:
		{
			int step = -get_page_size();
			on_transition( ToNewSelection, step );

			m_feSettings->step_current_selection( step );
			update( false );

			on_transition( FromOldSelection, -step );
		}
		break;

	case FeInputMap::RandomGame:
		{
			int ls = m_feSettings->get_filter_size( m_feSettings->get_current_filter_index() );
			if ( ls > 0 )
			{
				int step = rand() % ls;
				if ( step != 0 )
				{
					on_transition( ToNewSelection, step );

					m_feSettings->step_current_selection( step );
					update( false );

					on_transition( FromOldSelection, -step );
				}
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

	case FeInputMap::NextDisplay:
	case FeInputMap::PrevDisplay:
		if ( m_feSettings->navigate_display( ( c == FeInputMap::NextDisplay ) ? 1 : -1 ) )
			load_layout();
		else
			update_to_new_list();

		break;

	case FeInputMap::NextFilter:
	case FeInputMap::PrevFilter:
		{
			int offset = ( c == FeInputMap::NextFilter ) ? 1 : -1;
			if ( m_feSettings->navigate_filter( offset ) )
				load_layout();
			else
				update_to_new_list( offset );
		}
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
				on_transition( ToNewSelection, step );

				m_feSettings->step_current_selection( step );
				update( false );

				on_transition( FromOldSelection, -step );
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
				m_layoutScale.y,
				m_screenSaverActive );

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

void FePresent::on_end_navigation()
{
	std::vector<FeBaseTextureContainer *>::iterator itc;

	for ( itc=m_texturePool.begin(); itc != m_texturePool.end(); ++itc )
		(*itc)->on_end_navigation( m_feSettings, m_screenSaverActive );

	on_transition( EndNavigation, 0 );
}

void FePresent::load_screensaver()
{
	on_transition( EndLayout, FromToScreenSaver );
	clear();
	set_transforms();
	m_screenSaverActive=true;

	//
	// Run the script which actually sets up the screensaver
	//
	m_layoutTimer.restart();
	std::string path, filename;
	m_feSettings->get_screensaver_file( path, filename );

	on_new_layout( path, filename, m_feSettings->get_screensaver_config() );

	//
	// if there is no screen saver script then do a blank screen
	//
	update( true );
	on_transition( StartLayout, FromToNoValue );
}

void FePresent::load_layout( bool initial_load )
{
	int var = ( m_screenSaverActive ) ? FromToScreenSaver : FromToNoValue;

	if ( !initial_load )
		on_transition( EndLayout, FromToNoValue );
	else
		var = FromToFrontend;

	clear();
	m_baseRotation = FeSettings::RotateNone;

	set_transforms();
	m_screenSaverActive=false;

	if ( m_feSettings->displays_count() < 1 )
		return;

	//
	// Run the script which actually sets up the layout
	//
	std::string layout_dir = m_feSettings->get_current_layout_dir();
	std::string layout_file = m_feSettings->get_current_layout_file();

	m_layoutTimer.restart();
	on_new_layout( layout_dir, layout_file,
		m_feSettings->get_current_layout_config() );

	// make things usable if the layout is empty
	//
	if ( m_elements.empty() )
		init_with_default_layout();

	std::cout << " - Loaded layout: " << layout_dir << " (" << layout_file << ")" << std::endl;

	update_to_new_list( FromToNoValue );
	on_transition( StartLayout, var );
}

void FePresent::update_to_new_list( int var )
{
	update( true );
	on_transition( ToNewList, var );
}

bool FePresent::tick()
{
	bool ret_val = false;
	if ( on_tick())
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

	// Check if we need to loop any script sounds that are set to loop
	for ( std::vector<FeSound *>::iterator its=m_sounds.begin();
			its != m_sounds.end(); ++its )
		(*its)->tick();

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

int FePresent::get_page_size() const
{
	if ( m_user_page_size != -1 )
		return m_user_page_size;
	else if ( m_listBox )
		return m_listBox->getRowCount();
	else
		return 5;
}

void FePresent::set_page_size( int ps )
{
	if ( ps > 0 )
		m_user_page_size = ps;
	else
		m_user_page_size = -1;
}

void FePresent::on_stop_frontend()
{
	set_video_play_state( false );
	on_transition( EndLayout, FromToFrontend );
}

void FePresent::pre_run()
{
	on_transition( ToGame, FromToNoValue );
	set_video_play_state( false );
}

void FePresent::post_run()
{
	set_video_play_state( m_playMovies );
	on_transition( FromGame, FromToNoValue );

	reset_screen_saver();
	update( true );
}

void FePresent::toggle_movie()
{
	m_playMovies = !m_playMovies;
	set_video_play_state( m_playMovies );
}

void FePresent::set_video_play_state( bool state )
{
	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_play_state( state );
}

void FePresent::toggle_mute()
{
	int movie_vol = m_feSettings->get_play_volume( FeSoundInfo::Movie );
	int sound_vol = m_feSettings->get_play_volume( FeSoundInfo::Sound );

	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_vol( movie_vol );

	for ( std::vector<FeSound *>::iterator its=m_sounds.begin();
				its != m_sounds.end(); ++its )
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

void FePresent::script_do_update( FeBasePresentable *bp )
{
	FePresent *fep = script_get_fep();
	if ( fep )
	{
		bp->on_new_list( fep->m_feSettings,
			fep->get_layout_scale_x(),
			fep->get_layout_scale_y() );

		bp->on_new_selection( fep->m_feSettings );

		fep->flag_redraw();
	}
}

void FePresent::script_do_update( FeBaseTextureContainer *tc )
{
	FePresent *fep = script_get_fep();
	if ( fep )
	{
		tc->on_new_selection( fep->m_feSettings, fep->m_screenSaverActive );
		fep->flag_redraw();
	}
}

void FePresent::script_flag_redraw()
{
	FePresent *fep = script_get_fep();
	if ( fep )
		fep->flag_redraw();
}
