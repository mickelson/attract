/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-2016 Andrew Mickelson
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

#ifndef FE_OVERLAY_HPP
#define FE_OVERLAY_HPP

#include <SFML/Graphics.hpp>
#include "fe_present.hpp"

class FeSettings;
class FeBaseConfigMenu;
class FeTextPrimative;

class FeEventLoopCtx;

class FeOverlay
{
friend class FeConfigContextImp;

private:
	sf::RenderWindow &m_wnd;
	FeSettings &m_feSettings;
	FePresent &m_fePresent;
	const sf::Color m_textColour;
	const sf::Color m_bgColour;
	const sf::Color m_selColour;
	const sf::Color m_selBgColour;
	bool m_overlay_is_on;

	FeOverlay( const FeOverlay & );
	FeOverlay &operator=( const FeOverlay & );

	void get_common(
		sf::Vector2i &size,
		sf::Vector2f &text_scale,
		int &char_size ) const;

	void input_map_dialog( const std::string &msg_str, std::string &map_str,
			FeInputMap::Command &conflict );
	int display_config_dialog( FeBaseConfigMenu *, bool & );

	void init_event_loop( FeEventLoopCtx & );
	bool event_loop( FeEventLoopCtx & );

	bool edit_loop( std::vector<sf::Drawable *> draw_list,
			std::basic_string<sf::Uint32> &str, FeTextPrimative *lb );

public:
	FeOverlay( sf::RenderWindow &wnd,
		FeSettings &fes,
		FePresent &fep );

	void splash_message( const std::string &, const std::string &rep="",
		const std::string &aux="" );
	int confirm_dialog( const std::string &msg,
		const std::string &rep="",
		FeInputMap::Command extra_exit=FeInputMap::LAST_COMMAND );

	bool config_dialog();
	bool edit_game_dialog();
	int languages_dialog();
	int tags_dialog();

	int common_list_dialog(
		const std::string &title,
		const std::vector < std::string > &options,
		int default_sel,
		int cancel_sel,
		FeInputMap::Command extra_exit=FeInputMap::LAST_COMMAND );

	int common_basic_dialog(
		const std::string &message,
		const std::vector<std::string> &options,
		int default_sel,
		int cancel_sel,
		FeInputMap::Command extra_exit=FeInputMap::LAST_COMMAND );

	bool edit_dialog( const std::string &msg_str, std::string &text );

	bool overlay_is_on() const { return m_overlay_is_on; };

	bool check_for_cancel();
};

#endif
