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

#ifndef FE_OVERLAY_HPP
#define FE_OVERLAY_HPP

#include <SFML/Graphics.hpp>
#include "fe_present.hpp"

class FeSettings;
class FeBaseConfigMenu;
class FeTextPrimative;

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

	FeOverlay( const FeOverlay & );
	FeOverlay &operator=( const FeOverlay & );

	int get_char_size() const;

	int internal_dialog(
			const std::string &msg_str,
			const std::vector<std::string> &list );

	void input_map_dialog( const std::string &msg_str, std::string &map_str,
			FeInputMap::Command &conflict );
	void edit_dialog( const std::string &msg_str, std::string &text );
	int display_config_dialog( FeBaseConfigMenu *, bool & );

	bool event_loop( std::vector<sf::Drawable *> draw_list,
			int &sel, int default_sel, int max_sel );

	bool edit_loop( std::vector<sf::Drawable *> draw_list,
			std::basic_string<sf::Uint32> &str, FeTextPrimative *lb );

	bool check_for_cancel();

public:
	FeOverlay( sf::RenderWindow &wnd,
		FeSettings &fes,
		FePresent &fep );

	void splash_message( const std::string &, const std::string &rep="" );
	int confirm_dialog( const std::string &msg, const std::string &rep="" );
	bool config_dialog();
	int lists_dialog();
	int filters_dialog();
	int languages_dialog();
	int tags_dialog();
};

#endif
