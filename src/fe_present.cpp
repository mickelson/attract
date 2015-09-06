/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-15 Andrew Mickelson
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
#include "zip.hpp"

#include <iostream>

#ifdef SFML_SYSTEM_WINDOWS

#include <windows.h>

BOOL CALLBACK my_mon_enum_proc( HMONITOR, HDC, LPRECT mon_rect, LPARAM data )
{
	std::vector < FeMonitor > *monitors = (std::vector < FeMonitor > *)data;

	FeMonitor mon;
	mon.transform = sf::Transform().translate( mon_rect->left, mon_rect->top );
	mon.size.x = mon_rect->right - mon_rect->left;
	mon.size.y = mon_rect->bottom - mon_rect->top;
	mon.num = monitors->size();

	// make sure primary monitor is first in m_mon vector
	if (( mon_rect->left == 0 ) && ( mon_rect->top == 0 ))
		monitors->insert( monitors->begin(), mon );
	else
		monitors->push_back( mon );

	return TRUE;
}
#endif

#ifdef USE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

void FeFontContainer::set_font( const std::string &n )
{
	m_name = n;
	m_font.loadFromFile( m_name );
}

FeImage *FeMonitor::add_image(const char *n, int x, int y, int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_image( false, n, x, y, w, h, elements );

	return NULL;
}

FeImage *FeMonitor::add_image(const char *n, int x, int y )
{
	return add_image( n, x, y, 0, 0 );
}

FeImage *FeMonitor::add_image(const char *n )
{
	return add_image( n, 0, 0, 0, 0 );
}

FeImage *FeMonitor::add_artwork(const char *l, int x, int y, int w, int h )
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_image( true, l, x, y, w, h, elements );

	return NULL;
}

FeImage *FeMonitor::add_artwork(const char *l, int x, int y)
{
	return add_artwork( l, x, y, 0, 0 );
}

FeImage *FeMonitor::add_artwork(const char *l )
{
	return add_artwork( l, 0, 0, 0, 0 );
}

FeImage *FeMonitor::add_clone(FeImage *i )
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_clone( i, elements );

	return NULL;
}

FeText *FeMonitor::add_text(const char *t, int x, int y, int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_text( t, x, y, w, h, elements );

	return NULL;
}

FeListBox *FeMonitor::add_listbox(int x, int y, int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_listbox( x, y, w, h, elements );

	return NULL;
}

FeImage *FeMonitor::add_surface(int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_surface( w, h, elements );

	return NULL;
}

int FeMonitor::get_width()
{
	return size.x;
}

int FeMonitor::get_height()
{
	return size.y;
}

int FeMonitor::get_num()
{
	return num;
}

