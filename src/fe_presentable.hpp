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

#include <SFML/System/Vector2.hpp>
#include <vector>

class FeSettings;
class FeShader;
class FePresentableParent;

namespace sf
{
	class Drawable;
	class Color;
};

class FeBasePresentable
{
private:
	FePresentableParent &m_parent;
	FeShader *m_shader;
	bool m_visible;
	int m_zorder;

public:
	FeBasePresentable( FePresentableParent &p );
	virtual ~FeBasePresentable();

	virtual void on_new_selection( FeSettings * );
	virtual void on_new_list( FeSettings * );
	virtual void set_scale_factor( float, float );

	virtual const sf::Drawable &drawable() const=0;
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
	virtual int getFilterOffset() const=0;
	virtual void setFilterOffset( int io )=0;

	//
	// Accessor functions used in scripting implementation
	//
	float get_x() const;
	float get_y() const;
	void set_x( float x );
	void set_y( float y );

	float get_width() const;
	float get_height() const;
	void set_width( float w );
	void set_height( float h );

	void set_pos(float x, float y);
	void set_pos(float x, float y, float w, float h);

	int get_r() const;
	int get_g() const;
	int get_b() const;
	int get_a() const;
	void set_r(int r);
	void set_g(int g);
	void set_b(int b);
	void set_a(int a);
	void set_rgb(int r, int g, int b);

	bool get_visible() const;
	void set_visible( bool );

	FeShader *get_shader() const;
	FeShader *script_get_shader() const;
	void script_set_shader( FeShader *s );

	int get_zorder();
	void set_zorder( int );
};

class FeImage;
class FeText;
class FeListBox;

class FePresentableParent
{
public:
	std::vector< FeBasePresentable * > elements;

	FeImage *add_image(const char *,int, int, int, int);
	FeImage *add_image(const char *, int, int);
	FeImage *add_image(const char *);
	FeImage *add_artwork(const char *,int, int, int, int);
	FeImage *add_artwork(const char *, int, int);
	FeImage *add_artwork(const char *);
	FeImage *add_clone(FeImage *);
	FeText *add_text(const char *,int, int, int, int);
	FeListBox *add_listbox(int, int, int, int);
	FeImage *add_surface(int, int);
};

#endif
