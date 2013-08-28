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

#include "fe_image.hpp"
#include "fe_util.hpp"
#include "fe_settings.hpp"
#include "media.hpp"
#include <iostream>
#include <cstdlib>

FeTextureContainer::FeTextureContainer()
	: m_index_offset( 0 ), 
	m_is_artwork( false ),
	m_movie( NULL ),
	m_movie_status( NoPlay )
{
}

FeTextureContainer::FeTextureContainer( 
	bool is_artwork, 
	const std::string &name )
	: m_name( name ),
	m_index_offset( 0 ),
	m_is_artwork( is_artwork ),
	m_movie( NULL ),
	m_movie_status( NoPlay )
{
}

FeTextureContainer::~FeTextureContainer()
{
#ifndef NO_MOVIE
	if ( m_movie )
	{
		delete m_movie;
		m_movie=NULL;
	}
#endif
}

const sf::Texture &FeTextureContainer::load( const std::string &name )
{
#ifndef NO_MOVIE
	if ( m_movie )
	{
		delete m_movie;
		m_movie=NULL;
	}
#endif

	m_name = name;
	m_is_artwork = false;
	m_texture.loadFromFile( m_name );
	m_texture.setSmooth( true );

	return m_texture;
}

const sf::Texture &FeTextureContainer::get_texture()
{
#ifndef NO_MOVIE
	if (( m_is_artwork ) && ( m_movie ))
	{
		const sf::Texture *t = m_movie->get_texture();
		if ( t ) return *t;
	}
#endif

	return m_texture;
}

void FeTextureContainer::on_new_selection( FeSettings *feSettings )
{
	// We only need to do anything if this is flagged as an artwork
	//
	if ( !m_is_artwork )
		return;

	// if m_name is empty then check for the configured movie
	// artwork
	//
	std::string label;
	if ( m_name.empty() )
		label = feSettings->get_movie_artwork();
	else
		label = m_name;

#ifndef NO_MOVIE
	if ( ( m_movie_status != LockNoPlay )
		&& ( label.compare( feSettings->get_movie_artwork() ) == 0 ) )
	{
		// If a movie is running, close it and reset movie status
		//
		if (m_movie)
		{
			if (m_movie->is_playing())
				m_movie->stop();
	
			m_movie->close();
		}
		m_movie_status=Delayed;
	}
#endif

	m_texture = sf::Texture();

	if ( !label.empty() )
	{
		std::vector<std::string> file_list;
		feSettings->get_art_file( m_index_offset, label, file_list );

		for ( unsigned int i=0; i<file_list.size(); i++ )
		{
			if ( m_texture.loadFromFile( file_list[i] ) )
				break;
		}
	}

	m_texture.setSmooth( true );

	for ( unsigned int i=0; i < m_images.size(); i++ )
		m_images[i]->texture_changed();
}

bool FeTextureContainer::tick( FeSettings *feSettings )
{
#ifndef NO_MOVIE
	if (( m_movie_status == Playing )
		|| ( m_movie_status == Loading ))
	{
		bool ret = m_movie->tick();
		if (( m_movie_status == Loading ) && ( ret ))
		{
			m_movie_status = Playing;

			// Force a reload of the texture for all images that point to us
			//
			for ( unsigned int i=0; i < m_images.size(); i++ )
				m_images[i]->texture_changed();
		}
		return ret;
	}

	// We only need to do anything if we have the configured movie
	// artwork and movies are not running
	//
	if ( !m_is_artwork 
		|| ( m_movie_status == LockNoPlay )
		|| ( ( m_name.compare( feSettings->get_movie_artwork() ) != 0 ) 
			&& !m_name.empty() ))
		return false;

	if ( m_movie_status == Delayed )
	{
		std::string movie_file;
		std::vector <std::string> file_list;
		feSettings->get_movie_file( m_index_offset, file_list );

		for ( unsigned int i=0; i< file_list.size(); i++ )
		{
			if ( FeMedia::is_supported_media_file( file_list[i] ) )
			{
				movie_file = file_list[i];
				break;
			}
		}

		if ( movie_file.empty() )
		{
			m_movie_status = NoPlay;
		}
		else
		{
			if (m_movie == NULL)
			{
				//
				// We only want sound for movies when they are the
				// current selection.
				//
				if ( m_index_offset != 0 )
					m_movie = new FeMedia(FeMedia::Video);
				else
					m_movie = new FeMedia(FeMedia::AudioVideo);
			}

			if (!m_movie->openFromFile( movie_file ))
			{
				std::cout << "ERROR loading movie: " << movie_file << std::endl;
				m_movie_status = NoPlay;
			}
			else
			{
				m_movie->setVolume( feSettings->get_play_volume( FeSoundInfo::Movie ) );
				m_movie->play();
				m_movie_status = Loading;
			}
		}
	}
#endif

	return false;
}

void FeTextureContainer::set_play_state( bool play )
{
#ifndef NO_MOVIE
	if (m_movie && ( m_movie_status == Playing ))
	{
		if ( play )
			m_movie->play();
		else
			m_movie->stop();
	}
#endif
}

void FeTextureContainer::set_vol( float vol )
{
#ifndef NO_MOVIE
	if (m_movie)
		m_movie->setVolume( vol );
#endif
}

