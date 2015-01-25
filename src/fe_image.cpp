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
#include "fe_shader.hpp"
#include "fe_present.hpp"

#ifndef NO_MOVIE
#include "media.hpp"
#endif

#include <iostream>
#include <cstdlib>

FeBaseTextureContainer::FeBaseTextureContainer()
{
}

FeBaseTextureContainer::~FeBaseTextureContainer()
{
}

void FeBaseTextureContainer::set_play_state( bool play )
{
}

bool FeBaseTextureContainer::get_play_state() const
{
	return false;
}

void FeBaseTextureContainer::set_vol( float vol )
{
}

void FeBaseTextureContainer::set_index_offset( int io, bool do_update )
{
}

int FeBaseTextureContainer::get_index_offset() const
{
	return 0;
}

void FeBaseTextureContainer::set_filter_offset( int fo, bool do_update )
{
}

int FeBaseTextureContainer::get_filter_offset() const
{
	return 0;
}

void FeBaseTextureContainer::set_video_flags( FeVideoFlags f )
{
}

FeVideoFlags FeBaseTextureContainer::get_video_flags() const
{
	return VF_Normal;
}

int FeBaseTextureContainer::get_video_duration() const
{
	return 0;
}

int FeBaseTextureContainer::get_video_time() const
{
	return 0;
}

void FeBaseTextureContainer::set_file_name( const char *n )
{
}

const char *FeBaseTextureContainer::get_file_name() const
{
	return NULL;
}

void FeBaseTextureContainer::set_trigger( int t )
{
}

int FeBaseTextureContainer::get_trigger() const
{
	return ToNewSelection;
}

void FeBaseTextureContainer::transition_swap( FeBaseTextureContainer *o )
{
	//
	// Swap image lists
	//
	m_images.swap( o->m_images );

	//
	// Now update the images to point at their new parent textures.
	// texture_changed() will also cause them to update their sf::Sprite
	// accordingly
	//
	std::vector< FeImage * >::iterator itr;

	for ( itr = m_images.begin(); itr != m_images.end(); ++itr )
		(*itr)->texture_changed( this );

	for ( itr = o->m_images.begin(); itr != o->m_images.end(); ++itr )
		(*itr)->texture_changed( o );
}

FeTextureContainer *FeBaseTextureContainer::get_derived_texture_container()
{
	return NULL;
}

FeImage *FeBaseTextureContainer::add_image(const char *,int, int, int, int)
{
	return NULL;
}

FeImage *FeBaseTextureContainer::add_artwork(const char *,int, int, int, int)
{
	return NULL;
}

FeImage *FeBaseTextureContainer::add_clone(FeImage *)
{
	return NULL;
}

FeText *FeBaseTextureContainer::add_text(const char *,int, int, int, int)
{
	return NULL;
}

FeListBox *FeBaseTextureContainer::add_listbox(int, int, int, int)
{
	return NULL;
}

FeImage *FeBaseTextureContainer::add_surface(int, int)
{
	return NULL;
}

void FeBaseTextureContainer::register_image( FeImage *img )
{
	m_images.push_back( img );
}

void FeBaseTextureContainer::notify_texture_change()
{
	for ( std::vector<FeImage *>::iterator itr=m_images.begin();
			itr != m_images.end(); ++itr )
		(*itr)->texture_changed();
}

namespace
{
	//
	// The number of "ticks" after a video is first loaded before
	// playing starts.  This should be 3 or higher (freezing has
	// been experienced at 2 when returning from games).
	//
	const int PLAY_COUNT=5;
};

