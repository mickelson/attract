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
	//
	// Windows Notes:
	//
	// Out present strategy with Windows is to stick with the WS_POPUP window style for our
	// window.  SFML seems to always create windows with this style
	//
	// In previous FE versions, the WS_POPUP style was causing grief switching to MAME.
	// It also looked clunky/flickery when transitioning between frontend and emulator.
	// More recent tests suggest these WS_POPUP problems have gone away (fingers crossed)
	//
	// With Windows 10 v1607, it seems that the WS_POPUP style is required in order for a
	// window to be drawn over the taskbar.
	//
	// Windows API call to undo the WS_POPUP Style.  Seems to require a ShowWindow() call
	// afterwards to take effect:
	//
	//		SetWindowLongPtr( getSystemHandle(), GWL_STYLE,
	//			WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
	//
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

	if ( m_fes.get_window_mode() == FeSettings::Default ) // "fill screen" mode
	{
		// resize the window off screen 1 pixel in each direction so we don't see the window border
		left -= 1;
		top -= 1;
		width += 2;
		height += 2;
	}

	// As of 2.1, SFML caches the window size internally
	setPosition( sf::Vector2i( left, top ) );
	setSize( sf::Vector2u( width, height ) );

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
	int style_map[5] =
	{
		sf::Style::None,			// FeSettings::Default
		sf::Style::Fullscreen,	// FeSettings::Fullscreen
		sf::Style::Default,		// FeSettings::Window
		sf::Style::None,			// FeSettings::WindowNoBorder
		sf::Style::None				// FeSettings::BorderlessFullscreen
	};

	int win_mode = m_fes.get_window_mode();

#ifdef USE_XINERAMA
	if ( m_fes.get_info_bool( FeSettings::MultiMon ) && ( win_mode != FeSettings::Default ))
		FeLog() << " ! NOTE: Use the 'Fill Screen' window mode if you want multiple monitor support to function correctly" << std::endl;
#endif

	// Create window
	FeDebug() << "Creating Attract-Mode window" << std::endl;

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

#ifdef SFML_SYSTEM_WINDOWS
	// Fill Screen mode has an offset so the top row and the left column of the window is not visible.
	// We call setView to compensate this -1,-1 offset of the window.
	if ( win_mode == FeSettings::Default )
		setView(sf::View(sf::FloatRect(getPosition().x, getPosition().y, getSize().x, getSize().y)));

	SetForegroundWindow( getSystemHandle() );
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
	FeWindow *win = (FeWindow *)o;
	if ( win->m_fes.get_window_mode() == FeSettings::Fullscreen )
	{
#if defined(USE_XLIB)
		//
		// On X11 Linux, fullscreen mode is confirmed to block the emulator
		// from running on some systems...  So we wait 1 second and if we still
		// have focus then we close our main window now (and then recreate it (!)
		// when the emulator is done).
		//
		sf::sleep( sf::milliseconds( 1000 ) );

 // hasFocus() is only available as of SFML 2.2.
 #if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 ))
		if ( win->hasFocus() )
		{
			FeDebug() << "Attract-Mode window still has focus, closing now" << std::endl;
			win->close();
		}
 #else
		FeDebug() << "Closing Attract-Mode window" << std::endl;
		win->close();
 #endif

#else
		FeDebug() << "Closing Attract-Mode window" << std::endl;
		win->close(); // this fixes raspi version (w/sfml-pi) obscuring daphne (and others?)
#endif
	}
#endif
}

