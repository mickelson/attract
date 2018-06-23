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

#include "fe_listbox.hpp"
#include "fe_settings.hpp"
#include "fe_shader.hpp"
#include "fe_present.hpp"
#include "fe_util.hpp"
#include <iostream>

FeListBox::FeListBox( FePresentableParent &p, int x, int y, int w, int h )
	: FeBasePresentable( p ),
	m_selColour( sf::Color::Yellow ),
	m_selBg( sf::Color::Blue ),
	m_selStyle( sf::Text::Regular ),
	m_rows( 11 ),
	m_userCharSize( 0 ),
	m_filter_offset( 0 ),
	m_rotation( 0.0 ),
	m_scale_factor( 1.0 ),
	m_scripted( true ),
	m_custom_sel( -1 )
{
	m_base_text.setPosition( sf::Vector2f( x, y ) );
	m_base_text.setSize( sf::Vector2f( w, h ) );
	m_base_text.setColor( sf::Color::White );
	m_base_text.setBgColor( sf::Color::Transparent );
}

FeListBox::FeListBox(
		FePresentableParent &p,
		const sf::Font *font,
		const sf::Color &colour,
		const sf::Color &bgcolour,
		const sf::Color &selcolour,
		const sf::Color &selbgcolour,
		unsigned int charactersize,
		int rows )
	: FeBasePresentable( p ),
	m_base_text( font, colour, bgcolour, charactersize, FeTextPrimative::Centre ),
	m_selColour( selcolour ),
	m_selBg( selbgcolour ),
	m_selStyle( sf::Text::Regular ),
	m_rows( rows ),
	m_userCharSize( charactersize ),
	m_filter_offset( 0 ),
	m_rotation( 0.0 ),
	m_scale_factor( 1.0 ),
	m_scripted( false ),
	m_custom_sel( -1 )
{
}

void FeListBox::setFont( const sf::Font &f )
{
	m_base_text.setFont( f );
}

const sf::Vector2f &FeListBox::getPosition() const
{
	return m_base_text.getPosition();
}

void FeListBox::setPosition( const sf::Vector2f &p )
{
	m_base_text.setPosition( p );

	if ( m_scripted )
		FePresent::script_do_update( this );
}

const sf::Vector2f &FeListBox::getSize() const
{
	return m_base_text.getSize();
}

void FeListBox::setSize( const sf::Vector2f &s )
{
	m_base_text.setSize( s );

	if ( m_scripted )
		FePresent::script_do_update( this );
}

float FeListBox::getRotation() const
{
	return m_rotation;
}

const sf::Color &FeListBox::getColor() const
{
	return m_base_text.getColor();
}

void FeListBox::init_dimensions()
{
	sf::Vector2f size = getSize();
	sf::Vector2f pos = getPosition();

	int actual_spacing = (int)size.y / m_rows;
	int char_size = 8 * m_scale_factor;

	// Set the character size now
	//
	if ( m_userCharSize > 0 )
		char_size = m_userCharSize * m_scale_factor;
	else if ( actual_spacing > 12 )
		char_size = ( actual_spacing - 4 ) * m_scale_factor;

	m_base_text.setTextScale( sf::Vector2f( 1.f / m_scale_factor, 1.f / m_scale_factor ) );
	m_base_text.setCharacterSize( char_size );

	m_texts.clear();

	if ( m_rows > 0 )
		m_texts.reserve( m_rows );

	sf::Transform rotater;
	rotater.rotate( m_rotation, pos.x, pos.y );

	for ( int i=0; i< m_rows; i++ )
	{
		FeTextPrimative t( m_base_text );
		if ( i == m_rows / 2 )
		{
			t.setColor( m_selColour );
			t.setBgColor( m_selBg );
			t.setStyle( m_selStyle );
		}

		t.setPosition( rotater.transformPoint( pos.x, pos.y+(i*actual_spacing)) );
		t.setSize( size.x, actual_spacing );
		t.setRotation( m_rotation );

		m_texts.push_back( t );
	}
}

