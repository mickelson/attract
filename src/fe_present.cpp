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
#include "fe_file.hpp"
#include "fe_blend.hpp"
#include "zip.hpp"

#include <iostream>
#include <cmath>
#include <limits>

#ifndef NO_MOVIE
#include <Audio/AudioDevice.hpp>
#endif

#ifdef USE_XLIB
#include <X11/extensions/Xrandr.h>
#endif

#ifdef USE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#ifdef USE_BCM
#include <bcm_host.h>
#endif

#ifdef USE_DRM
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#endif

#ifdef SFML_SYSTEM_WINDOWS

#include <windows.h>

BOOL CALLBACK my_mon_enum_proc( HMONITOR, HDC, LPRECT mon_rect, LPARAM data )
{
	std::vector < FeMonitor > *monitors = (std::vector < FeMonitor > *)data;

	FeMonitor mon(
		monitors->size(),
		mon_rect->right - mon_rect->left,
		mon_rect->bottom - mon_rect->top );

	mon.transform = sf::Transform().translate( mon_rect->left, mon_rect->top );

	FeDebug() << "Multimon: monitor #" << monitors->size()
		<< ": " << mon.size.x << "x" << mon.size.y << " @ "
		<< mon_rect->left << "," << mon_rect->top << std::endl;

	// make sure primary monitor is first in m_mon vector
	//
	if (( mon_rect->left == 0 ) && ( mon_rect->top == 0 ))
		monitors->insert( monitors->begin(), mon );
	else
		monitors->push_back( mon );

	return TRUE;
}
#endif

FeFontContainer::FeFontContainer()
	: m_stream( NULL ),
	m_needs_reload( false )
{
}

FeFontContainer::~FeFontContainer()
{
	if ( m_stream )
		delete m_stream;
}

void FeFontContainer::set_font( const std::string &p, const std::string &n )
{
	m_name = n;

	if ( m_stream )
	{
		delete m_stream;
		m_stream = NULL;
	}

	if ( is_supported_archive( p ) )
	{
		FeZipStream *zs = new FeZipStream( p );
		zs->open( n );
		m_stream = zs;

		if ( !m_font.loadFromStream( *m_stream ) )
			FeLog() << "Error loading font: " << p << "[" << n << "]" << std::endl;
	}
	else
	{
		m_stream = new FeFileInputStream( p + n );
		if ( !m_font.loadFromStream( *m_stream ) )
			FeLog() << "Error loading font from file: " << p + n << std::endl;
	}
}

const sf::Font &FeFontContainer::get_font() const
{
	if ( m_needs_reload && m_stream )
	{
		m_font.loadFromStream( *m_stream );
		m_needs_reload=false;
	}

	return m_font;
}

void FeFontContainer::clear_font()
{
	m_font = sf::Font();
	m_needs_reload = true;
}

FeMonitor::FeMonitor( int n, int w, int h )
	: num( n )
{
	size.x = w;
	size.y = h;
}

FeMonitor::FeMonitor( const FeMonitor &o )
	: FePresentableParent( o ),
	transform( o.transform ),
	size( o.size ),
	num( o.num )
{
}