FePresent::FePresent( FeSettings *fesettings, FeFontContainer &defaultfont )
	: m_feSettings( fesettings ),
	m_currentFont( &defaultfont ),
	m_defaultFont( defaultfont ),
	m_baseRotation( FeSettings::RotateNone ),
	m_toggleRotation( FeSettings::RotateNone ),
	m_playMovies( true ),
	m_user_page_size( -1 ),
	m_listBox( NULL ),
	m_emptyShader( NULL )
{

	//
	// Handle multi-monitors now
	//
	// We support multi-monitor setups on MS-Windows when in fullscreen or "fillscreen" mode
	//
#ifdef SFML_SYSTEM_WINDOWS
	if ( m_feSettings->get_window_mode() != FeSettings::Window )
	{
		EnumDisplayMonitors( NULL, NULL, my_mon_enum_proc, (LPARAM)&m_mon );

		//
		// The Windows virtual screen can have a negative valued top left corner, whereas in SFML we
		// always have a 0,0 top left.  So we correct the transforms in m_mon to SFML's coordinates now.
		//
		sf::Transform correction = sf::Transform().translate( -GetSystemMetrics( SM_XVIRTUALSCREEN ),
																				-GetSystemMetrics( SM_YVIRTUALSCREEN ) );

		for ( std::vector<FeMonitor>::iterator itr=m_mon.begin(); itr!=m_mon.end(); ++itr )
			(*itr).transform *= correction;
	}
	else
#else
#ifdef USE_XINERAMA
	if ( m_feSettings->get_window_mode() != FeSettings::Window )
	{
		Display *xdisp = XOpenDisplay( NULL );
		int num=0;

		XineramaScreenInfo *si=XineramaQueryScreens( xdisp, &num );
		if ( si )
		{
			for ( int i=0; i<num; i++ )
			{
				FeMonitor mon;
				mon.transform = sf::Transform().translate(
					si[i].x_org,
					si[i].y_org );

				mon.size.x = si[i].width;
				mon.size.y = si[i].height;
				mon.num = si[i].screen_number;

				m_mon.push_back( mon );
			}
		}

		XFree( si );
		XCloseDisplay( xdisp );
	}
	else
#endif // USE_XINERAMA
#endif
	{
		//
		// Where there is no multi-monitor support, we just use the desktop dimensions returned by SFML
		//
		sf::VideoMode vm = sf::VideoMode::getDesktopMode();
		FeMonitor mc;
		mc.size.x = vm.width;
		mc.size.y = vm.height;
		mc.num = 0;
		m_mon.push_back( mc );
	}

	ASSERT( m_mon.size() > 0 );

	m_layoutSize = m_mon[0].size;
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
	m_listBox=NULL; // listbox gets deleted with the m_mon.elements below
	m_transform = sf::Transform();
	m_currentFont = &m_defaultFont;
	m_layoutFontName = m_feSettings->get_info( FeSettings::DefaultFont );
	m_user_page_size = -1;

	for ( std::vector<FeMonitor>::iterator itr=m_mon.begin(); itr!=m_mon.end(); ++itr )
	{
		while ( !(*itr).elements.empty() )
		{
			FeBasePresentable *p = (*itr).elements.back();
			(*itr).elements.pop_back();
			delete p;
		}
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

	m_layoutSize = m_mon[0].size;
	m_layoutScale.x = 1.0;
	m_layoutScale.y = 1.0;
}

void FePresent::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	std::vector<FeBasePresentable *>::const_iterator itl;

	//
	for ( unsigned int i=0; i<m_mon.size(); i++ )
	{
		// use m_transform on monitor 0
		states.transform = i ? m_mon[i].transform : m_transform;
		for ( itl=m_mon[i].elements.begin(); itl != m_mon[i].elements.end(); ++itl )
		{
			if ( (*itl)->get_visible() )
				target.draw( (*itl)->drawable(), states );
		}
	}
}

FeImage *FePresent::add_image( bool is_artwork,
		const std::string &n,
		int x,
		int y,
		int w,
		int h,
		std::vector<FeBasePresentable *> &l )
{
	FeTextureContainer *new_tex = new FeTextureContainer( is_artwork, n );
	new_tex->set_smooth( m_feSettings->get_info_bool( FeSettings::SmoothImages ) );

	FeImage *new_image = new FeImage( new_tex, x, y, w, h );
	new_image->set_scale_factor( m_layoutScale.x, m_layoutScale.y );

	// if this is a static image/video then load it now
	//
	if (( !is_artwork ) && ( n.find_first_of( "[" ) == std::string::npos ))
		new_tex->set_file_name( n.c_str() );

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
	new_text->set_scale_factor( m_layoutScale.x, m_layoutScale.y );

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
	new_lb->set_scale_factor( m_layoutScale.x, m_layoutScale.y );

	flag_redraw();
	m_listBox = new_lb;
	l.push_back( new_lb );
	return new_lb;
}

FeImage *FePresent::add_surface( int w, int h, std::vector<FeBasePresentable *> &l )
{
	FeSurfaceTextureContainer *new_surface = new FeSurfaceTextureContainer( w, h );
	new_surface->set_smooth( m_feSettings->get_info_bool( FeSettings::SmoothImages ) );

	//
	// Set the default sprite size to the same as the texture itself
	//
	FeImage *new_image = new FeImage( new_surface, 0, 0, w, h );
	new_image->set_scale_factor( m_layoutScale.x, m_layoutScale.y );

	new_image->texture_changed();

	flag_redraw();
	l.push_back( new_image );
	m_texturePool.push_back( new_surface );
	return new_image;
}

