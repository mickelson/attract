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

#include "fe_presentable.hpp"
#include "fe_present.hpp"

FeBasePresentable::FeBasePresentable()
	: m_shader( NULL ),
	m_visible( true )
{
}

FeBasePresentable::~FeBasePresentable()
{
}

void FeBasePresentable::on_new_selection( FeSettings * )
{
}

void FeBasePresentable::on_new_list( FeSettings *, float, float )
{
}

float FeBasePresentable::get_x() const
{
	return getPosition().x;
}

float FeBasePresentable::get_y() const
{
	return getPosition().y;
}

void FeBasePresentable::set_x( float x )
{
	setPosition( sf::Vector2f( x, get_y() ));
}

void FeBasePresentable::set_y( float y )
{
	setPosition( sf::Vector2f( get_x(), y ));
}

float FeBasePresentable::get_width() const
{
	return getSize().x;
}

float FeBasePresentable::get_height() const
{
	return getSize().y;
}

void FeBasePresentable::set_width( float w )
{
	setSize( sf::Vector2f( w, get_height() ));
}

void FeBasePresentable::set_height( float h )
{
	setSize( sf::Vector2f( get_width(), h ) );
}

void FeBasePresentable::set_pos(float x, float y)
{
	setPosition( sf::Vector2f( x, y ) );
}

void FeBasePresentable::set_pos(float x, float y, float w, float h)
{
	setPosition( sf::Vector2f( x, y ) );
	setSize( sf::Vector2f( w, h ) );
}

int FeBasePresentable::get_r() const
{
	return getColor().r;
}

int FeBasePresentable::get_g() const
{
	return getColor().g;
}

int FeBasePresentable::get_b() const
{
	return getColor().b;
}

int FeBasePresentable::get_a() const
{
	return getColor().a;
}

void FeBasePresentable::set_r(int r)
{
	sf::Color c=getColor();
	c.r=r;
	setColor(c);
}

void FeBasePresentable::set_g(int g)
{
	sf::Color c=getColor();
	c.g=g;
	setColor(c);
}

void FeBasePresentable::set_b(int b)
{
	sf::Color c=getColor();
	c.b=b;
	setColor(c);
}

void FeBasePresentable::set_a(int a)
{
	sf::Color c=getColor();
	c.a=a;
	setColor(c);
}

void FeBasePresentable::set_rgb(int r, int g, int b)
{
	sf::Color c=getColor();
	c.r=r;
	c.g=g;
	c.b=b;
	setColor(c);
}

bool FeBasePresentable::get_visible() const
{
	return m_visible;
}

void FeBasePresentable::set_visible( bool v )
{
	m_visible = v;
	FeVM::script_flag_redraw();
}

FeShader *FeBasePresentable::get_shader() const
{
	return m_shader;
}

FeShader *FeBasePresentable::script_get_shader() const
{
	if ( m_shader )
		return m_shader;
	else
	{
		FePresent *fep = FeVM::script_get_fep();
		return fep->get_empty_shader();
	}
}

void FeBasePresentable::script_set_shader( FeShader *sh )
{
	m_shader = sh;
}