void FeListBox::setColor( const sf::Color &c )
{
	m_base_text.setColor( c );

	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		if ( i != ( m_texts.size() / 2 ) )
			m_texts[i].setColor( c );
	}

	if ( m_scripted )
		FePresent::script_flag_redraw();
}

void FeListBox::setSelColor( const sf::Color &c )
{
	m_selColour = c;

	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		m_texts[ sel ].setColor( m_selColour );
	}

	if ( m_scripted )
		FePresent::script_flag_redraw();
}

void FeListBox::setSelBgColor( const sf::Color &c )
{
	m_selBg = c;
	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		m_texts[ sel ].setBgColor( m_selBg );
	}

	if ( m_scripted )
		FePresent::script_flag_redraw();
}

void FeListBox::setSelStyle( int s )
{
	m_selStyle = s;
	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		m_texts[ sel ].setStyle( m_selStyle );
	}

	if ( m_scripted )
		FePresent::script_flag_redraw();
}

int FeListBox::getSelStyle()
{
	return m_selStyle;
}

void FeListBox::setTextScale( const sf::Vector2f &scale )
{
	m_base_text.setTextScale( scale );

	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setTextScale( scale );
}

FeTextPrimative *FeListBox::setEditMode( bool e, sf::Color c )
{
	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		if ( e )
		{
			m_texts[ sel ].setColor( c );
			m_texts[ sel ].setOutlineColor( c );
			m_texts[ sel ].setOutlineThickness( -1 );

			return &(m_texts[ sel ]);
		}
		else
		{
			m_texts[ sel ].setColor( m_selColour );
			m_texts[ sel ].setOutlineThickness( 0 );
		}
	}
	return NULL;
}

void FeListBox::setCustomSelection( const int index )
{
	m_custom_sel = index;
	internalSetText( index );
}

void FeListBox::setCustomText( const int index,
			const std::vector<std::string> &list )
{
	if ( list.empty() )
		m_custom_sel = -1;
	else
		m_custom_sel = index;

	m_displayList = list;
	internalSetText( m_custom_sel );
}

void FeListBox::internalSetText( const int index )
{
	if ( !m_texts.empty() )
	{
		int offset = index - ( (int)m_texts.size() / 2 );

		for ( int i=0; i < (int)m_texts.size(); i++ )
		{
			int listentry = offset + i;
			if (( listentry < 0 ) || ( listentry >= (int)m_displayList.size() ))
				m_texts[i].setString("");
			else
				m_texts[i].setString( m_displayList[listentry] );
		}
	}
}

void FeListBox::setLanguageText( const int index,
			const std::vector<FeLanguage> &list,
			FePresent *fep )
{
	m_custom_sel = index;

	if ( !m_texts.empty() )
	{
		int offset = index - ( (int)m_texts.size() / 2 );

		for ( int i=0; i < (int)m_texts.size(); i++ )
		{
			int listentry = offset + i;
			if (( listentry < 0 ) || ( listentry >= (int)list.size() ))
				m_texts[i].setString("");
			else
			{
				m_texts[i].setString( list[listentry].label );
				const FeFontContainer *f = fep->get_pooled_font( list[listentry].font );
				if ( f )
					m_texts[i].setFont( f->get_font() );
			}
		}
	}
}

void FeListBox::setRotation( float r )
{
	m_rotation = r;

	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setRotation( m_rotation );

	if ( m_scripted )
		FePresent::script_flag_redraw();
}

