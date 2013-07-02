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

#include "fe_text.hpp"
#include "fe_settings.hpp"
#include "fe_util.hpp"
#include <iostream>

const char *FeTextConfigurable::baseSettings[] =
{
		"position",
		"size",
		"align",
		"textsize",
		"colour",
		"background",
		"style",
		"wrap",
		NULL
};

const char *FeTextConfigurable::styleTokens[] =
		{ "bold", "italic", "underlined", NULL };

FeTextConfigurable::FeTextConfigurable()
	: FeTextPrimative()
{
}

FeTextConfigurable::FeTextConfigurable( const sf::Font *font,
         const sf::Color &colour,
         const sf::Color &bgcolour,
         unsigned int charactersize,
         Alignment align )
	: FeTextPrimative( font, colour, bgcolour, charactersize, align )
{
}

int FeTextConfigurable::process_setting( const std::string &setting,
                        const std::string &value,
								const std::string &fn )
{
	size_t pos=0;
	std::string val;

   if ( setting.compare( baseSettings[0] ) == 0 ) // position
   {
      // position is XX,YY
      token_helper( value, pos, val, "," );
      int left = as_int( val );
      token_helper( value, pos, val );
      int top = as_int( val );

		setPosition( sf::Vector2f( left, top ));
   }
   else if ( setting.compare( baseSettings[1] ) == 0 ) // size
   {
      // size is WW,HH
      token_helper( value, pos, val, ",x" );
		sf::Vector2f size;
      size.x = as_int( val );
      token_helper( value, pos, val );
      size.y = as_int( val );
		setSize( size );
   }
   else if ( setting.compare( baseSettings[2] ) == 0 )  // align
	{
		const char *atokens[] = { "left", "right", "centre", NULL };

		if ( value.compare( atokens[0] ) == 0 )
			setAlignment( FeTextPrimative::Left );
		else if ( value.compare( atokens[1] ) == 0 )
			setAlignment( FeTextPrimative::Right );
		else if ( value.compare( atokens[2] ) == 0 )
			setAlignment( FeTextPrimative::Centre );
		else
		{
			invalid_setting( fn, "align", value, atokens, NULL, "value" );
			return 1;
		}
	}
   else if ( setting.compare( baseSettings[3] ) == 0 ) // textsize
	{
      setCharacterSize( as_int( value ) );
	}
   else if ( setting.compare( baseSettings[4] ) == 0 )  // colour
		setColor( colour_helper( value ) );
   else if ( setting.compare( baseSettings[5] ) == 0 )  // background
		setBgColor( colour_helper( value ) );
   else if ( setting.compare( baseSettings[6] ) == 0 )  // style
	{
		while ( pos < value.size() )
		{
			token_helper( value, pos, val, "," );
			if ( val.compare( styleTokens[0] ) == 0 )
				setStyle( sf::Text::Bold );
			else if ( val.compare( styleTokens[1] ) == 0 )
				setStyle( sf::Text::Italic );
			else if ( val.compare( styleTokens[2] ) == 0 )
				setStyle( sf::Text::Underlined );
			else
			{
				invalid_setting( fn, "style", val, styleTokens, NULL, "value" );
				return 1;
			}
		}
	}
   else if ( setting.compare( baseSettings[7] ) == 0 )  // wrap
	{
		if (( value.compare( "true" ) == 0 )
				|| ( value.compare( "yes" ) == 0 )
				|| ( value.compare( "1" ) == 0 ))
			setWordWrap( true );
	}
	else
	{
		// return 1 if nothing done
		return 1;
	}

	return 0;
}

FeText::FeText( const std::string &str )
	: m_string( str ), m_index_offset( 0 )
{
}

int FeText::process_setting( const std::string &setting,
                        const std::string &value,
								const std::string &fn )
{
	const char *stokens[] =
	{
		"index_offset",
		"rotation",
		NULL
	};

	if ( FeTextConfigurable::process_setting( setting, value, fn ) != 0 )
	{
   	if ( setting.compare( stokens[0] ) == 0 )  // index_offset
      	m_index_offset = as_int( value );
   	else if ( setting.compare( stokens[1] ) == 0 )  // rotation
      	setRotation( as_int( value ) );
		else
		{
			invalid_setting( fn, "text", setting, stokens, baseSettings );
			return 1;
		}
	}
	return 0;
}

void FeText::on_new_selection( FeSettings *feSettings )
{
	//
	// Perform substitutions of the [XXX] sequences occurring in m_string
	//
	size_t n = std::count( m_string.begin(), m_string.end(), '[' );

	std::string str = m_string;
	for ( int i=0; ((i< FeRomInfo::LAST_INDEX) && ( n > 0 )); i++ )
	{
		std::string from = "[";
		from += FeRomInfo::indexStrings[i];
		from += "]";

		n -= perform_substitution( str, from,
				feSettings->get_rom_info( m_index_offset, (FeRomInfo::Index)i) );
	}

	if ( n > 0 )
	{
		n -= perform_substitution( str, "[list_title]",
				feSettings->get_current_list_title() );
	}

	setString( str );
}

