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
#include "fe_window.hpp"

class FeImage;
class FeBaseTextureContainer;
class FeText;
class FeListBox;
class FeFontContainer;
class FeSurfaceTextureContainer;
class FePresentableParent;

namespace sf
{
	class InputStream;
};

enum FeTransitionType
{
	StartLayout=0,		// var: FromToScreenSaver, FromToFrontend or FromToNoValue
	EndLayout,			// var: FromToScreenSaver, FromToFrontend or FromToNoValue
	ToNewSelection,	// var = index_offset of new selection
	FromOldSelection,	// var = index_offset of old selection
	ToGame,				// var = 0
	FromGame,			// var = 0
	ToNewList,			// var = filter offset of new filter (if available), otherwise 0
	EndNavigation,		// var = 0
	ShowOverlay,		// var = Custom, Exit, Displays, Filters, Tags
	HideOverlay,		// var = 0
	NewSelOverlay,		// var = index of new selection
	ChangedTag		   // var = FeRomInfo::Favourite, FeRomInfo::Tags
};

//
// Container class for use in our font pool
//
class FeFontContainer
{
public:
	FeFontContainer();
	~FeFontContainer();

	void set_font( const std::string &p, const std::string &n );

	const sf::Font &get_font() const;
	const std::string &get_name() const { return m_name; };

	void clear_font();

private:
	FeFontContainer( const FeFontContainer & );
	const FeFontContainer &operator=( const FeFontContainer & );

	mutable sf::Font m_font;
	std::string m_name;
	sf::InputStream *m_stream;
	mutable bool m_needs_reload;
};

//
// Container class for our per-monitor settings
//
class FeMonitor : public FePresentableParent
{
public:
	FeMonitor( int num, int w, int h );
	FeMonitor( const FeMonitor & );
	FeMonitor &operator=( const FeMonitor & );

	int get_width();
	int get_height();
	int get_num();

	sf::Transform transform;
	sf::Vector2i size;
	int num;
};


class FePresent
	: public sf::Drawable
{
	friend class FePresentableParent;
	friend class FeVM;

protected:
	enum FromToType
	{
		FromToNoValue=0,
		FromToScreenSaver=1,
		FromToFrontend=2
	};

	FeSettings *m_feSettings;
	FeWindow &m_window;

	const FeFontContainer *m_currentFont;
	FeFontContainer &m_defaultFont;
	std::string m_layoutFontName;

	sf::Clock m_layoutTimer;
	sf::Time m_lastInput;

	FeSettings::RotationState m_baseRotation;
	FeSettings::RotationState m_toggleRotation;
	sf::Transform m_transform;

	std::vector<FeBaseTextureContainer *> m_texturePool;
	std::vector<FeSound *> m_sounds;
	std::vector<FeShader *> m_scriptShaders;
	std::vector<FeFontContainer *> m_fontPool;
	std::vector<FeMonitor> m_mon;
	int m_refresh_rate;
	bool m_playMovies;
	int m_user_page_size;
	bool m_preserve_aspect;
	bool m_custom_overlay;

	FeListBox *m_listBox; // we only keep this ptr so we can get page sizes
	sf::Vector2i m_layoutSize;
	sf::Vector2f m_layoutScale;

	FeShader *m_emptyShader;

	FeText *m_overlay_caption;
	FeListBox *m_overlay_lb;

	FePresent( const FePresent & );
	FePresent &operator=( const FePresent & );

	void toggle_movie();

	void toggle_rotate( FeSettings::RotationState ); // toggle between none and provided state
	void set_transforms();
	int update( bool reload_list=false, bool new_layout=false );

	// Overrides from base classes:
	//
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	FeImage *add_image(bool a, const std::string &n, int x, int y, int w, int h, FePresentableParent &p);
	FeImage *add_clone(FeImage *, FePresentableParent &p);
	FeText *add_text(const std::string &n, int x, int y, int w, int h, FePresentableParent &p);
	FeListBox *add_listbox(int x, int y, int w, int h, FePresentableParent &p);
	FeImage *add_surface(int w, int h, FePresentableParent &p);
	FeSound *add_sound(const char *n, bool reuse);
	FeShader *add_shader(FeShader::Type type, const char *shader1, const char *shader2);
	int get_layout_width() const;
	int get_layout_height() const;
	int get_base_rotation() const;
	int get_toggle_rotation() const;
	const char *get_display_name() const;
	int get_display_index() const;
	const char *get_filter_name() const;
	int get_filter_index() const;
	void set_filter_index( int );
	int get_current_filter_size() const;
	int get_selection_index() const;
	int get_sort_by() const;
	bool get_reverse_order() const;
	int get_list_limit() const;
	void set_search_rule( const char * );
	const char *get_search_rule();
	bool get_preserve_aspect_ratio();

	void set_selection_index( int );
	const char *get_layout_font() const;
	void set_layout_width( int );
	void set_layout_height( int );
	void set_base_rotation( int );
	void set_toggle_rotation( int );
	void set_layout_font( const char * );
	void set_preserve_aspect_ratio( bool );

public:
	FePresent( FeSettings *fesettings, FeFontContainer &defaultfont, FeWindow &wnd );
	virtual ~FePresent( void );

	virtual void clear();

	void init_monitors();

	bool load_intro(); // returns false if no intro is available
	void load_screensaver();
	void load_layout( bool initial_load=false, bool suppress_transition=false );

	virtual void update_to_new_list( int var=0, bool reset_display=false, bool suppress_transition=false ); // NOTE virtual function!
	void on_end_navigation();
	void redraw_surfaces();

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
	const sf::Font *get_default_font() const; // get the default font (used by config overlay)

	float get_layout_scale_x() const;
	float get_layout_scale_y() const;

	// Get a font from the font pool, loading it if necessary
	const FeFontContainer *get_pooled_font( const std::vector < std::string > &l );
	const FeFontContainer *get_pooled_font( const std::string &n );

	const sf::Vector2i &get_layout_size() const { return m_layoutSize; }
	FeShader *get_empty_shader();

	// Returns true if a script has set custom overlay controls.
	// parameters are set to those controls (which may be NULL!)
	//
	bool get_overlay_custom_controls( FeText *&, FeListBox *& );

	void set_video_play_state( bool state );
	bool get_video_toggle() { return m_playMovies; };

	int get_layout_ms();

	//
	// Script static functions
	//
	static FePresent *script_get_fep();
	static void script_do_update( FeBaseTextureContainer * );
	static void script_do_update( FeBasePresentable * );
	static void script_flag_redraw();
	static void script_process_magic_strings( std::string &str,
			int filter_offset,
			int index_offset );
	static std::string script_get_base_path();

	//
	//
	virtual bool on_new_layout()=0;
	virtual bool on_tick()=0;
	virtual void on_transition( FeTransitionType, int var )=0;
	virtual void flag_redraw()=0;
	virtual void init_with_default_layout()=0;
	virtual int get_script_id()=0;
	virtual void set_script_id( int )=0;
};


#endif
