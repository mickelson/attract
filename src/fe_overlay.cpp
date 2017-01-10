/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-16 Andrew Mickelson
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

#include "fe_overlay.hpp"
#include "fe_settings.hpp"
#include "fe_config.hpp"
#include "fe_util.hpp"
#include "fe_listbox.hpp"
#include "fe_text.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cmath>

class FeConfigContextImp : public FeConfigContext
{
private:
	FeOverlay &m_feo;

public:
	std::vector<std::string> left_list;
	std::vector<std::string> right_list;
	int exit_sel;

	FeConfigContextImp( FeSettings &fes, FeOverlay &feo );
	bool edit_dialog( const std::string &m, std::string &t );

	bool confirm_dialog( const std::string &m,
		const std::string &rep );

	void splash_message( const std::string &msg,
		const std::string &rep,
		const std::string &aux );

	void input_map_dialog( const std::string &m,
		std::string &ms,
		FeInputMap::Command &conflict );

	void tags_dialog();

	void update_to_menu( FeBaseConfigMenu *m );

	bool check_for_cancel();
};

FeConfigContextImp::FeConfigContextImp( FeSettings &fes, FeOverlay &feo )
	: FeConfigContext( fes ), m_feo( feo )
{
}

bool FeConfigContextImp::edit_dialog( const std::string &m, std::string &t )
{
	std::string trans;
	fe_settings.get_resource( m, trans );
	return m_feo.edit_dialog( trans, t );
}

bool FeConfigContextImp::confirm_dialog( const std::string &msg,
						const std::string &rep )
{
	return !m_feo.confirm_dialog( msg, rep );
}

void FeConfigContextImp::splash_message(
			const std::string &msg,
			const std::string &rep,
			const std::string &aux )
{
	m_feo.splash_message( msg, rep, aux );
}

void FeConfigContextImp::input_map_dialog( const std::string &m,
		std::string &ms,
		FeInputMap::Command &conflict )
{
	std::string t;
	fe_settings.get_resource( m, t );
	m_feo.input_map_dialog( t, ms, conflict );
}

void FeConfigContextImp::tags_dialog()
{
	m_feo.tags_dialog();
}

void FeConfigContextImp::update_to_menu(
		FeBaseConfigMenu *m )
{
	opt_list.clear();
	left_list.clear();
	right_list.clear();

	m->get_options( *this );

	for ( int i=0; i < (int)opt_list.size(); i++ )
	{
		left_list.push_back( opt_list[i].setting );
		right_list.push_back( opt_list[i].get_value() );

		if ( opt_list[i].type == Opt::DEFAULTEXIT )
			exit_sel=i;
	}
}

bool FeConfigContextImp::check_for_cancel()
{
	return m_feo.check_for_cancel();
}

class FeEventLoopCtx
{
public:
	FeEventLoopCtx(
		const std::vector<sf::Drawable *> &in_draw_list,
		int &in_sel, int in_default_sel, int in_max_sel );

	const std::vector<sf::Drawable *> &draw_list; // [in] draw list
	int &sel;				// [in,out] selection counter
	int default_sel;	// [in] default selection
	int max_sel;		// [in] maximum selection

	int move_count;
	sf::Event move_event;
	sf::Clock move_timer;
	FeInputMap::Command move_command;
	FeInputMap::Command extra_exit;
};

FeEventLoopCtx::FeEventLoopCtx(
			const std::vector<sf::Drawable *> &in_draw_list,
			int &in_sel, int in_default_sel, int in_max_sel )
	: draw_list( in_draw_list ),
	sel( in_sel ),
	default_sel( in_default_sel ),
	max_sel( in_max_sel ),
	move_count( 0 ),
	move_command( FeInputMap::LAST_COMMAND ),
	extra_exit( FeInputMap::LAST_COMMAND )
{
}

class FeFlagMinder
{
public:
	FeFlagMinder( bool &flag )
		: m_flag( flag ), m_flag_set( false )
	{
		if ( !m_flag )
		{
			m_flag = true;
			m_flag_set = true;
		}
	}

	~FeFlagMinder()
	{
		if ( m_flag_set )
			m_flag = false;
	}

	bool flag_set() { return m_flag_set; };

private:
	bool &m_flag;
	bool m_flag_set;
};

FeOverlay::FeOverlay( sf::RenderWindow &wnd,
		FeSettings &fes,
		FePresent &fep )
	: m_wnd( wnd ),
	m_feSettings( fes ),
	m_fePresent( fep ),
	m_textColour( sf::Color::White ),
	m_bgColour( sf::Color( 0, 0, 0, 220 ) ),
	m_selColour( sf::Color::Yellow ),
	m_selBgColour( sf::Color( 0, 0, 200, 220 ) ),
	m_overlay_is_on( false )
{
}

void FeOverlay::get_common(
		sf::Vector2i &size,
		sf::Vector2f &text_scale,
		int &char_size ) const
{
	size = m_fePresent.get_layout_size();

	float scale_x = m_fePresent.get_layout_scale_x();
	float scale_y = m_fePresent.get_layout_scale_y();

	float scale_factor = ( scale_x > scale_y ) ? scale_x : scale_y;
	if ( scale_factor <= 0.f )
		scale_factor = 1.f;

	text_scale.x = text_scale.y = 1.f / scale_factor;
	char_size = (size.y / 14) * scale_factor;
}

