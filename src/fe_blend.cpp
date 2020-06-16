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
#include "fe_util.hpp" // for FE_VERSION_INT macro

namespace
{
	const char *DEFAULT_SHADER_GLSL_MULTIPLIED = \
		"uniform sampler2D texture;" \
		"void main(){" \
		"vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);" \
		"gl_FragColor = gl_Color * pixel;" \
		"gl_FragColor.xyz *= gl_FragColor.w;}";

	const char *DEFAULT_SHADER_GLSL_OVERLAY = \
		"uniform sampler2D texture;" \
		"void main(){" \
		"vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);" \
		"gl_FragColor = gl_Color * pixel;" \
		"gl_FragColor = mix(vec4(0.5,0.5,0.5,1.0), gl_FragColor, gl_FragColor.w);}";

	const char *DEFAULT_SHADER_GLSL_PREMULTIPLIED = \
		"uniform sampler2D texture;" \
		"void main(){" \
		"vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);" \
		"gl_FragColor = gl_Color * pixel;" \
		"gl_FragColor.xyz *= gl_Color.w * sign(pixel.w);}";

	sf::Shader *default_shader_multiplied=NULL;
	sf::Shader *default_shader_overlay=NULL;
	sf::Shader *default_shader_premultiplied=NULL;
};

sf::BlendMode FeBlend::get_blend_mode( int blend_mode )
{
	switch( blend_mode )
	{
		case FeBlend::Add:
			return sf::BlendAdd;

#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 4, 0 ))
		case FeBlend::Subtract:
			return sf::BlendMode(sf::BlendMode::SrcAlpha, sf::BlendMode::One, sf::BlendMode::ReverseSubtract,
								 sf::BlendMode::One, sf::BlendMode::One, sf::BlendMode::ReverseSubtract);
#endif

#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 ))
		case FeBlend::Screen:
			return sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcColor);
		case FeBlend::Multiply:
			return sf::BlendMode(sf::BlendMode::DstColor, sf::BlendMode::OneMinusSrcAlpha);
		case FeBlend::Overlay:
			return sf::BlendMode(sf::BlendMode::DstColor, sf::BlendMode::SrcColor);
		case FeBlend::Premultiplied:
			return sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha);
#endif

		case FeBlend::None:
			return sf::BlendNone;

		case FeBlend::Alpha:
		default:
			return sf::BlendAlpha;
	}
}

sf::Shader* FeBlend::get_default_shader( int blend_mode )
{
	if ( !sf::Shader::isAvailable() )
		return NULL;

	switch( blend_mode )
	{
		case FeBlend::Alpha:
		case FeBlend::Add:
		case FeBlend::Subtract:
		case FeBlend::None:
			return NULL;
		case FeBlend::Screen:
		case FeBlend::Multiply:
			if ( !default_shader_multiplied )
			{
				default_shader_multiplied = new sf::Shader();
				default_shader_multiplied->loadFromMemory( DEFAULT_SHADER_GLSL_MULTIPLIED, sf::Shader::Fragment );
			}
			return default_shader_multiplied;
		case FeBlend::Overlay:
			if ( !default_shader_overlay )
			{
				default_shader_overlay = new sf::Shader();
				default_shader_overlay->loadFromMemory( DEFAULT_SHADER_GLSL_OVERLAY, sf::Shader::Fragment );
			}
			return default_shader_overlay;
		case FeBlend::Premultiplied:
			if ( !default_shader_premultiplied )
			{
				default_shader_premultiplied = new sf::Shader();
				default_shader_premultiplied->loadFromMemory( DEFAULT_SHADER_GLSL_PREMULTIPLIED, sf::Shader::Fragment );
			}
			return default_shader_premultiplied;
		default:
			return NULL;
	}
}

void FeBlend::clear_default_shaders()
{
	if ( default_shader_multiplied )
	{
		delete default_shader_multiplied;
		default_shader_multiplied = NULL;
	}

	if ( default_shader_overlay )
	{
		delete default_shader_overlay;
		default_shader_overlay = NULL;
	}

	if ( default_shader_premultiplied )
	{
		delete default_shader_premultiplied;
		default_shader_premultiplied = NULL;
	}
}
