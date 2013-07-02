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

#ifndef FE_TEXT_HPP
#define FE_TEXT_HPP

#include <SFML/Graphics.hpp>
#include "fe_base.hpp"
#include "fe_info.hpp"
#include "tp.hpp"

class FeSettings;

//
// Base class for FeText and FeListBox
//
class FeTextConfigurable : public FeBaseConfigurable, public FeTextPrimative
{
protected:
	FeTextConfigurable();
   FeTextConfigurable( const sf::Font *font,
         const sf::Color &colour,
         const sf::Color &bgcolour,
         unsigned int charactersize=30,
         Alignment align=Centre );

	static const char *baseSettings[];
	static const char *styleTokens[];

public:
   int process_setting( const std::string &setting,
                        const std::string &value,
								const std::string &fn );
};

//
// Text (w/ background) to display info on screen
//
class FeText : public FeTextConfigurable, public FeBasePresentable
{
public:
	FeText( const std::string &str );

   // Override from base class:
   int process_setting( const std::string &setting,
                        const std::string &value,
								const std::string &fn );

	// Overrides from base class:
	//
   void on_new_selection( FeSettings * );
	const sf::Drawable &drawable() { return (const sf::Drawable &)*this; };

private:
	std::string m_string;
	int m_index_offset;
};

//
// The text game list
//
class FeListBox : public FeTextConfigurable, public FeBasePresentable
{
public:
	FeListBox();

	FeListBox( const sf::Font *font,
			const sf::Color &colour,
			const sf::Color &bgcolour,
			const sf::Color &selcolour,
			const sf::Color &selbgcolour, 
			unsigned int characterSize=30,
			Alignment align=Centre );

	void setSelColor( sf::Color );
	void setSelBgColor( sf::Color );
	void setSelStyle( int );

	FeTextPrimative *setEditMode( bool, sf::Color );

	void setRotation( float );
	void setText( const int index, const std::vector<std::string> &list );
	int getRowCount();

	void clear();
	void init();

	// Overrides from base class:
	//
   int process_setting( const std::string &setting,
                        const std::string &value,
								const std::string &fn );
   void on_new_list( FeSettings * );
   void on_new_selection( FeSettings * );

	const sf::Drawable &drawable() { return (const sf::Drawable &)*this; };

private:
	std::vector<std::string> m_displayList;
	std::vector<FeTextPrimative> m_texts;
	sf::Color m_selColour;
	sf::Color m_selBg;
	int m_selStyle;
	float m_rotation;

	void draw( sf::RenderTarget &target, sf::RenderStates states ) const;
};

#endif
