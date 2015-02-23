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

#ifndef FE_PRESENT_HPP
#define FE_PRESENT_HPP

#include <SFML/Graphics.hpp>
#include "fe_presentable.hpp"
#include "fe_settings.hpp"
#include "fe_sound.hpp"
#include "fe_shader.hpp"

class FeImage;
class FeBaseTextureContainer;
class FeText;
class FeListBox;
class FeFontContainer;
class FeSurfaceTextureContainer;

enum FeTransitionType
{
	StartLayout=0,		// var: FromToScreenSaver, FromToFrontend or FromToNoValue
	EndLayout,			// var: FromToScreenSaver, FromToFrontend or FromToNoValue
	ToNewSelection,	// var = index_offset of new selection
	FromOldSelection,	// var == index_offset of old selection
	ToGame,				// var = 0
	FromGame,			// var = 0
	ToNewList,			// var = filter offset of new filter (if available), otherwise 0
	EndNavigation		// var = 0
};

//
// Container class for use in our font pool
//
class FeFontContainer
{
public:
	void set_font( const std::string &n );

	const sf::Font &get_font() const { return m_font; };
	const std::string &get_name() const { return m_name; };

private:
	sf::Font m_font;
	std::string m_name;
};

class FePresent
	: public sf::Drawable
{
	friend class FeSurfaceTextureContainer;
	friend class FeVM;

protected:
	enum FromToType
	{
		FromToNoValue=0,
		FromToScreenSaver=1,
		FromToFrontend=2
	};

	FeSettings *m_feSettings;

	const FeFontContainer *m_currentFont;
	FeFontContainer &m_defaultFont;
	std::string m_layoutFontName;

	sf::Clock m_layoutTimer;
	sf::Time m_lastInput;

	FeSettings::RotationState m_baseRotation;
	FeSettings::RotationState m_toggleRotation;
	sf::Transform m_transform;

	std::vector<FeBasePresentable *> m_elements;
	std::vector<FeBaseTextureContainer *> m_texturePool;
	std::vector<FeSound *> m_sounds;
	std::vector<FeShader *> m_scriptShaders;
	std::vector<FeFontContainer *> m_fontPool;
	bool m_playMovies;
	bool m_screenSaverActive;
	int m_user_page_size;

	FeListBox *m_listBox; // we only keep this ptr so we can get page sizes
	sf::Vector2i m_layoutSize;
	sf::Vector2f m_layoutScale;
	sf::Vector2i m_outputSize;

	FeShader *m_emptyShader;

	FePresent( const FePresent & );
	FePresent &operator=( const FePresent & );

	virtual void clear();
	void toggle_movie();

	void toggle_rotate( FeSettings::RotationState ); // toggle between none and provided state
	void set_transforms();

	// Overrides from base classes:
	//
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	FeImage *add_image(bool a, const std::string &n, int x, int y, int w, int h, std::vector<FeBasePresentable *> &l);
	FeImage *add_clone(FeImage *, std::vector<FeBasePresentable *> &l);
	FeText *add_text(const std::string &n, int x, int y, int w, int h, std::vector<FeBasePresentable *> &l);
	FeListBox *add_listbox(int x, int y, int w, int h, std::vector<FeBasePresentable *> &l);
	FeImage *add_surface(int w, int h, std::vector<FeBasePresentable *> &l);
	FeSound *add_sound(const char *n);
	FeShader *add_shader(FeShader::Type type, const char *shader1, const char *shader2);
	int get_layout_width() const;
	int get_layout_height() const;
	int get_base_rotation() const;
	int get_toggle_rotation() const;
	const char *get_display_name() const;
	const char *get_filter_name() const;
	int get_filter_index() const;
	void set_filter_index( int );
	int get_list_size() const;
	int get_selection_index() const;
	int get_sort_by() const;
	bool get_reverse_order() const;
	int get_list_limit() const;

	void set_selection_index( int );
	const char *get_layout_font() const;
	void set_layout_width( int );
	void set_layout_height( int );
	void set_base_rotation( int );
	void set_toggle_rotation( int );
	void set_layout_font( const char * );
	void set_video_play_state( bool state );

public:
	FePresent( FeSettings *fesettings, FeFontContainer &defaultfont );
	virtual ~FePresent( void );

	void load_screensaver();
	void load_layout( bool initial_load=false );

	int update( bool reload_list=false );
	void update_to_new_list( int var=0 );
	void on_end_navigation();

	bool tick(); // run vm on_tick and update videos.  return true if redraw required
	bool video_tick(); // update videos only. return true if redraw required

	bool saver_activation_check();
	void on_stop_frontend();
	void pre_run();
	void post_run();
	void toggle_mute();

	bool reset_screen_saver();
	bool handle_event( FeInputMap::Command );

	void change_selection( int step, bool end_navigation=true );

	FeSettings *get_fes() const { return m_feSettings; };

	int get_page_size() const;
	void set_page_size( int );

	const sf::Transform &get_transform() const;
	const sf::Font *get_font() const; // get the current font (used by overlay)

	float get_layout_scale_x() const;
	float get_layout_scale_y() const;

	// Get a font from the font pool, loading it if necessary
	const FeFontContainer *get_pooled_font( const std::string &n );

	const sf::Vector2i &get_layout_size() const { return m_layoutSize; }
	FeShader *get_empty_shader();

	//
	// Script static functions
	//
	static FePresent *script_get_fep();
	static void script_do_update( FeBaseTextureContainer * );
	static void script_do_update( FeBasePresentable * );
	static void script_flag_redraw();

	//
	//
	virtual void on_new_layout( const std::string &path, const std::string &filename, const FeLayoutInfo &layout_params )=0;
	virtual bool on_tick()=0;
	virtual bool on_transition( FeTransitionType, int var )=0;
	virtual void flag_redraw()=0;
	virtual void init_with_default_layout()=0;
};


#endif
