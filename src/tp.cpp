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

#include "tp.hpp"
#include <iostream>
#include <cmath>

FeTextPrimative::FeTextPrimative( )
	: m_texts( 1, sf::Text() ),
	m_align( Centre ),
	m_first_line( -1 ),
	m_needs_pos_set( false ),
	m_no_margin( 0 )
{
	setColor( sf::Color::White );
	setBgColor( sf::Color::Transparent );
}

FeTextPrimative::FeTextPrimative(
			const sf::Font *font,
         const sf::Color &colour,
         const sf::Color &bgcolour,
         unsigned int charactersize,
         Alignment align )
	: m_texts( 1, sf::Text() ),
	m_align( align ),
	m_first_line( -1 ),
	m_needs_pos_set( false ),
	m_no_margin( 0 )
{
	if ( font )
		setFont( *font );

	setColor( colour );
	setBgColor( bgcolour );
	setCharacterSize( charactersize );
}

FeTextPrimative::FeTextPrimative( const FeTextPrimative &c )
	: m_bgRect( c.m_bgRect ),
	m_texts( c.m_texts ),
	m_align( c.m_align ),
	m_first_line( c.m_first_line ),
	m_needs_pos_set( c.m_needs_pos_set ),
	m_no_margin( c.m_no_margin )
{
}

void FeTextPrimative::setColor( const sf::Color &c )
{
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setColor( c );
}

const sf::Color &FeTextPrimative::getColor() const
{
	return m_texts[0].getColor();
}

void FeTextPrimative::setBgColor( const sf::Color &c )
{
	m_bgRect.setFillColor( c );
}

const sf::Color &FeTextPrimative::getBgColor() const
{
	return m_bgRect.getFillColor();
}

void FeTextPrimative::fit_string(
			const std::basic_string<sf::Uint32> &s,
			int position,
			int &first_char,
			int &len )
{
	if ( position < 0 )
		position = 0;
	else if ( position > (int)s.size() )
		position = s.size();

	const sf::Font *font = getFont();
	unsigned int charsize = getCharacterSize();
	float width = m_bgRect.getLocalBounds().width / m_texts[0].getScale().x;

	int running_total( charsize * 2 ); // measure of line's pixel width

	// start from "position", "i" measures to right, "j" to the left
	int i( position );
	int j( position );
	int last_space( i );

	bool found_space( false );

	while (( running_total < width )
			&& (( i < (int)s.size() ) || (( m_first_line < 0 ) && ( j > 0 ))))
	{
		if ( i < (int)s.size() )
		{
			sf::Glyph g = font->getGlyph( s[i], charsize, false );
			running_total += g.advance;

			if ( s[i] == L' ' )
			{
				found_space = true;
				last_space = i;
			}
			else if ( s[i] == L'\n' )
			{
				// If we encounter a newline character, we break the
				// string there
				//
				len = i - j + 1;
				first_char = j;
				return;
			}

			i++;
		}

		if (( m_first_line < 0 ) && ( j > 0 ) && ( running_total < width ))
		{
			sf::Glyph g = font->getGlyph( s[j], charsize, false );
			running_total += g.advance;
			j--;
		}
	}

	first_char = j;

	//
	// If we are word wrapping and found a space, then break at the space.
	// Otherwise, fit as much on this line as we can
	//
	if ( ( m_first_line >= 0 ) && found_space && ( i != (int)s.size() ))
		len = last_space - j + 1;
	else
		len = i - j + 1;
}

void FeTextPrimative::setString( const std::string &t )
{
	//
	// UTF-8 character encoding is assumed.
	//	Need to convert to UTF-32 before giving string to SFML
	//
	std::basic_string<sf::Uint32> tmp;
	sf::Utf8::toUtf32( t.begin(), t.end(), std::back_inserter( tmp ) );
	setString( tmp );
}


sf::Vector2f FeTextPrimative::setString(
			const std::basic_string<sf::Uint32> &t,
			int position )
{
	//
	// Cut string if it is too big to fit our dimension
	//
	int first_char, len, disp_cpos;
	std::vector< int > fc_list;
	std::vector< int > len_list;

	if ( m_first_line >= 0 )
		position = 0;

	fit_string( t, position, first_char, len );

	fc_list.push_back( first_char );
	len_list.push_back( len );

	disp_cpos = position - first_char;

	if ( m_texts.size() > 1 )
		m_texts.resize( 1 );

	if (( m_first_line >= 0 ) && ( len < (int)t.size() ))
	{
		//
		// Calculate the number of lines we can fit in our RectShape
		//
		unsigned int spacing = getCharacterSize() * m_texts[0].getScale().y;
		const sf::Font *font = getFont();
		if ( font )
			spacing = font->getLineSpacing( spacing );

		sf::FloatRect rectSize = m_bgRect.getLocalBounds();
		int line_count = std::max( 1, (int)(rectSize.height / spacing) );

		//
		// Calculate the wrapped lines
		//
		position += len;
		int i=0;
		int actual_first_line=0;
		while (( position < (int)t.size() - 1 ) && ( i < line_count + m_first_line ))
		{
			if ( i >= line_count )
				actual_first_line++;

			fit_string( t, position, first_char, len );

			fc_list.push_back( first_char );
			len_list.push_back( len );
			position += len;
			i++;
		}

		m_first_line = actual_first_line;

		int actual_lines = ( (int)fc_list.size() < line_count ) ? fc_list.size() : line_count;
		int first_fc = fc_list.size() - actual_lines;

		//
		// Now create the wrapped lines
		//
		m_texts[0].setString( t.substr( fc_list[first_fc], len_list[first_fc] ) );
		for ( i = first_fc + 1; i < (int)fc_list.size(); i++ )
		{
			m_texts.push_back( m_texts[0] );
			m_texts.back().setString( t.substr( fc_list[i], len_list[i] ) );
		}
	}
	else
		// create the no-wrap or single non-clipped line
		m_texts[0].setString( t.substr( fc_list.front(), len_list.front() ) );

	set_positions(); // we need to set the positions now for findCharacterPos() to work below
	return m_texts[0].findCharacterPos( disp_cpos );
}

