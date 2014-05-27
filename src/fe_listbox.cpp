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

#include "fe_listbox.hpp"
#include "fe_settings.hpp"
#include "fe_shader.hpp"
#include <iostream>

FeListBox::FeListBox( int x, int y, int w, int h )
	: FeBasePresentable(),
	m_selColour( sf::Color::Yellow ),
	m_selBg( sf::Color::Blue ),
	m_selStyle( sf::Text::Regular ),
	m_rows( 11 ),
	m_userCharSize( 0 ),
	m_rotation( 0.0 )
{
	m_base_text.setPosition( sf::Vector2f( x, y ) );
	m_base_text.setSize( sf::Vector2f( w, h ) );
	m_base_text.setColor( sf::Color::White );
	m_base_text.setBgColor( sf::Color::Transparent );
}

FeListBox::FeListBox(
		const sf::Font *font,
		const sf::Color &colour,
		const sf::Color &bgcolour,
		const sf::Color &selcolour,
		const sf::Color &selbgcolour,
		unsigned int charactersize,
		int rows )
	: FeBasePresentable(),
	m_base_text( font, colour, bgcolour, charactersize, FeTextPrimative::Centre ),
	m_selColour( selcolour ),
	m_selBg( selbgcolour ),
	m_selStyle( sf::Text::Regular ),
	m_rows( rows ),
	m_userCharSize( charactersize ),
	m_rotation( 0.0 )
{
}

void FeListBox::setFont( const sf::Font &f )
{
	m_base_text.setFont( f );
}

const sf::Vector2f &FeListBox::getPosition() const
{
	return m_base_text.getPosition();
}

void FeListBox::setPosition( const sf::Vector2f &p )
{
	m_base_text.setPosition( p );
	script_do_update( this );
}

const sf::Vector2f &FeListBox::getSize() const
{
	return m_base_text.getSize();
}

void FeListBox::setSize( const sf::Vector2f &s )
{
	m_base_text.setSize( s );
	script_do_update( this );
}

float FeListBox::getRotation() const
{
	return m_rotation;
}

const sf::Color &FeListBox::getColor() const
{
	return m_base_text.getColor();
}

void FeListBox::init( float scale_x, float scale_y )
{
	sf::Vector2f size = getSize();
	sf::Vector2f pos = getPosition();

	float scale_factor( ( scale_x > scale_y ) ? scale_x : scale_y );
	if ( scale_factor <= 0.f )
		scale_factor = 1.f;

	int actual_spacing = (int)size.y / m_rows;
	int char_size = 8 * scale_factor;

	// Set the character size now
	//
	if ( m_userCharSize > 0 )
		char_size = m_userCharSize * scale_factor;
	else if ( actual_spacing > 12 )
		char_size = ( actual_spacing - 4 ) * scale_factor;

	m_base_text.setTextScale( sf::Vector2f( 1.f / scale_factor, 1.f / scale_factor ) );
	m_base_text.setCharacterSize( char_size );

	m_texts.clear();
	m_texts.reserve( m_rows );

	sf::Transform rotater;
	rotater.rotate( m_rotation, pos.x, pos.y );

	for ( int i=0; i< m_rows; i++ )
	{
		FeTextPrimative t( m_base_text );
		if ( i == m_rows / 2 )
		{
			t.setColor( m_selColour );
			t.setBgColor( m_selBg );
			t.setStyle( m_selStyle );
		}

		t.setPosition( rotater.transformPoint( pos.x, pos.y+(i*actual_spacing)) );
		t.setSize( size.x, actual_spacing );
		t.setRotation( m_rotation );

		m_texts.push_back( t );
	}
}

void FeListBox::setColor( const sf::Color &c )
{
	m_base_text.setColor( c );

	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		if ( i != ( m_texts.size() / 2 ) )
			m_texts[i].setColor( c );
	}

	script_flag_redraw();
}

