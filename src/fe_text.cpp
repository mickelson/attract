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
#include <iostream>

FeBaseText::FeBaseText()
{
}

FeBaseText::FeBaseText(
		const sf::Font *font,
		const sf::Color &colour,
		const sf::Color &bgcolour,
		unsigned int charactersize,
		FeTextPrimative::Alignment align )
	: m_base_text( font, colour, bgcolour, charactersize, align )
{
}

const sf::Font *FeBaseText::getFont() const
{
	return m_base_text.getFont();
}

void FeBaseText::setFont( const sf::Font &f )
{
	m_base_text.setFont( f );
}

const sf::Vector2f &FeBaseText::getPosition() const
{
	return m_base_text.getPosition();
}

void FeBaseText::setPosition( const sf::Vector2f &p )
{
	m_base_text.setPosition( p );
	script_flag_redraw();
}

const sf::Vector2f &FeBaseText::getSize() const
{
	return m_base_text.getSize();
}

void FeBaseText::setSize( const sf::Vector2f &s )
{
	m_base_text.setSize( s );
	script_flag_redraw();
}

float FeBaseText::getRotation() const
{
	return m_base_text.getRotation();
}

void FeBaseText::setRotation( float r )
{
	m_base_text.setRotation( r );
	script_flag_redraw();
}

const sf::Color &FeBaseText::getColor() const
{
	return m_base_text.getColor();
}

void FeBaseText::setColor( const sf::Color &c )
{
	m_base_text.setColor( c );
}

unsigned int FeBaseText::getCharacterSize() const
{
	return m_base_text.getCharacterSize();
}

FeText::FeText( const std::string &str )
	:  m_string( str ), 
	m_index_offset( 0 )
{
}

void FeText::setIndexOffset( int io )
{
	m_index_offset=io;
	script_do_update( this );
}

int FeText::getIndexOffset() const
{
	return m_index_offset;
}

void FeText::on_new_selection( FeSettings *feSettings )
{
	//
	// Perform substitutions of the [XXX] sequences occurring in m_string
	//
	size_t n = std::count( m_string.begin(), m_string.end(), '[' );

	std::string str = m_string;
	for ( int i=0; ((i< FeRomInfo::LAST_INDEX) && ( n > 0 )); i++ )
	{
		std::string from = "[";
		from += FeRomInfo::indexStrings[i];
		from += "]";

		n -= perform_substitution( str, from,
				feSettings->get_rom_info( m_index_offset, (FeRomInfo::Index)i) );
	}

	if ( n > 0 )
	{
		n -= perform_substitution( str, "[list_title]",
				feSettings->get_current_list_title() );
	}

	m_base_text.setString( str );
}

void FeText::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	target.draw( m_base_text, states );
}


const char *FeText::get_string()
{
	return m_string.c_str();
}

void FeText::set_string(const char *s)
{
	m_string=s;
	script_flag_redraw();
}

int FeText::get_bgr()
{
	return m_base_text.getBgColor().r;
}

int FeText::get_bgg()
{
	return m_base_text.getBgColor().g;
}

int FeText::get_bgb()
{
	return m_base_text.getBgColor().b;
}

int FeText::get_bga()
{
	return m_base_text.getBgColor().a;
}

int FeText::get_charsize()
{
	return m_base_text.getCharacterSize();
}

int FeText::get_style()
{
	return m_base_text.getStyle();
}

int FeText::get_align()
{
	return (int)m_base_text.getAlignment();
}

void FeText::set_bgr(int r)
{
	sf::Color c=m_base_text.getBgColor();
	c.r=r;
	m_base_text.setBgColor(c);
	script_flag_redraw();
}

void FeText::set_bgg(int g)
{
	sf::Color c=m_base_text.getBgColor();
	c.g=g;
	m_base_text.setBgColor(c);
	script_flag_redraw();
}

void FeText::set_bgb(int b)
{
	sf::Color c=m_base_text.getBgColor();
	c.b=b;
	m_base_text.setBgColor(c);
	script_flag_redraw();
}

void FeText::set_bga(int a)
{
	sf::Color c=m_base_text.getBgColor();
	c.a=a;
	m_base_text.setBgColor(c);
	script_flag_redraw();
}

void FeText::set_charsize(int s)
{
	m_base_text.setCharacterSize(s);
	script_flag_redraw();
}

void FeText::set_style(int s)
{
	m_base_text.setStyle(s);
	script_flag_redraw();
}