void FeOverlay::splash_message( const std::string &msg,
				const std::string &rep,
				const std::string &aux )
{
	sf::Vector2i size;
	sf::Vector2f text_scale;
	int char_size;
	get_common( size, text_scale, char_size );

	sf::RectangleShape bg( sf::Vector2f( size.x, size.y ) );
	bg.setFillColor( m_bgColour );
	bg.setOutlineColor( m_textColour );
	bg.setOutlineThickness( -2 );

	FeTextPrimative message(
		m_fePresent.get_font(),
		m_textColour,
		sf::Color::Transparent,
		char_size );

	message.setWordWrap( true );
	message.setPosition( 2, 2 );
	message.setSize( size.x - 4, size.y - char_size/2 - 4 );
	message.setTextScale( text_scale );

	std::string msg_str;
	m_feSettings.get_resource( msg, rep, msg_str );

	message.setString( msg_str );

	FeTextPrimative extra(
		m_fePresent.get_font(),
		m_textColour,
		sf::Color::Transparent,
		char_size/2 );

	extra.setAlignment( FeTextPrimative::Left );
	extra.setPosition( 2, size.y - char_size/2 + 2 );
	extra.setSize( size.x - 4, char_size/2 );
	extra.setTextScale( text_scale );

	extra.setString( aux );

	const sf::Transform &t = m_fePresent.get_transform();

	m_fePresent.tick();

	m_wnd.clear();
	m_wnd.draw( m_fePresent, t );
	m_wnd.draw( bg, t );
	m_wnd.draw( message, t );
	m_wnd.draw( extra, t );
	m_wnd.display();
}

int FeOverlay::confirm_dialog( const std::string &msg,
	const std::string &rep,
	FeInputMap::Command extra_exit )
{
	std::string msg_str;
	m_feSettings.get_resource( msg, rep, msg_str );

	std::vector<std::string> list(2);
	m_feSettings.get_resource( "Yes", list[0] );
	m_feSettings.get_resource( "No", list[1] );

	return common_basic_dialog( msg_str, list, 1, 1, extra_exit );
}

int FeOverlay::common_list_dialog(
	const std::string &title,
	const std::vector < std::string > &options,
	int default_sel,
	int cancel_sel,
	FeInputMap::Command extra_exit )
{
	int sel = default_sel;
	std::vector<sf::Drawable *> draw_list;
	FeEventLoopCtx c( draw_list, sel, cancel_sel, options.size() - 1 );
	c.extra_exit = extra_exit;

	FeFlagMinder fm( m_overlay_is_on );

	// var gets used if we trigger the ShowOverlay transition
	int var = 0;
	if ( extra_exit != FeInputMap::LAST_COMMAND )
		var = (int)extra_exit;

	FeText *custom_caption;
	FeListBox *custom_lb;
	bool do_custom = m_fePresent.get_overlay_custom_controls(
		custom_caption, custom_lb );

	if ( fm.flag_set() && do_custom )
	{
		//
		// Use custom controls set by script
		//
		std::string old_cap;
		if ( custom_caption )
		{
			old_cap = custom_caption->get_string();
			custom_caption->set_string( title.c_str() );
		}

		if ( custom_lb )
			custom_lb->setCustomText( sel, options );

		m_fePresent.on_transition( ShowOverlay, var );

		init_event_loop( c );
		while ( event_loop( c ) == false )
		{
			m_fePresent.on_transition( NewSelOverlay, sel );

			if ( custom_lb )
				custom_lb->setCustomSelection( sel );
		}

		m_fePresent.on_transition( HideOverlay, 0 );

		// reset to the old text in these controls when done
		if ( custom_caption )
			custom_caption->set_string( old_cap.c_str() );

		if ( custom_lb )
		{
			custom_lb->setCustomText( 0, std::vector<std::string>() );
			custom_lb->on_new_list( &m_feSettings );
		}
	}
	else
	{
		//
		// Use the default built-in controls
		//
		sf::Vector2i size;
		sf::Vector2f text_scale;
		int char_size;
		get_common( size, text_scale, char_size );

		if ( options.size() > 8 )
			char_size /= 2;

		sf::RectangleShape bg( sf::Vector2f( size.x, size.y ) );
		bg.setFillColor( m_bgColour );
		bg.setOutlineColor( m_textColour );
		bg.setOutlineThickness( -2 );
		draw_list.push_back( &bg );

		FeTextPrimative heading( m_fePresent.get_font(), m_selColour,
			sf::Color::Transparent, char_size );
		heading.setSize( size.x, size.y / 8 );
		heading.setOutlineColor( m_textColour );
		heading.setOutlineThickness( -2 );
		heading.setTextScale( text_scale );
		heading.setString( title );
		draw_list.push_back( &heading );

		FePresentableParent temp;
		FeListBox dialog( temp,
			m_fePresent.get_font(),
			m_textColour,
			sf::Color::Transparent,
			m_selColour,
			m_selBgColour,
			char_size,
			size.y / ( char_size * 1.5 * text_scale.y ) );

		dialog.setPosition( 2, size.y / 8 );
		dialog.setSize( size.x - 4, size.y * 7 / 8 );
		dialog.init_dimensions();
		dialog.setTextScale( text_scale );
		dialog.setCustomText( sel, options );
		draw_list.push_back( &dialog );

		if ( fm.flag_set() )
			m_fePresent.on_transition( ShowOverlay, var );

		init_event_loop( c );
		while ( event_loop( c ) == false )
		{
			if ( fm.flag_set() )
				m_fePresent.on_transition( NewSelOverlay, sel );
			dialog.setCustomSelection( sel );
		}

		if ( fm.flag_set() )
			m_fePresent.on_transition( HideOverlay, 0 );
	}

	return sel;
}