FeListBox::FeListBox()
	: FeTextConfigurable(),
	m_selColour( sf::Color::Yellow ),
	m_selBg( sf::Color::Blue ),
	m_selStyle( sf::Text::Regular ),
	m_rotation( 0.0 )
{
	setColor( sf::Color::White );
	setBgColor( sf::Color::Transparent );
}

FeListBox::FeListBox(
			const sf::Font *font,
         const sf::Color &colour,
         const sf::Color &bgcolour,
         const sf::Color &selcolour,
         const sf::Color &selbgcolour,
         unsigned int charactersize,
         Alignment align )
	: FeTextConfigurable( font, colour, bgcolour, charactersize, align ),
	m_selColour( selcolour ),
	m_selBg( selbgcolour ),
	m_selStyle( sf::Text::Regular ),
	m_rotation( 0.0 )
{
}

void FeListBox::init()
{
	const sf::Font *font = getFont();
	unsigned int font_linespacing = getCharacterSize();;

	if ( font )
	{
		font_linespacing = font->getLineSpacing( getCharacterSize() );
//		linespacing += linespacing / 5;  // pad it
	}

	sf::Vector2f size = getSize();
	sf::Vector2f pos = getPosition();

	int line_count = (int) size.y / font_linespacing;
	int actual_spacing = (int) size.y / line_count;

	int sel = line_count / 2;

	m_texts.clear();
	m_texts.reserve( line_count );

	sf::Transform trans;
	trans.rotate( m_rotation, pos.x, pos.y );

	for ( int i=0; i< line_count; i++ )
	{
		FeTextPrimative t( *this );
		if ( i == sel )
		{
			t.setColor( m_selColour );
			t.setBgColor( m_selBg );
			t.setStyle( m_selStyle );
		}

		t.setPosition( trans.transformPoint( pos.x, pos.y+(i*actual_spacing) ));
		t.setSize( sf::Vector2f(size.x, actual_spacing ));
		t.setRotation( m_rotation );

		m_texts.push_back( t );
	}
}

void FeListBox::setSelColor( sf::Color c )
{
	m_selColour = c;

	if ( m_texts.size() > 0 )
	{
		int sel = m_texts.size() / 2;
		m_texts[ sel ].setColor( m_selColour );
	}
}

void FeListBox::setSelBgColor( sf::Color c )
{
	m_selBg = c;
}

void FeListBox::setSelStyle( int s )
{
	m_selStyle = s;
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
			m_texts[ sel ].setOutlineThickness( -2 );

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

void FeListBox::setText( const int index,
			const std::vector<std::string> &list )
{
	if ( !m_texts.empty() )
	{
		int offset = index - ( (int)m_texts.size() / 2 );

		for ( int i=0; i < (int)m_texts.size(); i++ )
		{
			int listentry = offset + i;
			if (( listentry < 0 ) || ( listentry >= (int)list.size() ))
				m_texts[i].setString("");
			else
				m_texts[i].setString( list[listentry] );
		}
	}
}

void FeListBox::setRotation( float r )
{
	m_rotation = r;
}

int FeListBox::process_setting( const std::string &setting,
                        const std::string &value,
								const std::string &fn )
{
	const char *stokens[] =
	{
		"sel_colour",
		"sel_background",
		"sel_style",
		"rotation",
		NULL
	};

	if ( FeTextConfigurable::process_setting( setting, value, fn ) != 0 )
	{
   	if ( setting.compare( stokens[0] ) == 0 )
			setSelColor( colour_helper( value ));
   	else if ( setting.compare( stokens[1] ) == 0 )
			setSelBgColor( colour_helper( value ));
   	else if ( setting.compare( stokens[2] ) == 0 )
		{
			size_t pos=0;
			std::string val;

			while ( pos < value.size() )
			{
  		    	token_helper( value, pos, val, "," );
				if ( val.compare( styleTokens[0] ) == 0 )
					setSelStyle( sf::Text::Bold );
				else if ( val.compare( styleTokens[1] ) == 0 )
					setSelStyle( sf::Text::Italic );
				else if ( val.compare( styleTokens[2] ) == 0 )
					setSelStyle( sf::Text::Underlined );
				else
				{
					invalid_setting( fn, "sel_style", val, styleTokens, NULL, "value" );
					return 1;
				}
			}
		}
  		else if ( setting.compare( stokens[3] ) == 0 )  // rotation
		{
     		setRotation( as_int( value ) );
		}
		else
		{
			invalid_setting( fn, "list", setting, stokens, baseSettings );
			return 1;
		}
	}
	return 0;
}

void FeListBox::on_new_list( FeSettings *s )
{
	init();

	s->get_current_display_list( m_displayList );
	setText( s->get_rom_index(), m_displayList );
}

void FeListBox::on_new_selection( FeSettings *s )
{
	setText( s->get_rom_index(), m_displayList );
}

void FeListBox::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	for ( std::vector<FeTextPrimative>::const_iterator itl=m_texts.begin();
				itl != m_texts.end(); ++itl )
		target.draw( (*itl), states );
}

void FeListBox::clear()
{
	m_texts.clear();
}

int FeListBox::getRowCount()
{
	return m_texts.size();
}
