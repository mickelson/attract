/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-2016 Andrew Mickelson
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


#include "fe_util.hpp"
#include "fe_settings.hpp"
#include "fe_window.hpp"

#ifdef SFML_SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // SFML_SYSTEM_WINDOWS

#ifdef SFML_SYSTEM_MACOS
#include "fe_util_osx.hpp"
#endif // SFM_SYSTEM_MACOS

#include <iostream>
#include <fstream>

#include <SFML/System/Sleep.hpp>

class FeWindowPosition : public FeBaseConfigurable
{
public:
	sf::Vector2i m_pos;
	sf::Vector2u m_size;

	static const char *FILENAME;

	FeWindowPosition( const sf::Vector2i &pos, const sf::Vector2u &size )
		: m_pos( pos ),
		m_size( size )
	{
	}

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &filename )
	{
		size_t pos=0;
		std::string token;
		if ( setting.compare( "position" ) == 0 )
		{
			token_helper( value, pos, token, "," );
			m_pos.x = as_int( token );

			token_helper( value, pos, token );
			m_pos.y = as_int( token );
		}
		else if ( setting.compare( "size" ) == 0 )
		{
			token_helper( value, pos, token, "," );
			m_size.x = as_int( token );

			token_helper( value, pos, token );
			m_size.y = as_int( token );
		}
		return 1;
	};

	void save( const std::string &filename )
	{
		std::ofstream outfile( filename.c_str() );
		if ( outfile.is_open() )
		{
			outfile << "position " << m_pos.x << "," << m_pos.y << std::endl;
			outfile << "size " << m_size.x << "," << m_size.y << std::endl;
		}
		outfile.close();
	}
};

const char *FeWindowPosition::FILENAME = "window.am";

FeWindow::FeWindow( FeSettings &fes )
	: m_fes( fes )
{
}

void FeWindow::onCreate()
{
#ifdef SFML_SYSTEM_WINDOWS
	int left, top, width, height;
	if (( m_fes.get_info_bool( FeSettings::MultiMon ) )
		&& ( !is_windowed_mode( m_fes.get_window_mode() ) ))
	{
		left = GetSystemMetrics( SM_XVIRTUALSCREEN );
		top = GetSystemMetrics( SM_YVIRTUALSCREEN );
		width = GetSystemMetrics( SM_CXVIRTUALSCREEN );
		height = GetSystemMetrics( SM_CYVIRTUALSCREEN );
	}
	else
	{
		left = getPosition().x;
		top = getPosition().y;
		width = getSize().x;
		height = getSize().y;
	}

	sf::WindowHandle hw = getSystemHandle();

	//
	// The "WS_POPUP" style can cause grief switching to MAME.  It also looks clunky/flickery
	// when transitioning between frontend and emulator.
	//
	// With Windows 10 v1607, it seems that the "WS_POPUP" style is required in order for a
	// window to be drawn over the taskbar.
	//
	// So we keep WS_POPUP for "Fullscreen Mode", because the user wants to force full screen
	// with that setting.
	//
	// In "Fill screen" and "Window" modes, we use the WS_BORDER style for smoother transitions.
	//
	if (( m_fes.get_window_mode() != FeSettings::Fullscreen )
		&& (( GetWindowLong( hw, GWL_STYLE ) & WS_POPUP ) != 0 ))
	{
		SetWindowLong( hw, GWL_STYLE,
			WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );

		// resize the window off screen 1 pixel in each direction so we don't see the window border
		left -= 1;
		top -= 1;
		width += 2;
		height += 2;
	}

	SetWindowPos( hw,
		m_fes.get_window_mode() == FeSettings::Fullscreen ? HWND_BOTTOM : HWND_TOP,
		left, top, width, height, SWP_FRAMECHANGED);

	// As of 2.1, SFML caches the window size. We call setSize below to update SFML appropriately
	setSize( sf::Vector2u( width, height ) );

	ShowWindow(hw, SW_SHOW);
	SetFocus( hw );

#elif defined(USE_XLIB)
	//
	// Notes: if xinerama and multimon are enabled, this should set our window to cover all available
	// monitors
	//
	// If multimon is disabled, this fixes positioning problems that SFML(?) seems to have where
	// the window contents aren't drawn in the correct place vertically on fullscreen/fillscreen modes
	//
	int x, y, width, height;
	get_x11_geometry(
		m_fes.get_info_bool( FeSettings::MultiMon ) && !is_windowed_mode( m_fes.get_window_mode() ),
		x, y, width, height );

	setPosition( sf::Vector2i( x, y ) );
	setSize( sf::Vector2u( width, height ) );

#endif

	setVerticalSyncEnabled(true);
	setKeyRepeatEnabled(false);
	setMouseCursorVisible(false);
	setJoystickThreshold( 1.0 );

	sf::RenderWindow::onCreate();
}

