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

#ifndef FE_PRESENT_HPP
#define FE_PRESENT_HPP

#include <SFML/Graphics.hpp>
#include "fe_base.hpp"
#include "fe_settings.hpp"

class FeListBox;

class FePresent 
	: public sf::Drawable, public FeFileConfigurable
{
private:
	FeSettings *m_feSettings;
	sf::Font *m_currentFont;
	sf::Font &m_defaultFont;
	sf::Font m_layoutFont;

	enum MoveState { MoveNone, MoveUp, MoveDown, MovePageUp, MovePageDown };
	MoveState m_moveState;
	sf::Event m_moveEvent;
	sf::Clock m_moveTimer;
	sf::Clock m_movieStartTimer;

	FeSettings::RotationState m_baseRotation;
	FeSettings::RotationState m_toggleRotation;
	sf::Transform m_rotationTransform;
	sf::Transform m_scaleTransform;

	std::vector<FeBasePresentable *> m_elements;
	std::vector<FeBasePresentable *> m_movies;
	bool m_play_movies;

	FeBaseConfigurable *m_currentConfigObject;
	FeListBox *m_listBox; // we only keep this ptr so we can get page sizes

	void clear();
	void toggle_movie();

	void toggle_rotate( FeSettings::RotationState ); // toggle between none and provided state
	void set_rotation_transform();	

	// Overrides from base classes:
	//
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	int process_setting( const std::string &setting,
			const std::string &value,
			const std::string &fn );

	void set_to_no_lists_message();

public:
	FePresent( FeSettings *fesettings, sf::Font &defaultfont );
	~FePresent( void );

	int load_layout();
	int update( bool reload_list=false );

	bool tick(); // return true if display refresh required
	void play( bool play_state ); // true to play, false to stop
	void toggle_mute();

	bool handle_event( FeInputMap::Command, sf::Event ev );

	int get_page_size();
	const sf::Transform &get_rotation_transform() const;
	const sf::Font *get_font() const;
	void set_default_font( sf::Font &f );

	void perform_autorotate();
};

#endif
