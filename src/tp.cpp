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

FeTextPrimative::FeTextPrimative( )
	: m_align( Centre ), m_wrap( false ), m_texts( 1, sf::Text() )
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
	: m_align( align ), m_wrap( false ), m_texts( 1, sf::Text() )
{
	if ( font )
		setFont( *font );

	setColor( colour );
	setBgColor( bgcolour );
	setCharacterSize( charactersize );
}

FeTextPrimative::FeTextPrimative( const FeTextPrimative &c )
	: m_align( c.m_align ), m_wrap( c.m_wrap ), 
		m_bgRect( c.m_bgRect ), m_texts( c.m_texts )
{
}

void FeTextPrimative::setColor( sf::Color c )
{
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setColor( c );	
}

sf::Color FeTextPrimative::getColor()
{
	return m_texts[0].getColor();	
}

void FeTextPrimative::setBgColor( sf::Color c )
{
	m_bgRect.setFillColor( c );
}

sf::Color FeTextPrimative::getBgColor()
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
	float width = m_bgRect.getLocalBounds().width;
	int running_total( charsize * 2 );
	int i( (position == (int)s.size()) ? position : position + 1 );
	int j( position );
	int last_space( i );

	while (( running_total < width )
			&& (( i < (int)s.size() ) || ( !m_wrap && ( j > 0 ))))
	{
		if ( i < (int)s.size() )
		{
			sf::Glyph g = font->getGlyph( s[i], charsize, false );
			running_total += g.advance;

			if ( s[i] == L' ' )
				last_space = i;

			i++;
		}

		if (!m_wrap && ( j > 0 ) && ( running_total < width ))
		{
			sf::Glyph g = font->getGlyph( s[j], charsize, false );
			running_total += g.advance;
			j--;
		}
	}

	first_char = j;

	if (( m_wrap ) && ( i != (int)s.size() ))
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
	if ( m_wrap ) position = 0;

	fit_string( t, position, first_char, len );
	m_texts[0].setString( t.substr( first_char, len ) );
	disp_cpos = position - first_char;

	if ( m_texts.size() > 1 )
		m_texts.resize( 1 );

	if (( m_wrap ) && ( len < (int)t.size() ))
	{
		//
		// Calculate the number of lines we can fit in our RectShape
		//
		unsigned int spacing = getCharacterSize();
		const sf::Font *font = getFont();
		if ( font ) 
			spacing = font->getLineSpacing( getCharacterSize() );

		sf::FloatRect rectSize = m_bgRect.getLocalBounds();
		int line_count = rectSize.height / spacing; 

		//
		// Create the wrapped lines
		//
		position = len;
		int i=0;
		while (( position < (int)t.size() - 1 ) && ( i < line_count ))
		{
			fit_string( t, position, first_char, len );
			sf::Text new_text( m_texts[0] );
			new_text.setString( t.substr( first_char, len ) );
			position += len;
			m_texts.push_back( new_text );
			i++;
		}
	}

	set_positions();
	return m_texts[0].findCharacterPos( disp_cpos );
}

void FeTextPrimative::set_positions()
{
	unsigned int spacing = getCharacterSize();
	const sf::Font *font = getFont();
	if ( font ) 
		spacing = font->getLineSpacing( getCharacterSize() );

	sf::Vector2f rectPos = m_bgRect.getPosition();
	sf::FloatRect rectSize = m_bgRect.getLocalBounds();

	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		sf::Vector2f textPos;
		sf::FloatRect textSize = m_texts[i].getLocalBounds();

		textPos.y = rectPos.y 
				+ spacing * i 
				+ ( rectSize.height - ( spacing * m_texts.size() )) / 2;
	
		// set x position
		switch ( m_align )
		{
		case Left:
			textPos.x = rectPos.x + spacing/2;
			break;

		case Centre:
			textPos.x = rectPos.x + ( (rectSize.width - textSize.width) / 2 );
			break;

		case Right:
			textPos.x = rectPos.x + rectSize.width - textSize.width - spacing/2;
			break;
		}

		sf::Transform trans;
		trans.rotate( m_bgRect.getRotation(), rectPos.x, rectPos.y );
		m_texts[i].setPosition( trans.transformPoint( textPos ) );
		m_texts[i].setRotation( m_bgRect.getRotation() );
	}

} 

void FeTextPrimative::setFont( const sf::Font &font )
{
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setFont( font );
}

const sf::Font *FeTextPrimative::getFont()
{
	return m_texts[0].getFont();
}

void FeTextPrimative::setCharacterSize( unsigned int size )
{
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setCharacterSize( size );
}

unsigned int FeTextPrimative::getCharacterSize()
{
	return m_texts[0].getCharacterSize();
}

sf::Vector2f FeTextPrimative::getPosition()
{
	return m_bgRect.getPosition();
}

sf::Vector2f FeTextPrimative::getSize()
{
	return m_bgRect.getSize();
}

void FeTextPrimative::setPosition( sf::Vector2f p )
{
	m_bgRect.setPosition( p );
}

void FeTextPrimative::setSize( sf::Vector2f s )
{
	m_bgRect.setSize( s );
	set_positions();
}

void FeTextPrimative::setAlignment( Alignment a )
{
	m_align = a;
}

FeTextPrimative::Alignment FeTextPrimative::getAlignment()
{
	return m_align;
}

void FeTextPrimative::setStyle( int s )
{
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setStyle( s );
}

void FeTextPrimative::setOutlineColor( sf::Color c )
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
}

float FeTextPrimative::getRotation()
{
	return m_bgRect.getRotation();
}

void FeTextPrimative::setWordWrap( bool w )
{
	m_wrap = w;
}

void FeTextPrimative::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	target.draw( m_bgRect, states );

	for ( unsigned int i=0; i < m_texts.size(); i++ )
		target.draw( m_texts[i], states );
}
