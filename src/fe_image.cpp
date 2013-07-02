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

FeImage::FeImage()
	: m_size( 0, 0 ), m_index_offset( 0 )
{
}

FeImage::~FeImage()
{
}

void FeImage::loadFromFile( const std::string &filename )
{
   m_texture.loadFromFile( filename );
  	m_texture.setSmooth( true );
  	m_sprite.setTexture( m_texture, true );
}

int FeImage::get_index_offset()
{
	return m_index_offset;
}

void FeImage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw( m_sprite, states );
}

int FeImage::process_setting( const std::string &setting,
                        const std::string &value,
								const std::string &fn )
{
   size_t pos=0;
   std::string val;

	const char *stokens[] =
	{
		"position",
		"size",	
		"index_offset",
		"rotation",
		"colour",
		NULL
	};

   if ( setting.compare( stokens[0] ) == 0 ) // position
	{
		// position is XX,YY
   	token_helper( value, pos, val, "," );
		int left = as_int( val );
   	token_helper( value, pos, val );
		int top = as_int( val );

		m_sprite.setPosition( left, top );
	}
   else if ( setting.compare( stokens[1] ) == 0 ) // size
	{
		// size is WW,HH
   	token_helper( value, pos, val, ",x" );
		m_size.x = as_int( val );
   	token_helper( value, pos, val );
		m_size.y = as_int( val );

		scale();
	}
   else if ( setting.compare( stokens[2] ) == 0 ) // index_offset
		m_index_offset = as_int( value );
   else if ( setting.compare( stokens[3] ) == 0 ) // rotation
	{
		int rotation = as_int( value );
		m_sprite.setRotation( rotation );
	}
   else if ( setting.compare( stokens[4] ) == 0 ) // colour
		m_sprite.setColor( colour_helper( value ) );
	else
	{
		invalid_setting( fn, "image", setting, stokens );
      return 1;
	}

	return 0;
}

void FeImage::scale()
{
	sf::Vector2u texture_size = m_texture.getSize();
   bool scale=false;
   float scale_x( 1.0 ), scale_y( 1.0 );

	if ( m_size.x > 0 )
	{
		scale_x = (float) m_size.x / texture_size.x;
		scale = true;
	}
	if ( m_size.y > 0 )
	{
		scale_y = (float) m_size.y / texture_size.y;
		scale = true;
	}

	if ( scale )
		m_sprite.setScale( scale_x, scale_y );
}

sf::Vector2f FeImage::getPosition()
{
	return m_sprite.getPosition();
}

sf::Vector2f FeImage::getSize()
{
	return sf::Vector2f( m_size.x, m_size.y );
}

void FeImage::setSize( int x, int y )
{
	m_size = sf::Vector2i( x, y );
	scale();
}
void FeImage::setPosition( int x, int y )
{
	m_sprite.setPosition( x, y );
}

float FeImage::getRotation()
{
	return m_sprite.getRotation();
}

sf::Color FeImage::getColor()
{
	return m_sprite.getColor();
}

void FeImage::setColor( sf::Color c )
{
	m_sprite.setColor( c );
}

FeArtwork::FeArtwork( const std::string &artwork )
	: FeImage(), m_artwork_label( artwork )
{
}

FeArtwork::~FeArtwork()
{
}

void FeArtwork::on_new_selection( FeSettings *feSettings )
{
  	std::vector<std::string> file_list;
  	feSettings->get_art_file( 
						get_index_offset(), 	
						m_artwork_label,
						file_list );

  	m_texture = sf::Texture();
	for ( unsigned int i=0; i<file_list.size(); i++ )
	{
		if ( m_texture.loadFromFile( file_list[i] ) )
			break;
	}
  	m_texture.setSmooth( true );

	m_sprite.setTexture( m_texture, true );
	scale();
}

#ifndef NO_MOVIE 

FeMovie::FeMovie( const std::string &label )
	: FeArtwork( label ), m_movie( NULL ), m_status( NoPlay )
{
}

FeMovie::FeMovie( const FeMovie &c )
	: FeArtwork( c )
{
	m_status = c.m_status;
	m_movie = NULL; // don't copy the movie reference
}

FeMovie::~FeMovie()
{
	if (m_movie)
		delete m_movie;
}

void FeMovie::on_new_selection( FeSettings *feSettings )
{
	if (m_movie)
	{
		if (m_movie->is_playing())
			m_movie->stop();

		m_movie->close();
	}

	m_status=Loading;
	return FeArtwork::on_new_selection( feSettings );
}

