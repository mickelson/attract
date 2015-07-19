/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-15 Andrew Mickelson
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
#include "zip.hpp"

#ifdef WITH_MOVIE
#include "media.hpp"
#endif

#ifdef WITH_SWF
#include "swf.hpp"
#endif

#include <iostream>
#include <cstdlib>

namespace
{
	bool gather_artwork_filenames_from_archive(
			const std::string &archive_name,
			const std::string &target_name,
			std::vector<std::string> vids,
			std::vector<std::string> images )
	{
		std::vector<std::string> zdir;
		fe_zip_get_dir( archive_name.c_str(), zdir );

		for ( std::vector<std::string>::iterator itr=zdir.begin();
				itr!=zdir.end(); ++itr )
		{
			if ( (*itr).compare( 0, target_name.size(), target_name ) == 0 )
			{
				int i=0;
				bool is_image=false;
				while ( FE_ART_EXTENSIONS[i] )
				{
					if ( tail_compare( *itr, FE_ART_EXTENSIONS[i] ) )
					{
						is_image=true;
						images.push_back( *itr );
						break;
					}
					i++;
				}

#ifdef WITH_MOVIE
				if ( !is_image && FeMedia::is_supported_media_file( *itr ) )
					vids.push_back( *itr );
#endif
			}
		}

		return ( !images.empty() || !vids.empty() );
	}
};

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

