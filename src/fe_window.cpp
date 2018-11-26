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
#include "fe_present.hpp"

#ifdef SFML_SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // SFML_SYSTEM_WINDOWS

#ifdef SFML_SYSTEM_MACOS
#include "fe_util_osx.hpp"
#endif // SFM_SYSTEM_MACOS

#include <iostream>
#include "nowide/fstream.hpp"

#include <SFML/System/Sleep.hpp>

FeWindowPosition::FeWindowPosition( const sf::Vector2i &pos, const sf::Vector2u &size )
	: m_pos( pos ),
	m_size( size )
{
}

int FeWindowPosition::process_setting( const std::string &setting,
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

void FeWindowPosition::save( const std::string &filename )
{
	nowide::ofstream outfile( filename.c_str() );
	if ( outfile.is_open() )
	{
		outfile << "position " << m_pos.x << "," << m_pos.y << std::endl;
		outfile << "size " << m_size.x << "," << m_size.y << std::endl;
	}
	outfile.close();
}

bool is_multimon_config( FeSettings &fes )
{
	return (( fes.get_info_bool( FeSettings::MultiMon ) )
		&& ( !is_windowed_mode( fes.get_window_mode() ) ));
}

const char *FeWindowPosition::FILENAME = "window.am";

FeWindow::FeWindow( FeSettings &fes )
	: m_fes( fes ),
	m_running_pid( 0 ),
	m_running_wnd( NULL )
{
}

FeWindow::~FeWindow()
{
	if ( m_running_pid && process_exists( m_running_pid ) )
		kill_program( m_running_pid );
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
	if ( is_multimon_config( m_fes ) )
	{
		// Note that as of 2.1, SFML caches the window size internally
		setPosition( sf::Vector2i(
			GetSystemMetrics( SM_XVIRTUALSCREEN ),
			GetSystemMetrics( SM_YVIRTUALSCREEN ) ) );

		setSize( sf::Vector2u(
			GetSystemMetrics( SM_CXVIRTUALSCREEN ),
			GetSystemMetrics( SM_CYVIRTUALSCREEN ) ) );
	}

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
		is_multimon_config( m_fes ),
		x, y, width, height );

	// Note that as of 2.1, SFML caches the window size internally
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

	m_win_mode = m_fes.get_window_mode();

#ifdef USE_XINERAMA
	if ( is_multimon_config( m_fes ) && ( m_win_mode != FeSettings::Default ))
		FeLog() << " ! NOTE: Use the 'Fill Screen' window mode if you want multiple monitor support to function correctly" << std::endl;
#endif

	// Create window
	FeDebug() << "Creating Attract-Mode window" << std::endl;

	FeWindowPosition win_pos(
		sf::Vector2i( 0, 0 ),
		sf::Vector2u( 480, 320 ) );

	win_pos.load_from_file( m_fes.get_config_dir() + FeWindowPosition::FILENAME );
	sf::VideoMode video_mode;

	if ( is_windowed_mode( m_win_mode ))
		video_mode = sf::VideoMode( win_pos.m_size.x, win_pos.m_size.y, sf::VideoMode::getDesktopMode().bitsPerPixel );
	else 
		video_mode = sf::VideoMode::getDesktopMode();

	create(
		video_mode,
		"Attract-Mode",
		style_map[ m_win_mode ] );

	if ( is_windowed_mode( m_win_mode ) )
	{
		setPosition( win_pos.m_pos );
		setSize( win_pos.m_size );
	}
#ifdef SFML_SYSTEM_MACOS
	else if ( m_win_mode == FeSettings::Default )
	{
		osx_hide_menu_bar();
		setPosition( sf::Vector2i( 0, 0 ) );
	}
#endif

#ifdef SFML_SYSTEM_WINDOWS
	SetForegroundWindow( getSystemHandle() );
	LockSetForegroundWindow( LSFW_LOCK );
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
		//
		// On X11 Linux, fullscreen mode is confirmed to block the emulator
		// from running on some systems...
		//
#if defined(USE_XLIB)
		sf::sleep( sf::milliseconds( 1000 ) );
#endif
		FeDebug() << "Closing Attract-Mode window" << std::endl;
		win->close(); // this fixes raspi version (w/sfml-pi) obscuring daphne (and others?)
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
		if ( !is_multimon_config( win->m_fes ) )
		{
			win->clear();
			win->display();
		}
	}
}

