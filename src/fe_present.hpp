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

namespace Sqrat
{
	class Table;
};

enum FeTransitionType
{
	StartLayout=0,		// var: FromToScreenSaver, FromToFrontend or FromToNoValue
	EndLayout,			// var: FromToScreenSaver, FromToFrontend or FromToNoValue
	ToNewSelection,	// var = index_offset of new selection
	FromOldSelection,	// var == index_offset of old selection
	ToGame,				// var = 0
	FromGame,			// var = 0
	ToNewList			// var = 0
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
	friend void script_do_update( FeBasePresentable * );
	friend FeShader *script_get_empty_shader();
	friend class FeSurfaceTextureContainer;

private:
	static const char *transitionTypeStrings[];

	enum FromToType
	{
		FromToNoValue=0,
		FromToScreenSaver=1,
		FromToFrontend=2
	};

	FeSettings *m_feSettings;
	const FeScriptConfigurable *m_currentScriptConfig;

	const FeFontContainer *m_currentFont;
	FeFontContainer &m_defaultFont;
	std::string m_layoutFontName;

	enum MoveState { MoveNone, MoveUp, MoveDown, MovePageUp, MovePageDown };
	MoveState m_moveState;
	sf::Event m_moveEvent;
	sf::Clock m_moveTimer;
	sf::Clock m_layoutTimer;
	sf::Time m_lastInput;

	FeSettings::RotationState m_baseRotation;
	FeSettings::RotationState m_toggleRotation;
	sf::Transform m_transform;

	std::vector<FeBasePresentable *> m_elements;
	std::vector<FeBaseTextureContainer *> m_texturePool;
	std::vector<FeScriptSound *> m_scriptSounds;
	std::vector<FeShader *> m_scriptShaders;
	std::vector<FeFontContainer *> m_fontPool;
	std::vector<std::string> m_ticksList;
	std::vector<std::string> m_transitionList;
	bool m_playMovies;
	bool m_screenSaverActive;

	// flag if a redraw has been triggered during script callback execution
	//
	bool m_redrawTriggered;

	FeListBox *m_listBox; // we only keep this ptr so we can get page sizes
	sf::Vector2i m_layoutSize;
	sf::Vector2f m_layoutScale;
	sf::Vector2i m_outputSize;

	FeShader *m_emptyShader;

	FePresent( const FePresent & );
	FePresent &operator=( const FePresent & );

	int get_no_wrap_step( int step );
	void clear();
	void toggle_movie();

	void toggle_rotate( FeSettings::RotationState ); // toggle between none and provided state
	void set_transforms();

	// Overrides from base classes:
	//
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	// Scripting functionality
	//
	void vm_close();
	void vm_init();
	void vm_on_new_layout( const std::string &path, const std::string &filename, const FeLayoutInfo &layout_params );
	bool vm_on_tick();
	bool vm_on_transition( FeTransitionType, int var, sf::RenderWindow *wnd );

	FeImage *add_image(bool a, const std::string &n, int x, int y, int w, int h, std::vector<FeBasePresentable *> &l);
	FeImage *add_clone(FeImage *, std::vector<FeBasePresentable *> &l);
	FeText *add_text(const std::string &n, int x, int y, int w, int h, std::vector<FeBasePresentable *> &l);
	FeListBox *add_listbox(int x, int y, int w, int h, std::vector<FeBasePresentable *> &l);
	FeImage *add_surface(int w, int h, std::vector<FeBasePresentable *> &l);
	FeScriptSound *add_sound(const std::string &n);
	FeShader *add_shader(FeShader::Type type, const char *shader1, const char *shader2);
	void add_ticks_callback(const std::string &);
	void add_transition_callback(const std::string &);
	int get_layout_width() const;
	int get_layout_height() const;
	int get_base_rotation() const;
	int get_toggle_rotation() const;
	const char *get_list_name() const;
	const char *get_filter_name() const;
	int get_list_size() const;
	int get_list_index() const;
	void set_list_index( int );
	const char *get_layout_font() const;
	void set_layout_width( int );
	void set_layout_height( int );
	void set_base_rotation( int );
	void set_toggle_rotation( int );
	void set_layout_font( const char * );
	FeShader *get_empty_shader();

	static bool internal_do_nut(const std::string &, const std::string &);


public:
	FePresent( FeSettings *fesettings, FeFontContainer &defaultfont );
	~FePresent( void );

	void load_screensaver( sf::RenderWindow *wnd );
	void load_layout( sf::RenderWindow *wnd, bool initial_load=false );

	int update( bool reload_list=false );
	void update_to_new_list( sf::RenderWindow *wnd );

	bool tick( sf::RenderWindow *w ); // return true if display refresh required
	bool saver_activation_check(  sf::RenderWindow *w );
	void on_stop_frontend( sf::RenderWindow *w );
	void pre_run( sf::RenderWindow *w );
	void post_run( sf::RenderWindow *w );
	void toggle_mute();

	bool reset_screen_saver( sf::RenderWindow *w );
	bool handle_event( FeInputMap::Command, const sf::Event &ev, sf::RenderWindow *w );

	FeSettings *get_fes() const { return m_feSettings; };
	int get_page_size() const;
	const sf::Transform &get_transform() const;
	const sf::Font *get_font() const; // get the current font (used by overlay)

	float get_layout_scale_x() const;
	float get_layout_scale_y() const;

	// Get a font from the font pool, loading it if necessary
	const FeFontContainer *get_pooled_font( const std::string &n );

	const sf::Vector2i &get_layout_size() const { return m_layoutSize; }

	void perform_autorotate();

	bool get_screensaver_active() { return m_screenSaverActive; }

	void flag_redraw();

	static FeImage *cb_add_image(const char *,int, int, int, int);
	static FeImage *cb_add_image(const char *, int, int);
	static FeImage *cb_add_image(const char *);
	static FeImage *cb_add_artwork(const char *,int, int, int, int);
	static FeImage *cb_add_artwork(const char *, int, int);
	static FeImage *cb_add_artwork(const char *);
	static FeImage *cb_add_clone(FeImage *);
	static FeText *cb_add_text(const char *,int, int, int, int);
	static FeListBox *cb_add_listbox(int, int, int, int);
	static FeImage *cb_add_surface(int, int);
	static FeScriptSound *cb_add_sound(const char *);
	static FeShader *cb_add_shader(int, const char *, const char *);
	static FeShader *cb_add_shader(int, const char *);
	static FeShader *cb_add_shader(int);
	static void cb_add_ticks_callback(const char *);
	static void cb_add_transition_callback(const char *);
	static bool cb_is_keypressed(int);	// deprecated as of 1.2
	static bool cb_is_joybuttonpressed(int,int);	// deprecated as of 1.2
	static float cb_get_joyaxispos(int,int);	// deprecated as of 1.2
	static bool cb_get_input_state( const char *input );
	static int cb_get_input_pos( const char *input );
	static void do_nut(const char *);
	static bool load_module( const char *module_file );
	static bool cb_plugin_command(const char *, const char *, const char *);
	static bool cb_plugin_command(const char *, const char *);
	static bool cb_plugin_command_bg(const char *, const char *);
	static const char *cb_path_expand( const char *path );
	static const char *cb_game_info(int,int);
	static const char *cb_game_info(int);
	static Sqrat::Table cb_get_config();
};

extern FePresent *helper_get_fep();

#endif