FeTextureContainer::FeTextureContainer(
	bool is_artwork,
	const std::string &art_name )
	: m_index_offset( 0 ),
	m_filter_offset( 0 ),
	m_current_rom_index( -1 ),
	m_current_filter_index( -1 ),
	m_is_artwork( is_artwork ),
	m_art_update_trigger( ToNewSelection ),
	m_movie( NULL ),
	m_movie_status( -1 ),
	m_video_flags( VF_Normal )
{
	if ( m_is_artwork )
	{
		if ( art_name.empty() )
			m_art_name = FE_DEFAULT_ARTWORK;
		else
			m_art_name = art_name;
	}
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

bool FeTextureContainer::load_static(
	const std::string &file_name )
{
	clear();

	std::vector<std::string> image_names;
	std::vector<std::string> non_image_names;

	int i=0;
	while ( FE_ART_EXTENSIONS[i] != NULL )
	{
		if ( tail_compare( file_name, FE_ART_EXTENSIONS[i] ) )
		{
			image_names.push_back( file_name );
			break;
		}
		i++;
	}

	if ( image_names.empty() )
		non_image_names.push_back( file_name );

	bool retval = common_load( non_image_names, image_names );

	notify_texture_change();
	return retval;
}

bool FeTextureContainer::load_artwork(
	const std::vector < std::string > &art_paths,
	const std::string &target_name,
	bool ignore_images )
{
	for ( std::vector< std::string >::const_iterator itr = art_paths.begin();
			itr != art_paths.end(); ++itr )
	{
		std::vector<std::string> non_image_names;
		std::vector<std::string> image_names;
		std::vector<std::string> dummy;

		get_filename_from_base(
			image_names,
			non_image_names,
			(*itr),
			target_name + '.',
			FE_ART_EXTENSIONS );

		if ( common_load( non_image_names,
				ignore_images ? dummy : image_names ) )
			return true;

		//
		// If there is a subdirectory in art_path with the
		// given target name, then we load a random video or
		// image from it at this point (if available)
		//
		std::string sd_path = (*itr) + target_name;
		if ( directory_exists( sd_path ) )
		{
			sd_path += "/";

			get_filename_from_base(
				image_names,
				non_image_names,
				sd_path,
				"",
				FE_ART_EXTENSIONS );

			std::random_shuffle( non_image_names.begin(), non_image_names.end() );
			std::random_shuffle( image_names.begin(), image_names.end() );

			if ( common_load( non_image_names,
					ignore_images ? dummy : image_names ) )
				return true;
		}
	}
	return false;
}

bool FeTextureContainer::common_load(
	std::vector<std::string> &non_image_names,
	std::vector<std::string> &image_names )
{
	bool loaded=false;
#ifndef NO_MOVIE

	ASSERT( !m_movie ); // m_movie should always be empty at this point...

	if ( !(m_video_flags & VF_DisableVideo) )
	{
		std::string movie_file;
		for ( std::vector<std::string>::iterator itr = non_image_names.begin();
				itr != non_image_names.end(); ++itr  )
		{
			if ( FeMedia::is_supported_media_file( *itr ) )
			{
				movie_file = (*itr);
				break;
			}
		}

		if ( !movie_file.empty() )
 		{
			// We should have deleted this above...
			ASSERT( m_movie == NULL );

			m_movie = new FeMedia( FeMedia::AudioVideo );

			if (!m_movie->openFromFile( movie_file ))
			{
				std::cout << "ERROR loading video: "
					<< movie_file << std::endl;
			}
			else
			{
				loaded = true;
				m_file_name = movie_file;

				if ( m_movie->number_of_frames() == 1 )
				{
					m_movie_status = -1; // don't play if there is only one frame

					// if there is only one frame, then we can update the texture immediately
					// (the frame will have been preloaded) and delete our movie object now
					notify_texture_change();
					delete m_movie;
					m_movie = NULL;
				}
				else if (m_video_flags & VF_NoAutoStart)
					m_movie_status = 0; // 0=loaded but not on track to play
				else
					m_movie_status = 1; // 1=on track to be played
			}
		}
	}
#endif

	if ( !loaded )
	{
		for ( std::vector<std::string>::iterator itr=image_names.begin();
			itr != image_names.end(); ++itr )
		{
			if ( m_texture.loadFromFile( *itr ) )
			{
				m_texture.setSmooth( true );
				m_file_name = *itr;
				loaded = true;
				break;
			}
		}
	}

	non_image_names.clear();
	image_names.clear();

	return loaded;
}

const sf::Texture &FeTextureContainer::get_texture()
{
#ifndef NO_MOVIE
	if ( m_movie )
	{
		const sf::Texture *t = m_movie->get_texture();
		if ( t ) return *t;
	}
#endif

	return m_texture;
}

void FeTextureContainer::on_new_selection( FeSettings *feSettings, bool screen_saver_active )
{
	if (( m_is_artwork ) && ( m_art_update_trigger == ToNewSelection ))
		internal_update_selection( feSettings, screen_saver_active );
}

void FeTextureContainer::on_new_list( FeSettings *feSettings, float, float, bool screen_saver_active )
{
	if (( m_is_artwork ) && ( m_art_update_trigger == EndNavigation ))
		internal_update_selection( feSettings, screen_saver_active );
}

void FeTextureContainer::on_end_navigation( FeSettings *feSettings, bool screen_saver_active )
{
	if (( m_is_artwork ) && ( m_art_update_trigger == EndNavigation ))
		internal_update_selection( feSettings, screen_saver_active );
}

void FeTextureContainer::internal_update_selection( FeSettings *feSettings, bool screen_saver_active )
{
	int filter_index = feSettings->get_filter_index_from_offset( m_filter_offset );
	int rom_index = feSettings->get_rom_index( filter_index, m_index_offset );

	//
	// Optimization opportunity: We could already be showing the artwork for rom_index if the
	// layout uses the image swap() function... if we are, then there is no need to do anything...
	//
	if (( m_current_rom_index == rom_index )
				&& ( m_current_filter_index == filter_index ))
	{
#ifdef FE_DEBUG
		std::cout << "Optimization: " << m_file_name << " not loaded." << std::endl;
#endif
		return;
	}

	m_current_rom_index = rom_index;
	m_current_filter_index = filter_index;

	clear();

	const std::string &emu_name
		= feSettings->get_rom_info_absolute( filter_index, rom_index, FeRomInfo::Emulator );

	std::vector<std::string> movie_list;
	std::vector<std::string> image_list;

	std::string layout_path;
	if ( screen_saver_active )
	{
		std::string dummy;
		feSettings->get_screensaver_file( layout_path, dummy );
	}
	else
	{
		layout_path = feSettings->get_current_layout_dir();
	}

	FeEmulatorInfo *emu_info = feSettings->get_emulator( emu_name );

	std::vector < std::string > art_paths;
	if ( emu_info )
	{
		std::vector < std::string > temp_list;
		emu_info->get_artwork( m_art_name, temp_list );
		for ( std::vector< std::string >::iterator itr = temp_list.begin();
				itr != temp_list.end(); ++itr )
		{
			art_paths.push_back( clean_path( (*itr), true ) );
			perform_substitution( art_paths.back(), "$LAYOUT", layout_path );
		}
	}

	if ( !art_paths.empty() )
	{
		const std::string &romname = feSettings->get_rom_info_absolute( filter_index, rom_index, FeRomInfo::Romname );
		const std::string &altname = feSettings->get_rom_info_absolute( filter_index, rom_index, FeRomInfo::AltRomname );
		const std::string &cloneof = feSettings->get_rom_info_absolute( filter_index, rom_index, FeRomInfo::Cloneof );

		// test for "romname" specific videos
		if ( load_artwork( art_paths, romname, true ) )
			goto the_end;

		bool check_altname = ( !altname.empty() && ( romname.compare( altname ) != 0 ));

		// test for "altname" specific videos
		if ( check_altname && load_artwork( art_paths, altname, true ) )
			goto the_end;

		bool check_cloneof = ( !cloneof.empty() && (altname.compare( cloneof ) != 0 ));

		// then "cloneof" specific videos
		if ( check_cloneof && load_artwork( art_paths, cloneof, true ) )
			goto the_end;

		// test for "romname" specific images
		if ( load_artwork( art_paths, romname ) )
			goto the_end;

		// test for "altname" specific images
		if ( check_altname && load_artwork( art_paths, altname ) )
			goto the_end;

		// then "cloneof" specific images
		if ( check_cloneof && load_artwork( art_paths, cloneof ) )
			goto the_end;

		// then "emulator"
		if ( !emu_name.empty() && load_artwork( art_paths, emu_name ) )
			goto the_end;
	}

	if ( !layout_path.empty() )
	{
		std::vector< std::string > layout_paths;
		layout_paths.push_back( layout_path );

		if ( !emu_name.empty() )
		{
			// check for "[emulator]-[artlabel]" in layout directory
			if ( load_artwork( layout_paths, emu_name + "-" + m_art_name ) )
				goto the_end;
		}

		// check for file named with the artwork label in layout directory
		if ( load_artwork( layout_paths, m_art_name ) )
			goto the_end;
	}

the_end:
	//
	// Texture was replaced, so notify the attached images
	//
	notify_texture_change();
}

bool FeTextureContainer::tick( FeSettings *feSettings, bool play_movies, bool ok_to_start )
{
#ifndef NO_MOVIE
	if (( play_movies )
		&& ( !(m_video_flags & VF_DisableVideo) )
		&& ( m_movie ))
	{
		if (( m_movie_status > 0 )
			&& ( m_movie_status < PLAY_COUNT )
			&& ( ok_to_start ))
		{
			//
			// We skip the first few "ticks" after the movie
			// is first loaded because the user may just be
			// scrolling rapidly through the game list (there
			// are ticks between each selection scrolling by)
			//
			m_movie_status++;
			return false;
		}
		else if (( m_movie_status == PLAY_COUNT ) && ( ok_to_start ))
		{
			m_movie_status++;

			//
			// Start playing now if this is a video...
			//
			if ( m_video_flags & VF_NoAudio )
				m_movie->setVolume( 0.f );
			else
				m_movie->setVolume(feSettings->get_play_volume( FeSoundInfo::Movie ) );

			m_movie->setLoop( !(m_video_flags & VF_NoLoop) );
			m_movie->play();
		}
		return m_movie->tick();
	}
#endif

	return false;
}

void FeTextureContainer::set_play_state( bool play )
{
#ifndef NO_MOVIE
	if (m_movie)
	{
		if ( m_movie_status >= PLAY_COUNT )
		{
			if ( play )
				m_movie->play();
			else
				m_movie->stop();
		}
		else if ( m_movie_status >= 0 )
		{
			// m_movie_status is 0 if a movie is loaded but the VF_NoAutoStart flag is set.
			// If movie is in this state and user wants to play then put it on track to be played.
			//
			if (( m_movie_status == 0 ) && ( play ))
				m_movie_status = 1;
			else if ( !play )
				m_movie_status = 0;
		}
	}
#endif
}

bool FeTextureContainer::get_play_state() const
{
#ifndef NO_MOVIE
	if ( m_movie )
	{
		if ( m_movie_status >= PLAY_COUNT )
			return m_movie->is_playing();
		else
			// if status > 0, we are in the process of starting to play
			return ( m_movie_status > 0 );
	}
#endif

	return false;
}

void FeTextureContainer::set_vol( float vol )
{
#ifndef NO_MOVIE
	if ( (m_movie) && !(m_video_flags & VF_NoAudio) )
		m_movie->setVolume( vol );
#endif
}

void FeTextureContainer::set_index_offset( int io, bool do_update )
{
	if ( m_index_offset != io )
	{
		m_index_offset = io;

		if ( do_update )
			FePresent::script_do_update( this );
	}
}

int FeTextureContainer::get_index_offset() const
{
	return m_index_offset;
}

void FeTextureContainer::set_filter_offset( int fo, bool do_update )
{
	if ( m_filter_offset != fo )
	{
		m_filter_offset = fo;

		if ( do_update )
			FePresent::script_do_update( this );
	}
}

int FeTextureContainer::get_filter_offset() const
{
	return m_filter_offset;
}

void FeTextureContainer::set_video_flags( FeVideoFlags f )
{
	m_video_flags = f;

#ifndef NO_MOVIE
	if ( m_movie )
	{
		m_movie->setLoop( !(m_video_flags & VF_NoLoop) );

		if (m_video_flags & VF_NoAudio)
			m_movie->setVolume( 0.f );
		else
		{
			float volume( 100.f );
			FePresent *fep = FePresent::script_get_fep();
			if ( fep )
				volume = fep->get_fes()->get_play_volume( FeSoundInfo::Movie );

			m_movie->setVolume( volume );
		}
	}
#endif
}

FeVideoFlags FeTextureContainer::get_video_flags() const
{
	return m_video_flags;
}

int FeTextureContainer::get_video_duration() const
{
#ifndef NO_MOVIE
	if ( m_movie )
		return m_movie->get_duration().asMilliseconds();
#endif

	return 0;
}

int FeTextureContainer::get_video_time() const
{
#ifndef NO_MOVIE
	if ( m_movie )
		return m_movie->get_video_time().asMilliseconds();
#endif

	return 0;
}

void FeTextureContainer::set_file_name( const char *n )
{
	std::string name = n;

	if ( name.empty() )
	{
		clear();
		notify_texture_change();
		return;
	}

	// If it is a relative path we assume it is in the layout directory
	//
	if ( is_relative_path( name ) )
	{
		FePresent *fep = FePresent::script_get_fep();
		if ( fep )
		{
			FeSettings *fes = fep->get_fes();
			if ( fes )
			{
				name = fes->get_current_layout_dir() + std::string( n );
			}
		}
	}

	if ( m_file_name.compare( name ) != 0 )
		load_static( name );
}

const char *FeTextureContainer::get_file_name() const
{
	return m_file_name.c_str();
}

void FeTextureContainer::set_trigger( int t )
{
	m_art_update_trigger = t;
}

int FeTextureContainer::get_trigger() const
{
	return m_art_update_trigger;
}


void FeTextureContainer::transition_swap( FeBaseTextureContainer *o )
{
	FeTextureContainer *o_up = o->get_derived_texture_container();
	if ( o_up )
	{
		m_art_name.swap( o_up->m_art_name );
		std::swap( m_index_offset, o_up->m_index_offset );
		std::swap( m_filter_offset, o_up->m_filter_offset );
		std::swap( m_is_artwork, o_up->m_is_artwork );
		std::swap( m_art_update_trigger, o_up->m_art_update_trigger );
	}

	FeBaseTextureContainer::transition_swap( o );
}

FeTextureContainer *FeTextureContainer::get_derived_texture_container()
{
	return this;
}

void FeTextureContainer::clear()
{
	m_texture = sf::Texture();
	m_movie_status = -1;
	m_file_name.clear();

#ifndef NO_MOVIE
	// If a movie is running, close it...
	if ( m_movie )
	{
		delete m_movie;
		m_movie=NULL;
	}
#endif
}

FeSurfaceTextureContainer::FeSurfaceTextureContainer( int width, int height )
{
	m_texture.create( width, height );
	m_texture.setSmooth( true );
}

FeSurfaceTextureContainer::~FeSurfaceTextureContainer()
{
	while ( !m_draw_list.empty() )
	{
		FeBasePresentable *p = m_draw_list.back();
		m_draw_list.pop_back();
		delete p;
	}
}

const sf::Texture &FeSurfaceTextureContainer::get_texture()
{
	return m_texture.getTexture();
}

void FeSurfaceTextureContainer::on_new_selection( FeSettings *s, bool screen_saver_active )
{
	for ( std::vector<FeBasePresentable *>::iterator itr = m_draw_list.begin();
				itr != m_draw_list.end(); ++itr )
		(*itr)->on_new_selection( s );
}

void FeSurfaceTextureContainer::on_end_navigation( FeSettings *feSettings, bool screen_saver_active )
{
}

void FeSurfaceTextureContainer::on_new_list( FeSettings *s, float scale_x, float scale_y, bool )
{
	//
	// The scale factors passed to this function are ignored on purpose.
	//
	// We don't do any scaling of the objects when they are being drawn
	// to the surface.
	//
	for ( std::vector<FeBasePresentable *>::iterator itr = m_draw_list.begin();
				itr != m_draw_list.end(); ++itr )
		(*itr)->on_new_list( s, 1.f, 1.f );
}

bool FeSurfaceTextureContainer::tick( FeSettings *feSettings, bool play_movies, bool ok_to_start )
{
	//
	// Draw the surface's draw list to the render texture
	//
	m_texture.clear( sf::Color::Transparent );
	for ( std::vector<FeBasePresentable *>::const_iterator itr = m_draw_list.begin();
				itr != m_draw_list.end(); ++itr )
		m_texture.draw( (*itr)->drawable() );

	m_texture.display();
	return true;
}

FeImage *FeSurfaceTextureContainer::add_image(const char *n, int x, int y, int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_image( false, n, x, y, w, h, m_draw_list );

	return NULL;
}

FeImage *FeSurfaceTextureContainer::add_artwork(const char *l, int x, int y, int w, int h )
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_image( true, l, x, y, w, h, m_draw_list );

	return NULL;
}