FeMonitor &FeMonitor::operator=( const FeMonitor &o )
{
	elements = o.elements;
	transform = o.transform;
	size = o.size;
	num = o.num;

	return *this;
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

FePresent::FePresent( FeSettings *fesettings, FeFontContainer &defaultfont, FeWindow &wnd )
	: m_feSettings( fesettings ),
	m_window( wnd ),
	m_currentFont( &defaultfont ),
	m_defaultFont( defaultfont ),
	m_baseRotation( FeSettings::RotateNone ),
	m_toggleRotation( FeSettings::RotateNone ),
	m_playMovies( true ),
	m_user_page_size( -1 ),
	m_preserve_aspect( false ),
	m_custom_overlay( false ),
	m_listBox( NULL ),
	m_emptyShader( NULL ),
	m_overlay_caption( NULL ),
	m_overlay_lb( NULL ),
	m_refresh_rate( 0 )
{
	m_layoutFontName = m_feSettings->get_info( FeSettings::DefaultFont );
	init_monitors();
}

void FePresent::init_monitors()
{
#if defined(SFML_SYSTEM_WINDOWS)
	FeLog() << "FLAG: SFML_SYSTEM_WINDOWS" << std::endl;
#endif
#if defined(SFML_SYSTEM_MACOS)
	FeLog() << "FLAG: SFML_SYSTEM_MACOS" << std::endl;
#endif
#if defined(USE_XLIB)
	FeLog() << "FLAG: USE_XLIB" << std::endl;
#endif
#if defined(USE_BCM)
	FeLog() << "FLAG: USE_BCM" << std::endl;
#endif
#if defined(USE_DRM)
	FeLog() << "FLAG: USE_DRM" << std::endl;
#endif

	m_mon.clear();

	//
	// Handle multi-monitors
	//
	// We support multi-monitor setups on MS-Windows when in fullscreen or "fillscreen" mode
	// We also determine display's refresh rate here
	//

#if defined(USE_BCM)
	bcm_host_init();
	TV_DISPLAY_STATE_T tvstate;
	if ( vc_tv_get_display_state( &tvstate ) == 0 )
		m_refresh_rate = tvstate.display.hdmi.frame_rate;
#endif

#if defined(USE_DRM)
	#define MAX_DRM_DEVICES 64

	drmDevicePtr devices[MAX_DRM_DEVICES] = { NULL };
	int num_devices, fd = -1;

	num_devices = drmGetDevices2( 0, devices, MAX_DRM_DEVICES );
	for ( int i = 0; i < num_devices; i++ )
	{
		drmDevicePtr device = devices[i];
		int ret;

		if ( !( device->available_nodes & ( 1 << DRM_NODE_PRIMARY )))
			continue;

		int drm_fd = open( device->nodes[DRM_NODE_PRIMARY], O_RDWR | O_CLOEXEC );
		drmModeRes *p_res = drmModeGetResources( drm_fd );

		for ( int i = 0; i < p_res->count_connectors; i++ )
		{
			drmModeConnector *p_connector = drmModeGetConnector( drm_fd, p_res->connectors[i] );
			drmModeEncoder *encoder;
			drmModeCrtc *crtc;
			encoder = drmModeGetEncoder( drm_fd, p_connector->encoder_id );
			drmModeModeInfo mode_info;
			memset( &mode_info, 0, sizeof( drmModeModeInfo ));
			if ( encoder != NULL )
			{
				crtc = drmModeGetCrtc( drm_fd, encoder->crtc_id );
				drmModeFreeEncoder( encoder );
				if ( crtc != NULL )
				{
					if ( crtc->mode_valid )
						m_refresh_rate = crtc->mode.vrefresh;
					drmModeFreeCrtc( crtc );
				}
			}
		}
		close( drm_fd );
	}
	drmFreeDevices( devices, num_devices );
#endif

#if defined(SFML_SYSTEM_WINDOWS)
	DEVMODE devMode;
	memset( &devMode, 0, sizeof(DEVMODE) );
	devMode.dmSize = sizeof(DEVMODE);
	devMode.dmDriverExtra = 0;

	if ( EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &devMode ) != 0 )
		m_refresh_rate = devMode.dmDisplayFrequency;

	if ( m_feSettings->get_info_bool( FeSettings::MultiMon )
		&& !is_windowed_mode( m_feSettings->get_window_mode() ) )
	{
		EnumDisplayMonitors( NULL, NULL, my_mon_enum_proc, (LPARAM)&m_mon );

		//
		// The Windows virtual screen can have a negative valued top left corner, whereas in SFML we
		// always have a 0,0 top left.  So we correct the transforms in m_mon to SFML's coordinates now.
		//
		int translate_x = -GetSystemMetrics( SM_XVIRTUALSCREEN );
		int translate_y = -GetSystemMetrics( SM_YVIRTUALSCREEN );

		//
		// On Windows 'Fill screen' mode our window is offscreen 1 pixel in each direction, so correct
		// for that here to align draw area with screen
		//
		if ( m_feSettings->get_window_mode() == FeSettings::Default )
		{
			translate_x += 1;
			translate_y += 1;
		}

		sf::Transform correction = sf::Transform().translate( translate_x, translate_y );

		for ( std::vector<FeMonitor>::iterator itr=m_mon.begin(); itr!=m_mon.end(); ++itr )
			(*itr).transform *= correction;
	}
	else
#elif defined(USE_XLIB)
	if ( 1 )
	{
		Display *xdisp = XOpenDisplay( NULL );
		Window root = RootWindow( xdisp, 0 );
		XRRScreenConfiguration *xconf = XRRGetScreenInfo( xdisp, root );
		m_refresh_rate = XRRConfigCurrentRate( xconf );
#if defined(USE_XINERAMA)
		int num = 0;
		XineramaScreenInfo *si = XineramaQueryScreens( xdisp, &num );

		if ( si )
		{
			if (( m_feSettings->get_info_bool( FeSettings::MultiMon ) )
					&& ( !is_windowed_mode( m_feSettings->get_window_mode() )))
			{
				for ( int i=0; i<num; i++ )
				{
					FeMonitor mon(
						si[i].screen_number,
						si[i].width,
						si[i].height );

					mon.transform = sf::Transform().translate(
						si[i].x_org,
						si[i].y_org );

					FeDebug() << "Multimon: monitor #" << si[i].screen_number
						<< ": " << mon.size.x << "x" << mon.size.y << " @ "
						<< si[i].x_org << "," << si[i].y_org << std::endl;

					m_mon.push_back( mon );
				}
			}
			else
			{
				FeMonitor mon(
					si[0].screen_number,
					si[0].width,
					si[0].height );

				m_mon.push_back( mon );
			}
		}

		XFree( si );
#endif
		XCloseDisplay( xdisp );
	}
	else
#endif
	{
		FeMonitor mc( 0, m_window.get_win().getSize().x, m_window.get_win().getSize().y );

#ifdef SFML_SYSTEM_WINDOWS
		//
		// On Windows 'Fill screen' mode our window is offscreen 1 pixel in each direction, so correct
		// for that here to align draw area with screen.
		//
		if ( m_feSettings->get_window_mode() == FeSettings::Default )
		{
			mc.size.x -= 2;
			mc.size.y -= 2;
			mc.transform = sf::Transform().translate( 1, 1 );
		}
#endif

		m_mon.push_back( mc );
	}

	ASSERT( m_mon.size() > 0 );

	m_layoutSize = m_mon[0].size;

	if ( m_refresh_rate == 0 )
	{
		FeLog() << "Failed to detect the refresh rate. Defaulting to 60 Hz" << std::endl;
		m_refresh_rate = 60;
	}
	else
		FeLog() << "Monitor's Refresh Rate: " << m_refresh_rate << " Hz" << std::endl;
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
	m_defaultFont.clear_font();
	m_layoutFontName = m_feSettings->get_info( FeSettings::DefaultFont );
	m_user_page_size = -1;
	m_preserve_aspect = false;
	m_custom_overlay = false;
	m_overlay_caption = NULL;
	m_overlay_lb = NULL;

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

	FeBlend::clear_default_shaders();
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
		FePresentableParent &p )
{
	FeTextureContainer *new_tex = new FeTextureContainer( is_artwork, n );
	new_tex->set_smooth( m_feSettings->get_info_bool( FeSettings::SmoothImages ) );

	FeImage *new_image = new FeImage( p, new_tex, x, y, w, h );
	new_image->set_scale_factor( m_layoutScale.x, m_layoutScale.y );

	// if this is a static image/video then load it now
	//
	if (( !is_artwork ) && ( n.find_first_of( "[" ) == std::string::npos ))
		new_image->setFileName( n.c_str() );

	flag_redraw();
	m_texturePool.push_back( new_tex );
	p.elements.push_back( new_image );

	return new_image;
}

