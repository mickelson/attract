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

#include "fe_overlay.hpp"
#include "fe_settings.hpp"
#include "fe_config.hpp"
#include "fe_util.hpp"
#include "fe_listbox.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>

class FeConfigContextImp : public FeConfigContext
{
private:
	FeOverlay &m_feo;

public:
	std::vector<std::string> left_list;
	std::vector<std::string> right_list;
	int exit_sel;

	FeConfigContextImp( FeSettings &fes, FeOverlay &feo );
	void edit_dialog( const std::string &m, std::string &t );

	bool confirm_dialog( const std::string &m,
		const std::string &rep );

	void splash_message( const std::string &msg,
		const std::string &rep );

	void input_map_dialog( const std::string &m,
		std::string &ms,
		FeInputMap::Command &conflict );

	void update_to_menu( FeBaseConfigMenu *m );

	bool check_for_cancel();
};

FeConfigContextImp::FeConfigContextImp( FeSettings &fes, FeOverlay &feo )
	: FeConfigContext( fes ), m_feo( feo )
{
}

void FeConfigContextImp::edit_dialog( const std::string &m, std::string &t )
{
	std::string trans;
	fe_settings.get_resource( m, trans );
	m_feo.edit_dialog( trans, t );
}

bool FeConfigContextImp::confirm_dialog( const std::string &msg,
						const std::string &rep )
{
	return !m_feo.confirm_dialog( msg, rep );
}

void FeConfigContextImp::splash_message(
			const std::string &msg, const std::string &rep )
{
	m_feo.splash_message( msg, rep );
}

void FeConfigContextImp::input_map_dialog( const std::string &m,
		std::string &ms,
		FeInputMap::Command &conflict )
{
	std::string t;
	fe_settings.get_resource( m, t );
	m_feo.input_map_dialog( t, ms, conflict );
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

	sf::Event move_event;
	sf::Clock move_timer;
	FeInputMap::Command move_command;
};

FeEventLoopCtx::FeEventLoopCtx(
			const std::vector<sf::Drawable *> &in_draw_list,
			int &in_sel, int in_default_sel, int in_max_sel )
	: draw_list( in_draw_list ),
	sel( in_sel ),
	default_sel( in_default_sel ),
	max_sel( in_max_sel ),
	move_command( FeInputMap::LAST_COMMAND )
{
}

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
				const std::string &rep )
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
	message.setSize( size.x - 4, size.y - 4 );
	message.setTextScale( text_scale );

	std::string msg_str;
	m_feSettings.get_resource( msg, rep, msg_str );

	message.setString( msg_str );

	const sf::Transform &t = m_fePresent.get_transform();

	m_fePresent.tick();

	m_wnd.clear();
	m_wnd.draw( m_fePresent, t );
	m_wnd.draw( bg, t );
	m_wnd.draw( message, t );
	m_wnd.display();
}

int FeOverlay::confirm_dialog( const std::string &msg, const std::string &rep )
{
	std::string msg_str;
	m_feSettings.get_resource( msg, rep, msg_str );

	std::vector<std::string> list(2);
	m_feSettings.get_resource( "Yes", list[0] );
	m_feSettings.get_resource( "No", list[1] );

	return common_basic_dialog(	msg_str, list, 1, 1 );
}

int FeOverlay::common_list_dialog(
			const std::string &title,
			const std::vector < std::string > &options,
			int default_sel,
			int cancel_sel	)
{
	sf::Vector2i size;
	sf::Vector2f text_scale;
	int char_size;
	get_common( size, text_scale, char_size );

	if ( options.size() > 8 )
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

	heading.setString( title );
	draw_list.push_back( &heading );

	FeListBox dialog(
		m_fePresent.get_font(),
		m_textColour,
		sf::Color::Transparent,
		m_selColour,
		m_selBgColour,
		char_size,
		size.y / ( char_size * 1.5 * text_scale.y ) );

	dialog.setPosition( 2, size.y / 8 );
	dialog.setSize( size.x - 4, size.y * 7 / 8 );
	dialog.init();
	dialog.setTextScale( text_scale );
	draw_list.push_back( &dialog );

	int sel = default_sel;;
	dialog.setText( sel, options );

	FeEventLoopCtx c( draw_list, sel, cancel_sel, options.size() - 1 );

	m_overlay_is_on = true;
	while ( event_loop( c ) == false )
		dialog.setText( sel, options );
	m_overlay_is_on = false;

	return sel;
}