FeImage::FeImage( FeTextureContainer *tc )
	: m_tex( tc ), 
	m_size( 0, 0 ), 
	m_shear( 0, 0 )
{
	ASSERT( m_tex );

	if (!m_tex->m_is_artwork)
		m_sprite.setTexture( m_tex->load( m_tex->m_name ), true );

	m_tex->m_images.push_back( this );
}

FeImage::FeImage( FeImage *o )
	: m_tex( o->m_tex ), 
	m_sprite( o->m_sprite ),
	m_size( o->m_size ), 
	m_shear( o->m_shear )
{
	m_tex->m_images.push_back( this );
}

FeImage::~FeImage()
{
}

void FeImage::texture_changed()
{
	m_sprite.setTexture( m_tex->get_texture(), true );
	scale();
}

void FeImage::loadFromFile( const std::string &file )
{
  	m_sprite.setTexture( m_tex->load( file ), true );
}

int FeImage::getIndexOffset() const
{
	return m_tex->m_index_offset;
}

void FeImage::setIndexOffset( int io )
{
	m_tex->m_index_offset = io;
	script_do_update( m_tex );
}

void FeImage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (( m_shear.x != 0 ) || (m_shear.y != 0 ))
		states.transform *= sf::Transform( 
				1.f, (float)m_shear.x / (float)m_size.x, 0.f,
				(float)m_shear.y / (float)m_size.y, 1.f, 0.f,
				0.f, 0.f, 1.f );

	target.draw( m_sprite, states );
}

void FeImage::scale()
{
	sf::IntRect texture_rect = m_sprite.getTextureRect();
	bool scale=false;
	float scale_x( 1.0 ), scale_y( 1.0 );

	if ( m_size.x > 0.0 )
	{
		scale_x = (float) m_size.x / texture_rect.width;
		scale = true;
	}
	if ( m_size.y > 0.0 )
	{
		scale_y = (float) m_size.y / texture_rect.height;
		scale = true;
	}

	if ( scale )
		m_sprite.setScale( scale_x, scale_y );
}

const sf::Vector2f &FeImage::getPosition() const
{
	return m_sprite.getPosition();
}

const sf::Vector2f &FeImage::getSize() const
{
	return m_size;
}

void FeImage::setSize( const sf::Vector2f &s )
{
	m_size = s;
	scale();
	script_flag_redraw();
}

void FeImage::setPosition( const sf::Vector2f &p )
{
	m_sprite.setPosition( p );
	script_flag_redraw();
}

float FeImage::getRotation() const
{
	return m_sprite.getRotation();
}

void FeImage::setRotation( float r )
{
	m_sprite.setRotation( r );
	script_flag_redraw();
}

const sf::Color &FeImage::getColor() const
{
	return m_sprite.getColor();
}

void FeImage::setColor( const sf::Color &c )
{
	m_sprite.setColor( c );
	script_flag_redraw();
}

const sf::Vector2u FeImage::getTextureSize() const
{
	return m_tex->get_texture().getSize();
}

const sf::IntRect &FeImage::getTextureRect() const
{
	return m_sprite.getTextureRect();
}

void FeImage::setTextureRect( const sf::IntRect &r )
{
	m_sprite.setTextureRect( r );
	scale();
	script_flag_redraw();
}

bool FeImage::getMovieEnabled() const
{
	return ( (m_tex->m_movie_status != FeTextureContainer::LockNoPlay) 
		? true : false );
}

void FeImage::setMovieEnabled( bool f )
{
	if ( f == false )
	{
#ifndef NO_MOVIE
		if ( m_tex->m_movie )
		{
			delete m_tex->m_movie;
			m_tex->m_movie=NULL;
		}
#endif
		m_tex->m_movie_status = FeTextureContainer::LockNoPlay;

		for ( unsigned int i=0; i < m_tex->m_images.size(); i++ )
			m_tex->m_images[i]->texture_changed();

		script_flag_redraw();
	}
	else
	{
		if ( m_tex->m_movie_status == FeTextureContainer::LockNoPlay )
		{
			m_tex->m_movie_status = FeTextureContainer::NoPlay;
			script_flag_redraw();
		}
	}
}

int FeImage::get_shear_x()
{
	return m_shear.x;
}

int FeImage::get_shear_y()
{
	return m_shear.y;
}

void FeImage::set_shear_x( int x )
{
	m_shear.x = x;
	script_flag_redraw();
}

void FeImage::set_shear_y( int y )
{
	m_shear.y = y;
	script_flag_redraw();
}

int FeImage::get_texture_width()
{
	return getTextureSize().x;
}

int FeImage::get_texture_height()
{
	return getTextureSize().y;
}

int FeImage::get_subimg_x()
{
	return getTextureRect().left;
}

int FeImage::get_subimg_y()
{
	return getTextureRect().top;
}

int FeImage::get_subimg_width()
{
	return getTextureRect().width;
}

int FeImage::get_subimg_height()
{
	return getTextureRect().height;
}

void FeImage::set_subimg_x( int x )
{
	sf::IntRect r = getTextureRect();
	r.left=x;
	setTextureRect( r );
}

void FeImage::set_subimg_y( int y )
{
	sf::IntRect r = getTextureRect();
	r.top=y;
	setTextureRect( r );
}

void FeImage::set_subimg_width( int w )
{
	sf::IntRect r = getTextureRect();
	r.width=w;
	setTextureRect( r );
}

void FeImage::set_subimg_height( int h )
{
	sf::IntRect r = getTextureRect();
	r.height=h;
	setTextureRect( r );
}

