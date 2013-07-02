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
#include "fe_base.hpp"
#include "media.hpp"

class FeSettings;

//
// Class representing a static image used in the presentation
// This is a base class for artworks and for movies
//
class FeImage : public sf::Drawable, public FeBaseConfigurable, public FeBasePresentable
{
private:
	sf::Vector2i m_size;
	int m_index_offset;

protected:
	sf::Texture m_texture;  
	sf::Sprite m_sprite;

	void scale();
	int get_index_offset();

	// Override from base class:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

public:
	FeImage();
	~FeImage();

	void loadFromFile( const std::string &filename );
	sf::Vector2f getSize();
	sf::Vector2f getPosition();
	float getRotation();
	sf::Color getColor();
	void setSize( int, int );
	void setPosition( int, int );
	void setColor( sf::Color );

	// Overrides from base class:
	//
   int process_setting( const std::string &setting,
                        const std::string &value,
								const std::string &fn );

   const sf::Drawable &drawable() { return (const sf::Drawable &)*this; };
};

//
// Class representing an artwork
//
class FeArtwork : public FeImage
{
private:
	std::string m_artwork_label;

public:
	FeArtwork( const std::string &artwork_label );
	~FeArtwork();

	// Overrides from base class:
	//
	void on_new_selection( FeSettings * );
};

#ifndef NO_MOVIE

//
// Class representing a movie
//
class FeMovie : public FeArtwork
{
private:
	enum Status
	{
		Loading,		// short loading time so that we don't play movie 
						// if the user is just scrolling past

		Playing,		// play movie
		NoPlay		// don't play this movie, show image instead
	};

	FeMedia *m_movie;
	Status m_status;

	// Overridden from base class
	//
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

public:
	FeMovie( const std::string & );
	FeMovie( const FeMovie & );
	~FeMovie();

	void set_play_state( bool );
	void set_vol( float );

	// Overrides from base class
	//
	bool tick( FeSettings * ); // returns true if display refresh required
   void on_new_selection( FeSettings * );
};

#endif // NO_MOVIE

//
// Class representing a layout animation
//
class FeAnimate : public FeImage
{
private:

#ifndef NO_MOVIE
	FeMedia m_animate;
#endif

	int m_freq;
	bool m_playing;

	// Overridden from base class
	//
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

public:
	FeAnimate( const std::string &filename );

	void set_play_state( bool );
	void set_vol( float );
	bool tick( FeSettings * ); // returns true if display refresh required
};


class FeScreenSaver : public sf::Drawable
{
public:
	FeScreenSaver( FeSettings *fes );

	void tick();
	void enable( bool );
	bool is_enabled() { return m_is_enabled; }

private:
	bool m_is_enabled;
	bool m_show_movie;
	FeImage m_image;
	sf::Clock m_timer;	
#ifndef NO_MOVIE
	FeMedia m_movie;
#endif
	FeSettings *m_feSettings;

	void prep_next();

	// override from base
	void draw( sf::RenderTarget &target, sf::RenderStates states ) const;
};

#endif