FeSound *FePresent::add_sound( const char *n, bool reuse )
{
	//
	// Behaviour:
	//
	// - if n is empty, return a new (empty) sound object
	// - if n is supplied:
	//      - if n matches an existing sound and reuse is true,
	//        return that matching sound object
	//      - otherwise return a new sound object loaded with n
	//

	std::string path;
	std::string name=n;
	if ( !name.empty() )
	{
		if ( is_relative_path( name ) )
		{
			int script_id = get_script_id();
			if ( script_id < 0 )
				m_feSettings->get_path( FeSettings::Current, path );
			else
				m_feSettings->get_plugin_full_path( script_id, path );
		}

		if ( reuse )
		{
			std::string test;
			if ( tail_compare( path, FE_ZIP_EXT ) )
				test = name;
			else
				test = path + name;

			for ( std::vector<FeSound *>::iterator itr=m_sounds.begin();
						itr!=m_sounds.end(); ++itr )
			{
				if ( test.compare( (*itr)->get_file_name() ) == 0 )
					return (*itr);
			}
		}
	}

	// Sound not found, try loading now
	//
	FeSound *new_sound = new FeSound();

	if ( !name.empty() )
		new_sound->load( path, name );

	new_sound->set_volume(
		m_feSettings->get_play_volume( FeSoundInfo::Sound ) );

	m_sounds.push_back( new_sound );
	return new_sound;
}

FeShader *FePresent::add_shader( FeShader::Type type, const char *shader1, const char *shader2 )
{
	std::string path;
	m_feSettings->get_path( FeSettings::Current, path );

	m_scriptShaders.push_back( new FeShader() );
	FeShader *sh = m_scriptShaders.back();

	switch ( type )
	{
		case FeShader::VertexAndFragment:
			if ( tail_compare( path, FE_ZIP_EXT ) )
			{
				FeZipStream zs1( path );
				zs1.open( shader1 );

				FeZipStream zs2( path );
				zs2.open( shader2 );

				sh->load( zs1, zs2 );
			}
			else
			{
				sh->load( path + shader1, path + shader2 );
			}
			break;

		case FeShader::Vertex:
		case FeShader::Fragment:
			if ( tail_compare( path, FE_ZIP_EXT ) )
			{
				FeZipStream zs( path );
				zs.open( shader1 );

				sh->load( zs, type );
			}
			else
			{
				sh->load( path + shader1, type );
			}
			break;

		case FeShader::Empty:
		default:
			break;
	}

	return sh;
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
	m_layoutScale.x = (float) m_mon[0].size.x / w;

	for ( std::vector<FeBasePresentable *>::iterator itr=m_mon[0].elements.begin();
			itr!=m_mon[0].elements.end(); ++itr )
		(*itr)->set_scale_factor( m_layoutScale.x, m_layoutScale.y );

	set_transforms();
	flag_redraw();
}

void FePresent::set_layout_height( int h )
{
	m_layoutSize.y = h;
	m_layoutScale.y = (float) m_mon[0].size.y / h;

	for ( std::vector<FeBasePresentable *>::iterator itr=m_mon[0].elements.begin();
			itr!=m_mon[0].elements.end(); ++itr )
		(*itr)->set_scale_factor( m_layoutScale.x, m_layoutScale.y );

	set_transforms();
	flag_redraw();
}

const FeFontContainer *FePresent::get_pooled_font( const std::string &n )
{
	std::vector<std::string> my_list;
	my_list.push_back( n );

	return get_pooled_font( my_list );
}

const FeFontContainer *FePresent::get_pooled_font(
		const std::vector < std::string > &l )
{
	std::string fullpath;

	// search list for a font
	for ( std::vector<std::string>::const_iterator itr=l.begin();
			itr != l.end(); ++itr )
	{
		if ( m_feSettings->get_font_file( fullpath, *itr ) )
			break;
	}

	// Check if this just matches the default font
	//
	if ( fullpath.empty()
			|| ( fullpath.compare( m_defaultFont.get_name() ) == 0 ) )
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
		change_selection( new_offset );
}

