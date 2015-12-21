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

//
// Text (w/ background) to display info on screen
//
class FeText : public FeBasePresentable, public sf::Drawable
{
public:
	FeText( FePresentableParent &p,
		const std::string &str, int x, int y, int w, int h );

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

	// Overrides from base class:
	//
	void on_new_list( FeSettings * );
	void on_new_selection( FeSettings * );
	void set_scale_factor( float, float );

	const sf::Drawable &drawable() const { return (const sf::Drawable &)*this; };

	int getIndexOffset() const;
	void setIndexOffset( int );
	int getFilterOffset() const;
	void setFilterOffset( int );

	void set_word_wrap( bool );
	bool get_word_wrap();

	void set_first_line_hint( int l );
	int get_first_line_hint();

	const char *get_string();
	void set_string(const char *s);

	int get_actual_width() { return m_draw_text.getActualWidth(); };

	int get_bgr();
	int get_bgg();
	int get_bgb();
	int get_bga();
	int get_charsize();
	int get_style();
	int get_align();
	const char *get_font();
	void set_bgr(int r);
	void set_bgg(int g);
	void set_bgb(int b);
	void set_bga(int a);
	void set_bg_rgb( int, int, int );
	void set_charsize(int s);
	void set_style(int s);
	void set_align(int a);
	void set_font(const char *f);

protected:
	void draw( sf::RenderTarget &target, sf::RenderStates states ) const;

private:
	FeText( const FeText & );
	FeText &operator=( const FeText & );

	FeTextPrimative m_draw_text;
	std::string m_string;
	std::string m_font_name;
	int m_index_offset;
	int m_filter_offset;
	int m_user_charsize;	 	// -1 if no charsize specified
	sf::Vector2f m_size;		// unscaled size
	sf::Vector2f m_position;	// unscaled position
	float m_scale_factor;
};

#endif