int FeOverlay::languages_dialog()
{
	std::vector<FeLanguage> ll;
	m_feSettings.get_languages_list( ll );

	if ( ll.size() <= 1 )
	{
		// if there is nothing to select, then set what we can and get out of here
		//
		m_feSettings.set_language( ll.empty() ? "en" : ll.front().language );
		return 0;
	}

	// Try and get a useful default setting based on POSIX locale value...
	//
	// TODO: figure out how to do this right on Windows...
	//
	std::string loc, test( "en" );
	try { loc = std::locale("").name(); } catch (...) {}
	if ( loc.size() > 1 )
		test = loc;

	int status( -3 ), current_i( 0 ), i( 0 );
	for ( std::vector<FeLanguage>::iterator itr=ll.begin(); itr != ll.end(); ++itr )
	{
		if (( status < 0 ) && ( test.compare( 0, 5, (*itr).language ) == 0 ))
		{
			current_i = i;
			status = 0;
		}
		else if (( status < -1 ) && ( test.compare( 0, 2, (*itr).language) == 0 ))
		{
			current_i = i;
			status = -1;
		}
		else if (( status < -2 ) && ( (*itr).language.compare( 0, 2, "en" ) == 0 ))
		{
			current_i = i;
			status = -2;
		}

		i++;
	}

	sf::Vector2i size;
	sf::Vector2f text_scale;
	int char_size;
	get_common( size, text_scale, char_size );

	if ( ll.size() > 8 )
		char_size /= 2;

	std::vector<sf::Drawable *> draw_list;

	sf::RectangleShape bg( sf::Vector2f( size.x, size.y ) );
	bg.setFillColor( m_bgColour );
	bg.setOutlineColor( m_textColour );
	bg.setOutlineThickness( -2 );
	draw_list.push_back( &bg );

	FeTextPrimative heading( m_fePresent.get_font(), m_selColour, sf::Color::Transparent, char_size );
	heading.setSize( size.x, size.y / 8 );
	heading.setOutlineColor( m_textColour );
	heading.setOutlineThickness( -2 );
	heading.setTextScale( text_scale );

	heading.setString( "" );
	draw_list.push_back( &heading );

	FePresentableParent temp;
	FeListBox dialog( temp,
		m_fePresent.get_font(),
		m_textColour,
		sf::Color::Transparent,
		m_selColour,
		m_selBgColour,
		char_size,
		size.y / ( char_size * 1.5 * text_scale.y ) );

	dialog.setPosition( 2, size.y / 8 );
	dialog.setSize( size.x - 4, size.y * 7 / 8 );
	dialog.init_dimensions();
	dialog.setTextScale( text_scale );
	draw_list.push_back( &dialog );

	int sel = current_i;
	dialog.setLanguageText( sel, ll, &m_fePresent );

	FeEventLoopCtx c( draw_list, sel, -1, ll.size() - 1 );
	FeFlagMinder fm( m_overlay_is_on );

	init_event_loop( c );
	while ( event_loop( c ) == false )
		dialog.setLanguageText( sel, ll, &m_fePresent );

	if ( sel >= 0 )
	{
		std::string temp1,temp2;
		m_feSettings.set_language( ll[sel].language );

		for ( std::vector<std::string>::iterator itr=ll[sel].font.begin();
				itr != ll[sel].font.end(); ++itr )
		{
			if ( m_feSettings.get_font_file( temp1, temp2, *itr ) )
			{
				m_feSettings.set_info( FeSettings::DefaultFont,
					*itr );
				break;
			}
		}
	}

	return sel;
}

int FeOverlay::tags_dialog()
{
	std::vector< std::pair<std::string, bool> > tags_list;
	m_feSettings.get_current_tags_list( tags_list );

	std::vector<std::string> list;

	for ( std::vector< std::pair<std::string, bool> >::iterator itr=tags_list.begin();
			itr!=tags_list.end(); ++itr )
	{
		std::string msg;
		m_feSettings.get_resource(
				(*itr).second ? "Remove tag: '$1'" : "Add tag: '$1'",
				(*itr).first,
				msg );

		list.push_back( msg );
	}

	list.push_back( std::string() );
	m_feSettings.get_resource( "Create new tag", list.back() );

	list.push_back( std::string() );
	m_feSettings.get_resource( "Back", list.back() );

	std::string temp;
	m_feSettings.get_resource( "Tags", temp );

	int sel = common_list_dialog( temp,
		list, 0,
		list.size() - 1,
		FeInputMap::ToggleTags );

	if ( sel == (int)tags_list.size() )
	{
		std::string title;
		m_feSettings.get_resource( "Enter new tag name", title );

		std::string name;
		edit_dialog( title, name );

		if ( !name.empty() )
		{
			if ( m_feSettings.set_current_tag( name, true ) )
				m_fePresent.update_to_new_list( 0, true ); // changing tag status altered our current list
		}
	}
	else if (( sel >=0 ) && ( sel < (int)tags_list.size() ))
	{
		if ( m_feSettings.set_current_tag( tags_list[sel].first, !(tags_list[sel].second) ) )
			m_fePresent.update_to_new_list( 0, true ); // changing tag status altered our current list
	}

	return sel;
}

