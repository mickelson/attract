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

#ifndef FE_LISTBOX_HPP
#define FE_LISTBOX_HPP

#include <SFML/Graphics.hpp>
#include "fe_presentable.hpp"
#include "tp.hpp"

class FeSettings;
class FePresent;
class FeLanguage;

//
// The text game list
//
class FeListBox : public FeBasePresentable, public sf::Drawable
{
public:

	// Constructor for use in scripts.  sets m_scripted to true
	FeListBox( int x, int y, int w, int h );

	// Constructor for use in overlay.  sets m_scripted to false
	FeListBox( const sf::Font *font,
			const sf::Color &colour,
			const sf::Color &bgcolour,
			const sf::Color &selcolour,
			const sf::Color &selbgcolour,
			unsigned int characterSize,
			int rows );

	void setFont( const sf::Font & );
	const sf::Vector2f &getPosition() const;
	void setPosition( const sf::Vector2f & );
	void setPosition( int x, int y ) {return setPosition(sf::Vector2f(x,y));};
	const sf::Vector2f &getSize() const;
	void setSize( const sf::Vector2f & );
	void setSize( int w, int h ) {return setSize(sf::Vector2f(w,h));};
	float getRotation() const;
	const sf::Color &getColor() const;

	int getIndexOffset() const;
	void setIndexOffset( int );
	int getFilterOffset() const;
	void setFilterOffset( int );

	void setColor( const sf::Color & );
	void setBgColor( const sf::Color & );
	void setSelColor( const sf::Color & );
	void setSelBgColor( const sf::Color & );
	void setSelStyle( int );
	int getSelStyle();
	void setTextScale( const sf::Vector2f & );

	FeTextPrimative *setEditMode( bool, sf::Color );

	void setRotation( float );
	void setText( const int index, const std::vector<std::string> &list );

	// special case for the language selection listbox (different fonts)
	void setText( const int index,
			const std::vector<FeLanguage> &list,
			FePresent *fep );

	int getRowCount() const;

	void clear();
	void init();

	// Overrides from base class:
	//
	void on_new_list( FeSettings * );
	void on_new_selection( FeSettings * );
	void set_scale_factor( float, float );

	const sf::Drawable &drawable() const { return (const sf::Drawable &)*this; };

	int get_bgr();
	int get_bgg();
	int get_bgb();
	int get_bga();
	int get_charsize();
	int get_rows();
	int get_style();
	int get_align();
	void set_bgr(int r);
	void set_bgg(int g);
	void set_bgb(int b);
	void set_bga(int a);
	void set_bg_rgb( int, int, int );
	void set_charsize(int s);
	void set_rows(int r);
	void set_style(int s);
	void set_align(int a);
	int get_selr();
	int get_selg();
	int get_selb();
	int get_sela();
	void set_selr(int r);
	void set_selg(int g);
	void set_selb(int b);
	void set_sela(int a);
	void set_sel_rgb( int, int, int );
	int get_selbgr();
	int get_selbgg();
	int get_selbgb();
	int get_selbga();
	const char *get_font();
	const char *get_format_string();

	void set_selbgr(int r);
	void set_selbgg(int g);
	void set_selbgb(int b);
	void set_selbga(int a);
	void set_selbg_rgb( int, int, int );
	void set_font( const char *f );
	void set_format_string( const char *s );

private:
	FeListBox( const FeListBox & );
	FeListBox &operator=( const FeListBox & );

	FeTextPrimative m_base_text;
	std::vector<std::string> m_displayList;
	std::vector<FeTextPrimative> m_texts;
	std::string m_font_name;
	std::string m_format_string;
	sf::Color m_selColour;
	sf::Color m_selBg;
	int m_selStyle;
	int m_rows;
	int m_userCharSize;
	int m_filter_offset;
	float m_rotation;
	float m_scale_factor;
	bool m_scripted;

	void draw( sf::RenderTarget &target, sf::RenderStates states ) const;
};

#endif