void FePresent::change_selection( int step, bool end_navigation )
{
		on_transition( ToNewSelection, step );

		m_feSettings->step_current_selection( step );
		update( false );

		on_transition( FromOldSelection, -step );

		if ( end_navigation )
			on_end_navigation();
}

bool FePresent::reset_screen_saver()
{
	if ( m_feSettings->get_present_state() == FeSettings::ScreenSaver_Showing )
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
		change_selection( 1, false );
		break;

	case FeInputMap::Up:
		change_selection( -1, false );
		break;

	case FeInputMap::PageDown:
		change_selection( get_page_size(), false );
		break;

	case FeInputMap::PageUp:
		change_selection( -get_page_size(), false );
		break;

	case FeInputMap::RandomGame:
		{
			int ls = m_feSettings->get_filter_size( m_feSettings->get_current_filter_index() );
			if ( ls > 0 )
			{
				int step = rand() % ls;
				if ( step != 0 )
					change_selection( step );
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
			update_to_new_list( 0, true );

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
				change_selection( step, false );
		}
		break;

	case FeInputMap::ScreenSaver:
		load_screensaver();
		break;

	case FeInputMap::Intro:
		if ( !load_intro() )
			load_layout();
		break;

	case FeInputMap::LAST_COMMAND:
	default:
		// Not handled by us, return false so calling function knows
		//
		return false;
	}

	return true;
}

int FePresent::update( bool new_list, bool new_display )
{
	std::vector<FeBaseTextureContainer *>::iterator itc;
	std::vector<FeBasePresentable *>::iterator itl;
	std::vector<FeMonitor>::iterator itm;

	if ( new_list )
	{
		for ( itc=m_texturePool.begin(); itc != m_texturePool.end(); ++itc )
			(*itc)->on_new_list( m_feSettings, new_display );

		for ( itm=m_mon.begin(); itm != m_mon.end(); ++itm )
		{
			for ( itl=(*itm).elements.begin(); itl != (*itm).elements.end(); ++itl )
				(*itl)->on_new_list( m_feSettings );
		}
	}

	for ( itc=m_texturePool.begin(); itc != m_texturePool.end(); ++itc )
		(*itc)->on_new_selection( m_feSettings );

	for ( itm=m_mon.begin(); itm != m_mon.end(); ++itm )
	{
		for ( itl=(*itm).elements.begin(); itl != (*itm).elements.end(); ++itl )
			(*itl)->on_new_selection( m_feSettings );
	}

	return 0;
}

void FePresent::on_end_navigation()
{
	std::vector<FeBaseTextureContainer *>::iterator itc;

	for ( itc=m_texturePool.begin(); itc != m_texturePool.end(); ++itc )
		(*itc)->on_end_navigation( m_feSettings );

	on_transition( EndNavigation, 0 );
}

// return false if the into should be cancelled
bool FePresent::load_intro()
{
	clear();
	m_baseRotation = FeSettings::RotateNone;
	set_transforms();
	m_feSettings->set_present_state( FeSettings::Intro_Showing );

	m_layoutTimer.restart();
	if ( !on_new_layout() )
		return false;

	bool retval = false;
	for ( std::vector<FeBasePresentable *>::iterator itr=m_mon[0].elements.begin();
			itr!=m_mon[0].elements.end(); ++itr )
	{
		if ( (*itr)->get_visible() )
			retval = true;
	}


	update( true, true );
	on_transition( StartLayout, FromToNoValue );
	return retval;
}

void FePresent::load_screensaver()
{
	on_transition( EndLayout, FromToScreenSaver );
	clear();
	set_transforms();
	m_feSettings->set_present_state( FeSettings::ScreenSaver_Showing );

	//
	// Run the script which actually sets up the screensaver
	//
	m_layoutTimer.restart();
	on_new_layout();

	//
	// if there is no screen saver script then do a blank screen
	//
	update( true, true );
	on_transition( StartLayout, FromToNoValue );
}

