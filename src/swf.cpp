/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2015 Andrew Mickelson
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

#include "swf.hpp"
#include "zip.hpp"

#include <base/tu_file.h>
#include "gameswf/gameswf_types.h"
#include "gameswf/gameswf_impl.h"
#include "gameswf/gameswf_root.h"
#include <gameswf/gameswf_player.h>

#if TU_CONFIG_LINK_TO_FREETYPE == 1
#include "gameswf/gameswf_freetype.h"
#endif

#include <SFML/OpenGL.hpp>

#include <iostream>

namespace
{
	gameswf::render_handler *swf_render=NULL;
	gameswf::sound_handler *swf_sound=NULL;
	int swf_count=0;
	FeZipStream *swf_zip=NULL;

	static tu_file *swf_file_opener( const char *url )
	{
		if ( swf_zip )
			return new tu_file( tu_file::memory_buffer,
				swf_zip->getSize(), swf_zip->getData() );
		else
			return new tu_file(url, "rb");
	}

	void open_swf()
	{
		if ( swf_render == NULL )
		{
#ifdef FE_RPI
			swf_render = gameswf::create_render_handler_ogles();
#else
			swf_render = gameswf::create_render_handler_ogl();
#endif

			gameswf::set_render_handler( swf_render );
			swf_render->open();
//			swf_render->set_antialiased( true );

			swf_sound = gameswf::create_sound_handler_openal();
			gameswf::set_sound_handler( swf_sound );

			gameswf::register_file_opener_callback( swf_file_opener );
#if TU_CONFIG_LINK_TO_FREETYPE == 1
			gameswf::set_glyph_provider( gameswf::create_glyph_provider_freetype() );
#else
			gameswf::set_glyph_provider( gameswf::create_glyph_provider_tu() );
#endif
		}
		swf_count++;
	}

	void close_swf()
	{
		swf_count--;
		if ( swf_count == 0 )
		{
			gameswf::set_render_handler( NULL );
			delete swf_render;
			swf_render = NULL;

			gameswf::set_sound_handler( NULL );
			delete swf_sound;
			swf_sound = NULL;
		}
	}
};

struct FeSwfState
{
	gameswf::gc_ptr<gameswf::player> play;
	gameswf::gc_ptr<gameswf::root> root;
	sf::Clock timer;
	sf::Time last_tick;
};


FeSwf::FeSwf()
{
	open_swf();
	m_imp = new FeSwfState();
}

FeSwf::~FeSwf()
{
	delete m_imp;
	close_swf();
}

bool FeSwf::open_from_archive( const std::string &path, const std::string &file )
{
	FeZipStream zs( path );
	if ( !zs.open( file ) )
		return false;

	swf_zip = &zs;
	bool retval = open_from_file( file );
	swf_zip = NULL;

	return retval;
}

bool FeSwf::open_from_file( const std::string &file )
{
	m_imp->play = NULL;
	m_imp->root = NULL;

	m_imp->play = new gameswf::player();
	m_imp->play->set_separate_thread( false );

#ifdef FE_DEBUG
	m_imp->play->verbose_action( true );
	m_imp->play->verbose_parse( true );
//	m_imp->play->set_log_bitmap_info( true );
#endif

	m_imp->root = m_imp->play->load_file( file.c_str() );

	if ( m_imp->root == NULL )
		return false;

	m_texture.create( m_imp->root->get_movie_width() * 3,
		m_imp->root->get_movie_height() * 3);

	m_texture.setActive();

	// alpha blending
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

	glMatrixMode( GL_PROJECTION );
	glOrtho( -1.f, 1.f, 1.f, -1.f, -1, 1 );

	glMatrixMode( GL_MODELVIEW );	
	glLoadIdentity();

	glDisable( GL_LIGHTING );
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	m_imp->root->set_display_viewport( 0, 0,
		m_imp->root->get_movie_width() * 3,
		m_imp->root->get_movie_height() * 3 );

	m_imp->root->set_background_alpha( 1.0f );

	do_frame( false );
	return true; 
}

const sf::Vector2u FeSwf::get_size() const
{
	return m_texture.getSize();
}

const sf::Texture &FeSwf::get_texture() const
{
	return m_texture.getTexture();
}

bool FeSwf::tick()
{
	return do_frame( true );
}

bool FeSwf::do_frame( bool is_tick )
{
	m_texture.setActive();
	m_texture.clear();

	if ( m_imp->root != NULL )
	{
		if ( is_tick )
		{
			m_imp->root->advance(
				m_imp->timer.getElapsedTime().asMilliseconds() );

			m_imp->timer.restart();
		}

		m_imp->root->display();
	}

	m_texture.display();

	return ( m_imp->root != NULL );
}

void FeSwf::set_smooth( bool s )
{
	m_texture.setSmooth( s );
}
