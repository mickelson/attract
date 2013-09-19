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
#include "fe_presentable.hpp"
#include "tp.hpp"

class FeSettings;

class FeBaseText : public FeBasePresentable
{
public:
	FeBaseText();
	FeBaseText(
		const sf::Font *font,
		const sf::Color &colour,
		const sf::Color &bgcolour,
		unsigned int charactersize,
		FeTextPrimative::Alignment align );

	const sf::Font *getFont() const;
	void setFont( const sf::Font & );
	const sf::Vector2f &getPosition() const;
	void setPosition( const sf::Vector2f & );
	void setPosition( int x, int y ) {return setPosition(sf::Vector2f(x,y));};
	const sf::Vector2f &getSize() const;
	void setSize( const sf::Vector2f & );
	void setSize( int w, int h ) {return setSize(sf::Vector2f(w,h));};
	float getRotation() const;
	void setRotation( float );
	const sf::Color &getColor() const;
	void setColor( const sf::Color & );

protected:
	FeBaseText( const FeBaseText & );
	FeBaseText &operator=( const FeBaseText & );

	FeTextPrimative m_base_text;
};

//
// Text (w/ background) to display info on screen
//
class FeText : public FeBaseText, public sf::Drawable
{
public:
	FeText( const std::string &str );

	// Overrides from base class:
	//
	void on_new_list( FeSettings *, float, float );
	void on_new_selection( FeSettings * );
	const sf::Drawable &drawable() { return (const sf::Drawable &)*this; };

	int getIndexOffset() const;
	void setIndexOffset( int );

	const char *get_string();
	void set_string(const char *s);

	int get_bgr();
	int get_bgg();
	int get_bgb();
	int get_bga();
	int get_charsize();
	int get_style();
	int get_align();
	void set_bgr(int r);
	void set_bgg(int g);
	void set_bgb(int b);
	void set_bga(int a);
	void set_bg_rgb( int, int, int );
	void set_charsize(int s);
	void set_style(int s);
	void set_align(int a);

protected:
	void draw( sf::RenderTarget &target, sf::RenderStates states ) const;

private:
	FeText( const FeText & );
	FeText &operator=( const FeText & );

	FeTextPrimative m_draw_text;
	std::string m_string;
	int m_index_offset;
	bool m_user_charsize; 	// set to true if the layout has set a specific
									// charsize
};

//
// The text game list
//
class FeListBox : public FeBaseText, public sf::Drawable
{
public:
	FeListBox();

	FeListBox( const sf::Font *font,
			const sf::Color &colour,
			const sf::Color &bgcolour,
			const sf::Color &selcolour,
			const sf::Color &selbgcolour, 
			unsigned int characterSize,
			int rows );

	int getIndexOffset() const;
	void setIndexOffset( int );

	void setColor( const sf::Color & );
	void setBgColor( const sf::Color & );
	void setSelColor( const sf::Color & );
	void setSelBgColor( const sf::Color & );
	void setSelStyle( int );
	int getSelStyle();

	FeTextPrimative *setEditMode( bool, sf::Color );

	void setRotation( float );
	void setText( const int index, const std::vector<std::string> &list );
	int getRowCount() const;

	void clear();
	void init( float scale_x = 1.0, float scale_y = 1.0 );

	// Overrides from base class:
	//
	void on_new_list( FeSettings *, float, float );
	void on_new_selection( FeSettings * );

	const sf::Drawable &drawable() { return (const sf::Drawable &)*this; };

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
	void set_selbgr(int r);
	void set_selbgg(int g);
	void set_selbgb(int b);
	void set_selbga(int a);
	void set_selbg_rgb( int, int, int );

private:
	FeListBox( const FeListBox & );
	FeListBox &operator=( const FeListBox & );

	std::vector<std::string> m_displayList;
	std::vector<FeTextPrimative> m_texts;
	sf::Color m_selColour;
	sf::Color m_selBg;
	int m_selStyle;
	int m_rows;
	int m_userCharSize;
	float m_rotation;

	void draw( sf::RenderTarget &target, sf::RenderStates states ) const;
};

#endif
