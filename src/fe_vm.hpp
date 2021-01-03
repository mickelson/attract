/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2014-2016 Andrew Mickelson
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

#include <sqrat/sqratObject.h>
#include <sqrat/sqratFunction.h>

class FeWindow;
class FeOverlay;
class FeConfigContext;

namespace Sqrat
{
	class Table;
	class Array;
};

namespace sf
{
	class Event;
};

class FeCallback
{
public:
	FeCallback( int pid,
		const Sqrat::Object &env,
		const std::string &fn,
		FeSettings &fes );
	Sqrat::Function &get_fn();

	int m_sid;		// -1 for layout, otherwise the plugin index
	Sqrat::Object m_env;	// callback function environment
	std::string m_fn;	// callback function name

	std::string m_path;
	std::string m_file;
	const FeScriptConfigurable *m_cfg;

private:
	Sqrat::Function m_cached_fn;
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
	bool m_process_console_input;
	const FeScriptConfigurable *m_script_cfg;
	int m_script_id;
	sf::Time m_last_ui_cmd;

	std::queue< FeInputMap::Command > m_posted_commands;
	std::vector< FeCallback > m_ticks;
	std::vector< FeCallback > m_trans;
	std::vector< FeCallback > m_sig_handlers;

	FeVM( const FeVM & );
	FeVM &operator=( const FeVM & );

	void add_ticks_callback( Sqrat::Object, const char * );
	void add_transition_callback( Sqrat::Object, const char * );
	void add_signal_handler( Sqrat::Object, const char * );
	void remove_signal_handler( Sqrat::Object, const char * );
	void set_for_callback( const FeCallback & );
	bool process_console_input();

	static bool internal_do_nut(const std::string &, const std::string &);

public:
	FeVM( FeSettings &fes, FeFontContainer &defaultfont, FeWindow &wnd, FeSound &ambient_sound, bool console_input );
	~FeVM();

	void set_overlay( FeOverlay *feo );

	void flag_redraw() { m_redraw_triggered = true; };
	bool poll_command( FeInputMap::Command &c, sf::Event &ev, bool &from_ui );
	void clear(); // override of base class clear()

	void update_to_new_list( int var=0, bool reset_display=false, bool suppress_transition=false ); // NOTE: override virtual function from FePresent

	// runs .attract/emulators/template/setup.nut to generate default emulator
	// configs and detect emulators.  Prompts user to automaticallly import emulators
	//
	bool setup_wizard();

	// Scripting functionality
	//
	void vm_close();
	void vm_init();
	bool on_new_layout();
	bool on_tick();
	void on_transition( FeTransitionType, int var );
	void init_with_default_layout();
	int get_script_id() { return m_script_id; };
	void set_script_id( int id ) { m_script_id=id; };

	bool script_handle_event( FeInputMap::Command c );

	//
	// overlay functions used from scripts
	//
	int list_dialog( Sqrat::Array, const char *, int, int );
	int list_dialog( Sqrat::Array, const char *, int );
	int list_dialog( Sqrat::Array, const char * );
	int list_dialog( Sqrat::Array );
	const char *edit_dialog( const char *, const char * );
	bool overlay_is_on();
	void overlay_set_custom_controls( FeText *caption, FeListBox *opts );
	void overlay_set_custom_controls( FeText *caption );
	void overlay_set_custom_controls();
	void overlay_clear_custom_controls();
	bool splash_message( const char *, const char * );
	bool splash_message( const char * );

	static void script_get_config_options(
			FeConfigContext &ctx,
			std::string &gen_help,
			FeScriptConfigurable &configurable,
			const std::string &script_path,
			const std::string &script_file );

	// Simply get the general help message for the specified script:
	static void script_get_config_options(
			std::string &gen_help,
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

	enum PathTestType
	{
		IsFileOrDirectory,
		IsFile,
		IsDirectory,
		IsRelativePath,
		IsSupportedArchive,
		IsSupportedMedia,
	};
	static bool cb_path_test( const char *, int );

	static const char *cb_game_info( int,int,int);
	static const char *cb_game_info(int,int);
	static const char *cb_game_info(int);

	enum ArtFlags
	{
		AF_Default=0,
		AF_ImagesOnly=1,
		AF_IncludeLayout=2,
		AF_FullList=4
	};
	static const char *cb_get_art( const char *,int,int,int);
	static const char *cb_get_art( const char *,int,int);
	static const char *cb_get_art( const char *,int);
	static const char *cb_get_art( const char *);
	static Sqrat::Table cb_get_config();
	static void cb_signal( const char * );
	static void cb_set_display( int, bool );
	static void cb_set_display( int );
	static const char *cb_get_text( const char * );
};

#endif
