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

#ifndef TEXT_PRIMATIVE_HPP
#define TEXT_PRIMATIVE_HPP

#include <SFML/Graphics.hpp>
#include <vector>

class FeTextPrimative : public sf::Drawable
{
public:
	enum Alignment {
		Left=0,
		Centre,
		Right
	};

	FeTextPrimative();

	FeTextPrimative( const sf::Font *font, 
			const sf::Color &colour,
			const sf::Color &bgcolour,
			unsigned int charactersize,
			Alignment align=Centre );

	FeTextPrimative( const FeTextPrimative & );

	void setColor( const sf::Color & );
	void setBgColor( const sf::Color & );

	void setString( const std::string & );	// does utf-8 conversion!

	//
	// cursor_string_pos is the character in the string that the cursor
	// should be under.  Valid positions are from 0 to t.size().  This 
	// function returns the screen location of the cursor character PROVIDED 
	// that WordWrap is set to false (cursor positioning is not supported 
	// with // wordwrapping on)
	//
	sf::Vector2f setString( const std::basic_string<sf::Uint32> &t,
					int cursor_string_pos=0 ); // no utf-8 conversion

	void setFont( const sf::Font & );
	void setCharacterSize( unsigned int );
	void setAlignment( Alignment );
	void setPosition( int x, int y ) {return setPosition(sf::Vector2f(x,y));};
	void setPosition( const sf::Vector2f & );
	void setSize( int w, int h ) {return setSize(sf::Vector2f(w,h));};
	void setSize( const sf::Vector2f & );
	void setStyle( int );
	void setRotation( float );
	void setOutlineColor( const sf::Color & );
	void setOutlineThickness( int );
	void setWordWrap( bool );

	const sf::Font *getFont() const;
	const sf::Color &getColor() const;
	const sf::Color &getBgColor() const;
	unsigned int getCharacterSize() const;
	Alignment getAlignment() const;
	const sf::Vector2f &getPosition() const;
	const sf::Vector2f &getSize() const;
	float getRotation() const;
	int getStyle() const;

private:
	Alignment m_align;
	bool m_wrap;
	sf::RectangleShape m_bgRect;
	std::vector<sf::Text> m_texts;

	//
	// Determines how to fit the given string "s" into the text space
	// parameters:
	//		[in] s 			- string to be displayed
	//		[in] position	- If WordWrap is true, this is the first position in 
	//							"s" that can be displayed.  If WordWrap is false, this
	//							is the location of the cursor in "s".
	//		[out] first_char	- position of the first character to display
	//		[out] len			- len of substring in s to display
	//
	void fit_string( 
			const std::basic_string<sf::Uint32> &s, 
			int position, 
			int &first_char,
			int &len );

	void set_positions();

	// override from base
	void draw( sf::RenderTarget &target, sf::RenderStates states ) const;
};

#endif
