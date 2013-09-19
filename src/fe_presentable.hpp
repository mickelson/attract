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

#ifndef FE_PRESENTABLE_HPP
#define FE_PRESENTABLE_HPP

#include <SFML/Graphics.hpp>

class FeSettings;
class FeBasePresentable
{
private:
	const bool m_draw_apply_scale; // true for images, false for texts
	bool m_visible;

	FeBasePresentable( const FeBasePresentable & );
	FeBasePresentable &operator=( const FeBasePresentable & );

public:
	FeBasePresentable( const bool draw_apply_scale );
	virtual ~FeBasePresentable();

	virtual void on_new_selection( FeSettings * );
	virtual void on_new_list( FeSettings *, float, float );

	virtual const sf::Drawable &drawable()=0;
	virtual const sf::Vector2f &getPosition() const=0;
	virtual void setPosition( const sf::Vector2f & )=0;
	virtual const sf::Vector2f &getSize() const=0;
	virtual void setSize( const sf::Vector2f & )=0;
	virtual float getRotation() const=0;
	virtual void setRotation( float )=0;
	virtual const sf::Color &getColor() const=0;
	virtual void setColor( const sf::Color & )=0;
	virtual int getIndexOffset() const=0;
	virtual void setIndexOffset( int io )=0;

	//
	// Accessor functions used in scripting implementation
	//
	float get_x();
	float get_y();
	void set_x( float x );
	void set_y( float y );

	float get_width();
	float get_height();
	void set_width( float w );
	void set_height( float h );

	int get_r();
	int get_g();
	int get_b();
	int get_a();
	void set_r(int r);
	void set_g(int g);
	void set_b(int b);
	void set_a(int a);
	void set_rgb(int r, int g, int b);

	bool get_visible();
	void set_visible( bool );

	bool get_draw_apply_scale() const { return m_draw_apply_scale; };
};

void script_do_update( FeBasePresentable * );
void script_flag_redraw();

#endif