int FeOverlay::languages_dialog()
{
	std::vector<std::string> ll;
	m_feSettings.get_languages_list( ll );

	if ( ll.size() <= 1 )
	{
		// if there is nothing to select, then set what we can and get out of here
		//
		m_feSettings.set_language( ll.empty() ? "en" : ll.front() );
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

	int status( -2 ), current_i( 0 ), i( 0 );
	std::vector<std::string> pl;
	for ( std::vector<std::string>::iterator itr=ll.begin(); itr != ll.end(); ++itr )
	{
		if (( status < 0 ) && ( test.compare( 0, 5, (*itr) ) == 0 ))
		{
			current_i = i;
			status = 0;
		}
		else if (( status < -1 ) && ( test.compare( 0, 2, (*itr)) == 0 ))
		{
			current_i = i;
			status = -1;
		}

		pl.push_back( std::string() );
		m_feSettings.get_resource( (*itr), pl.back() );
		i++;
	}

	int sel = common_list_dialog( std::string(),
					pl, current_i,
					pl.size() - 1 );

	if ( sel >= 0 )
		m_feSettings.set_language( ll[sel] );

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
					list.size() - 1 );

	if ( sel == (int)tags_list.size() )
	{
		std::string name;
		edit_dialog( "Enter new tag name", name );

		if ( !name.empty() )
		{
			if ( m_feSettings.set_current_tag( name, true ) )
				m_fePresent.update_to_new_list(); // changing tag status altered our current list
		}
	}
	else if (( sel >=0 ) && ( sel < (int)tags_list.size() ))
	{
		if ( m_feSettings.set_current_tag( tags_list[sel].first, !(tags_list[sel].second) ) )
			m_fePresent.update_to_new_list(); // changing tag status altered our current list
	}

	return sel;
}

