/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2014 Andrew Mickelson
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

#ifndef FE_VM_HPP
#define FE_VM_HPP

#include <vector>
#include <string>

class FeBaseTextureContainer;
class FeBasePresentable;
class FeImage;
class FeText;
class FeListBox;
class FeShader;
class FeSettings;
class FePresent;
class FeScriptSound;
class FeScriptConfigurable;
class FeLayoutInfo;
class FeWindow;
class FeConfigContext;
class FeScriptConfigurable;

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

class FeVM
{
private:
	friend FePresent *helper_get_fep();
	static const char *transitionTypeStrings[];

	enum FromToType
	{
		FromToNoValue=0,
		FromToScreenSaver=1,
		FromToFrontend=2
	};

	FeSettings &m_fes;
	FePresent &m_fep;
	FeWindow &m_window;

	bool m_redraw_triggered;
	const FeScriptConfigurable *m_script_cfg;
	std::vector< std::string > m_ticks_list;
	std::vector< std::string > m_transition_list;

	FeVM( const FeVM & );
	FeVM &operator=( const FeVM & );

	void add_ticks_callback(const std::string &);
	void add_transition_callback(const std::string &);

	static bool internal_do_nut(const std::string &, const std::string &);

public:
	FeVM( FeSettings &fes, FePresent &fep, FeWindow &wnd );
	~FeVM();

	void flag_redraw() { m_redraw_triggered = true; };
	void clear();

	// Scripting functionality
	//
	void vm_close();
	void vm_init();
	void on_new_layout( const std::string &path, const std::string &filename, const FeLayoutInfo &layout_params );
	bool on_tick();
	bool on_transition( FeTransitionType, int var );

	//
	// Script static functions
	//
	static FePresent *script_get_fep();
	static void script_do_update( FeBaseTextureContainer * );
	static void script_do_update( FeBasePresentable * );
	static void script_flag_redraw();

	static void script_get_config_options(
			FeConfigContext &ctx,
			std::string &gen_help,
			FeScriptConfigurable &configurable,
			const std::string &script_file );

	//
	// Script callback functions
	//
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

#endif