FeImage *FeSurfaceTextureContainer::add_clone(FeImage *i )
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_clone( i, m_draw_list );

	return NULL;
}

FeText *FeSurfaceTextureContainer::add_text(const char *t, int x, int y, int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_text( t, x, y, w, h, m_draw_list );

	return NULL;
}

FeListBox *FeSurfaceTextureContainer::add_listbox(int x, int y, int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_listbox( x, y, w, h, m_draw_list );

	return NULL;
}

FeImage *FeSurfaceTextureContainer::add_surface(int w, int h)
{
	FePresent *fep = FePresent::script_get_fep();

	if ( fep )
		return fep->add_surface( w, h, m_draw_list );

	return NULL;
}

FeImage::FeImage( FeBaseTextureContainer *tc, float x, float y, float w, float h )
	: FeBasePresentable(),
	m_tex( tc ),
	m_pos( x, y ),
	m_size( w, h ),
	m_preserve_aspect_ratio( false )
{
	ASSERT( m_tex );
	m_tex->register_image( this );
	scale();
}

FeImage::FeImage( FeImage *o )
	: FeBasePresentable(),
	m_tex( o->m_tex ),
	m_sprite( o->m_sprite ),
	m_pos( o->m_pos ),
	m_size( o->m_size ),
	m_preserve_aspect_ratio( o->m_preserve_aspect_ratio )
{
	m_tex->register_image( this );
}