bool FeMovie::tick( FeSettings *feSettings )
{
	bool ret_val = false;

	if ( m_status == Loading )
	{
		std::string movie_file;
		std::vector <std::string> file_list;
		feSettings->get_movie_file( get_index_offset(), file_list );

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
			m_status = NoPlay;
		}
		else
		{
			if (m_movie == NULL)
			{
				//
				// We only want sound for movies when they are the
				// current selection. 
				//
				if ( get_index_offset() != 0 )
					m_movie = new FeMedia(FeMedia::Video);
				else
					m_movie = new FeMedia(FeMedia::AudioVideo);
			}

			if (!m_movie->openFromFile( movie_file ))
			{
				std::cout << "ERROR loading movie: " << movie_file << std::endl;
				m_status = NoPlay;
			}
			else
			{
				m_movie->setPosition( getPosition() );
				m_movie->setSize( getSize() );
				m_movie->setRotation( getRotation() );
				m_movie->setColor( getColor() );
				m_movie->setVolume( feSettings->get_play_volume( FeSoundInfo::Movie ) );
				m_movie->play();
				m_status = Playing;
			}
		}
	}

	if ( m_status == Playing )
		ret_val = m_movie->tick();

	return ret_val;
}

void FeMovie::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	if (m_movie && ( m_status == Playing ) && ( m_movie->get_display_ready() ))
		target.draw( *m_movie, states );
	else
		FeArtwork::draw( target, states );
}

void FeMovie::set_play_state( bool play )
{
	if (m_movie && ( m_status == Playing ))
	{
		if ( play )
			m_movie->play();
		else
			m_movie->stop();
	}
}

void FeMovie::set_vol( float vol )
{
	if (m_movie)
		m_movie->setVolume( vol );
}
#endif

void FeAnimate::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
#ifndef NO_MOVIE
	if ( m_playing && m_animate.get_display_ready() )
	{
		target.draw( m_animate, states );
	}
#endif
}

FeAnimate::FeAnimate( const std::string &filename )
	: 
#ifndef NO_MOVIE
	m_animate( FeMedia::AudioVideo ), 
#endif
	m_freq( 100 ), m_playing( false )
{
#ifndef NO_MOVIE
	m_animate.openFromFile( filename );
#endif
}

void FeAnimate::set_play_state( bool play )
{
/*
	m_playing = play;

	if ( m_playing )
		m_animate.play();
	else
		m_animate.stop();
*/
}

void FeAnimate::set_vol( float vol )
{
#ifndef NO_MOVIE
	m_animate.setVolume( vol );
#endif
}

bool FeAnimate::tick( FeSettings *fes )
{
#ifndef NO_MOVIE
	if ( m_playing == false )
	{
		// Determine if we want to start the animation.  m_freq is the
		// % chance we will restart
		//
		bool restart=false;
		if ( m_freq == 100 )
			restart=true;
		else
			restart = (( rand() % 100 ) < m_freq );

		if ( restart )
		{
			m_animate.setPosition( getPosition() );
			m_animate.setSize( getSize() );
			m_animate.setRotation( getRotation() );
			m_animate.setColor( getColor() );
			m_animate.setVolume( fes->get_play_volume( FeSoundInfo::Movie ) );
			m_animate.setLoop( false );

			m_animate.play();
			m_playing = true;
			return true;
		}
	}
	else
		return m_animate.tick(); // playing 

#endif
	return false;
}


FeScreenSaver::FeScreenSaver( FeSettings *fes )
	: m_is_enabled( false ),
#ifndef NO_MOVIE
	m_show_movie( false ),
	m_movie( FeMedia::AudioVideo ),
#endif
	m_feSettings( fes )
{
	srand( m_feSettings->get_rom_index() );
}

void FeScreenSaver::prep_next()
{
	m_timer.restart();

#ifndef NO_MOVIE
	m_show_movie = (( rand() % 100 ) < 15 );
	if ( m_show_movie )
	{
		std::vector<std::string> mlist;
		m_feSettings->get_movie_file( rand(), mlist );
		if ( !mlist.empty() )
		{
			sf::VideoMode vm = sf::VideoMode::getDesktopMode();

			m_movie.openFromFile( mlist.front() );
			m_movie.setSize( sf::Vector2f( vm.width, vm.height ) );
			m_movie.setLoop( false );
			m_movie.play();
		}
		return;
	}
#endif

	std::vector<std::string> ilist;
	m_feSettings->get_art_file( rand(),
				m_feSettings->get_movie_artwork(), 
				ilist );
	if ( !ilist.empty() )
	{
		sf::VideoMode vm = sf::VideoMode::getDesktopMode();

		m_image.loadFromFile( ilist.front() );
		m_image.setSize( vm.width, vm.height );
	}
}

void FeScreenSaver::enable( bool e )
{
	m_is_enabled = e;

	if ( m_is_enabled )
	{
		prep_next();
	}
	else
	{
#ifndef NO_MOVIE
		m_movie.stop();
		m_show_movie = false;
#endif
	}
}

void FeScreenSaver::tick()
{
#ifndef NO_MOVIE
	if ( m_show_movie )
	{
		m_movie.tick();

		if ( m_movie.is_playing() == false )
		{
			m_show_movie = false;
			prep_next();
		}
		return;
	}
#endif

	if ( m_timer.getElapsedTime().asSeconds() > 5 )
		prep_next();
	
}

void FeScreenSaver::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
#ifndef NO_MOVIE
	if ( m_show_movie && m_movie.get_display_ready() )
	{
		target.draw( m_movie, states );
		return;
	}
#endif

	target.draw( m_image, states );
}
