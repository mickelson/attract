/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2015 Andrew Mickelson
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

#ifndef SWF_HPP
#define SWF_HPP

#include <SFML/Graphics.hpp>

struct FeSwfState;

class FeSwf
{
public:
	FeSwf();
	~FeSwf();

	bool open_from_file( const std::string &filename );

	bool open_from_archive( const std::string &archive,
		const std::string &filename );

	bool tick();

	const sf::Vector2u get_size() const;
	const sf::Texture &get_texture() const;

	void set_smooth( bool s );

private:
	FeSwf( FeSwf & );
	FeSwf &operator=( FeSwf & );
	bool do_frame( bool is_tick );

	FeSwfState *m_imp;
	sf::RenderTexture m_texture;
	sf::Context m_context;
};

#endif