int FeOverlay::common_basic_dialog(
			const std::string &msg_str,
			const std::vector<std::string> &list,
			int default_sel,
			int cancel_sel,
			FeInputMap::Command extra_exit )
{
	std::vector<sf::Drawable *> draw_list;
	int sel=default_sel;

	FeEventLoopCtx c( draw_list, sel, cancel_sel, list.size() - 1 );
	c.extra_exit = extra_exit;

	FeFlagMinder fm( m_overlay_is_on );

	// var gets used if we trigger the ShowOverlay transition
	int var = 0;
	if ( extra_exit != FeInputMap::LAST_COMMAND )
		var = (int)extra_exit;

	FeText *custom_caption;
	FeListBox *custom_lb;
	bool do_custom = m_fePresent.get_overlay_custom_controls(
		custom_caption, custom_lb );

	if ( fm.flag_set() && do_custom )
	{
		//
		// Custom overlay controlled by the script
		//
		std::string old_cap;
		if ( custom_caption )
		{
			old_cap = custom_caption->get_string();
			custom_caption->set_string( msg_str.c_str() );
		}

		if ( custom_lb )
			custom_lb->setCustomText( sel, list );

		m_fePresent.on_transition( ShowOverlay, var );

		init_event_loop( c );
		while ( event_loop( c ) == false )
		{
			m_fePresent.on_transition( NewSelOverlay, sel );

			if ( custom_lb )
				custom_lb->setCustomSelection( sel );
		}

		m_fePresent.on_transition( HideOverlay, 0 );

		// reset to the old text in these controls when done
		if ( custom_caption )
			custom_caption->set_string( old_cap.c_str() );

		if ( custom_lb )
		{
			custom_lb->setCustomText( 0, std::vector<std::string>() );
			custom_lb->on_new_list( &m_feSettings );
		}
	}
	else
	{
		//
		// Use the default built-in controls
		//
		sf::Vector2i size;
		sf::Vector2f text_scale;
		int char_size;
		get_common( size, text_scale, char_size );

		float slice = size.y / 2;

		sf::RectangleShape bg( sf::Vector2f( size.x, size.y ) );
		bg.setFillColor( m_bgColour );
		bg.setOutlineColor( m_textColour );
		bg.setOutlineThickness( -2 );

		FeTextPrimative message(
			m_fePresent.get_font(),
			m_textColour,
			sf::Color::Transparent,
			char_size );
		message.setWordWrap( true );
		message.setTextScale( text_scale );

		FePresentableParent temp;
		FeListBox dialog( temp,
			m_fePresent.get_font(),
			m_textColour,
			sf::Color::Transparent,
			m_selColour,
			m_selBgColour,
			char_size,
			( size.y - slice ) / ( char_size * 1.5 * text_scale.y ) );

		message.setPosition( 2, 2 );
		message.setSize( size.x - 4, slice );
		message.setString( msg_str );

		dialog.setPosition( 2, slice );
		dialog.setSize( size.x - 4, size.y - 4 - slice );
		dialog.init_dimensions();
		dialog.setTextScale( text_scale );

		draw_list.push_back( &bg );
		draw_list.push_back( &message );
		draw_list.push_back( &dialog );

		dialog.setCustomText( sel, list );

		if ( fm.flag_set() )
			m_fePresent.on_transition( ShowOverlay, var );

		init_event_loop( c );
		while ( event_loop( c ) == false )
		{
			if ( fm.flag_set() )
				m_fePresent.on_transition( NewSelOverlay, sel );
			dialog.setCustomSelection( sel );
		}

		if ( fm.flag_set() )
			m_fePresent.on_transition( HideOverlay, 0 );
	}

	return sel;
}

bool FeOverlay::edit_dialog(
			const std::string &msg_str,
			std::string &text )
{
	sf::Vector2i size;
	sf::Vector2f text_scale;
	int char_size;
	get_common( size, text_scale, char_size );

	FeTextPrimative message( m_fePresent.get_font(), m_textColour,
		m_bgColour, char_size );
	message.setWordWrap( true );

	FeTextPrimative tp( m_fePresent.get_font(), m_textColour,
		m_bgColour, char_size );

	float slice = size.y / 3;

	message.setSize( size.x, slice );
	message.setTextScale( text_scale );
	message.setString( msg_str );

	tp.setPosition( 0, slice );
	tp.setSize( size.x, size.y - slice );
	tp.setTextScale( text_scale );

	std::vector<sf::Drawable *> draw_list;
	draw_list.push_back( &message );
	draw_list.push_back( &tp );

	std::basic_string<sf::Uint32> str;
	sf::Utf8::toUtf32( text.begin(), text.end(), std::back_inserter( str ) );

	FeFlagMinder fm( m_overlay_is_on );

	// NOTE: ShowOverlay and HideOverlay events are  not sent when a
	// script triggers the edit dialog.  This is on purpose.
	//
	if ( edit_loop( draw_list, str, &tp ) )
	{
		text.clear();
		sf::Utf32::toUtf8( str.begin(), str.end(), std::back_inserter( text ) );
		return true;
	}

	return false;
}

void FeOverlay::input_map_dialog(
			const std::string &msg_str,
			std::string &map_str,
			FeInputMap::Command &conflict )
{
	sf::Vector2i s;
	sf::Vector2f text_scale;
	int char_size;
	get_common( s, text_scale, char_size );

	FeTextPrimative message( m_fePresent.get_font(), m_textColour,
		m_bgColour, char_size );
	message.setWordWrap( true );
	message.setTextScale( text_scale );

	// Make sure the appropriate mouse capture variables are set, in case
	// the user has just changed the mouse threshold
	//
	m_feSettings.init_mouse_capture( s.x, s.y );

	message.setSize( s.x, s.y );
	message.setString( msg_str );

	// Centre the mouse in case the user is mapping a mouse move event
	s.x /= 2;
	s.y /= 2;
	sf::Mouse::setPosition( sf::Vector2i( s ), m_wnd );

	// empty the window event queue
	sf::Event ev;
	while ( m_wnd.pollEvent(ev) )
	{
		// no op
	}

	bool redraw=true;

	// this should only happen from the config dialog
	ASSERT( m_overlay_is_on );

	bool multi_mode=false; // flag if we are checking for multiple inputs.
	bool done=false;

	sf::IntRect mc_rect;
	int joy_thresh;
	m_feSettings.get_input_config_metrics( mc_rect, joy_thresh );

	std::set < std::pair<int,int> > joystick_moves;
	FeInputMapEntry entry;
	sf::Clock timeout;

	const sf::Transform &t = m_fePresent.get_transform();
	while ( m_wnd.isOpen() )
	{
		while (m_wnd.pollEvent(ev))
		{
			if ( ev.type == sf::Event::Closed )
				return;

			if ( multi_mode && ((ev.type == sf::Event::KeyReleased )
					|| ( ev.type == sf::Event::JoystickButtonReleased )
					|| ( ev.type == sf::Event::MouseButtonReleased )))
				done = true;
			else
			{
				FeInputSingle single( ev, mc_rect, joy_thresh );
				if ( single.get_type() != FeInputSingle::Unsupported )
				{
					if (( ev.type == sf::Event::KeyPressed )
							|| ( ev.type == sf::Event::JoystickButtonPressed )
							|| ( ev.type == sf::Event::MouseButtonPressed ))
						multi_mode = true;
					else if ( ev.type == sf::Event::JoystickMoved )
					{
						multi_mode = true;
						joystick_moves.insert( std::pair<int,int>( ev.joystickMove.joystickId, ev.joystickMove.axis ) );
					}

					bool dup=false;

					std::vector< FeInputSingle >::iterator it;
					for ( it = entry.inputs.begin(); it != entry.inputs.end(); ++it )
					{
						if ( (*it) == single )
						{
							dup=true;
							break;
						}
					}

					if ( !dup )
					{
						entry.inputs.push_back( single );
						timeout.restart();
					}

					if ( !multi_mode )
						done = true;
				}
				else if ( ev.type == sf::Event::JoystickMoved )
				{
					// test if a joystick has been released
					std::pair<int,int> test( ev.joystickMove.joystickId, ev.joystickMove.axis );

					if ( joystick_moves.find( test ) != joystick_moves.end() )
						done = true;
				}
			}
		}

		if ( timeout.getElapsedTime() > sf::seconds( 6 ) )
			done = true;

		if ( done )
		{
			map_str = entry.as_string();
			conflict = m_feSettings.input_conflict_check( entry );
			return;
		}

		if ( m_fePresent.tick() )
			redraw = true;

		if ( redraw )
		{
			m_wnd.clear();
			m_wnd.draw( m_fePresent, t );
			m_wnd.draw( message, t );
			m_wnd.display();
			redraw = false;
		}
		else
			sf::sleep( sf::milliseconds( 30 ) );
	}
}