void FeListBox::on_new_list( FeSettings *s )
{
	init_dimensions();

	if ( m_custom_sel >= 0 )
	{
		internalSetText( m_custom_sel );
		return;
	}

	int filter_index = s->get_filter_index_from_offset( m_filter_offset );
	int filter_size = s->get_filter_size( filter_index );
	int current_sel = s->get_rom_index( filter_index, 0 );

	m_displayList.clear();
	m_displayList.reserve( filter_size );

	std::string format_string = m_format_string;
	if ( format_string.empty() )
		format_string = "[Title]";

	for ( int i=0; i < filter_size; i++ )
	{
		m_displayList.push_back( format_string );

		FePresent::script_process_magic_strings(
			m_displayList.back(),
			m_filter_offset, i - current_sel );

		s->do_text_substitutions_absolute(
			m_displayList.back(), filter_index, i );
	}

	internalSetText( current_sel );
}

void FeListBox::on_new_selection( FeSettings *s )
{
	if ( m_custom_sel >= 0 )
		internalSetText( m_custom_sel );
	else
		internalSetText(
			s->get_rom_index(
				s->get_filter_index_from_offset(
					m_filter_offset ), 0 ) );
}

void FeListBox::set_scale_factor( float scale_x, float scale_y )
{
	m_scale_factor = ( scale_x > scale_y ) ? scale_x : scale_y;
	if ( m_scale_factor <= 0.f )
		m_scale_factor = 1.f;
}

void FeListBox::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	FeShader *s = get_shader();
	if ( s )
	{
		const sf::Shader *sh = s->get_shader();
		if ( sh )
			states.shader = sh;
	}

	for ( std::vector<FeTextPrimative>::const_iterator itl=m_texts.begin();
				itl != m_texts.end(); ++itl )
		target.draw( (*itl), states );
}

void FeListBox::clear()
{
	m_texts.clear();
}

int FeListBox::getRowCount() const
{
	return m_texts.size();
}

void FeListBox::setIndexOffset( int io )
{
}

int FeListBox::getIndexOffset() const
{
	return 0;
}

void FeListBox::setFilterOffset( int fo )
{
	m_filter_offset = fo;
}

int FeListBox::getFilterOffset() const
{
	return m_filter_offset;
}

int FeListBox::get_bgr()
{
	return m_base_text.getBgColor().r;
}

int FeListBox::get_bgg()
{
	return m_base_text.getBgColor().g;
}

int FeListBox::get_bgb()
{
	return m_base_text.getBgColor().b;
}

int FeListBox::get_bga()
{
	return m_base_text.getBgColor().a;
}

int FeListBox::get_charsize()
{
	return m_userCharSize;
}

float FeListBox::get_spacing()
{
	return m_base_text.getCharacterSpacing();
}

int FeListBox::get_rows()
{
	return m_rows;
}

int FeListBox::get_list_size()
{
	return m_displayList.size();
}

int FeListBox::get_style()
{
	return m_base_text.getStyle();
}

int FeListBox::get_align()
{
	return (int)m_base_text.getAlignment();
}

void FeListBox::setBgColor( const sf::Color &c )
{
	m_base_text.setBgColor(c);
	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		if ( i != ( m_texts.size() / 2 ) )
			m_texts[i].setBgColor( c );
	}

	if ( m_scripted )
		FePresent::script_flag_redraw();
}

void FeListBox::set_bgr(int r)
{
	sf::Color c=m_base_text.getBgColor();
	c.r=r;
	setBgColor( c );
}

void FeListBox::set_bgg(int g)
{
	sf::Color c=m_base_text.getBgColor();
	c.g=g;
	setBgColor( c );
}

void FeListBox::set_bgb(int b)
{
	sf::Color c=m_base_text.getBgColor();
	c.b=b;
	setBgColor( c );
}

void FeListBox::set_bga(int a)
{
	sf::Color c=m_base_text.getBgColor();
	c.a=a;
	setBgColor( c );
}

void FeListBox::set_bg_rgb(int r, int g, int b )
{
	sf::Color c=m_base_text.getBgColor();
	c.r=r;
	c.g=g;
	c.b=b;
	setBgColor(c);
}

