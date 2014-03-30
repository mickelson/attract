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

#ifndef FE_IMAGE_HPP
#define FE_IMAGE_HPP

#include <SFML/Graphics.hpp>
#include "sprite.hpp"
#include "fe_presentable.hpp"

class FeSettings;
class FeMedia;
class FeImage;

class FeTextureContainer
{
friend class FeImage;

public:
	enum MovieStatus
	{
		Delayed,			// short delay so that we don't play movie
							// if the user is just scrolling past
		Loading,			// processing first frames but display not ready
		Playing,			// play movie
		NoPlay,			// don't play this movie, show image instead
		LockNoPlay		// don't ever play a movie
	};

	FeTextureContainer();
	FeTextureContainer( bool is_artwork, const std::string &name );
	~FeTextureContainer();

	const sf::Texture &load( const std::string & );
	const sf::Texture &get_texture();

	void on_new_selection( FeSettings *feSettings );
	bool tick( FeSettings *feSettings ); // returns true if redraw required
	void set_play_state( bool play );
	void set_vol( float vol );

private:
	sf::Texture m_texture;
	std::string m_name;
	int m_index_offset;
	bool m_is_artwork;
	FeMedia *m_movie;
	MovieStatus m_movie_status;
	std::vector< FeImage * > m_images;
};

class FeImage : public sf::Drawable, public FeBasePresentable
{
protected:
	FeTextureContainer *m_tex;
	FeSprite m_sprite;
	sf::Vector2f m_pos;
	sf::Vector2f m_size;
	bool m_preserve_aspect_ratio;

	void scale();

	// Override from base class:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

public:
	FeImage( FeTextureContainer * );
	FeImage( FeImage * ); // clone the given image (texture is not copied)
	~FeImage();

	const sf::Texture *get_texture();

	void loadFromFile( const std::string & );

	const sf::Vector2f &getSize() const;
	void setSize( const sf::Vector2f &s );
	void setSize( int w, int h ) { setSize( sf::Vector2f( w, h ) ); };
	const sf::Vector2f &getPosition() const;
	void setPosition( const sf::Vector2f & );
	void setPosition( int x, int y ) { setPosition( sf::Vector2f( x, y ));};
	float getRotation() const;
	void setRotation( float );
	const sf::Color &getColor() const;
	void setColor( const sf::Color & );
	int getIndexOffset() const;
	void setIndexOffset(int);
	const sf::Vector2u getTextureSize() const;
	const sf::IntRect &getTextureRect() const;
	void setTextureRect( const sf::IntRect &);
	bool getMovieEnabled() const;
	void setMovieEnabled( bool );

	// Overrides from base class:
	//
	const sf::Drawable &drawable() { return (const sf::Drawable &)*this; };
	void texture_changed();

	int get_skew_x() const ;
	int get_skew_y() const;
	int get_pinch_x() const ;
	int get_pinch_y() const;
	int get_texture_width() const;
	int get_texture_height() const;
	int get_subimg_x() const;
	int get_subimg_y() const;
	int get_subimg_width() const;
	int get_subimg_height() const;
	bool get_preserve_aspect_ratio() const;

	void set_skew_x( int x );
	void set_skew_y( int y );
	void set_pinch_x( int x );
	void set_pinch_y( int y );
	void set_subimg_x( int x );
	void set_subimg_y( int y );
	void set_subimg_width( int w );
	void set_subimg_height( int h );
	void set_preserve_aspect_ratio( bool p );
};

void script_do_update( FeTextureContainer * );

#endif
