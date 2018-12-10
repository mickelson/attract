/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-2018 Andrew Mickelson
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
		nowide::ofstream outfile( filename.c_str() );
		if ( outfile.is_open() )
		{
			outfile << "position " << m_pos.x << "," << m_pos.y << std::endl;
			outfile << "size " << m_size.x << "," << m_size.y << std::endl;
		}
		outfile.close();
	}
};

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

	sf::VideoMode vm = sf::VideoMode::getDesktopMode(); // width/height/bpp of OpenGL surface to create

	sf::Vector2i wpos( 0, 0 );  // position to set window to

	bool do_multimon = is_multimon_config( m_fes );
	int win_mode = m_fes.get_window_mode();

#if defined(USE_XLIB)

	if ( !do_multimon && ( win_mode != FeSettings::Fullscreen ))
	{
		// If we aren't doing multimonitor mode (it isn't configured or we are in a window)
		// then use the primary screen size as our OpenGL surface size and 'fillscreen' window
		// size.
		//
		// We don't do this on "Fullscreen Mode", which has to be set to a valid videomode
		// returned by SFML.

		// Known issue: Linux Mint 18.3 Cinnamon w/ SFML 2.5.1, w/ fullscreen and multimon disabled:
		// SFML fullscreen is extended across all monitors but positioned incorrectly
		// (it is positioned to accomodate window decoration that isn't there).  setPosition()
		// doesn't work
		//
		get_x11_primary_screen_size( vm.width, vm.height );
	}
	else
	{
		// In testing on Linux Mint Cinnamon 18.3 w/ SFML 2.5.1, this call to get_x11_multimon_geometry()
		// isn't needed and multimon works without any further repositioning of our window.  I'm
		// keeping it though because it has been needed historically (earlier versions of SFML,
		// other window managers etc) and it seems to lead to the same results
		//
		get_x11_multimon_geometry( wpos.x, wpos.y, vm.width, vm.height );
	}

#elif defined(SFML_SYSTEM_WINDOWS)

	//
	// Windows General Notes:
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

	//
	//
	if ( do_multimon
			&& ( win_mode == FeSettings::Fullscreen )
			&& ( GetSystemMetrics( SM_CMONITORS ) > 1 ) )
	{
		//
		// Tested on Windows 10 w/ SFML 2.5.1 - SFML seems to be forcing the window to being the primary
		// monitor size notwithstanding that we tell it to use bigger (i.e. full multimon desktop) dimensions.
		//
		// As a workaround we force 'Fill Screen' mode here, since the user seems to want multimon to work and we
		// have detected that multiple monitors are available
		//
		FeLog() << " ! NOTE: Switching to 'Fill Screen' window mode (required for multiple monitor support)." << std::endl;
		win_mode = FeSettings::Default;
	}

	// Cover all available monitors with our window in multimonitor config
	//
	if ( do_multimon )
	{
		wpos.x = GetSystemMetrics( SM_XVIRTUALSCREEN );
		wpos.y = GetSystemMetrics( SM_YVIRTUALSCREEN );

		vm.width = GetSystemMetrics( SM_CXVIRTUALSCREEN );
		vm.height = GetSystemMetrics( SM_CYVIRTUALSCREEN );
	}

	// Some Windows users are reporting emulators hanging/failing to get focus when launched
	// from 'fullscreen' (fullscreen mode, fillscreen where window dimensions = screen dimensions)
	// They also report that the same emulator does work when Attract-Mode is in one of its windowed
	// modes.
	//
	// We work around this issue for these users by having the default "fillscreen" mode actually
	// extend 1 pixel offscreen in each direction (-1, -1, scr_width+2, scr_height+2).  The expectation
	// is that this will prevent Windows from giving the frontend window the "fullscreen mode" treatment
	// which seems to be the cause of this issue.  This is actually the same behaviour that earlier
	// versions of Attract-Mode had (first by design, then by accident).
	//
	if ( win_mode == FeSettings::Default )
	{
		wpos.x -= 1;
		wpos.y -= 1;
		vm.width += 2;
		vm.height += 2;
	}
#endif

	//
	// Create window
	//
	create( vm, "Attract-Mode", style_map[ win_mode ] );

	//
	// Set Size and position of window in window manager
	//
	sf::Vector2u wsize( vm.width, vm.height ); // default wsize = OpenGL surface size

	if ( is_windowed_mode( win_mode ) )
	{
		FeWindowPosition win_pos(
			sf::Vector2i( 0, 0 ),
			sf::Vector2u( 480, 320 ) );

		win_pos.load_from_file( m_fes.get_config_dir() + FeWindowPosition::FILENAME );

		wpos = win_pos.m_pos;
		wsize = win_pos.m_size;
	}

#ifdef SFML_SYSTEM_MACOS
	if ( win_mode == FeSettings::Default )
	{
		// note ordering req: pretty sure this needs to be before the setPosition() call below
		osx_hide_menu_bar();
	}
#endif

	// Known issue: Linux Mint 18.3 Cinnamon w/ SFML 2.5.1, position isn't being set
	// (Window always winds up at 0,0)
	setPosition( wpos );
	setSize( wsize );

	FeDebug() << "Created Attract-Mode Window: " << wsize.x << "x" << wsize.y << " @ "
		<< wpos.x << "," << wpos.y << " [OpenGL surface: "
		<< vm.width << "x" << vm.height << " bpp=" << vm.bitsPerPixel << "]" << std::endl;

#ifdef SFML_SYSTEM_WINDOWS
	SetForegroundWindow( getSystemHandle() );
	LockSetForegroundWindow( LSFW_LOCK );
#endif

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
		// Don't clear if Multimonitor is enabled
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
	if ( is_windowed_mode( m_fes.get_window_mode() ) )
	{
		FeWindowPosition win_pos( getPosition(), getSize() );
		win_pos.save( m_fes.get_config_dir() + FeWindowPosition::FILENAME );
	}
}

bool FeWindow::has_running_process()
{
	return ( m_running_pid != 0 );
}