FeImage::~FeImage()
{
}

const sf::Texture *FeImage::get_texture()
{
	if ( m_tex )
		return &(m_tex->get_texture());
	else
		return NULL;
}


void FeImage::texture_changed( FeBaseTextureContainer *new_tex )
{
	if ( new_tex )
		m_tex = new_tex;

	m_sprite.setTexture( m_tex->get_texture(), true );
	scale();
}

int FeImage::getIndexOffset() const
{
	return m_tex->get_index_offset();
}

void FeImage::setIndexOffset( int io )
{
	m_tex->set_index_offset( io );
}

int FeImage::getFilterOffset() const
{
	return m_tex->get_filter_offset();
}

void FeImage::setFilterOffset( int fo )
{
	m_tex->set_filter_offset( fo );
}

void FeImage::rawset_index_offset( int io )
{
	m_tex->set_index_offset( io, false );
}

void FeImage::rawset_filter_offset( int fo )
{
	m_tex->set_filter_offset( fo, false );
}


void FeImage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	FeShader *s = get_shader();
	if ( s )
	{
		const sf::Shader *sh = s->get_shader();
		if ( sh )
			states.shader = sh;
	}

	target.draw( m_sprite, states );
}

void FeImage::scale()
{
	sf::IntRect texture_rect = m_sprite.getTextureRect();

	if (( texture_rect.width == 0 ) || ( texture_rect.height == 0 ))
		return;

	bool scale=false;
	float scale_x( 1.0 ), scale_y( 1.0 );
	sf::Vector2f final_pos = m_pos;

	if ( m_size.x > 0.0 )
	{
		scale_x = (float) m_size.x / abs( texture_rect.width );

		if ( m_preserve_aspect_ratio )
			scale_y = scale_x;

		scale = true;
	}

	if ( m_size.y > 0.0 )
	{
		scale_y = (float) m_size.y / abs( texture_rect.height );

		if ( m_preserve_aspect_ratio )
		{
			if ( m_size.x <= 0.0 )
			{
				scale_x = scale_y;
			}
			else
			{
				// We have a size set in both directions and are preserving the aspect
				// ratio, so calculate how we will centre the image in the space we have
				//
				if ( scale_x > scale_y ) // centre in x direction
					final_pos.x += ( m_size.x - ( abs( texture_rect.width ) * scale_y )) / 2.0;
				else // centre in y direction
					final_pos.y += ( m_size.y - ( abs( texture_rect.height ) * scale_x )) / 2.0;
			}
		}

		scale = true;
	}

	if ( m_preserve_aspect_ratio && ( scale_y != scale_x ))
	{
		if ( scale_y > scale_x )
			scale_y = scale_x;
		else
			scale_x = scale_y;
	}

	if ( scale )
		m_sprite.setScale( scale_x, scale_y );

	m_sprite.setPosition( final_pos );
}

