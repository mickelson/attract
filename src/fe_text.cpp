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

#include "fe_text.hpp"
#include "fe_settings.hpp"
#include "fe_util.hpp"
#include "fe_shader.hpp"
#include "fe_present.hpp"
#include <iostream>

FeText::FeText( const std::string &str, int x, int y, int w, int h )
	: FeBasePresentable(),
	m_string( str ),
	m_index_offset( 0 ),
	m_filter_offset( 0 ),
	m_user_charsize( -1 ),
	m_size( w, h ),
	m_position( x, y ),
	m_scale_factor( 1.0 )
{
}

void FeText::setFont( const sf::Font &f )
{
	m_draw_text.setFont( f );
}

const sf::Vector2f &FeText::getPosition() const
{
	return m_position;
}

void FeText::setPosition( const sf::Vector2f &p )
{
	m_position = p;
	FePresent::script_do_update( this );
}

const sf::Vector2f &FeText::getSize() const
{
	return m_size;
}

void FeText::setSize( const sf::Vector2f &s )
{
	m_size = s;
	FePresent::script_do_update( this );
}

float FeText::getRotation() const
{
	return m_draw_text.getRotation();
}

void FeText::setRotation( float r )
{
	m_draw_text.setRotation( r );
	FePresent::script_do_update( this );
}

void FeText::setColor( const sf::Color &c )
{
	m_draw_text.setColor( c );
	FePresent::script_flag_redraw();
}

const sf::Color &FeText::getColor() const
{
	return m_draw_text.getColor();
}

void FeText::setIndexOffset( int io )
{
	if ( m_index_offset != io )
	{
		m_index_offset=io;
		FePresent::script_do_update( this );
	}
}

int FeText::getIndexOffset() const
{
	return m_index_offset;
}

void FeText::setFilterOffset( int fo )
{
	if ( m_filter_offset != fo )
	{
		m_filter_offset=fo;
		FePresent::script_do_update( this );
	}
}

int FeText::getFilterOffset() const
{
	return m_filter_offset;
}

void FeText::on_new_list( FeSettings *s )
{
	int char_size = 8 * m_scale_factor;
	if ( m_user_charsize > 0 )
		char_size = m_user_charsize * m_scale_factor;
	else if ( m_size.y > 12 )
		char_size = ( m_size.y - 4 ) * m_scale_factor;

	m_draw_text.setTextScale( sf::Vector2f( 1.f / m_scale_factor, 1.f / m_scale_factor ) );
	m_draw_text.setCharacterSize( char_size );
	m_draw_text.setPosition( m_position );
	m_draw_text.setSize( m_size );
}

void FeText::on_new_selection( FeSettings *feSettings )
{
	std::string str = m_string;
	feSettings->do_text_substitutions( str, m_filter_offset, m_index_offset );

	m_draw_text.setString( str );
}

void FeText::set_scale_factor( float scale_x, float scale_y )
{
	m_scale_factor = ( scale_x > scale_y ) ? scale_x : scale_y;
	if ( m_scale_factor <= 0.f )
		m_scale_factor = 1.f;
}

void FeText::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	FeShader *s = get_shader();
	if ( s )
	{
		const sf::Shader *sh = s->get_shader();
		if ( sh )
			states.shader = sh;
	}

	target.draw( m_draw_text, states );
}

void FeText::set_word_wrap( bool w )
{
	m_draw_text.setWordWrap( w );
	FePresent::script_do_update( this );
}

bool FeText::get_word_wrap()
{
	return ( m_draw_text.getFirstLineHint() >= 0 );
}

void FeText::set_first_line_hint( int l )
{
	if ( l != m_draw_text.getFirstLineHint() )
	{
		m_draw_text.setFirstLineHint( l );
		m_draw_text.setString( m_string );

		FePresent::script_do_update( this );
	}
}

int FeText::get_first_line_hint()
{
	return m_draw_text.getFirstLineHint();
}

const char *FeText::get_string()
{
	return m_string.c_str();
}

void FeText::set_string(const char *s)
{
	m_string=s;
	FePresent::script_do_update( this );
}

int FeText::get_bgr()
{
	return m_draw_text.getBgColor().r;
}

int FeText::get_bgg()
{
	return m_draw_text.getBgColor().g;
}

int FeText::get_bgb()
{
	return m_draw_text.getBgColor().b;
}

int FeText::get_bga()
{
	return m_draw_text.getBgColor().a;
}

int FeText::get_charsize()
{
	return m_draw_text.getCharacterSize();
}

int FeText::get_style()
{
	return m_draw_text.getStyle();
}

int FeText::get_align()
{
	return (int)m_draw_text.getAlignment();
}

const char *FeText::get_font()
{
	return m_font_name.c_str();
}

void FeText::set_bgr(int r)
{
	sf::Color c=m_draw_text.getBgColor();
	c.r=r;
	m_draw_text.setBgColor(c);
	FePresent::script_flag_redraw();
}

void FeText::set_bgg(int g)
{
	sf::Color c=m_draw_text.getBgColor();
	c.g=g;
	m_draw_text.setBgColor(c);
	FePresent::script_flag_redraw();
}

void FeText::set_bgb(int b)
{
	sf::Color c=m_draw_text.getBgColor();
	c.b=b;
	m_draw_text.setBgColor(c);
	FePresent::script_flag_redraw();
}

void FeText::set_bga(int a)
{
	sf::Color c=m_draw_text.getBgColor();
	c.a=a;
	m_draw_text.setBgColor(c);
	FePresent::script_flag_redraw();
}

void FeText::set_bg_rgb(int r, int g, int b )
{
	sf::Color c=m_draw_text.getBgColor();
	c.r=r;
	c.g=g;
	c.b=b;

	if ( c.a == 0 )
		c.a = 255;

	m_draw_text.setBgColor(c);
	FePresent::script_flag_redraw();
}

void FeText::set_charsize(int s)
{
	m_user_charsize = s;
	FePresent::script_do_update( this );
}

void FeText::set_style(int s)
{
	m_draw_text.setStyle(s);
	FePresent::script_flag_redraw();
}

void FeText::set_align(int a)
{
	m_draw_text.setAlignment( (FeTextPrimative::Alignment)a);
	FePresent::script_do_update( this );
}

void FeText::set_font( const char *f )
{
	FePresent *fep = FePresent::script_get_fep();
	if ( !fep )
		return;

	const FeFontContainer *fc = fep->get_pooled_font( f );

	if ( !fc )
		return;

	const sf::Font *font=&(fc->get_font());
	if ( font )
	{
		setFont( *font );
		m_font_name = f;

		FePresent::script_do_update( this );
	}
}
