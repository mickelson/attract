/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-2014 Andrew Mickelson
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

#ifndef FE_WINDOW_HPP
#define FE_WINDOW_HPP

#include <SFML/Graphics/RenderWindow.hpp>

class FeSettings;

class FeWindow : public sf::RenderWindow
{
	friend void launch_callback( void *o );
	friend void wait_callback( void *o );

protected:
	FeSettings &m_fes;
	unsigned int m_running_pid;
	void *m_running_wnd;

	// override from base class
	void onCreate();

private:
#if defined(SFML_SYSTEM_WINDOWS)
	sf::RenderWindow m_blackout;
#endif
	int m_win_mode;

public:
	FeWindow( FeSettings &fes );
	~FeWindow();

	void initial_create();		// first time window creation
	bool run();						// run the currently selected game (blocking). returns false if window closed in the interim
	void on_exit();				// called before exiting frontend

	// return true if there is another process (i.e. a paused emulator) that we are currently running
	bool has_running_process();
	
	// override from base class
	void display();
};

#endif