bool FeOverlay::config_dialog()
{
	FeConfigMenu m;
	bool settings_changed=false;
	if ( display_config_dialog( &m, settings_changed ) < 0 )
		m_wnd.close();

	return settings_changed;
}

bool FeOverlay::edit_game_dialog()
{
	FeEditGameMenu m;
	bool settings_changed=false;

	if ( display_config_dialog( &m, settings_changed ) < 0 )
		m_wnd.close();

	// TODO: should only return true when setting_changed is true or when user deleted
	// the rom completely
	return true;
}

int FeOverlay::display_config_dialog(
	FeBaseConfigMenu *m,
	bool &parent_setting_changed )
{
	FeConfigContextImp ctx( m_feSettings, *this );
	ctx.update_to_menu( m );

	//
	// Set up display objects
	//
	sf::Vector2i size;
	sf::Vector2f text_scale;
	int char_size;
	get_common( size, text_scale, char_size );

	const sf::Font *font = m_fePresent.get_default_font();
	std::vector<sf::Drawable *> draw_list;
	float slice = size.y / 8;

	sf::RectangleShape bg( sf::Vector2f( size.x, size.y ) );
	bg.setFillColor( m_bgColour );
	bg.setOutlineColor( m_textColour );
	bg.setOutlineThickness( -2 );
	draw_list.push_back( &bg );

	FeTextPrimative heading( font, m_selColour, sf::Color::Transparent, char_size / 2 );
	heading.setSize( size.x, slice );
	heading.setOutlineColor( m_textColour );
	heading.setOutlineThickness( -2 );
	heading.setTextScale( text_scale );
	heading.setString( ctx.title );
	draw_list.push_back( &heading );

	unsigned int width = size.x - 4;
	if ( ctx.style == FeConfigContext::EditList )
		width = size.x / 2 - 2;

	int rows = ( size.y - slice * 2 ) / ( char_size * 0.75 * text_scale.y );

	//
	// The "settings" (left) list, also used to list submenu and exit options...
	//
	FePresentableParent temp;
	FeListBox sdialog( temp,
		font,
		m_textColour,
		sf::Color::Transparent,
		m_selColour,
		sf::Color( 0, 0, 200, 200 ),
		char_size / 2,
		rows );

	sdialog.setPosition( 2, slice );
	sdialog.setSize( width, size.y - slice * 2 );
	sdialog.init_dimensions();
	sdialog.setTextScale( text_scale );
	draw_list.push_back( &sdialog );

	//
	// The "values" (right) list shows the values corresponding to a setting.
	//
	FeListBox vdialog( temp,
		font,
		m_textColour,
		sf::Color::Transparent,
		m_textColour,
		sf::Color( 0, 0, 200, 200 ),
		char_size / 2,
		rows );

	if ( ctx.style == FeConfigContext::EditList )
	{
		//
		// We only use the values listbox in the "EditList" mode
		//
		vdialog.setPosition( width + 2, slice );
		vdialog.setSize( width, size.y - slice * 2 );
		vdialog.init_dimensions();
		vdialog.setTextScale( text_scale );
		draw_list.push_back( &vdialog );
	}

	FeTextPrimative footer( font,
		m_textColour,
		sf::Color::Transparent,
		char_size / 3 );

	footer.setPosition( 0, size.y - slice );
	footer.setSize( size.x, slice );
	footer.setOutlineColor( m_textColour );
	footer.setOutlineThickness( -2 );
	footer.setWordWrap( true );
	footer.setTextScale( text_scale );
	draw_list.push_back( &footer );

	ctx.curr_sel = sdialog.getRowCount() / 2;
	if ( ctx.curr_sel >= (int)ctx.left_list.size() )
		ctx.curr_sel = 0;

	sdialog.setCustomText( ctx.curr_sel, ctx.left_list );
	vdialog.setCustomText( ctx.curr_sel, ctx.right_list );

	if ( !ctx.help_msg.empty() )
	{
		footer.setString( ctx.help_msg );
		ctx.help_msg = "";
	}
	else
	{
		footer.setString( ctx.curr_opt().help_msg );
	}

	FeFlagMinder fm( m_overlay_is_on );

	//
	// Event loop processing
	//
	while ( true )
	{
		FeEventLoopCtx c( draw_list, ctx.curr_sel, ctx.exit_sel, ctx.left_list.size() - 1 );

		init_event_loop( c );
		while ( event_loop( c ) == false )
		{
			footer.setString( ctx.curr_opt().help_msg );

			// we reset the entire Text because edit mode may
			// have changed our list contents
			//
			sdialog.setCustomText( ctx.curr_sel, ctx.left_list );
			vdialog.setCustomText( ctx.curr_sel, ctx.right_list );
		}

		//
		// User selected something, process it
		//
		if ( ctx.curr_sel < 0 ) // exit
		{
			if ( ctx.save_req )
			{
				if ( m->save( ctx ) )
					parent_setting_changed = true;
			}

			return ctx.curr_sel;
		}

		FeBaseConfigMenu *sm( NULL );
		if ( m->on_option_select( ctx, sm ) == false )
			continue;

		if ( !ctx.help_msg.empty() )
		{
			footer.setString( ctx.help_msg );
			ctx.help_msg = "";
		}

		int t = ctx.curr_opt().type;

		if ( t > Opt::INFO )
		{
			if ( ctx.save_req )
			{
				if ( m->save( ctx ) )
					parent_setting_changed = true;

				ctx.save_req=false;
			}

			switch (t)
			{
			case Opt::RELOAD:
				return display_config_dialog( m, parent_setting_changed );
			case Opt::MENU:
				if ( sm )
					return display_config_dialog( sm, parent_setting_changed );
				break;
			case Opt::SUBMENU:
				if ( sm )
				{
					bool test( false );
					int sm_ret = display_config_dialog( sm, test );
					if ( sm_ret < 0 )
						return sm_ret;

					if ( test )
						ctx.save_req = true;

					//
					// The submenu may have changed stuff in this menu, need
					// to update our variables as a result
					//
					ctx.update_to_menu( m );

					sdialog.setCustomText( ctx.curr_sel, ctx.left_list );
					vdialog.setCustomText( ctx.curr_sel, ctx.right_list );
				}
				break;
			case Opt::EXIT:
			case Opt::DEFAULTEXIT:
			default:
				return ctx.curr_sel;
			}
		}
		else if ( t != Opt::INFO ) // Opt::EDIT and Opt::LIST
		{
			//
			// User has selected to edit a specific entry.
			//	Update the UI and enter the appropriate
			// event loop
			//
			sdialog.setSelColor( m_textColour );
			FeTextPrimative *tp = vdialog.setEditMode( true, m_selColour );

			if ( tp == NULL )
				continue;

			if ( t == Opt::LIST )
			{
				if ( !ctx.curr_opt().values_list.empty() )
				{
					int original_value = ctx.curr_opt().get_vindex();
					int new_value = original_value;

					FeEventLoopCtx c( draw_list, new_value, -1, ctx.curr_opt().values_list.size() - 1 );

					init_event_loop( c );
					while ( event_loop( c ) == false )
					{
						tp->setString(ctx.curr_opt().values_list[new_value]);
					}

					if (( new_value >= 0 ) && ( new_value != original_value ))
					{
						ctx.save_req = true;
						ctx.curr_opt().set_value( new_value );
						ctx.right_list[ctx.curr_sel] = ctx.curr_opt().get_value();
					}
				}
			}
			else
			{
				const std::string &e_str( ctx.curr_opt().get_value() );
				std::basic_string<sf::Uint32> str;
				sf::Utf8::toUtf32( e_str.begin(), e_str.end(),
						std::back_inserter( str ) );

				if ( edit_loop( draw_list, str, tp ) == true )
				{
					ctx.save_req = true;

					std::string d_str;
					sf::Utf32::toUtf8(
						str.begin(),
						str.end(),
						std::back_inserter( d_str ) );
					ctx.curr_opt().set_value( d_str );
					ctx.right_list[ctx.curr_sel] = d_str;
				}
			}

			tp->setString( ctx.right_list[ctx.curr_sel] );
			sdialog.setSelColor( m_selColour );
			vdialog.setEditMode( false, m_textColour );
		}
	}
	return ctx.curr_sel;
}

