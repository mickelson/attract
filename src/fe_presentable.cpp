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

FeBasePresentable::FeBasePresentable( FePresentableParent &p )
	: m_parent( p ),
	m_shader( NULL ),
	m_visible( true ),
	m_zorder( 0 )
{
}

FeBasePresentable::~FeBasePresentable()
{
}

FePresentableParent::FePresentableParent( )
	: m_nesting_level ( 0 )
{
}

FePresentableParent::~FePresentableParent()
{
}

void FeBasePresentable::on_new_selection( FeSettings * )
{
}

void FeBasePresentable::on_new_list( FeSettings * )
{
}

void FeBasePresentable::set_scale_factor( float, float )
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
	if ( v != m_visible )
	{
		m_visible = v;
		FePresent::script_flag_redraw();
	}
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
		FePresent *fep = FePresent::script_get_fep();
		return fep->get_empty_shader();
	}
}

void FeBasePresentable::script_set_shader( FeShader *sh )
{
	m_shader = sh;
}

int FeBasePresentable::get_zorder()
{
	return m_zorder;
}

namespace
{
	bool zcompare( FeBasePresentable *one, FeBasePresentable *two )
	{
		return ( one->get_zorder() < two->get_zorder() );
	}
};

void FeBasePresentable::set_zorder( int pos )
{
	if ( pos == m_zorder )
		return;

	m_zorder = pos;

	std::stable_sort( m_parent.elements.begin(), m_parent.elements.end(), zcompare );
	FePresent::script_flag_redraw();
}

int FePresentableParent::get_nesting_level()
{
	return m_nesting_level;
}

void FePresentableParent::set_nesting_level( int p )
{
	m_nesting_level = p;
}

FeImage *FePresentableParent::add_image(const char *n, int x, int y, int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_image( false, n, x, y, w, h, *this );

	return NULL;
}

FeImage *FePresentableParent::add_image(const char *n, int x, int y )
{
	return add_image( n, x, y, 0, 0 );
}

FeImage *FePresentableParent::add_image(const char *n )
{
	return add_image( n, 0, 0, 0, 0 );
}

FeImage *FePresentableParent::add_artwork(const char *l, int x, int y, int w, int h )
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_image( true, l, x, y, w, h, *this );

	return NULL;
}

FeImage *FePresentableParent::add_artwork(const char *l, int x, int y)
{
	return add_artwork( l, x, y, 0, 0 );
}

FeImage *FePresentableParent::add_artwork(const char *l )
{
	return add_artwork( l, 0, 0, 0, 0 );
}

FeImage *FePresentableParent::add_clone(FeImage *i )
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_clone( i, *this );

	return NULL;
}

FeText *FePresentableParent::add_text(const char *t, int x, int y, int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_text( t, x, y, w, h, *this );

	return NULL;
}

FeListBox *FePresentableParent::add_listbox(int x, int y, int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_listbox( x, y, w, h, *this );

	return NULL;
}

FeImage *FePresentableParent::add_surface(int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_surface( w, h, *this );

	return NULL;
}