bool FeWindow::run()
{
	sf::Vector2i reset_pos;

	if ( m_fes.get_info_bool( FeSettings::MoveMouseOnLaunch ) )
	{
		// Move the mouse to the bottom right corner so it isn't visible
		// when the emulator launches.
		//
		reset_pos = sf::Mouse::getPosition();

		sf::Vector2i hide_pos = getPosition();
		hide_pos.x += getSize().x - 1;
		hide_pos.y += getSize().y - 1;

		sf::Mouse::setPosition( hide_pos );
	}

	sf::Clock timer;

	// We need to get this variable before calling m_fes.prep_for_launch(),
	// which goes and resets the last launch tracking to the current selection
	bool is_relaunch = m_fes.is_last_launch( 0, 0 );

	std::string command, args, work_dir;
	FeEmulatorInfo *emu = NULL;
	m_fes.prep_for_launch( command, args, work_dir, emu );
	FePresent::script_process_magic_strings( args, 0, 0 );

	if ( !emu )
	{
		FeLog() << "Error getting emulator info for launch" << std::endl;
		return true;
	}

	// non-blocking mode wait time (in seconds)
	int nbm_wait = as_int( emu->get_info( FeEmulatorInfo::NBM_wait ) );

	run_program_options_class opt;
	opt.exit_hotkey = emu->get_info( FeEmulatorInfo::Exit_hotkey );
	opt.pause_hotkey = emu->get_info( FeEmulatorInfo::Pause_hotkey );
	opt.joy_thresh = m_fes.get_joy_thresh();
	opt.launch_cb = (( nbm_wait <= 0 ) ? launch_callback : NULL );
	opt.wait_cb = wait_callback;
	opt.launch_opaque = this;

	bool have_paused_prog = m_running_pid && process_exists( m_running_pid );

#if defined(SFML_SYSTEM_WINDOWS)
	LockSetForegroundWindow( LSFW_UNLOCK );
#endif

	// check if we need to resume a previously paused game
	if ( have_paused_prog && is_relaunch )
	{
		FeLog() << "*** Resuming previously paused program, pid: " << m_running_pid << std::endl;
		resume_program( m_running_pid, m_running_wnd, &opt );
	}
	else
	{
		if ( have_paused_prog )
		{
			FeLog() << "*** Killing previously paused program, pid: " << m_running_pid << std::endl;
			kill_program( m_running_pid );

			m_running_pid = 0;
			m_running_wnd = NULL;
		}

		if ( !work_dir.empty() )
			FeLog() << " - Working directory: " << work_dir << std::endl;

		FeLog() << "*** Running: " << command << " " << args << std::endl;

		run_program(
			command,
			args,
			work_dir,
			NULL,
			NULL,
			( nbm_wait <= 0 ), // don't block if nbm_wait > 0
			&opt );
	}

	if ( opt.running_pid != 0 )
	{
		// User has paused the progam and it is still running in the background
		// (Note that this only can happen when run_program is blocking (i.e. nbm_wait <= 0)
		m_running_pid = opt.running_pid;
		m_running_wnd = opt.running_wnd;
	}
	else
	{
		m_running_pid = 0;
		m_running_wnd = NULL;
	}

	//
	// If nbm_wait > 0, then m_fes.run() above is non-blocking and we need
	// to wait at most nbm_wait seconds for Attract-Mode to lose focus to
	// the launched program.  If it loses focus, we continue waiting until
	// focus returns to Attract-Mode
	//
	if ( nbm_wait > 0 )
	{
		FeDebug() << "Non-Blocking Wait Mode: nb_mode_wait=" << nbm_wait << " seconds, waiting..." << std::endl;
		bool done_wait=false, has_focus=false;

		while ( !done_wait && isOpen() )
		{
			sf::Event ev;

#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 ))
			while (pollEvent(ev))
			{
				if ( ev.type == sf::Event::Closed )
					return false;
			}

			has_focus = hasFocus();
#else
			//
			// flakey pre-SFML 2.2 implementation
			// to be removed if SFML 2.0/2.1 support is ever dropped
			//
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
				}
				else if ( ev.type == sf::Event::Closed )
					return false;
			}
#endif

			if (( timer.getElapsedTime() >= sf::seconds( nbm_wait ) )
				&& ( has_focus ))
			{
				FeDebug() << "Attract-Mode has focus, stopped non-blocking wait after "
					<< timer.getElapsedTime().asSeconds() << "s" << std::endl;

				done_wait = true;
			}
			else
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
 #if defined(USE_XLIB)
	set_x11_foreground_window( getSystemHandle() );
 #endif

#elif defined(SFML_SYSTEM_MACOS)
	osx_take_focus();
#elif defined(SFML_SYSTEM_WINDOWS)
	SetForegroundWindow( getSystemHandle() );
	LockSetForegroundWindow( LSFW_LOCK );
#endif

	if ( m_fes.get_info_bool( FeSettings::MoveMouseOnLaunch ) )
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
	if ( is_windowed_mode( m_win_mode ) )
	{
		FeWindowPosition win_pos( getPosition(), getSize() );
		win_pos.save( m_fes.get_config_dir() + FeWindowPosition::FILENAME );
	}
}

bool FeWindow::has_running_process()
{
	return ( m_running_pid != 0 );
}
