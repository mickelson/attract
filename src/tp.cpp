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

// included for SFML_VERSION_INT macros
#include "fe_util.hpp"

FeTextPrimative::FeTextPrimative( )
	: m_texts( 1, sf::Text() ),
	m_align( Centre ),
	m_first_line( -1 ),
	m_no_margin( false ),
	m_needs_pos_set( false )
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
	m_no_margin( false ),
	m_needs_pos_set( false )
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
	m_no_margin( c.m_no_margin ),
	m_needs_pos_set( c.m_needs_pos_set )
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
			int &position,
			int &first_char,
			int &last_char )
{
	// There is a special case where we edit the string in UI
	// We do not cut the trailing space in this situation
	//
	bool cut_space( true );
	if (( position >= 0 ) && ( m_first_line < 0 ))
		cut_space = false;

	if ( position < 0 )
		position = 0;

	// start from "position", "i" measures to right, "j" to the left
	int i( position );
	int j( position );
	int last_space( 0 );
	first_char = position;

	const sf::Font *font = getFont();
	unsigned int charsize = m_texts[0].getCharacterSize();
	unsigned int spacing = charsize;
	float width = m_bgRect.getLocalBounds().width / m_texts[0].getScale().x;

	int running_total( charsize * 2 );
	int running_width( 0 );
	int kerning( 0 );

	const sf::Glyph *g = &font->getGlyph( s[i], charsize, false );
	running_total = -g->bounds.left;

	if ( font->getLineSpacing( spacing ) > spacing )
		spacing = font->getLineSpacing( spacing );

	if ( !m_no_margin )
		width -= floorf( spacing );

	// We need to adjust the width to make sure that at least one character is displayed per line
	width = std::max( width, g->bounds.width );

	while (( running_width <= width ) && (( i < (int)s.size() ) || (( m_first_line < 0 ) && ( j > 0 ))))
	{
		if ( i < (int)s.size() )
		{
			if ( i > first_char )
			{
				kerning = font->getKerning( s[std::max( 0, i - 1 )], s[i], charsize );
				running_total += kerning;
			}

			g = &font->getGlyph( s[i], charsize, false );
			running_width = std::max( running_width, (int)( running_total + g->bounds.left + g->bounds.width ));
			running_total += g->advance;

			if ( s[i] == L' ' )
				last_space = i;

			if ( s[i] == L'\n' )
			{
				// If we encounter a newline character, we break the string there
				//
				last_char = i - 1;
				i++;
				return;
			}

			i++;
		}

		if (( m_first_line < 0 ) && ( j > 0 ) && ( running_width <= width ))
		{
			if ( j < first_char )
			{
				kerning = font->getKerning( s[std::max( 0, j - 2 )], s[j - 1], charsize );
				running_total += kerning;
			}

			g = &font->getGlyph( s[j - 1], charsize, false );
			running_width = std::max( running_width, (int)( running_total + g->bounds.left + g->bounds.width ));
			running_total += g->advance;
			j--;
		}
	}

	first_char = j;

	// If we are word wrapping and found a space, then break at the space.
	// Otherwise, fit as much on this line as we can.
	//
	if (( last_space > 0 ) && ( m_first_line >= 0 ))
	{
		last_char = last_space - 1;
		i = last_space + 1;
	}
	else
	{
		if ( cut_space )
		{
			i--;
			last_char = i - 1;
			if ( s[last_char] == L' ' ) last_char--;
		}
		else
			last_char = i - 1;
	}

	position = i;
}

void FeTextPrimative::setString( const std::string &t )
{
	//
	// UTF-8 character encoding is assumed.
	// Need to convert to UTF-32 before giving string to SFML
	//
	std::basic_string<sf::Uint32> tmp;
	sf::Utf8::toUtf32( t.begin(), t.end(), std::back_inserter( tmp ));

	// We need to add one trailing space to the string
	// for the word wrap function to work properly
	//
	std::fill_n( back_inserter( tmp ), 1, L' ' );

	setString( tmp );
}

