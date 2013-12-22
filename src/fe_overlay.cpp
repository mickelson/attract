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
	std::string t;
	fe_settings.get_resource( msg, rep, t );

	std::vector<std::string> list(2);
	fe_settings.get_resource( "Yes", list[0] );
	fe_settings.get_resource( "No", list[1] );

	return (( m_feo.internal_dialog( t, list ) == 0 ) ? true : false ); 
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

FeOverlay::FeOverlay( sf::RenderWindow &wnd,
		FeSettings &fes,
		FePresent &fep )
	: m_wnd( wnd ), 
	m_feSettings( fes ),
	m_fePresent( fep ), 
	m_textColour( sf::Color::White ),
	m_bgColour( sf::Color( 0, 0, 0, 200 ) ),
	m_selColour( sf::Color::Yellow ),
	m_selBgColour( sf::Color( 0, 0, 200, 200 ) )
{
	sf::VideoMode vm = sf::VideoMode::getDesktopMode();
	m_characterSize = vm.height / 12; // 64 on a 1024x768 display
}

void FeOverlay::splash_message( const std::string &msg,
				const std::string &rep )
{
	FeTextPrimative message(
		m_fePresent.get_font(),
		m_textColour,
		m_bgColour, 
		m_characterSize );

	message.setWordWrap( true );

	sf::Vector2u size = m_wnd.getSize();
	message.setSize( size.x, size.y );

	std::string msg_str;
	m_feSettings.get_resource( msg, rep, msg_str );

	message.setString( msg_str );

	const sf::Transform &t = m_fePresent.get_rotation_transform();

	m_fePresent.tick( NULL );

	m_wnd.clear();
	m_wnd.draw( m_fePresent, t );
	m_wnd.draw( message, t );
	m_wnd.display();
}

int FeOverlay::exit_dialog()
{
	std::string exit_str;
	m_feSettings.get_resource( "Exit Attract-Mode?", exit_str );

	std::vector<std::string> list(2);
	m_feSettings.get_resource( "Yes", list[0] );
	m_feSettings.get_resource( "No", list[1] );

	return internal_dialog(	exit_str, list );
}

int FeOverlay::lists_dialog()
{
	sf::Vector2u size = m_wnd.getSize();
	std::vector<std::string> list;
	m_feSettings.get_list_names( list );

	FeListBox dialog(
		m_fePresent.get_font(),
		m_textColour,
		m_bgColour,
		m_selColour,
		m_selBgColour,
		m_characterSize,
		size.y / m_characterSize );

	dialog.setPosition( 0, 0 );
	dialog.setSize( size.x, size.y );
	dialog.init();

	std::vector<sf::Drawable *> draw_list( 1, &dialog );

	int last_list = list.size() - 1;

	if ( m_feSettings.get_lists_menu_exit() )
	{
		//
		// Add an exit option at the end of the lists menu
		//
		std::string exit_str;
		m_feSettings.get_resource( "Exit Attract-Mode", exit_str );
		list.push_back( exit_str );
	}

	int current_i = m_feSettings.get_current_list_index();
	int sel = current_i;
	dialog.setText( sel, list );
	while ( event_loop( draw_list, sel, current_i, list.size() - 1 ) == false )
		dialog.setText( sel, list );

	// test if the exit option selected, return -2 if it has been
	if ( sel > last_list )
		sel = -2;

	return sel;
}

int FeOverlay::filters_dialog()
{
	sf::Vector2u size = m_wnd.getSize();
	std::vector<std::string> list;
	m_feSettings.get_current_list_filter_names( list );

	FeListBox dialog(
		m_fePresent.get_font(),
		m_textColour,
		m_bgColour,
		m_selColour,
		m_selBgColour,
		m_characterSize,
		size.y / m_characterSize );

	dialog.setPosition( 0, 0 );
	dialog.setSize( size.x, size.y );
	dialog.init();

	std::vector<sf::Drawable *> draw_list( 1, &dialog );

	int current_i = m_feSettings.get_current_filter_index();
	int sel = current_i;
	dialog.setText( sel, list );
	while ( event_loop( draw_list, sel, current_i, list.size() - 1 ) == false )
		dialog.setText( sel, list );

	return sel;
}

int FeOverlay::internal_dialog( 
			const std::string &msg_str,
			const std::vector<std::string> &list )
{
	sf::Vector2u size = m_wnd.getSize();
	float slice = size.y / 2;

	FeTextPrimative message(
		m_fePresent.get_font(),
		m_textColour,
		m_bgColour,
		m_characterSize );
	message.setWordWrap( true );

	FeListBox dialog(
		m_fePresent.get_font(),
		m_textColour,
		m_bgColour,
		m_selColour,
		m_selBgColour,
		m_characterSize,
		( size.y - slice ) / m_characterSize );

	message.setSize( size.x, slice );
	message.setString( msg_str );

	dialog.setPosition( 0, slice );
	dialog.setSize( size.x, size.y - slice );
	dialog.init();

	std::vector<sf::Drawable *> draw_list;
	draw_list.push_back( &message );
	draw_list.push_back( &dialog );

	int sel=1;
	dialog.setText( sel, list );
	while ( event_loop( draw_list, sel, 1, list.size() - 1 ) == false )
		dialog.setText( sel, list );

	return sel;
}

