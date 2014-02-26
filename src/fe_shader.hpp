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

#ifndef FE_SHADER_HPP
#define FE_SHADER_HPP

#include <SFML/Graphics/Shader.hpp>
class FeImage;

class FeShader
{
public:
	enum Type
	{
		Empty,
		VertexAndFragment,
		Vertex,
		Fragment
	};

	FeShader( Type t, const std::string &vert_shader, const std::string &frag_shader );

	void set_param( const char *name, float x );
	void set_param( const char *name, float x, float y );
	void set_param( const char *name, float x, float y, float z );
	void set_param( const char *name, float x, float y, float z, float w );
	void set_texture_param( const char *name );
	void set_texture_param( const char *name, FeImage *image );

	const sf::Shader *get_shader() const { return ( m_type != Empty ) ? &m_shader : NULL; };
	Type get_type() const { return m_type; };

private:
	Type m_type;
	sf::Shader m_shader;
};

#endif