bool FeOverlay::check_for_cancel()
{
	sf::Event ev;
	while (m_wnd.pollEvent(ev))
	{
		FeInputMap::Command c = m_feSettings.map_input( ev );

		if (( c == FeInputMap::Back )
				|| ( c == FeInputMap::Exit )
				|| ( c == FeInputMap::ExitToDesktop ))
			return true;
	}

	return false;
}

void FeOverlay::init_event_loop( FeEventLoopCtx &ctx )
{
	//
	// Make sure the Back and Select buttons are NOT down, to avoid immediately
	// triggering an exit/selection
	//
	const sf::Transform &t = m_fePresent.get_transform();

	sf::Clock timer;
	while (( timer.getElapsedTime() < sf::seconds( 6 ) )
			&& ( m_feSettings.get_current_state( FeInputMap::Back )
				|| m_feSettings.get_current_state( FeInputMap::ExitToDesktop )
				|| m_feSettings.get_current_state( FeInputMap::Select ) ))
	{
		sf::Event ev;
		while (m_wnd.pollEvent(ev))
		{
		}

		if ( m_fePresent.tick() )
		{
			m_wnd.clear();
			m_wnd.draw( m_fePresent, t );

			for ( std::vector<sf::Drawable *>::const_iterator itr=ctx.draw_list.begin();
					itr < ctx.draw_list.end(); ++itr )
				m_wnd.draw( *(*itr), t );

			m_wnd.display();
		}
		else
			sf::sleep( sf::milliseconds( 30 ) );
	}
}