void FeOverlay::edit_dialog( 
			const std::string &msg_str,
			std::string &text )
{
	FeTextPrimative message( m_fePresent.get_font(), m_textColour,
		m_bgColour, m_characterSize );
	message.setWordWrap( true );

	FeTextPrimative tp( m_fePresent.get_font(), m_textColour,
		m_bgColour, m_characterSize );

	sf::Vector2u size = m_wnd.getSize();
	float slice = size.y / 3;

	message.setSize( size.x, slice );
	message.setString( msg_str );

	tp.setPosition( 0, slice );
	tp.setSize( size.x, size.y - slice );

	std::vector<sf::Drawable *> draw_list;
	draw_list.push_back( &message );
	draw_list.push_back( &tp );

	std::basic_string<sf::Uint32> str;
	sf::Utf8::toUtf32( text.begin(), text.end(), std::back_inserter( str ) );

	if ( edit_loop( draw_list, str, &tp ) == true )
	{
		text.clear();
		sf::Utf32::toUtf8( str.begin(), str.end(), std::back_inserter( text ) );
	}
}

void FeOverlay::input_map_dialog( 
			const std::string &msg_str,
			std::string &map_str,
			FeInputMap::Command &conflict )
{
	FeTextPrimative message( m_fePresent.get_font(), m_textColour,
		m_bgColour, m_characterSize );
	message.setWordWrap( true );

	sf::Vector2u s = m_wnd.getSize();
	message.setSize( s.x, s.y );
	message.setString( msg_str );

	FeInputMap &im = m_feSettings.get_input_map();
	im.init_config_map_input();

	const sf::Transform &t = m_fePresent.get_rotation_transform();
	while ( m_wnd.isOpen() )
	{
		sf::Event ev;
		while (m_wnd.pollEvent(ev))
		{
			if ( ev.type == sf::Event::Closed )
				return;

			if ( im.config_map_input( ev, map_str, conflict ) )
				return;
		}

		m_fePresent.tick( NULL );
		m_wnd.clear();
		m_wnd.draw( m_fePresent, t );
		m_wnd.draw( message, t );
		m_wnd.display();
	}
}

