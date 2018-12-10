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

class FeWindowPosition : public FeBaseConfigurable
{
public:
	sf::Vector2i m_pos;
	sf::Vector2u m_size;

	static const char *FILENAME;

	FeWindowPosition( const sf::Vector2i &pos, const sf::Vector2u &size );

	int process_setting( const std::string &setting,
		const std::string &value,
		const std::string &filename );

	void save( const std::string &filename );
};

class FeWindow : public sf::RenderWindow
{
	friend void launch_callback( void *o );
	friend void wait_callback( void *o );

private:
	int m_win_mode;

protected:
	FeSettings &m_fes;
	unsigned int m_running_pid;
	void *m_running_wnd;

	// override from base class
	void onCreate();

public:
	FeWindow( FeSettings &fes );
	~FeWindow();

	void initial_create();		// first time window creation
	bool run();						// run the currently selected game (blocking). returns false if window closed in the interim
	void on_exit();				// called before exiting frontend

	// return true if there is another process (i.e. a paused emulator) that we are currently running
	bool has_running_process();
};

#endif