void FeText::set_align(int a)
{
	m_base_text.setAlignment( (FeTextPrimative::Alignment)a);
	script_flag_redraw();
}

FeListBox::FeListBox()
	: m_selColour( sf::Color::Yellow ),
	m_selBg( sf::Color::Blue ),
	m_selStyle( sf::Text::Regular ),
	m_rotation( 0.0 ),
	m_needs_init( true )
{
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
		FeTextPrimative::Alignment align )
	: FeBaseText( font, colour, bgcolour, charactersize, align ),
	m_selColour( selcolour ),
	m_selBg( selbgcolour ),
	m_selStyle( sf::Text::Regular ),
	m_rotation( 0.0 ),
	m_needs_init( true )
{
}

void FeListBox::init()
{
	//
	// Only do something if our needs_init flag is set
	//
	if ( !m_needs_init )
		return;

	const sf::Font *font = getFont();
	int fls = getCharacterSize();
	if ( fls < 10 ) fls = 8; // don't go smaller than 8

	if (( font ) && ( font->getLineSpacing( fls ) > fls ))
		fls = font->getLineSpacing( fls );

	fls += fls / 5;  // pad it

	sf::Vector2f size = getSize();
	sf::Vector2f pos = getPosition();

	int line_count = (int)size.y / fls;

	if ( line_count < 1 )
		return;

	int actual_spacing = (int)size.y / line_count;
	int sel = line_count / 2;

	m_texts.clear();
	m_texts.reserve( line_count );

	sf::Transform trans;
	trans.rotate( m_rotation, pos.x, pos.y );

	for ( int i=0; i< line_count; i++ )
	{
		FeTextPrimative t( m_base_text );
		if ( i == sel )
		{
			t.setColor( m_selColour );
			t.setBgColor( m_selBg );
			t.setStyle( m_selStyle );
		}

		t.setPosition( trans.transformPoint( pos.x, pos.y+(i*actual_spacing)) );
		t.setSize( size.x, actual_spacing );
		t.setRotation( m_rotation );

		m_texts.push_back( t );
	}

	m_needs_init=false;
}

void FeListBox::setSelColor( sf::Color c )
{
	m_selColour = c;

	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		m_texts[ sel ].setColor( m_selColour );
	}
	script_flag_redraw();
}

void FeListBox::setSelBgColor( sf::Color c )
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
	m_needs_init = true;
}

void FeListBox::on_new_list( FeSettings *s )
{
	init();

	s->get_current_display_list( m_displayList );
	setText( s->get_rom_index(), m_displayList );
}

void FeListBox::on_new_selection( FeSettings *s )
{
	setText( s->get_rom_index(), m_displayList );
}

void FeListBox::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	for ( std::vector<FeTextPrimative>::const_iterator itl=m_texts.begin();
				itl != m_texts.end(); ++itl )
		target.draw( (*itl), states );
}

void FeListBox::clear()
{
	m_texts.clear();
}

int FeListBox::getRowCount()
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
	return m_base_text.getCharacterSize();
}

int FeListBox::get_style()
{
	return m_base_text.getStyle();
}

int FeListBox::get_align()
{
	return (int)m_base_text.getAlignment();
}

void FeListBox::set_bgr(int r)
{
	sf::Color c=m_base_text.getBgColor();
	c.r=r;
	m_base_text.setBgColor(c);
	m_needs_init = true;
	script_flag_redraw();
}

void FeListBox::set_bgg(int g)
{
	sf::Color c=m_base_text.getBgColor();
	c.g=g;
	m_base_text.setBgColor(c);
	m_needs_init = true;
	script_flag_redraw();
}

void FeListBox::set_bgb(int b)
{
	sf::Color c=m_base_text.getBgColor();
	c.b=b;
	m_base_text.setBgColor(c);
	m_needs_init = true;
	script_flag_redraw();
}

void FeListBox::set_bga(int a)
{
	sf::Color c=m_base_text.getBgColor();
	c.a=a;
	m_base_text.setBgColor(c); 
	m_needs_init = true;
	script_flag_redraw();
}

void FeListBox::set_charsize(int s)
{
	m_base_text.setCharacterSize(s);
	m_needs_init = true;
	script_flag_redraw();
}

void FeListBox::set_style(int s)
{
	m_base_text.setStyle(s);
	m_needs_init = true;
	script_flag_redraw();
}

void FeListBox::set_align(int a)
{
	m_base_text.setAlignment( (FeTextPrimative::Alignment)a);
	m_needs_init = true;
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