sf::Vector2f FeTextPrimative::setString(
			const std::basic_string<sf::Uint32> &t,
			int position )
{
	//
	// Cut the string if it is too big to fit our dimension
	//
	int first_char, last_char;
	int disp_cpos( position );

	if ( m_first_line >= 0 )
		position = -1;

	if ( m_texts.size() > 1 )
		m_texts.resize( 1 );

	const sf::Font *font = getFont();
	
	//
	// We cut the first line of text here
	//
	fit_string( t, position, first_char, last_char );
	m_texts[0].setString( t.substr( first_char, last_char - first_char + 1 ));

	disp_cpos -= first_char;

	//
	// If we are word wrapping calculate the rest of lines
	//
	if ( m_first_line >= 0 )
	{
		//
		// Calculate the number of lines we can fit in our RectShape
		//
		float spacing = m_texts[0].getCharacterSize() * m_texts[0].getScale().y;
		spacing = font->getLineSpacing( spacing );

		sf::FloatRect rectSize = m_bgRect.getLocalBounds();
		float glyphSize = getGlyphSize();

		const int charSize = m_texts[0].getCharacterSize();
		const sf::Glyph *glyph = &font->getGlyph( L'X', charSize, false );
		glyphSize = glyph->bounds.height * m_texts[0].getScale().y;

		int line_count = 1;
		if ( m_align & ( Top | Bottom | Middle ))
		{
			if ( m_no_margin )
				line_count = std::max( 1, (int)floorf(( rectSize.height + spacing - glyphSize ) / spacing ));
			else
				line_count = std::max( 1, (int)floorf(( rectSize.height - glyphSize ) / spacing ));
		}
		else
			line_count = std::max( 1, (int)( rectSize.height / spacing ));

		//
		// We break the string to lines here starting from the second line
		//
		for ( int i = 1; i < line_count; i++ )
		{
			if ( position >= t.size() ) break;
			fit_string( t, position, first_char, last_char );
			m_texts.push_back( m_texts[0] );
			m_texts.back().setString( t.substr( first_char, last_char - first_char + 1 ));
		}
	}

	set_positions(); // We need to set the positions now for findCharacterPos() to work below
	int kerning = font->getKerning( m_texts[0].getString()[std::max( 0, disp_cpos - 1 )], m_texts[0].getString()[disp_cpos], m_texts[0].getCharacterSize() ) * m_texts[0].getScale().x;
	return m_texts[0].findCharacterPos( disp_cpos ) + sf::Vector2f( kerning, 0.0 );
}

void FeTextPrimative::set_positions() const
{
	float charSize = m_texts[0].getCharacterSize() * m_texts[0].getScale().y;
	float spacing = charSize;
	int glyphSize = getGlyphSize();

	const sf::Font *font = getFont();
	if (( font ) && ( font->getLineSpacing( spacing ) > spacing ))
		spacing = font->getLineSpacing( spacing );
	int margin = floorf( spacing / 2.0 );

	sf::Vector2f rectPos = m_bgRect.getPosition();
	sf::FloatRect rectSize = m_bgRect.getLocalBounds();

	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		sf::Vector2f textPos;

		// we need to account for the scaling that we have applied to our text...
		sf::FloatRect textSize = m_texts[i].getLocalBounds();
		textSize.width *= m_texts[i].getScale().x;
		textSize.height *= m_texts[i].getScale().y;
		textSize.left *= m_texts[i].getScale().x;

		// set position x
		if ( m_align & Left )
			textPos.x = rectPos.x;
		else if ( m_align & Right )
			textPos.x = rectPos.x + floorf( rectSize.width ) - textSize.width;
		else if ( m_align & Centre )
			textPos.x = rectPos.x + floorf(( rectSize.width - textSize.width ) / 2.0 );

		if ( m_align & ( Top | Bottom | Middle ))
			textPos.x -= textSize.left;

		// set position y
		if ( m_align & Top )
			textPos.y = rectPos.y + ceilf( spacing * (int)i - charSize + glyphSize );
		else if ( m_align & Bottom )
			textPos.y = rectPos.y + floorf( rectSize.height  - charSize - spacing * ( m_texts.size() - (int)i - 1 ));
		else if ( m_align & Middle )
			textPos.y = rectPos.y + ceilf( spacing * (int)i + ( rectSize.height + glyphSize - charSize * 2 - spacing * ( m_texts.size() - 1 )) / 2.0 );
		else
			textPos.y = rectPos.y + ceilf( spacing * (int)i + ( rectSize.height - ( spacing * m_texts.size() )) / 2.0 );

		if ( !m_no_margin )
		{
			if ( m_align & Top ) textPos.y += margin;
			if ( m_align & Bottom ) textPos.y -= margin;
			if ( m_align & Left ) textPos.x += margin;
			if ( m_align & Right ) textPos.x -= margin;
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
		textSize.width = ceilf( textSize.width * m_texts[i].getScale().x );

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
	return ceilf( m_texts[0].getCharacterSize() * m_texts[0].getScale().y );
}

unsigned int FeTextPrimative::getGlyphSize() const
{
	const sf::Font *font = getFont();
	const int charSize = m_texts[0].getCharacterSize();
	const sf::Glyph *glyph = &font->getGlyph( L'X', charSize, false );
	return floorf(glyph->bounds.height * m_texts[0].getScale().y);
}

void FeTextPrimative::setCharacterSpacing( float factor )
{
// setLetterSpacing() only available as of SFML 2.5
#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 5, 0 ))
	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setLetterSpacing( factor );

	m_needs_pos_set = true;
#endif
}

float FeTextPrimative::getCharacterSpacing() const
{
// getLetterSpacing() only available as of SFML 2.5
#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 5, 0 ))
	return m_texts[0].getLetterSpacing();
#else
	return 1.f;
#endif
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