void wait_callback( void *o )
{
	FeWindow *win = (FeWindow *)o;

	if ( win->isOpen() )
	{
		sf::Event ev;
		while ( win->pollEvent( ev ) )
		{
			if ( ev.type == sf::Event::Closed )
				return;
		}
		// Clear the frame buffer so there is no stale frame flashing on game launch/exit 
		// Don't clear if Multimonitor is enabled and window mode is set to Fill Screen 
		if( !win->m_fes.get_info_bool( FeSettings::MultiMon ) || ( win->m_fes.get_window_mode() != FeSettings::Default ) )
		{ 
			win->clear();   
			win->display();  
		}
	}
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

	int nbm_wait; // non-blocking mode wait time (in seconds)
	m_fes.run( nbm_wait, launch_callback, wait_callback, this );

	//
	// If nbm_wait > 0, then m_fes.run() above is non-blocking and instead
	// we wait at most nbm_wait seconds for Attract-Mode to lose focus to
	// the launched program.
	//
	// The frontend will start up again within the nbm_wait time if focus is
	// lost and returned to it for at least MAX_WAIT_ON_REGAIN_FOCUS ms.
	//
	// This mode is unfortunate and flakey, but necessary to support some
	// programs (such as steam)
	//
	const int MAX_WAIT_ON_REGAIN_FOCUS=4000;

	if ( nbm_wait > 0 )
	{
		FeDebug() << "Non-Blocking Wait Mode: nb_mode_wait=" << nbm_wait << " seconds, waiting..." << std::endl;
		bool done_wait=false, has_focus=false, in_pad=false, focus_lost=false;

		sf::Clock pad_timer;

#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 ))
		has_focus = hasFocus();
#endif

		while ( !done_wait && isOpen() )
		{
			sf::Event ev;
			while (pollEvent(ev))
			{
				if ( ev.type == sf::Event::GainedFocus )
				{
					if ( !has_focus )
						FeDebug() << "Gained focus at "
							<< timer.getElapsedTime().asMilliseconds() << "ms" << std::endl;

					has_focus = true;
				}
				else if ( ev.type == sf::Event::LostFocus )
				{
					if ( has_focus )
						FeDebug() << "Lost focus at "
							<< timer.getElapsedTime().asMilliseconds() << "ms" << std::endl;

					has_focus = false;
					focus_lost = true;
				}
				else if ( ev.type == sf::Event::Closed )
					return false;
			}

			bool in_nbm_wait = ( timer.getElapsedTime() < sf::seconds( nbm_wait ) );

			if ( !focus_lost && !in_nbm_wait )
			{
				FeDebug() << "Focus not lost, nbm_wait reached.  Stopped waiting at "
					<< timer.getElapsedTime().asMilliseconds() << "ms" << std::endl;

				done_wait = true;
			}

			if ( has_focus )
			{
				if ( !in_nbm_wait )
				{
						FeDebug() << "Focus regained. Stopped waiting at "
							<< timer.getElapsedTime().asMilliseconds() << "ms" << std::endl;

						done_wait=true;
				}
				else if ( !in_pad )
				{
					pad_timer.restart();
					in_pad = true;
				}
				else
				{
					if (in_nbm_wait && focus_lost && (pad_timer.getElapsedTime() > sf::milliseconds( MAX_WAIT_ON_REGAIN_FOCUS )))
					{
						FeDebug() << "Focus regained for MAX_WAIT_ON_REGAIN_FOCUS (" << MAX_WAIT_ON_REGAIN_FOCUS << "ms).  Stopped waiting at "
							<< timer.getElapsedTime().asMilliseconds() << "ms" << std::endl;
						done_wait=true;
					}
				}
			}
			else
				in_pad = false;

			if ( !done_wait )
				sf::sleep( sf::milliseconds( 25 ) );
		}
	}

	if ( m_fes.get_info_bool( FeSettings::TrackUsage ) )
		m_fes.update_stats( 1, timer.getElapsedTime().asSeconds() );

#if defined(SFML_SYSTEM_LINUX)
	if ( m_fes.get_window_mode() == FeSettings::Fullscreen )
	{
#if defined(USE_XLIB)
		//
		// On X11 Linux fullscreen mode we might have forcibly closed our window after launching the
		// emulator. Recreate it now if we did.
		//
		// Note that simply hiding and then showing the window again doesn't work right... focus
		// doesn't come back
		//
		if ( !isOpen() )
			initial_create();
#else
		initial_create(); // On raspberry pi, we have forcibly closed the window, so recreate it now
#endif
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

	FeDebug() << "Resuming frontend after game launch" << std::endl;

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