void FePresent::load_layout( bool initial_load )
{
	int var = ( m_feSettings->get_present_state() == FeSettings::ScreenSaver_Showing )
			? FromToScreenSaver : FromToNoValue;

	if ( !initial_load )
		on_transition( EndLayout, FromToNoValue );
	else
		var = FromToFrontend;

	clear();
	m_baseRotation = FeSettings::RotateNone;

	set_transforms();
	m_feSettings->set_present_state( FeSettings::Layout_Showing );

	if ( m_feSettings->displays_count() < 1 )
		return;

	//
	// Run the script which actually sets up the layout
	//
	m_layoutTimer.restart();
	on_new_layout();

	// make things usable if the layout is empty
	//
	bool empty_layout=true;
	for ( std::vector<FeBasePresentable *>::iterator itr=m_mon[0].elements.begin(); itr!=m_mon[0].elements.end(); ++itr )
	{
		if ( (*itr)->get_visible() )
		{
			empty_layout=false;
			break;
		}
	}

	if ( empty_layout )
	{
		std::cout << " - Layout is empty, initializing with the default layout settings" << std::endl;
		init_with_default_layout();
	}

	update_to_new_list( FromToNoValue, true );
	on_transition( StartLayout, var );
}

void FePresent::update_to_new_list( int var, bool new_layout )
{
	update( true, new_layout );
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
	bool saver_active = ( m_feSettings->get_present_state() == FeSettings::ScreenSaver_Showing );

	if ( !saver_active && ( saver_timeout > 0 ))
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
	m_transform = m_mon[0].transform;

	FeSettings::RotationState actualRotation
		= (FeSettings::RotationState)(( m_baseRotation + m_toggleRotation ) % 4);

	switch ( actualRotation )
	{
	case FeSettings::RotateNone:
		// do nothing
		break;
	case FeSettings::RotateRight:
		m_transform.translate( m_mon[0].size.x, 0 );
		m_transform.scale( (float) m_mon[0].size.x / m_mon[0].size.y,
												(float) m_mon[0].size.y / m_mon[0].size.x );
		m_transform.rotate(90);
		break;
	case FeSettings::RotateFlip:
		m_transform.translate( m_mon[0].size.x, m_mon[0].size.y );
		m_transform.rotate(180);
		break;
	case FeSettings::RotateLeft:
		m_transform.translate( 0, m_mon[0].size.y );
		m_transform.scale( (float) m_mon[0].size.x / m_mon[0].size.y,
											(float) m_mon[0].size.y / m_mon[0].size.x );
		m_transform.rotate(270);
		break;
	}

	m_transform.scale( m_layoutScale.x, m_layoutScale.y );
}

FeShader *FePresent::get_empty_shader()
{
	if ( !m_emptyShader )
		m_emptyShader = new FeShader();

	return m_emptyShader;
}

void FePresent::script_do_update( FeBasePresentable *bp )
{
	FePresent *fep = script_get_fep();
	if ( fep )
	{
		bp->on_new_list( fep->m_feSettings );
		bp->on_new_selection( fep->m_feSettings );
		fep->flag_redraw();
	}
}

void FePresent::script_do_update( FeBaseTextureContainer *tc )
{
	FePresent *fep = script_get_fep();
	if ( fep )
	{
		tc->on_new_list( fep->m_feSettings, false );
		tc->on_new_selection( fep->m_feSettings );
		fep->flag_redraw();
	}
}

void FePresent::script_flag_redraw()
{
	FePresent *fep = script_get_fep();
	if ( fep )
		fep->flag_redraw();
}

std::string FePresent::script_get_base_path()
{
	std::string path;

	FePresent *fep = script_get_fep();
	if ( fep )
	{
		FeSettings *fes = fep->get_fes();
		if ( fes )
		{
			int script_id = fep->get_script_id();
			if ( script_id < 0 )
				fes->get_path( FeSettings::Current, path );
			else
				fes->get_plugin_full_path(
					script_id, path );
		}
	}

	return path;
}