FeImage *FePresent::add_clone( FeImage *o,
			FePresentableParent &p )
{
	FeImage *new_image = new FeImage( o );
	flag_redraw();
	p.elements.push_back( new_image );
	return new_image;
}

FeText *FePresent::add_text( const std::string &n, int x, int y, int w, int h,
			FePresentableParent &p )
{
	FeText *new_text = new FeText( p, n, x, y, w, h );

	ASSERT( m_currentFont );
	new_text->setFont( m_currentFont->get_font() );
	new_text->set_scale_factor( m_layoutScale.x, m_layoutScale.y );

	flag_redraw();
	p.elements.push_back( new_text );
	return new_text;
}

FeListBox *FePresent::add_listbox( int x, int y, int w, int h,
		FePresentableParent &p )
{
	FeListBox *new_lb = new FeListBox( p, x, y, w, h );

	ASSERT( m_currentFont );
	new_lb->setFont( m_currentFont->get_font() );
	new_lb->set_scale_factor( m_layoutScale.x, m_layoutScale.y );

	flag_redraw();
	m_listBox = new_lb;
	p.elements.push_back( new_lb );
	return new_lb;
}

FeImage *FePresent::add_surface( int w, int h, FePresentableParent &p )
{
	FeSurfaceTextureContainer *new_surface = new FeSurfaceTextureContainer( w, h );
	new_surface->set_smooth( m_feSettings->get_info_bool( FeSettings::SmoothImages ) );
	new_surface->set_nesting_level( p.get_nesting_level() + 1 );

	//
	// Set the default sprite size to the same as the texture itself
	//
	FeImage *new_image = new FeImage( p, new_surface, 0, 0, w, h );
	new_image->set_scale_factor( m_layoutScale.x, m_layoutScale.y );
	if ( sf::Shader::isAvailable() )
		new_image->set_blend_mode( FeBlend::Premultiplied );

	new_image->texture_changed();

	flag_redraw();
	p.elements.push_back( new_image );
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
			if ( is_supported_archive( path ) )
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

	std::string s1;
	if ( shader1 )
		s1 = clean_path( shader1 );

	m_scriptShaders.push_back( new FeShader() );
	FeShader *sh = m_scriptShaders.back();

	if ( !is_relative_path( s1 ) )
		path.clear();

	switch ( type )
	{
		case FeShader::VertexAndFragment:
			if ( is_supported_archive( path ) )
			{
				//
				// Known Issue: We don't properly handle the
				// situation where one shader is specified as a
				// relative path and is in a zip, while the other
				// is specified as an absolute path.  If the first
				// is in a zip, the second is assumed to be in the
				// same zip as well.
				//
				FeZipStream zs1( path );
				zs1.open( shader1 );

				FeZipStream zs2( path );
				zs2.open( shader2 );

				sh->load( zs1, zs2 );
			}
			else
			{
				std::string path2 = path;
				std::string s2;
				if ( shader2 )
					s2 = clean_path( shader2 );

				if ( !is_relative_path( s2 ) )
					path2.clear();

				sh->load( path + s1, path2 + s2 );
			}
			break;

		case FeShader::Vertex:
		case FeShader::Fragment:
			if ( is_supported_archive( path ) )
			{
				FeZipStream zs( path );
				zs.open( shader1 );

				sh->load( zs, type );
			}
			else
			{
				sh->load( path + s1, type );
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
	if ( w != m_layoutSize.x )
	{
		m_layoutSize.x = w;
		set_transforms();
		flag_redraw();
	}
}

void FePresent::set_layout_height( int h )
{
	if ( h != m_layoutSize.y )
	{
		m_layoutSize.y = h;
		set_transforms();
		flag_redraw();
	}
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
	std::string fpath, ffile;

	// search list for a font
	for ( std::vector<std::string>::const_iterator itr=l.begin();
			itr != l.end(); ++itr )
	{
		if ( m_feSettings->get_font_file( fpath, ffile, *itr ) )
			break;
	}

	// Check if this just matches the default font
	//
	if ( ffile.empty()
			|| ( ffile.compare( m_defaultFont.get_name() ) == 0 ) )
		return &m_defaultFont;

	// Next check if this font is already loaded in our pool
	//
	for ( std::vector<FeFontContainer *>::iterator itr=m_fontPool.begin();
		itr != m_fontPool.end(); ++itr )
	{
		if ( ffile.compare( (*itr)->get_name() ) == 0 )
			return *itr;
	}

	// No match, so load this font and add it to the pool
	//
	m_fontPool.push_back( new FeFontContainer() );
	m_fontPool.back()->set_font( fpath, ffile );

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
	if ( r != m_baseRotation )
	{
		m_baseRotation = (FeSettings::RotationState)r;
		set_transforms();
		flag_redraw();
	}
}

int FePresent::get_base_rotation() const
{
	return m_baseRotation;
}

void FePresent::set_toggle_rotation( int r )
{
	if ( r != m_toggleRotation )
	{
		m_toggleRotation = (FeSettings::RotationState)r;
		set_transforms();
		flag_redraw();
	}
}

int FePresent::get_toggle_rotation() const
{
	return m_toggleRotation;
}

const char *FePresent::get_display_name() const
{
	return m_feSettings->get_current_display_title().c_str();
}

int FePresent::get_display_index() const
{
	return m_feSettings->get_current_display_index();
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

int FePresent::get_current_filter_size() const
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
	case FeInputMap::NextGame:
		change_selection( 1, false );
		break;

	case FeInputMap::PrevGame:
		change_selection( -1, false );
		break;

	case FeInputMap::NextPage:
		change_selection( get_page_size(), false );
		break;

	case FeInputMap::PrevPage:
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

void FePresent::redraw_surfaces()
{
	std::vector<FeBaseTextureContainer *>::iterator itc;

	for ( itc=m_texturePool.begin(); itc != m_texturePool.end(); ++itc )
		(*itc)->on_redraw_surfaces();
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

	// Don't do the StartLayout signal for the intro
	update( true, true );
	return ( !m_mon[0].elements.empty() );
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
	on_transition( StartLayout, FromToNoValue );
	update( true, true );
}

void FePresent::load_layout( bool initial_load, bool suppress_transition )
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
		FeLog() << " - Layout is empty, initializing with the default layout settings" << std::endl;
		init_with_default_layout();
	}

	if ( !suppress_transition )
		on_transition( StartLayout, var );

	update_to_new_list( FromToNoValue, true, suppress_transition );
}

void FePresent::update_to_new_list( int var, bool reset_display, bool suppress_transition )
{
	update( true, reset_display );

	if ( !suppress_transition )
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
		if ( (*itm)->tick( m_feSettings, m_playMovies ) )
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

	// Protect against integer overflow of the layout time,  which is limited by our usage of
	// sf::Time::asMilliseconds().  asMillseconds() returns sf::Int32, which maxes out at ~2billion
	//
	// THis means the layout is forced by AM to reset after about a month of running
	//
	if ( m_layoutTimer.getElapsedTime().asMilliseconds() > std::numeric_limits<sf::Int32>::max() - 10000 )
		load_layout();

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

#ifndef NO_MOVIE
	//
	// Release all the openAL buffers, contexts and devices so that we don't block
	// the emulator from accessing the system's sound
	//
	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->release_audio( true );

	for ( std::vector<FeSound *>::iterator its=m_sounds.begin();
				its != m_sounds.end(); ++its )
		(*its)->release_audio( true );

	sf::AudioDevice::release_audio( true );
#endif
}

void FePresent::post_run()
{
	std::vector<FeSound *>::iterator its;

#ifndef NO_MOVIE
	//
	// Re-establish openAL stuff now that we are back from the emulator
	//
	sf::AudioDevice::release_audio( false );

	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->release_audio( false );

	for ( its=m_sounds.begin(); its != m_sounds.end(); ++its )
		(*its)->release_audio( false );
#endif

	for ( std::vector<FeBaseTextureContainer *>::iterator itm=m_texturePool.begin();
				itm != m_texturePool.end(); ++itm )
		(*itm)->set_vol( m_feSettings->get_play_volume( FeSoundInfo::Movie ) );

	for ( its=m_sounds.begin(); its != m_sounds.end(); ++its )
		(*its)->set_volume( m_feSettings->get_play_volume( FeSoundInfo::Sound ) );

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

const sf::Font *FePresent::get_default_font() const
{
	return &(m_defaultFont.get_font());
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

	if ( m_preserve_aspect )
	{
		if (( actualRotation == FeSettings::RotateRight )
			|| ( actualRotation == FeSettings::RotateLeft ))
		{
			m_layoutScale.x = m_layoutScale.y = std::min(
				(float) m_mon[0].size.y / m_layoutSize.x,
				(float) m_mon[0].size.x / m_layoutSize.y );

			float adjust_x = std::fabs( m_layoutSize.y * m_layoutScale.x - m_mon[0].size.x ) / 2;
			float adjust_y = std::fabs( m_layoutSize.x * m_layoutScale.y - m_mon[0].size.y ) / 2;

			if ( actualRotation == FeSettings::RotateRight )
			{
				m_transform.translate( m_mon[0].size.x - adjust_x, adjust_y );
				m_transform.rotate(90);
			}
			else
			{
				m_transform.translate( adjust_x, m_mon[0].size.y - adjust_y );
				m_transform.rotate(270);
			}
		}
		else
		{
			m_layoutScale.x = m_layoutScale.y = std::min(
				(float) m_mon[0].size.x / m_layoutSize.x,
				(float) m_mon[0].size.y / m_layoutSize.y );

			float adjust_x = (m_layoutSize.x * m_layoutScale.x - m_mon[0].size.x) / 2;
			float adjust_y = (m_layoutSize.y * m_layoutScale.y - m_mon[0].size.y) / 2;
			if ( actualRotation == FeSettings::RotateNone )
				m_transform.translate( -adjust_x, -adjust_y );
			else
			{
				m_transform.translate(
						m_mon[0].size.x + adjust_x, m_mon[0].size.y + adjust_y );
				m_transform.rotate(180);
			}
		}
	}
	else // !m_preserve_aspect
	{
		m_layoutScale.x = (float) m_mon[0].size.x / m_layoutSize.x;
		m_layoutScale.y = (float) m_mon[0].size.y / m_layoutSize.y;

		switch ( actualRotation )
		{
		case FeSettings::RotateNone:
			break;

		case FeSettings::RotateRight:
			m_transform.translate( m_mon[0].size.x, 0 );
			m_transform.scale( (float) m_mon[0].size.x / m_mon[0].size.y,
				(float) m_mon[0].size.y / m_mon[0].size.x );
			m_transform.rotate(90);
			break;

		case FeSettings::RotateLeft:
			m_transform.translate( 0, m_mon[0].size.y );
			m_transform.scale( (float) m_mon[0].size.x / m_mon[0].size.y,
				(float) m_mon[0].size.y / m_mon[0].size.x );
			m_transform.rotate(270);
			break;

		case FeSettings::RotateFlip:
			m_transform.translate( m_mon[0].size.x, m_mon[0].size.y );
			m_transform.rotate(180);
			break;
		}
	}

	m_transform.scale( m_layoutScale.x, m_layoutScale.y );

	for ( std::vector<FeBasePresentable *>::iterator itr=m_mon[0].elements.begin();
			itr!=m_mon[0].elements.end(); ++itr )
		(*itr)->set_scale_factor( m_layoutScale.x, m_layoutScale.y );
}

FeShader *FePresent::get_empty_shader()
{
	if ( !m_emptyShader )
		m_emptyShader = new FeShader();

	return m_emptyShader;
}

void FePresent::set_search_rule( const char *s )
{
	m_feSettings->set_search_rule( s );
	update_to_new_list( 0 );
}

const char *FePresent::get_search_rule()
{
	return m_feSettings->get_search_rule().c_str();
}

void FePresent::set_preserve_aspect_ratio( bool p )
{
	if ( p != m_preserve_aspect )
	{
		m_preserve_aspect = p;
		set_transforms();
		flag_redraw();
	}
}

bool FePresent::get_preserve_aspect_ratio()
{
	return m_preserve_aspect;
}

bool FePresent::get_overlay_custom_controls( FeText *&t, FeListBox *&lb )
{
	t = m_overlay_caption;
	lb = m_overlay_lb;

	return m_custom_overlay;
}

int FePresent::get_layout_ms()
{
	return m_layoutTimer.getElapsedTime().asMilliseconds();
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