const sf::Vector2f &FeImage::getPosition() const
{
	return m_pos;
}

const sf::Vector2f &FeImage::getSize() const
{
	return m_size;
}

void FeImage::setSize( const sf::Vector2f &s )
{
	m_size = s;
	scale();
	FePresent::script_flag_redraw();
}

void FeImage::setPosition( const sf::Vector2f &p )
{
	m_pos = p;
	scale();
	FePresent::script_flag_redraw();
}

float FeImage::getRotation() const
{
	return m_sprite.getRotation();
}

void FeImage::setRotation( float r )
{
	m_sprite.setRotation( r );
	FePresent::script_flag_redraw();
}

const sf::Color &FeImage::getColor() const
{
	return m_sprite.getColor();
}

void FeImage::setColor( const sf::Color &c )
{
	m_sprite.setColor( c );
	FePresent::script_flag_redraw();
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
	FePresent::script_flag_redraw();
}

int FeImage::getVideoFlags() const
{
	return (int)m_tex->get_video_flags();
}

void FeImage::setVideoFlags( int f )
{
	m_tex->set_video_flags( (FeVideoFlags)f );
}

bool FeImage::getVideoPlaying() const
{
	return m_tex->get_play_state();
}

void FeImage::setVideoPlaying( bool f )
{
	m_tex->set_play_state( f );
}