bool FeOverlay::config_dialog()
{
	m_wnd.setKeyRepeatEnabled( true );

	FeConfigMenu m;
	bool settings_changed=false;
	if ( display_config_dialog( &m, settings_changed ) < 0 )
		m_wnd.close();

	m_wnd.setKeyRepeatEnabled( false );

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
	const sf::Font *font = m_fePresent.get_font();
	std::vector<sf::Drawable *> draw_list;
	sf::Vector2u size = m_wnd.getSize();
	float slice = size.y / 8;

	sf::RectangleShape bg( sf::Vector2f( size.x, size.y ) );
	bg.setFillColor( m_bgColour );
	bg.setOutlineColor( m_textColour );
	bg.setOutlineThickness( -2 );
	draw_list.push_back( &bg );

	FeTextPrimative heading( font, m_selColour, sf::Color::Transparent, m_characterSize / 2 );
	heading.setSize( size.x, slice );
	heading.setOutlineColor( m_textColour );
	heading.setOutlineThickness( -2 );
	heading.setString( ctx.title );
	draw_list.push_back( &heading );

	unsigned int width = size.x - 4;
	if ( ctx.style == FeConfigContext::EditList )
		width = size.x / 2 - 2;

	int rows = ( size.y - slice * 2 ) / ( m_characterSize / 2 );

	//
	// The "settings" (left) list, also used to list submenu and exit options...
	//
	FeListBox sdialog( 
		font,
		m_textColour, 
		sf::Color::Transparent, 
		m_selColour,
		sf::Color( 0, 0, 200, 200 ),
		m_characterSize / 2,
		rows );

	sdialog.setPosition( 2, slice );
	sdialog.setSize( width, size.y - slice * 2 );
	sdialog.init();
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
		m_characterSize / 2,
		rows );

	if ( ctx.style == FeConfigContext::EditList )
	{
		//	
		// We only use the values listbox in the "EditList" mode
		//
		vdialog.setPosition( width + 2, slice );
		vdialog.setSize( width, size.y - slice * 2 );
		vdialog.init();
		draw_list.push_back( &vdialog );
	}

	FeTextPrimative footer( font, 
				m_textColour, 
				sf::Color::Transparent, 
				m_characterSize / 3 );
	
	footer.setPosition( 0, size.y - slice );
	footer.setSize( size.x, slice );
	footer.setOutlineColor( m_textColour );
	footer.setOutlineThickness( -2 );
	footer.setWordWrap( true );
	draw_list.push_back( &footer );

	ctx.curr_sel = sdialog.getRowCount() / 2;
	if ( ctx.curr_sel >= (int)ctx.left_list.size() )
		ctx.curr_sel = ctx.left_list.size() - 1;

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
		while ( event_loop( 
				draw_list, 
				ctx.curr_sel, 
				ctx.exit_sel, 
				ctx.left_list.size() - 1 ) == false )
		{
			footer.setString( ctx.curr_opt().help_msg );
			sdialog.setText( ctx.curr_sel, ctx.left_list );
			vdialog.setText( ctx.curr_sel, ctx.right_list );
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
				int original_value = ctx.curr_opt().get_vindex();
				int new_value = original_value;

				while ( event_loop( 
							draw_list, 
							new_value, -1, 
							ctx.curr_opt().values_list.size() - 1 ) == false )
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
		FeInputMap::Command c = m_feSettings.map( ev );

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
bool FeOverlay::event_loop( std::vector<sf::Drawable *> d, 
			int &sel, int default_sel, int max_sel )
{
	const sf::Transform &t = m_fePresent.get_rotation_transform();

	while ( m_wnd.isOpen() )
	{
		sf::Event ev;
		while (m_wnd.pollEvent(ev))
		{
			FeInputMap::Command c = m_feSettings.map( ev );

			switch( c )
			{
			case FeInputMap::ExitMenu:
				sel = default_sel;
				return true;
			case FeInputMap::ExitNoMenu:
				sel = -1;
				return true;
			case FeInputMap::Select:
				return true;
			case FeInputMap::Up:
			case FeInputMap::PageUp:
				if ( sel > 0 )
				{
					sel--;
					return false;
				}
				break;
			case FeInputMap::Down:
			case FeInputMap::PageDown:
				if ( sel < max_sel )
				{
					sel++;
					return false;
				}
				break;
			default:
				break;
			}
		}

		m_fePresent.tick( NULL );
		m_wnd.clear();
		m_wnd.draw( m_fePresent, t );

		for ( std::vector<sf::Drawable *>::iterator itr=d.begin(); 
				itr < d.end(); ++itr )
			m_wnd.draw( *(*itr), t ); 

		m_wnd.display();
	}
	return true;
}

bool FeOverlay::edit_loop( std::vector<sf::Drawable *> d,
			std::basic_string<sf::Uint32> &str, FeTextPrimative *tp )
{
	const sf::Transform &t = m_fePresent.get_rotation_transform();

	const sf::Font *font = tp->getFont();
	sf::Text cursor( "_", *font, tp->getCharacterSize() );
	cursor.setColor( tp->getColor() );
	cursor.setStyle( sf::Text::Bold );

	int cursor_pos=str.size();
	cursor.setPosition( tp->setString( str, cursor_pos ) );

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

				switch ( ev.text.unicode )
				{
				case 8: // Backspace
					if ( cursor_pos > 0 )
					{
						str.erase( cursor_pos - 1, 1 );
						cursor_pos--;
					}
					break;

				case 13: // Return (ignore here, deal with as keypress event)
					break;

				case 127: // Delete
					if ( cursor_pos < (int)str.size() )
						str.erase( cursor_pos, 1 );
					break;

				default: // General text entry
					if ( cursor_pos < (int)str.size() )
						str.insert( cursor_pos, 1, ev.text.unicode );
					else
						str += ev.text.unicode;

					cursor_pos++;
				}

				break;

			case sf::Event::KeyPressed:
				switch ( ev.key.code )
				{
				case sf::Keyboard::Left:
					if ( cursor_pos > 0 )
						cursor_pos--;
					break;

				case sf::Keyboard::Right:
					if ( cursor_pos < (int)str.size() )
						cursor_pos++;
					break;

				case sf::Keyboard::Return:
					return true;

				case sf::Keyboard::Escape:
					return false;

				default:
					break;
				}
			default:
			break;
			}

			cursor.setPosition( tp->setString( str, cursor_pos ) );
		}

		m_fePresent.tick( NULL );
		m_wnd.clear();
		m_wnd.draw( m_fePresent, t );

		for ( std::vector<sf::Drawable *>::iterator itr=d.begin(); 
				itr < d.end(); ++itr )
			m_wnd.draw( *(*itr), t );

		m_wnd.draw( cursor, t );
		m_wnd.display();
	}
	return true;
}