void FeListBox::set_charsize(int s)
{
	m_userCharSize = s;

	// We call script_do_update to trigger a call to our init_demensions() function
	// with the appropriate parameters
	//
	if ( m_scripted )
		FePresent::script_do_update( this );
}

void FeListBox::set_spacing(float s)
{
	m_base_text.setCharacterSpacing(s);
	FePresent::script_do_update( this );
}

void FeListBox::set_rows(int r)
{
	//
	// Don't allow m_rows to ever be zero or negative
	//
	if ( m_rows > 0 )
	{
		m_rows = r;

		if ( m_scripted )
			FePresent::script_do_update( this );
	}
}

void FeListBox::set_style(int s)
{
	m_base_text.setStyle(s);
	for ( unsigned int i=0; i < m_texts.size(); i++ )
	{
		if ( i != ( m_texts.size() / 2 ) )
			m_texts[i].setStyle( s );
	}

	if ( m_scripted )
		FePresent::script_flag_redraw();
}

void FeListBox::set_align(int a)
{
	m_base_text.setAlignment( (FeTextPrimative::Alignment)a);

	for ( unsigned int i=0; i < m_texts.size(); i++ )
		m_texts[i].setAlignment( (FeTextPrimative::Alignment)a );

	if ( m_scripted )
		FePresent::script_flag_redraw();
}

int FeListBox::get_selr()
{
	return m_selColour.r;
}

int FeListBox::get_selg()
{
	return m_selColour.g;
}

int FeListBox::get_selb()
{
	return m_selColour.b;
}

int FeListBox::get_sela()
{
	return m_selColour.a;
}

void FeListBox::set_selr(int r)
{
	m_selColour.r=r;
	setSelColor( m_selColour );
}

void FeListBox::set_selg(int g)
{
	m_selColour.g=g;
	setSelColor( m_selColour );
}

void FeListBox::set_selb(int b)
{
	m_selColour.b=b;
	setSelColor( m_selColour );
}

void FeListBox::set_sela(int a)
{
	m_selColour.a=a;
	setSelColor( m_selColour );
}

void FeListBox::set_sel_rgb(int r, int g, int b )
{
	m_selColour.r = r;
	m_selColour.g = g;
	m_selColour.b = b;
	setSelColor( m_selColour );
}

int FeListBox::get_selbgr()
{
	return m_selBg.r;
}

int FeListBox::get_selbgg()
{
	return m_selBg.g;
}

int FeListBox::get_selbgb()
{
	return m_selBg.b;
}

int FeListBox::get_selbga()
{
	return m_selBg.a;
}

const char *FeListBox::get_font()
{
	return m_font_name.c_str();
}

void FeListBox::set_selbgr(int r)
{
	m_selBg.r=r;
	setSelBgColor( m_selBg );
}

void FeListBox::set_selbgg(int g)
{
	m_selBg.g=g;
	setSelBgColor( m_selBg );
}

void FeListBox::set_selbgb(int b)
{
	m_selBg.b=b;
	setSelBgColor( m_selBg );
}

void FeListBox::set_selbga(int a)
{
	m_selBg.a=a;
	setSelBgColor( m_selBg );
}

void FeListBox::set_selbg_rgb(int r, int g, int b )
{
	m_selBg.r = r;
	m_selBg.g = g;
	m_selBg.b = b;
	setSelBgColor( m_selBg );
}

void FeListBox::set_font( const char *f )
{
	FePresent *fep = FePresent::script_get_fep();
	if ( !fep )
		return;

	const FeFontContainer *fc = fep->get_pooled_font( f );

	if ( !fc )
		return;

	const sf::Font *font=&(fc->get_font());

	if ( font )
	{
		setFont( *font );
		m_font_name = f;

		FePresent::script_flag_redraw();
	}
}

const char *FeListBox::get_format_string()
{
	return m_format_string.c_str();
}

void FeListBox::set_format_string( const char *s )
{
	m_format_string = s;

	if ( m_scripted )
		FePresent::script_do_update( this );
}