bool FeBaseTextureContainer::fix_masked_image()
{
	return false;
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
	m_art_update_trigger( ToNewSelection ),
	m_movie( NULL ),
	m_swf( NULL ),
	m_movie_status( -1 ),
	m_video_flags( VF_Normal )
{
	if ( is_artwork )
		m_type = IsArtwork;
	else if ( art_name.find_first_of( "[" ) != std::string::npos )
		m_type = IsDynamic;
	else
		m_type = IsStatic;

	if ( m_type != IsStatic )
	{
		m_art_name = art_name;

		if (( m_type == IsArtwork ) && m_art_name.empty() )
			m_art_name = FE_DEFAULT_ARTWORK;
	}
}

FeTextureContainer::~FeTextureContainer()
{
#ifdef WITH_MOVIE
	if ( m_movie )
	{
		delete m_movie;
		m_movie=NULL;
	}
#endif

#ifdef WITH_SWF
	if ( m_swf )
	{
		delete m_swf;
		m_swf=NULL;
	}
#endif
}

bool FeTextureContainer::fix_masked_image()
{
	bool retval=false;

	sf::Image tmp_img = m_texture.copyToImage();
	sf::Vector2u tmp_s = tmp_img.getSize();

	if (( tmp_s.x > 0 ) && ( tmp_s.y > 0 ))
	{
		sf::Color p = tmp_img.getPixel( 0, 0 );
		tmp_img.createMaskFromColor( p );

		if ( m_texture.loadFromImage( tmp_img ) )
			retval=true;

		notify_texture_change();
	}

	return retval;
}

#ifdef WITH_MOVIE
bool FeTextureContainer::load_with_ffmpeg(
	const std::string &path,
	const std::string &filename,
	bool is_image )
{
	std::string loaded_name;
	bool res=false;

	if ( tail_compare( path, FE_ZIP_EXT ) )
	{
		loaded_name = filename;
		if ( loaded_name.compare( m_file_name ) == 0 )
			return true;

		clear();
		m_movie = new FeMedia( FeMedia::AudioVideo );
		res = m_movie->openFromArchive( path, filename, &m_texture );
	}
	else
	{
		loaded_name = path + filename;
		if ( loaded_name.compare( m_file_name ) == 0 )
			return true;

		clear();
		m_movie = new FeMedia( FeMedia::AudioVideo );
		res = m_movie->openFromFile( loaded_name, &m_texture );
	}

	if ( !res )
	{
		std::cout << "ERROR loading video: "
			<< loaded_name << std::endl;

		delete m_movie;
		m_movie = NULL;
		return false;
	}

	if ( is_image && (!m_movie->is_multiframe()) )
	{
		m_movie_status = -1; // don't play if there is only one frame

		// if there is only one frame, then we can update the texture immediately
		// (the frame will have been preloaded) and delete our movie object now
		delete m_movie;
		m_movie = NULL;
	}
	else if (m_video_flags & VF_NoAutoStart)
		m_movie_status = 0; // 0=loaded but not on track to play
	else
		m_movie_status = 1; // 1=on track to be played

	m_file_name = loaded_name;
	return true;
}
#endif

bool FeTextureContainer::try_to_load(
	const std::string &path,
	const std::string &filename,
	bool is_image )
{
	std::string loaded_name;

#ifdef WITH_SWF
	if ( !is_image && tail_compare( filename, FE_SWF_EXT ) )
	{

		if ( tail_compare( path, FE_ZIP_EXT ) )
		{
			loaded_name = filename;
			if ( loaded_name.compare( m_file_name ) == 0 )
				return true;

			m_swf = new FeSwf();

			if (!m_swf->open_from_archive( path, filename ))
			{
				std::cout << " ! ERROR loading SWF from zip: "
					<< path << " (" << loaded_name << ")" << std::endl;

				delete m_swf;
				m_swf = NULL;
				return false;
			}
		}
		else
		{
			loaded_name = path + filename;
			if ( loaded_name.compare( m_file_name ) == 0 )
				return true;

			m_swf = new FeSwf();
			if (!m_swf->open_from_file( loaded_name ))
			{
				std::cout << " ! ERROR loading SWF: "
					<< loaded_name << std::endl;

				delete m_swf;
				m_swf = NULL;
				return false;
			}
		}

		m_file_name = loaded_name;
		return true;
	}
#endif

#ifdef WITH_MOVIE
	if ( !is_image && FeMedia::is_supported_media_file( filename ) )
		return load_with_ffmpeg( path, filename, false );
#endif

	if ( tail_compare( path, FE_ZIP_EXT ) )
	{
		loaded_name = filename;
		if ( loaded_name.compare( m_file_name ) == 0 )
			return true;

		clear();
		FeZipStream zs( path );
		zs.open( filename );

		if ( m_texture.loadFromStream( zs ) )
		{
			m_file_name = filename;
			return true;
		}
	}
	else
	{
		loaded_name = path + filename;
		if ( loaded_name.compare( m_file_name ) == 0 )
			return true;

		clear();
		if ( m_texture.loadFromFile( loaded_name ) )
		{
			m_file_name = loaded_name;
			return true;
		}
	}

#ifdef WITH_MOVIE
	//
	// we should only get here if we failed to load an image with SFML
	// try loading it with ffmpeg instead, which can handle more image
	// formats...
	//
	return load_with_ffmpeg( path, filename, is_image );
#else
	return false;
#endif
}

const sf::Texture &FeTextureContainer::get_texture()
{
#ifdef WITH_SWF
	if ( m_swf )
		return m_swf->get_texture();
#endif
	return m_texture;
}

void FeTextureContainer::on_new_selection( FeSettings *feSettings )
{
	if (( m_type != IsStatic ) && ( m_art_update_trigger == ToNewSelection ))
		internal_update_selection( feSettings );
}

void FeTextureContainer::on_new_list( FeSettings *feSettings, bool new_display )
{
	if ( new_display )
		m_current_filter_index=-1;

	if (( m_type != IsStatic ) && ( m_art_update_trigger == EndNavigation ))
		internal_update_selection( feSettings );
}

void FeTextureContainer::on_end_navigation( FeSettings *feSettings )
{
	if (( m_type != IsStatic ) && ( m_art_update_trigger == EndNavigation ))
		internal_update_selection( feSettings );
}

void FeTextureContainer::internal_update_selection( FeSettings *feSettings )
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

	std::vector<std::string> vid_list;
	std::vector<std::string> image_list;
	std::string archive_name; // empty if no archive

	if ( m_type == IsArtwork )
	{
		FeRomInfo *rom	= feSettings->get_rom_absolute( filter_index, rom_index );
		if ( !rom )
			return;

		feSettings->get_best_artwork_file( *rom,
			m_art_name,
			vid_list,
			image_list );

		if ( vid_list.empty() && image_list.empty() )
		{
			// check for layout fallback images/videos
			std::string layout_path;
			feSettings->get_path( FeSettings::Current,
				layout_path );

			if ( !layout_path.empty() )
			{
				const std::string &emu_name = rom->get_info(
					FeRomInfo::Emulator );

				if ( tail_compare( layout_path, FE_ZIP_EXT ) )
				{
					archive_name = layout_path;

					// check for "[emulator-[artlabel]" artworks first
					if ( !gather_artwork_filenames_from_archive(
							layout_path, emu_name + "-" + m_art_name,
							vid_list, image_list ) )
					{
						// then "[artlabel]"
						gather_artwork_filenames_from_archive( layout_path,
							m_art_name, vid_list, image_list );
					}
				}
				else
				{
					std::vector<std::string> layout_paths;
					layout_paths.push_back( layout_path );

					// check for "[emulator-[artlabel]" artworks first
					if ( !gather_artwork_filenames( layout_paths,
							emu_name + "-" + m_art_name,
							vid_list, image_list ) )
					{
						// then "[artlabel]"
						gather_artwork_filenames( layout_paths,
							m_art_name, vid_list, image_list );
					}
				}
			}
		}
	}
	else if ( m_type == IsDynamic )
	{
		std::string work = m_art_name;
		FePresent::script_process_magic_strings( work,
				m_filter_offset,
				m_index_offset );

		feSettings->get_best_dynamic_image_file( filter_index,
			rom_index,
			work,
			vid_list,
			image_list );
	}

	// Load any found videos/images now
	//
	bool loaded=false;
	std::vector<std::string>::iterator itr;

#ifdef WITH_MOVIE
	if ( m_video_flags & VF_DisableVideo )
		vid_list.clear();

	for ( itr=vid_list.begin(); itr != vid_list.end(); ++itr )
	{
		if ( try_to_load( archive_name, *itr ) )
		{
			loaded = true;
			break;
		}
	}
#endif

	if ( !loaded )
	{
		if ( image_list.empty() )
			clear();
		else
		{
			for ( itr=image_list.begin();
				itr != image_list.end(); ++itr )
			{
				if ( try_to_load( archive_name, *itr, true ) )
				{
					loaded = true;
					break;
				}
			}
		}
	}

	//
	// Texture was replaced, so notify the attached images
	//
	notify_texture_change();
}

