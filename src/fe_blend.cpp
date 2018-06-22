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

#include "fe_blend.hpp"
#include <SFML/Graphics.hpp>

sf::BlendMode FeBlend::get_blend_mode( int b )
{
	switch( b )
	{
		case FeBlend::Alpha:
			return sf::BlendAlpha;
		case FeBlend::Add:
			return sf::BlendAdd;
		case FeBlend::Screen:
			return sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcColor);
		case FeBlend::Multiply:
			return sf::BlendMode(sf::BlendMode::DstColor, sf::BlendMode::OneMinusSrcAlpha);
		case FeBlend::Overlay:
			return sf::BlendMode(sf::BlendMode::DstColor, sf::BlendMode::SrcColor);
		case FeBlend::Premultiplied:
			return sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha);
		case FeBlend::None:
			return sf::BlendNone;
	}
}