void FeWindow::initial_create()
{
	int style_map[4] =
	{
		sf::Style::None,			// FeSettings::Default
		sf::Style::Fullscreen,	// FeSettings::Fullscreen
		sf::Style::Default,		// FeSettings::Window
		sf::Style::None			// FeSettings::WindowNoBorder
	};

	int win_mode = m_fes.get_window_mode();

#ifdef USE_XINERAMA
	if ( m_fes.get_info_bool( FeSettings::MultiMon ) && ( win_mode != FeSettings::Default ))
		std::cout << " ! NOTE: Use the 'Fill Screen' window mode if you want multiple monitor support to function correctly" << std::endl;
#endif

	// Create window
	create(
		sf::VideoMode::getDesktopMode(),
		"Attract-Mode",
		style_map[ win_mode ] );

	if ( is_windowed_mode( win_mode ) )
	{
		FeWindowPosition win_pos(
			sf::Vector2i( 0, 0 ),
			sf::Vector2u( 480, 320 ) );

		win_pos.load_from_file( m_fes.get_config_dir() + FeWindowPosition::FILENAME );

		setPosition( win_pos.m_pos );
		setSize( win_pos.m_size );
	}
#ifdef SFML_SYSTEM_MACOS
	else if ( win_mode == FeSettings::Default )
	{
		osx_hide_menu_bar();
		setPosition( sf::Vector2i( 0, 0 ) );
	}
#endif

	sf::Vector2u wsize = getSize();
	m_fes.init_mouse_capture( wsize.x, wsize.y );

	// Only mess with the mouse position if mouse moves mapped
	if ( m_fes.test_mouse_reset( 0, 0 ) )
		sf::Mouse::setPosition( sf::Vector2i( wsize.x / 2, wsize.y / 2 ), *this );
}

void launch_callback( void *o )
{
#if defined(SFML_SYSTEM_LINUX)
	//
	// On Linux, fullscreen mode is confirmed to block the emulator
	// from running...  So we close our main window each time we run
	// an emulator and then recreate it (!) when the emulator is done.
	//
	FeWindow *win = (FeWindow *)o;
	if ( win->m_fes.get_window_mode() == FeSettings::Fullscreen )
	{
		sf::sleep( sf::milliseconds( 1000 ) );
		win->close(); // this fixes raspi version (w/sfml-pi) obscuring daphne (and others?)
	}
#endif
}

bool FeWindow::run()
{
#ifndef SFML_SYSTEM_MACOS
	// Don't move so much to the corner on Macs due to hot corners
	//
	const int HIDE_OFFSET=3;
#else
	const int HIDE_OFFSET=1;
#endif
	// Move the mouse to the bottom left corner so it isn't visible
	// when the emulator launches.
	//
	sf::Vector2i reset_pos = sf::Mouse::getPosition();

	sf::Vector2i hide_pos = getPosition();
	hide_pos.x += getSize().x - HIDE_OFFSET;
	hide_pos.y += getSize().y - HIDE_OFFSET;

	sf::Mouse::setPosition( hide_pos );

	sf::Clock timer;

	//
	// For Steam support (at least on Windows) we have a "minimum run"
	// value that can be set per emulator.  run() below sets this value.
	// and we wait at least this amount of time (in seconds) and then wait
	// for focus to return to Attract-Mode if this value is set greater than 0
	//
	int min_run;
	m_fes.run( min_run, launch_callback, this );

	if ( min_run > 0 )
	{
		sf::Time elapsed = timer.getElapsedTime();
		if ( elapsed < sf::seconds( min_run ) )
			sf::sleep( sf::seconds( min_run ) - elapsed );

		//
		// Wait for focus to return
		//
		bool done_wait=false;
		while ( !done_wait && isOpen() )
		{
			sf::Event ev;
			while (pollEvent(ev))
			{
				if ( ev.type == sf::Event::GainedFocus )
				{
					done_wait=true;
					break;
				}
				else if ( ev.type == sf::Event::Closed )
					return false;
			}

			sf::sleep( sf::milliseconds( 250 ) );
		}
	}

#if defined(SFML_SYSTEM_LINUX)
	if ( m_fes.get_window_mode() == FeSettings::Fullscreen )
	{
		//
		// On linux fullscreen mode we hide our window after launching the program.  Simply showing
		// the window again doesn't work right (focus doesn't come back), so we close the window
		// and recreate it completely
		//
		initial_create();
	}
#elif defined(SFML_SYSTEM_MACOS)
	osx_take_focus();
#elif defined(SFML_SYSTEM_WINDOWS)
	SetForegroundWindow( getSystemHandle() );
#endif

	sf::Mouse::setPosition( reset_pos );

	// Empty the window event queue, so we don't go triggering other
	// right away after running an emulator
	sf::Event ev;

	while (isOpen() && pollEvent(ev))
	{
		if ( ev.type == sf::Event::Closed )
			return false;
	}

	return true;
}

void FeWindow::on_exit()
{
	if ( is_windowed_mode( m_fes.get_window_mode() ) )
	{
		FeWindowPosition win_pos( getPosition(), getSize() );
		win_pos.save( m_fes.get_config_dir() + FeWindowPosition::FILENAME );
	}
}