void FeListBox::setSelColor( const sf::Color &c )
{
	m_selColour = c;

	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		m_texts[ sel ].setColor( m_selColour );
	}
	script_flag_redraw();
}

void FeListBox::setSelBgColor( const sf::Color &c )
{
	m_selBg = c;
	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		m_texts[ sel ].setBgColor( m_selBg );
	}
	script_flag_redraw();
}

void FeListBox::setSelStyle( int s )
{
	m_selStyle = s;
	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		m_texts[ sel ].setStyle( m_selStyle );
	}
	script_flag_redraw();
}

int FeListBox::getSelStyle()
{
	return m_selStyle;
}

void FeListBox::setTextScale( const sf::Vector2f &scale )
{
	m_base_text.setTextScale( scale );

	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setTextScale( scale );
}

FeTextPrimative *FeListBox::setEditMode( bool e, sf::Color c )
{
	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		if ( e )
		{
			m_texts[ sel ].setColor( c );
			m_texts[ sel ].setOutlineColor( c );
			m_texts[ sel ].setOutlineThickness( -2 );

			return &(m_texts[ sel ]);
		}
		else
		{
			m_texts[ sel ].setColor( m_selColour );
			m_texts[ sel ].setOutlineThickness( 0 );
		}
	}
	return NULL;
}

void FeListBox::setText( const int index,
			const std::vector<std::string> &list )
{
	if ( !m_texts.empty() )
	{
		int offset = index - ( (int)m_texts.size() / 2 );

		for ( int i=0; i < (int)m_texts.size(); i++ )
		{
			int listentry = offset + i;
			if (( listentry < 0 ) || ( listentry >= (int)list.size() ))
				m_texts[i].setString("");
			else
				m_texts[i].setString( list[listentry] );
		}
	}
}

void FeListBox::setRotation( float r )
{
	m_rotation = r;

	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setRotation( m_rotation );

	script_flag_redraw();
}

void FeListBox::on_new_list( FeSettings *s, float scale_x, float scale_y )
{
	init( scale_x, scale_y );

	s->get_current_display_list( m_displayList );
	setText( s->get_rom_index(), m_displayList );
}

void FeListBox::on_new_selection( FeSettings *s )
{
	setText( s->get_rom_index(), m_displayList );
}

void FeListBox::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	FeShader *s = get_shader();
	if ( s )
	{
		const sf::Shader *sh = s->get_shader();
		if ( sh )
			states.shader = sh;
	}

	for ( std::vector<FeTextPrimative>::const_iterator itl=m_texts.begin();
				itl != m_texts.end(); ++itl )
		target.draw( (*itl), states );
}

void FeListBox::clear()
{
	m_texts.clear();
}

int FeListBox::getRowCount() const
{
	return m_texts.size();
}

void FeListBox::setIndexOffset( int io )
{
}

int FeListBox::getIndexOffset() const
{
	return 0;
}

int FeListBox::get_bgr()
{
	return m_base_text.getBgColor().r;
}

int FeListBox::get_bgg()
{
	return m_base_text.getBgColor().g;
}

int FeListBox::get_bgb()
{
	return m_base_text.getBgColor().b;
}

int FeListBox::get_bga()
{
	return m_base_text.getBgColor().a;
}

int FeListBox::get_charsize()
{
	return m_userCharSize;
}

int FeListBox::get_rows()
{
	return m_rows;
}

int FeListBox::get_style()
{
	return m_base_text.getStyle();
}

int FeListBox::get_align()
{
	return (int)m_base_text.getAlignment();
}

void FeListBox::setBgColor( const sf::Color &c )
{
	m_base_text.setBgColor(c);
	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		if ( i != ( m_texts.size() / 2 ) )
			m_texts[i].setBgColor( c );
	}
	script_flag_redraw();
}

void FeListBox::set_bgr(int r)
{
	sf::Color c=m_base_text.getBgColor();
	c.r=r;
	setBgColor( c );
}