int FeImage::getVideoDuration() const
{
	return m_tex->get_video_duration();
}

int FeImage::getVideoTime() const
{
	return m_tex->get_video_time();
}

const char *FeImage::getFileName() const
{
	return m_tex->get_file_name();
}

void FeImage::setFileName( const char *n )
{
	m_tex->set_file_name( n );
}

int FeImage::getTrigger() const
{
	return m_tex->get_trigger();
}

void FeImage::setTrigger( int t )
{
	m_tex->set_trigger( t );
}

bool FeImage::getMovieEnabled() const
{
	return !(m_tex->get_video_flags() & VF_DisableVideo );
}

void FeImage::setMovieEnabled( bool f )
{
	int c = (int)m_tex->get_video_flags();

	if ( f )
		c |= VF_DisableVideo;
	else
		c &= ~VF_DisableVideo;

	m_tex->set_video_flags( (FeVideoFlags)c );
}

int FeImage::get_skew_x() const
{
	return m_sprite.getSkewX();
}

int FeImage::get_skew_y() const
{
	return m_sprite.getSkewY();
}

int FeImage::get_pinch_x() const
{
	return m_sprite.getPinchX();
}

int FeImage::get_pinch_y() const
{
	return m_sprite.getPinchY();
}

void FeImage::set_skew_x( int x )
{
	m_sprite.setSkewX( x );
	FePresent::script_flag_redraw();
}