//
// Return true if the user selected something.  False if they have simply
// navigated the selection up or down.
//
bool FeOverlay::event_loop( FeEventLoopCtx &ctx )
{
	const sf::Transform &t = m_fePresent.get_transform();

	bool redraw=true;

	while ( m_wnd.isOpen() )
	{
		sf::Event ev;
		while (m_wnd.pollEvent(ev))
		{
			FeInputMap::Command c = m_feSettings.map_input( ev );

			if (( c != FeInputMap::LAST_COMMAND )
					&& ( c == ctx.extra_exit ))
				c = FeInputMap::Exit;

			switch( c )
			{
			case FeInputMap::Back:
				ctx.sel = ctx.default_sel;
				return true;
			case FeInputMap::ExitToDesktop:
				ctx.sel = -1;
				return true;
			case FeInputMap::Select:
				return true;
			case FeInputMap::Up:
				if (( ev.type == sf::Event::JoystickMoved )
						&& ( ctx.move_event.type == sf::Event::JoystickMoved ))
					return false;

				if ( ctx.sel > 0 )
					ctx.sel--;
				else
					ctx.sel=ctx.max_sel;

				ctx.move_event = ev;
				ctx.move_command = FeInputMap::Up;
				ctx.move_count = 0;
				ctx.move_timer.restart();
				return false;

			case FeInputMap::Down:
				if (( ev.type == sf::Event::JoystickMoved )
						&& ( ctx.move_event.type == sf::Event::JoystickMoved ))
					return false;

				if ( ctx.sel < ctx.max_sel )
					ctx.sel++;
				else
					ctx.sel = 0;

				ctx.move_event = ev;
				ctx.move_command = FeInputMap::Down;
				ctx.move_count = 0;
				ctx.move_timer.restart();
				return false;

			default:
				break;
			}
		}

		if ( m_fePresent.tick() )
			redraw = true;

		if ( redraw )
		{
			m_wnd.clear();
			m_wnd.draw( m_fePresent, t );

			for ( std::vector<sf::Drawable *>::const_iterator itr=ctx.draw_list.begin();
					itr < ctx.draw_list.end(); ++itr )
				m_wnd.draw( *(*itr), t );

			m_wnd.display();
			redraw = false;
		}
		else
			sf::sleep( sf::milliseconds( 30 ) );

		if ( ctx.move_command != FeInputMap::LAST_COMMAND )
		{
			bool cont=false;

			switch ( ctx.move_event.type )
			{
			case sf::Event::KeyPressed:
				if ( sf::Keyboard::isKeyPressed( ctx.move_event.key.code ) )
					cont=true;
				break;

			case sf::Event::MouseButtonPressed:
				if ( sf::Mouse::isButtonPressed( ctx.move_event.mouseButton.button ) )
					cont=true;
				break;

			case sf::Event::JoystickButtonPressed:
				if ( sf::Joystick::isButtonPressed(
						ctx.move_event.joystickButton.joystickId,
						ctx.move_event.joystickButton.button ) )
					cont=true;
				break;

			case sf::Event::JoystickMoved:
				{
					sf::Joystick::update();

					float pos = sf::Joystick::getAxisPosition(
							ctx.move_event.joystickMove.joystickId,
							ctx.move_event.joystickMove.axis );
					if ( std::abs( pos ) > m_feSettings.get_joy_thresh() )
						cont=true;
				}
				break;

			default:
				break;
			}

			if ( cont )
			{
				int t = ctx.move_timer.getElapsedTime().asMilliseconds();
				if ( t > 500 + ctx.move_count * m_feSettings.selection_speed() )
				{
					if (( ctx.move_command == FeInputMap::Up )
								&& ( ctx.sel > 0 ))
					{
						ctx.sel--;
						ctx.move_count++;
						return false;
					}
					else if (( ctx.move_command == FeInputMap::Down )
								&& ( ctx.sel < ctx.max_sel ))
					{
						ctx.sel++;
						ctx.move_count++;
						return false;
					}
				}
			}
			else
			{
				ctx.move_command = FeInputMap::LAST_COMMAND;
				ctx.move_event = sf::Event();
			}
		}
	}
	return true;
}

class FeKeyRepeat
{
private:
	sf::RenderWindow &m_wnd;
public:
	FeKeyRepeat( sf::RenderWindow &wnd )
	: m_wnd( wnd )
	{
		m_wnd.setKeyRepeatEnabled( true );
	}

	~FeKeyRepeat()
	{
		m_wnd.setKeyRepeatEnabled( false );
	}
};

namespace
{

unsigned char my_char_table[] =
{
	' ',
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'a',
	'b',
	'c',
	'd',
	'e',
	'f',
	'g',
	'h',
	'i',
	'j',
	'k',
	'l',
	'm',
	'n',
	'o',
	'p',
	'q',
	'r',
	's',
	't',
	'u',
	'v',
	'w',
	'x',
	'y',
	'z',
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'?',
	'!',
	',',
	'.',
	':',
	';',
	'/',
	'\\',
	'\"',
	'\'',
	')',
	'(',
	'*',
	'+',
	'-',
	'=',
	'_',
	'$',
	'@',
	'%',
	'&',
	'~',
	0
};

int get_char_idx( unsigned char c )
{
	int i=0;
	while ( my_char_table[i] )
	{
		if ( my_char_table[i] == c )
			return i;
		i++;
	}

	return 0;
}

};