bool FeTextureContainer::tick( FeSettings *feSettings, bool play_movies, bool ok_to_start )
{
#ifdef WITH_SWF
	if (( play_movies ) && ( m_swf ))
	{
		return m_swf->tick();
	}
#endif

#ifdef WITH_MOVIE
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
#ifdef WITH_MOVIE
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
#ifdef WITH_SWF
	if ( m_swf )
		return true;
#endif

#ifdef WITH_MOVIE
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
#ifdef WITH_MOVIE
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

#ifdef WITH_MOVIE
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
#ifdef WITH_MOVIE
	if ( m_movie )
		return m_movie->get_duration().asMilliseconds();
#endif

	return 0;
}

int FeTextureContainer::get_video_time() const
{
#ifdef WITH_MOVIE
	if ( m_movie )
		return m_movie->get_video_time().asMilliseconds();
#endif

	return 0;
}

void FeTextureContainer::set_file_name( const char *n )
{
	std::string path, filename=n;

	if ( filename.empty() )
	{
		clear();
		notify_texture_change();
		return;
	}

	// If it is a relative path we assume it is in the
	// layout/screensaver/intro directory
	//
	if ( is_relative_path( filename ) )
		path = FePresent::script_get_base_path();

	bool is_image=false;

	int i=0;
	while ( FE_ART_EXTENSIONS[i] )
	{
		if ( tail_compare( filename, FE_ART_EXTENSIONS[i] ) )
		{
			is_image=true;
			break;
		}
		i++;
	}

	try_to_load( path, filename, is_image );
	notify_texture_change();
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
		std::swap( m_type, o_up->m_type );
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
	bool s = m_texture.isSmooth();
	m_texture = sf::Texture();
	m_texture.setSmooth( s );

	m_movie_status = -1;
	m_file_name.clear();

#ifdef WITH_SWF
	if ( m_swf )
	{
		delete m_swf;
		m_swf=NULL;
	}
#endif

#ifdef WITH_MOVIE
	// If a movie is running, close it...
	if ( m_movie )
	{
		delete m_movie;
		m_movie=NULL;
	}
#endif
}

void FeTextureContainer::set_smooth( bool s )
{
#ifdef WITH_SWF
	if ( m_swf )
		m_swf->set_smooth( s );
#endif
	m_texture.setSmooth( s );
}

bool FeTextureContainer::get_smooth() const
{
	return m_texture.isSmooth();
}

FeSurfaceTextureContainer::FeSurfaceTextureContainer( int width, int height )
{
	m_texture.create( width, height );
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

void FeSurfaceTextureContainer::on_new_selection( FeSettings *s )
{
	for ( std::vector<FeBasePresentable *>::iterator itr = m_draw_list.begin();
				itr != m_draw_list.end(); ++itr )
		(*itr)->on_new_selection( s );
}

void FeSurfaceTextureContainer::on_end_navigation( FeSettings *feSettings )
{
}

void FeSurfaceTextureContainer::on_new_list( FeSettings *s, bool )
{
	//
	// We don't do any scaling of the objects when they are being drawn
	// to the surface.
	//
	for ( std::vector<FeBasePresentable *>::iterator itr = m_draw_list.begin();
				itr != m_draw_list.end(); ++itr )
		(*itr)->on_new_list( s );
}

bool FeSurfaceTextureContainer::tick( FeSettings *feSettings, bool play_movies, bool ok_to_start )
{
	//
	// Draw the surface's draw list to the render texture
	//
	m_texture.clear( sf::Color::Transparent );
	for ( std::vector<FeBasePresentable *>::const_iterator itr = m_draw_list.begin();
				itr != m_draw_list.end(); ++itr )
	{
		if ( (*itr)->get_visible() )
			m_texture.draw( (*itr)->drawable() );
	}

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

void FeSurfaceTextureContainer::set_smooth( bool s )
{
	m_texture.setSmooth( s );
}

bool FeSurfaceTextureContainer::get_smooth() const
{
	return m_texture.isSmooth();
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

bool FeImage::fix_masked_image()
{
	return m_tex->fix_masked_image();
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
		m_sprite.setScale( sf::Vector2f( scale_x, scale_y ) );

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

void FeImage::set_smooth( bool s )
{
	m_tex->set_smooth( s );
}

bool FeImage::get_smooth() const
{
	return m_tex->get_smooth();
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
