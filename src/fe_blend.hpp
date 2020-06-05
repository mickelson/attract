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

#ifndef FE_BLEND_HPP
#define FE_BLEND_HPP

#include <SFML/Graphics.hpp>

class FeBlend
{
public:
	enum Mode
	{
		Alpha,
		Add,
		Subtract,
		Screen,
		Multiply,
		Overlay,
		Premultiplied,
		None
	};

	static sf::BlendMode get_blend_mode( int blend_mode );
	static sf::Shader* get_default_shader( int blend_mode );
};

#endif