int FeOverlay::common_basic_dialog(
			const std::string &msg_str,
			const std::vector<std::string> &list,
			int default_sel,
			int cancel_sel )
{
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

	FeListBox dialog(
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
	dialog.init();
	dialog.setTextScale( text_scale );

	std::vector<sf::Drawable *> draw_list;
	draw_list.push_back( &bg );
	draw_list.push_back( &message );
	draw_list.push_back( &dialog );

	int sel=default_sel;
	dialog.setText( sel, list );

	FeEventLoopCtx c( draw_list, sel, cancel_sel, list.size() - 1 );

	m_overlay_is_on = true;
	while ( event_loop( c ) == false )
		dialog.setText( sel, list );
	m_overlay_is_on = false;

	return sel;
}

void FeOverlay::edit_dialog(
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

	m_overlay_is_on = true;
	if ( edit_loop( draw_list, str, &tp ) == true )
	{
		text.clear();
		sf::Utf32::toUtf8( str.begin(), str.end(), std::back_inserter( text ) );
	}
	m_overlay_is_on = false;
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
	m_overlay_is_on = true;

	const sf::Transform &t = m_fePresent.get_transform();
	while ( m_wnd.isOpen() )
	{
		while (m_wnd.pollEvent(ev))
		{
			if ( ev.type == sf::Event::Closed )
			{
				m_overlay_is_on = false;
				return;
			}

			if ( m_feSettings.config_map_input( ev, map_str, conflict ) )
			{
				m_overlay_is_on = false;
				return;
			}
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
	m_overlay_is_on = false;
}

bool FeOverlay::config_dialog()
{
	FeConfigMenu m;
	bool settings_changed=false;
	if ( display_config_dialog( &m, settings_changed ) < 0 )
		m_wnd.close();

	return settings_changed;
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

	const sf::Font *font = m_fePresent.get_font();
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
	FeListBox sdialog(
		font,
		m_textColour,
		sf::Color::Transparent,
		m_selColour,
		sf::Color( 0, 0, 200, 200 ),
		char_size / 2,
		rows );

	sdialog.setPosition( 2, slice );
	sdialog.setSize( width, size.y - slice * 2 );
	sdialog.init();
	sdialog.setTextScale( text_scale );
	draw_list.push_back( &sdialog );

	//
	// The "values" (right) list shows the values corresponding to a setting.
	//
	FeListBox vdialog(
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
		vdialog.init();
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

	sdialog.setText( ctx.curr_sel, ctx.left_list );
	vdialog.setText( ctx.curr_sel, ctx.right_list );

	if ( !ctx.help_msg.empty() )
	{
		footer.setString( ctx.help_msg );
		ctx.help_msg = "";
	}
	else
	{
		footer.setString( ctx.curr_opt().help_msg );
	}


	//
	// Event loop processing
	//
	while ( true )
	{
		FeEventLoopCtx c( draw_list, ctx.curr_sel, ctx.exit_sel, ctx.left_list.size() - 1 );

		m_overlay_is_on = true;
		while ( event_loop( c ) == false )
		{
			footer.setString( ctx.curr_opt().help_msg );
			sdialog.setText( ctx.curr_sel, ctx.left_list );
			vdialog.setText( ctx.curr_sel, ctx.right_list );
		}
		m_overlay_is_on = false;

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

					sdialog.setText( ctx.curr_sel, ctx.left_list );
					vdialog.setText( ctx.curr_sel, ctx.right_list );
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

					m_overlay_is_on = true;
					while ( event_loop( c ) == false )
					{
						tp->setString(ctx.curr_opt().values_list[new_value]);
					}
					m_overlay_is_on = false;

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

				m_overlay_is_on = true;
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
				m_overlay_is_on = false;
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

		if (( c == FeInputMap::ExitMenu )
				|| ( c == FeInputMap::ExitNoMenu ))
			return true;
	}

	return false;
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

			switch( c )
			{
			case FeInputMap::ExitMenu:
				ctx.sel = ctx.default_sel;
				return true;
			case FeInputMap::ExitNoMenu:
				ctx.sel = -1;
				return true;
			case FeInputMap::Select:
				return true;
			case FeInputMap::Up:
			case FeInputMap::PageUp:
				if ( ctx.sel > 0 )
					ctx.sel--;
				else
					ctx.sel=ctx.max_sel;

				ctx.move_event = ev;
				ctx.move_command = FeInputMap::Up;
				ctx.move_timer.restart();
				return false;

			case FeInputMap::Down:
			case FeInputMap::PageDown:
				if ( ctx.sel < ctx.max_sel )
					ctx.sel++;
				else
					ctx.sel = 0;

				ctx.move_event = ev;
				ctx.move_command = FeInputMap::Down;
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
					if ( abs( pos ) > m_feSettings.get_joy_thresh() )
						cont=true;
				}
				break;

			default:
				break;
			}

			if ( cont )
			{
				int t = ctx.move_timer.getElapsedTime().asMilliseconds();
				if ( t > 500 )
				{
					if (( ctx.move_command == FeInputMap::Up )
								&& ( ctx.sel > 0 ))
					{
						ctx.sel--;
						return false;
					}
					else if (( ctx.move_command == FeInputMap::Down )
								&& ( ctx.sel < ctx.max_sel ))
					{
						ctx.sel++;
						return false;
					}
				}
			}
			else
				ctx.move_command = FeInputMap::LAST_COMMAND;
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
			default:
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

	}
	return true;
}