void FeTextPrimative::set_positions() const
{
	int spacing = getCharacterSize() * m_texts[0].getScale().y;

	const sf::Font *font = getFont();
	if (( font ) && ( font->getLineSpacing( spacing ) > spacing ))
		spacing = font->getLineSpacing( spacing );

	sf::Vector2f rectPos = m_bgRect.getPosition();
	sf::FloatRect rectSize = m_bgRect.getLocalBounds();

	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		sf::Vector2f textPos;

		// we need to account for the scaling that we have applied to our text...
		sf::FloatRect textSize = m_texts[i].getLocalBounds();
		textSize.width *= m_texts[i].getScale().x;
		textSize.height *= m_texts[i].getScale().y;

		textPos.y = rectPos.y
				+ ceilf( spacing * i
				+ ( rectSize.height - ( spacing * m_texts.size() )) / 2);

		// set x position
		switch ( m_align )
		{
		case Left:
			if( m_no_margin ) textPos.x = rectPos.x;
			else textPos.x = rectPos.x + ( spacing/2 );
			break;

		case Centre:
			textPos.x = rectPos.x + ( (rectSize.width - textSize.width) / 2 );
			break;

		case Right:
			if( m_no_margin ) textPos.x = rectPos.x + rectSize.width - textSize.width;
			else textPos.x = rectPos.x + rectSize.width - textSize.width - ( spacing/2 );
			break;
		}

		sf::Transform trans;
		trans.rotate( m_bgRect.getRotation(), rectPos.x, rectPos.y );
		m_texts[i].setPosition( trans.transformPoint( textPos ) );
		m_texts[i].setRotation( m_bgRect.getRotation() );
	}

	m_needs_pos_set = false;
}

int FeTextPrimative::getActualWidth()
{
	float w = 0;

	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		sf::FloatRect textSize = m_texts[i].getLocalBounds();
		textSize.width *= m_texts[i].getScale().x;

		if ( textSize.width > w )
			w = textSize.width;
	}

	return (int)w;
}

void FeTextPrimative::setFont( const sf::Font &font )
{
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setFont( font );

	m_needs_pos_set = true;
}

const sf::Font *FeTextPrimative::getFont() const
{
	return m_texts[0].getFont();
}

void FeTextPrimative::setCharacterSize( unsigned int size )
{
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setCharacterSize( size );

	m_needs_pos_set = true;
}

unsigned int FeTextPrimative::getCharacterSize() const
{
	return m_texts[0].getCharacterSize();
}

const sf::Vector2f &FeTextPrimative::getPosition() const
{
	return m_bgRect.getPosition();
}

const sf::Vector2f &FeTextPrimative::getSize() const
{
	return m_bgRect.getSize();
}

void FeTextPrimative::setPosition( const sf::Vector2f &p )
{
	m_bgRect.setPosition( p );
	m_needs_pos_set = true;
}

void FeTextPrimative::setSize( const sf::Vector2f &s )
{
	m_bgRect.setSize( s );
	m_needs_pos_set = true;
}

void FeTextPrimative::setAlignment( Alignment a )
{
	m_align = a;
	m_needs_pos_set = true;
}

FeTextPrimative::Alignment FeTextPrimative::getAlignment() const
{
	return m_align;
}

void FeTextPrimative::setStyle( int s )
{
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setStyle( s );
}

void FeTextPrimative::setOutlineColor( const sf::Color &c )
{
	m_bgRect.setOutlineColor( c );
}

void FeTextPrimative::setOutlineThickness( int i )
{
	m_bgRect.setOutlineThickness( i );
}

void FeTextPrimative::setRotation( float r )
{
	m_bgRect.setRotation( r );
	m_needs_pos_set = true;
}

float FeTextPrimative::getRotation() const
{
	return m_bgRect.getRotation();
}

int FeTextPrimative::getStyle() const
{
	return m_texts[0].getStyle();
}

void FeTextPrimative::setFirstLineHint( int line )
{
	m_first_line = ( line < 0 ) ? 0 : line;
}

void FeTextPrimative::setWordWrap( bool wrap )
{
	m_first_line = wrap ? 0 : -1;
}

void FeTextPrimative::setNoMargin( bool margin )
{
	m_no_margin = margin;
}

bool FeTextPrimative::getNoMargin()
{
	return m_no_margin;
}

void FeTextPrimative::setTextScale( const sf::Vector2f &s )
{
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setScale( s );

	m_needs_pos_set = true;
}

const sf::Vector2f &FeTextPrimative::getTextScale() const
{
		return m_texts[0].getScale();
}

int FeTextPrimative::getFirstLineHint() const
{
	return m_first_line;
}

void FeTextPrimative::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	if ( m_needs_pos_set )
		set_positions();

	target.draw( m_bgRect, states );

	for ( unsigned int i=0; i < m_texts.size(); i++ )
		target.draw( m_texts[i], states );
}