void FeListBox::set_bgg(int g)
{
	sf::Color c=m_base_text.getBgColor();
	c.g=g;
	setBgColor( c );
}

void FeListBox::set_bgb(int b)
{
	sf::Color c=m_base_text.getBgColor();
	c.b=b;
	setBgColor( c );
}

void FeListBox::set_bga(int a)
{
	sf::Color c=m_base_text.getBgColor();
	c.a=a;
	setBgColor( c );
}

void FeListBox::set_bg_rgb(int r, int g, int b )
{
	sf::Color c=m_base_text.getBgColor();
	c.r=r;
	c.g=g;
	c.b=b;
	m_base_text.setBgColor(c);
	script_flag_redraw();
}

void FeListBox::set_charsize(int s)
{
	m_userCharSize = s;

	// We call script_do_update to trigger a call to our init() function
	// with the appropriate parameters
	//
	script_do_update( this );
}

void FeListBox::set_rows(int r)
{
	//
	// Don't allow m_rows to ever be zero or negative
	//
	if ( m_rows > 0 )
	{
		m_rows = r;
		script_flag_redraw();
	}
}

void FeListBox::set_style(int s)
{
	m_base_text.setStyle(s);
	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		if ( i != ( m_texts.size() / 2 ) )
			m_texts[i].setStyle( s );
	}
	script_flag_redraw();
}

void FeListBox::set_align(int a)
{
	m_base_text.setAlignment( (FeTextPrimative::Alignment)a);

	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setAlignment( (FeTextPrimative::Alignment)a );

	script_flag_redraw();
}

int FeListBox::get_selr()
{
	return m_selColour.r;
}

int FeListBox::get_selg()
{
	return m_selColour.g;
}

int FeListBox::get_selb()
{
	return m_selColour.b;
}

int FeListBox::get_sela()
{
	return m_selColour.a;
}

void FeListBox::set_selr(int r)
{
	m_selColour.r=r;
	setSelColor( m_selColour );
}

void FeListBox::set_selg(int g)
{
	m_selColour.g=g;
	setSelColor( m_selColour );
}

void FeListBox::set_selb(int b)
{
	m_selColour.b=b;
	setSelColor( m_selColour );
}

void FeListBox::set_sela(int a)
{
	m_selColour.a=a;
	setSelColor( m_selColour );
}

void FeListBox::set_sel_rgb(int r, int g, int b )
{
	m_selColour.r = r;
	m_selColour.g = g;
	m_selColour.b = b;
	setSelColor( m_selColour );
}

int FeListBox::get_selbgr()
{
	return m_selBg.r;
}

int FeListBox::get_selbgg()
{
	return m_selBg.g;
}

int FeListBox::get_selbgb()
{
	return m_selBg.b;
}

int FeListBox::get_selbga()
{
	return m_selBg.a;
}

const char *FeListBox::get_font()
{
	return m_font_name.c_str();
}

void FeListBox::set_selbgr(int r)
{
	m_selBg.r=r;
	setSelBgColor( m_selBg );
}

void FeListBox::set_selbgg(int g)
{
	m_selBg.g=g;
	setSelBgColor( m_selBg );
}

void FeListBox::set_selbgb(int b)
{
	m_selBg.b=b;
	setSelBgColor( m_selBg );
}

void FeListBox::set_selbga(int a)
{
	m_selBg.a=a;
	setSelBgColor( m_selBg );
}

void FeListBox::set_selbg_rgb(int r, int g, int b )
{
	m_selBg.r = r;
	m_selBg.g = g;
	m_selBg.b = b;
	setSelBgColor( m_selBg );
}

void FeListBox::set_font( const char *f )
{
	const sf::Font *font = script_get_font( f );

	if ( font )
	{
		setFont( *font );
		m_font_name = f;

		script_flag_redraw();
	}
}