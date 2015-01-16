/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2014 Andrew Mickelson
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

#include "fe_shader.hpp"
#include "fe_presentable.hpp"
#include "fe_image.hpp"
#include "fe_present.hpp"
#include <iostream>

FeShader::FeShader( Type t, const std::string &vert_shader, const std::string &frag_shader )
	: m_type( Empty )
{
	if ( !sf::Shader::isAvailable() || ( t == Empty ) ) // silent fail if shaders aren't available
		return;

	if ( !vert_shader.empty() && !frag_shader.empty() )
	{
		if ( !m_shader.loadFromFile( vert_shader, frag_shader ) )
		{
			std::cerr << "Error loading vertex and fragment shaders: vertex=" << vert_shader << ", fragment="
				<< frag_shader << std::endl;
			return;
		}

		m_type = VertexAndFragment;
	}
	else if ( !vert_shader.empty() )
	{
		if ( !m_shader.loadFromFile( vert_shader, sf::Shader::Vertex ) )
		{
			std::cerr << "Error loading vertex shader: " << vert_shader << std::endl;
			return;
		}

		m_type = Vertex;
	}
	else if ( !frag_shader.empty() )
	{
		if ( !m_shader.loadFromFile( frag_shader, sf::Shader::Fragment ) )
		{
			std::cerr << "Error loading fragment shader: " << frag_shader << std::endl;
			return;
		}

		m_type = Fragment;
	}
}

void FeShader::set_param( const char *name, float x )
{
	if ( m_type != Empty )
	{
		m_shader.setParameter( name, x );
		FePresent::script_flag_redraw();
	}
}

void FeShader::set_param( const char *name, float x, float y )
{
	if ( m_type != Empty )
	{
		m_shader.setParameter( name, x, y );
		FePresent::script_flag_redraw();
	}
}

void FeShader::set_param( const char *name, float x, float y, float z )
{
	if ( m_type != Empty )
	{
		m_shader.setParameter( name, x, y, z );
		FePresent::script_flag_redraw();
	}
}

void FeShader::set_param( const char *name, float x, float y, float z, float w )
{
	if ( m_type != Empty )
	{
		m_shader.setParameter( name, x, y, z, w );
		FePresent::script_flag_redraw();
	}
}

void FeShader::set_texture_param( const char *name )
{
	if ( m_type != Empty )
	{
		m_shader.setParameter( name, sf::Shader::CurrentTexture );
		FePresent::script_flag_redraw();
	}
}

void FeShader::set_texture_param( const char *name, FeImage *image )
{
	if (( m_type != Empty ) && ( image ))
	{
		const sf::Texture *texture = image->get_texture();

		if ( texture )
		{
			m_shader.setParameter( name, *texture );
			FePresent::script_flag_redraw();
		}
	}
}