bool FeOverlay::edit_loop( std::vector<sf::Drawable *> d,
			std::basic_string<sf::Uint32> &str, FeTextPrimative *tp )
{
	const sf::Transform &t = m_fePresent.get_transform();

	const sf::Font *font = tp->getFont();
	sf::Text cursor( "_", *font, tp->getCharacterSize() );
	cursor.setColor( tp->getColor() );
	cursor.setStyle( sf::Text::Bold );
	cursor.setScale( tp->getTextScale() );

	int cursor_pos=str.size();
	cursor.setPosition( tp->setString( str, cursor_pos ) );

	bool redraw=true;
	FeKeyRepeat key_repeat_enabler( m_wnd );

	sf::Event joy_guard;
	bool did_delete( false ); // flag if the user just deleted a character using the UI controls

	while ( m_wnd.isOpen() )
	{
		sf::Event ev;
		while (m_wnd.pollEvent(ev))
		{
			switch ( ev.type )
			{
			case sf::Event::Closed:
				return false;

			case sf::Event::TextEntered:

				did_delete = false;

				if ( ev.text.unicode == 8 ) // backspace
				{
					if ( cursor_pos > 0 )
					{
						str.erase( cursor_pos - 1, 1 );
						cursor_pos--;
					}
					redraw = true;
					break;
				}

				//
				// Don't respond to control characters < 32 (line feeds etc.)
				// and don't handle 127 (delete) here, it is dealt with as a keypress
				//
				if (( ev.text.unicode < 32 ) || ( ev.text.unicode == 127 ))
					break;

				if ( cursor_pos < (int)str.size() )
					str.insert( cursor_pos, 1, ev.text.unicode );
				else
					str += ev.text.unicode;

				cursor_pos++;
				redraw = true;
				break;

			case sf::Event::KeyPressed:

				did_delete = false;

				switch ( ev.key.code )
				{
				case sf::Keyboard::Left:
					if ( cursor_pos > 0 )
						cursor_pos--;

					redraw = true;
					break;

				case sf::Keyboard::Right:
					if ( cursor_pos < (int)str.size() )
						cursor_pos++;

					redraw = true;
					break;

				case sf::Keyboard::Return:
					return true;

				case sf::Keyboard::Escape:
					return false;

				case sf::Keyboard::End:
					cursor_pos = str.size();
					redraw = true;
					break;

				case sf::Keyboard::Home:
					cursor_pos = 0;
					redraw = true;
					break;

				case sf::Keyboard::Delete:
					if ( cursor_pos < (int)str.size() )
						str.erase( cursor_pos, 1 );

					redraw = true;
					break;

				case sf::Keyboard::V:
#ifdef SFML_SYSTEM_MACOS
					if ( ev.key.system )
#else
					if ( ev.key.control )
#endif
					{
						std::basic_string<sf::Uint32> temp = clipboard_get_content();
						str.insert( cursor_pos, temp.c_str() );
						cursor_pos += temp.length();
					}
					redraw = true;
					break;

				default:
					break;
				}
				break;

			default:
				//
				// Handle UI actions from non-keyboard input
				//
				{
					FeInputMap::Command c = m_feSettings.map_input( ev );

					switch ( c )
					{
					case FeInputMap::Left:
						if (( ev.type == sf::Event::JoystickMoved )
								&& ( joy_guard.type == sf::Event::JoystickMoved ))
							break;

						did_delete = false;

						if ( cursor_pos > 0 )
							cursor_pos--;

						redraw = true;

						if ( ev.type == sf::Event::JoystickMoved )
							joy_guard = ev;
						break;

					case FeInputMap::Right:
						if (( ev.type == sf::Event::JoystickMoved )
								&& ( joy_guard.type == sf::Event::JoystickMoved ))
							break;

						did_delete = false;

						if ( cursor_pos < (int)str.size() )
							cursor_pos++;

						redraw = true;

						if ( ev.type == sf::Event::JoystickMoved )
							joy_guard = ev;
						break;

					case FeInputMap::Up:
						if (( ev.type == sf::Event::JoystickMoved )
								&& ( joy_guard.type == sf::Event::JoystickMoved ))
							break;

						if ( cursor_pos < (int)str.size() )
						{
							if ( did_delete )
							{
								str.insert( cursor_pos, 1, my_char_table[0] );
								redraw = true;
								did_delete = false;
							}
							else
							{
								int idx = get_char_idx( str[cursor_pos] );

								if ( my_char_table[idx+1] )
								{
									str[cursor_pos] = my_char_table[idx+1];
									redraw = true;
									did_delete = false;
								}
							}
						}
						else // allow inserting new characters at the end by pressing Up
						{
							str += my_char_table[0];
							redraw = true;
							did_delete = false;
						}

						if ( ev.type == sf::Event::JoystickMoved )
							joy_guard = ev;
						break;

					case FeInputMap::Down:
						if (( ev.type == sf::Event::JoystickMoved )
								&& ( joy_guard.type == sf::Event::JoystickMoved ))
							break;

						if ( did_delete ) // force user to do something else to confirm delete
							break;

						if ( cursor_pos < (int)str.size() )
						{
							int idx = get_char_idx( str[cursor_pos] );

							if ( idx > 0 )
							{
								str[cursor_pos] = my_char_table[idx-1];
								redraw = true;
							}
							else //if ( idx == 0 )
							{
								// Special case allowing the user to delete
								// a character
								str.erase( cursor_pos, 1 );
								redraw = true;
								did_delete = true;
							}
						}

						if ( ev.type == sf::Event::JoystickMoved )
							joy_guard = ev;
						break;

					case FeInputMap::Back:
						return false;

					case FeInputMap::Select:
						return true;
					default:
						break;
					}
				}
			break;
			}

			if ( redraw )
				cursor.setPosition( tp->setString( str, cursor_pos ) );
		}

		if ( m_fePresent.tick() )
			redraw = true;

		if ( redraw )
		{
			m_wnd.clear();
			m_wnd.draw( m_fePresent, t );

			for ( std::vector<sf::Drawable *>::iterator itr=d.begin();
					itr < d.end(); ++itr )
				m_wnd.draw( *(*itr), t );

			m_wnd.draw( cursor, t );
			m_wnd.display();

			redraw = false;
		}
		else
			sf::sleep( sf::milliseconds( 30 ) );

		//
		// Check if previous joystick move is now done (in which case we clear the guard)
		//
		if ( joy_guard.type == sf::Event::JoystickMoved )
		{
			sf::Joystick::update();

			float pos = sf::Joystick::getAxisPosition(
				joy_guard.joystickMove.joystickId,
				joy_guard.joystickMove.axis );

			if ( std::abs( pos ) < m_feSettings.get_joy_thresh() )
				joy_guard = sf::Event();
		}

	}
	return true;
}

bool FeOverlay::common_exit()
{
	if ( !m_feSettings.get_info_bool( FeSettings::ConfirmExit ) )
	{
		m_feSettings.exit_command();
		return true;
	}

	int retval = confirm_dialog( "Exit Attract-Mode?", "", FeInputMap::Exit );

	//
	// retval is 0 if the user confirmed exit.
	// it is <0 if we are being forced to close
	//
	if ( retval < 1 )
	{
		if ( retval == 0 )
			m_feSettings.exit_command();

		return true;
	}

	return false;
}