void FeImage::set_skew_y( int y )
{
	m_sprite.setSkewY( y );
	FePresent::script_flag_redraw();
}

void FeImage::set_pinch_x( int x )
{
	m_sprite.setPinchX( x );
	FePresent::script_flag_redraw();
}

void FeImage::set_pinch_y( int y )
{
	m_sprite.setPinchY( y );
	FePresent::script_flag_redraw();
}

int FeImage::get_texture_width() const
{
	return getTextureSize().x;
}

int FeImage::get_texture_height() const
{
	return getTextureSize().y;
}

int FeImage::get_subimg_x() const
{
	return getTextureRect().left;
}

int FeImage::get_subimg_y() const
{
	return getTextureRect().top;
}

int FeImage::get_subimg_width() const
{
	return getTextureRect().width;
}

int FeImage::get_subimg_height() const
{
	return getTextureRect().height;
}

bool FeImage::get_preserve_aspect_ratio() const
{
	return m_preserve_aspect_ratio;
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

void FeImage::set_preserve_aspect_ratio( bool p )
{
	m_preserve_aspect_ratio = p;
}

void FeImage::transition_swap( FeImage *o )
{
	// if we're pointing at the same texture, don't do anything
	//
	if ( m_tex == o->m_tex )
		return;

	// otherwise swap the textures
	//
	m_tex->transition_swap( o->m_tex );
}

FeImage *FeImage::add_image(const char *n, int x, int y, int w, int h)
{
	return m_tex->add_image( n, x, y, w, h );
}

FeImage *FeImage::add_image(const char *n, int x, int y )
{
	return add_image( n, x, y, 0, 0 );
}

FeImage *FeImage::add_image(const char *n )
{
	return add_image( n, 0, 0, 0, 0 );
}

FeImage *FeImage::add_artwork(const char *l, int x, int y, int w, int h )
{
	return m_tex->add_artwork( l, x, y, w, h );
}

FeImage *FeImage::add_artwork(const char *l, int x, int y)
{
	return add_artwork( l, x, y, 0, 0 );
}

FeImage *FeImage::add_artwork(const char *l )
{
	return add_artwork( l, 0, 0, 0, 0 );
}

FeImage *FeImage::add_clone(FeImage *i )
{
	return m_tex->add_clone( i );
}

FeText *FeImage::add_text(const char *t, int x, int y, int w, int h)
{
	return m_tex->add_text( t, x, y, w, h );
}

FeListBox *FeImage::add_listbox(int x, int y, int w, int h)
{
	return m_tex->add_listbox( x, y, w, h );
}

FeImage *FeImage::add_surface(int w, int h)
{
	return m_tex->add_surface( w, h );
}
