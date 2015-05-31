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
#include <queue>
#include <string>

#include "fe_input.hpp"
#include "fe_present.hpp"

class FeWindow;
class FeOverlay;
class FeConfigContext;

namespace Sqrat
{
	class Object;
	class Table;
	class Array;
};

namespace sf
{
	class Event;
};

class FeVM : public FePresent
{
private:
	friend class FeConfigVM;

	static const char *transitionTypeStrings[];

	enum FromToType
	{
		FromToNoValue=0,
		FromToScreenSaver=1,
		FromToFrontend=2
	};

	FeWindow &m_window;
	FeOverlay *m_overlay;
	FeSound &m_ambient_sound;

	bool m_redraw_triggered;
	const FeScriptConfigurable *m_script_cfg;
	std::queue< FeInputMap::Command > m_posted_commands;
	std::vector< std::pair< Sqrat::Object, std::string > > m_ticks;
	std::vector< std::pair< Sqrat::Object, std::string > > m_trans;
	std::vector< std::pair< Sqrat::Object, std::string > > m_sig_handlers;

	FeVM( const FeVM & );
	FeVM &operator=( const FeVM & );

	void add_ticks_callback( Sqrat::Object, const char * );
	void add_transition_callback( Sqrat::Object, const char * );
	void add_signal_handler( Sqrat::Object, const char * );
	void remove_signal_handler( Sqrat::Object, const char * );

	static bool internal_do_nut(const std::string &, const std::string &);

public:
	FeVM( FeSettings &fes, FeFontContainer &defaultfont, FeWindow &wnd, FeSound &ambient_sound );
	~FeVM();

	void set_overlay( FeOverlay *feo );

	void flag_redraw() { m_redraw_triggered = true; };
	bool poll_command( FeInputMap::Command &c, sf::Event &ev );
	void clear(); // override of base class clear()

	// Scripting functionality
	//
	void vm_close();
	void vm_init();
	void on_new_layout( const std::string &path, const std::string &filename, const FeLayoutInfo &layout_params );
	bool on_tick();
	bool on_transition( FeTransitionType, int var );
	void init_with_default_layout();

	bool script_handle_event( FeInputMap::Command c, bool &redraw );

	//
	// overlay functions used from scripts
	//
	int list_dialog( Sqrat::Array, const char *, int, int );
	int list_dialog( Sqrat::Array, const char *, int );
	int list_dialog( Sqrat::Array, const char * );
	int list_dialog( Sqrat::Array );
	const char *edit_dialog( const char *, const char * );
	bool overlay_is_on();
	bool splash_message( const char * );

	static void script_get_config_options(
			FeConfigContext &ctx,
			std::string &gen_help,
			FeScriptConfigurable &configurable,
			const std::string &script_path,
			const std::string &script_file );

	static void script_run_config_function(
			const FeScriptConfigurable &configurable,
			const std::string &script_path,
			const std::string &script_file,
			const std::string &func_name,
			std::string &returned_message );

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
	static FeSound *cb_add_sound(const char *, bool);
	static FeSound *cb_add_sound(const char *);
	static FeShader *cb_add_shader(int, const char *, const char *);
	static FeShader *cb_add_shader(int, const char *);
	static FeShader *cb_add_shader(int);
	static void cb_add_ticks_callback( Sqrat::Object, const char *);
	static void cb_add_ticks_callback(const char *);
	static void cb_add_transition_callback( Sqrat::Object, const char *);
	static void cb_add_transition_callback(const char *);
	static void cb_add_signal_handler( Sqrat::Object, const char *);
	static void cb_add_signal_handler( const char * );
	static void cb_remove_signal_handler( Sqrat::Object, const char *);
	static void cb_remove_signal_handler( const char * );
	static bool cb_get_input_state( const char *input );
	static int cb_get_input_pos( const char *input );
	static void do_nut(const char *);
	static bool load_module( const char *module_file );
	static bool cb_plugin_command(const char *, const char *, Sqrat::Object, const char * );
	static bool cb_plugin_command(const char *, const char *, const char *);
	static bool cb_plugin_command(const char *, const char *);
	static bool cb_plugin_command_bg(const char *, const char *);
	static const char *cb_path_expand( const char *path );
	static const char *cb_game_info( int,int,int);
	static const char *cb_game_info(int,int);
	static const char *cb_game_info(int);

	enum ArtFlags { AF_Default=0,AF_ImagesOnly=1,AF_IncludeLayout=2 };
	static const char *cb_get_art( const char *,int,int,int);
	static const char *cb_get_art( const char *,int,int);
	static const char *cb_get_art( const char *,int);
	static const char *cb_get_art( const char *);
	static Sqrat::Table cb_get_config();
	static void cb_signal( const char * );
};

#endif
