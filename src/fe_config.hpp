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

#ifndef FE_CONFIG_HPP
#define FE_CONFIG_HPP

#include <string>
#include <vector>
#include "fe_settings.hpp"
#include "fe_input.hpp"

class FeSettings;
class FeEmulatorInfo;
class FeDisplayInfo;
class FeFilter;
class FeRule;
class FePlugInfo;
class FeLayoutInfo;
class FeScriptConfigurable;

namespace Opt
{
	const int	EDIT 				= 1; // option text can be editted by user
	const int	LIST				= 2; // option gets selected from values_list
	const int	INFO				= 3; // option is just for info (no changes)
	const int	MENU				= 4; // option leads to another menu
	const int	SUBMENU			= 5; // option leads to submenu of current menu
	const int	RELOAD			= 6; // option reloads menu
	const int	EXIT				= 7; // option results in exiting the menu
	const int	DEFAULTEXIT		= 8; // option is default for exiting the menu
};

//
// This class is used to set out a single menu option
//
class FeMenuOpt
{
private:
	std::string m_edit_value;	// setting value (edit or unmatched list)
	int m_list_index;	// selected index for list options (-1 if unmatched)

public:
	int type;		// see Opt namespace for values
	std::string setting;	// the name of the setting
	std::string help_msg;	// the help message for this option
	std::vector<std::string> values_list; // list options

	int opaque;	// private variables available to the menu to track menu options
	std::string opaque_str;

	FeMenuOpt(int t,
		const std::string &set,
		const std::string &val="" );

	FeMenuOpt(int t,
		const std::string &set,
		const std::string &val,
		const std::string &help,
		int opq,
		const std::string &opq_str );

	void set_value( const std::string & );
	void set_value( int );

	const std::string &get_value() const;
	int get_vindex() const;

	void append_vlist( const char **clist );
	void append_vlist( const std::vector< std::string > &list );
};

//
// This class sets out the variables and functions that are shared between
// the config menus and FeOverlay (which draws them)
//
class FeConfigContext
{
private:
	FeConfigContext( const FeConfigContext & );
	FeConfigContext &operator=( const FeConfigContext & );

public:
	enum Style
	{
		SelectionList,
		EditList
	};

	FeSettings &fe_settings;
	Style style;
	std::string title;
	std::string help_msg; // temporary help message,displayed until next input

	std::vector<FeMenuOpt> opt_list;	// menu options
	int curr_sel;		// index of currently selected menu option in opt_list
	bool save_req; 	// flag whether save() should be called on this menu

	FeConfigContext( FeSettings & );

	//
	// Convenient addition of menu options
	//
	// set and val are added without language lookup.  help gets lookup.
	void add_opt( int type, const std::string &set,
		const std::string &val="", const std::string &help="" );

	// set and help get language lookup
	void add_optl( int type, const std::string &set,
		const std::string &val="", const std::string &help="" );

	// set the Style and title for the menu.
	void set_style( Style s, const std::string &t );

	//
	// Convenient access
	//
	FeMenuOpt &curr_opt() { return opt_list[curr_sel]; }
	FeMenuOpt &back_opt() { return opt_list.back(); }

	//
	// Dialog Toolbox.  "msg" strings get translated
	//
	virtual bool edit_dialog( const std::string &msg, std::string &text )=0;

	virtual bool confirm_dialog( const std::string &msg,
		const std::string &rep="" )=0;

	virtual void splash_message( const std::string &msg,
		const std::string &rep,
		const std::string &aux )=0;

	virtual void input_map_dialog( const std::string &msg,
		std::string &map_str,
		FeInputMap::Command &conflict )=0;

	virtual void tags_dialog()=0;

	virtual bool check_for_cancel()=0;
};

class FeBaseConfigMenu
{
protected:
	FeBaseConfigMenu();
	FeBaseConfigMenu( const FeBaseConfigMenu & );
	FeBaseConfigMenu &operator=( const FeBaseConfigMenu & );

public:
	// Called to populate the list of options for this menu (ctx.opt_list)
	// and to set the style and title
	//
	virtual void get_options( FeConfigContext &ctx );

	// Called when a menu option is selected
	// Return true to proceed with option, false to cancel
	//
	virtual bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );


	// Called when leaving the menu if ctx.save_req is true (it is either
	// set to true in on_option_select above or when an EDIT or LIST option
	// is changed
	// Return true if save required on parent menu
	//
	virtual bool save( FeConfigContext &ctx );
};

// Utility class where script parameters are being configured
class FeScriptConfigMenu : public FeBaseConfigMenu
{
protected:
	FeScriptConfigMenu();

	virtual bool save( FeConfigContext &ctx )=0;

	bool on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu );

	bool save_helper( FeConfigContext &ctx );

	FeScriptConfigurable *m_configurable;
	std::string m_file_path;
	std::string m_file_name;
	FeSettings::FePresentState m_state;
	int m_script_id;
};

class FeLayoutEditMenu : public FeScriptConfigMenu
{
private:
	FeLayoutInfo *m_layout;

public:
	FeLayoutEditMenu();
	void get_options( FeConfigContext &ctx );

	bool save( FeConfigContext &ctx );
	void set_layout( FeLayoutInfo *layout );
};

class FeIntroEditMenu : public FeScriptConfigMenu
{
public:
	void get_options( FeConfigContext &ctx );

	bool save( FeConfigContext &ctx );
};

class FeSaverEditMenu : public FeScriptConfigMenu
{
public:
	void get_options( FeConfigContext &ctx );

	bool save( FeConfigContext &ctx );
};

class FeEmuArtEditMenu : public FeBaseConfigMenu
{
private:
	std::string m_art_name;
	FeEmulatorInfo *m_emulator;

public:
	FeEmuArtEditMenu();

	void get_options( FeConfigContext &ctx );

	bool save( FeConfigContext &ctx );
	void set_art( FeEmulatorInfo *emu, const std::string &art_name );
};

class FeEmulatorEditMenu : public FeBaseConfigMenu
{
private:
	FeEmuArtEditMenu m_art_menu;
	FeEmulatorInfo *m_emulator;
	bool m_is_new;
	bool m_romlist_exists;
	bool m_parent_save;

public:
	FeEmulatorEditMenu();

	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
	bool save( FeConfigContext &ctx );

	void set_emulator( FeEmulatorInfo *emu, bool is_new,
		const std::string &romlist_dir );
};

class FeEmulatorGenMenu : public FeBaseConfigMenu
{
private:
	std::string m_default_name;

public:
	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
};

class FeEmulatorSelMenu : public FeBaseConfigMenu
{
private:
	FeEmulatorEditMenu m_edit_menu;
	FeEmulatorGenMenu m_gen_menu;

public:
	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
};

class FeRuleEditMenu : public FeBaseConfigMenu
{
private:
	FeFilter *m_filter;
	int m_index;

public:
	FeRuleEditMenu();

	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
	bool save( FeConfigContext &ctx );
	void set_rule_index( FeFilter *f, int i );
};

class FeFilterEditMenu : public FeBaseConfigMenu
{
private:
	FeDisplayInfo *m_display;
	int m_index;
	FeRuleEditMenu m_rule_menu;

public:
	FeFilterEditMenu();

	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
	bool save( FeConfigContext &ctx );
	void set_filter_index( FeDisplayInfo *d, int i );
};

class FeDisplayEditMenu : public FeBaseConfigMenu
{
private:
	FeFilterEditMenu m_filter_menu;
	FeLayoutEditMenu m_layout_menu;
	FeDisplayInfo *m_display;
	int m_index; // the index for m_display in FeSettings' master list

public:
	FeDisplayEditMenu();

	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
	bool save( FeConfigContext &ctx );
	void set_display( FeDisplayInfo *d, int index );
};

class FeDisplayMenuEditMenu : public FeBaseConfigMenu
{
private:
	FeLayoutEditMenu m_layout_menu;
public:
	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
	bool save( FeConfigContext &ctx );
};

class FeDisplaySelMenu : public FeBaseConfigMenu
{
private:
	FeDisplayEditMenu m_edit_menu;
	FeDisplayMenuEditMenu m_menu_menu;

public:
	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
};

class FeMapping;

class FeInputEditMenu : public FeBaseConfigMenu
{
private:
	FeMapping *m_mapping;

public:
	FeInputEditMenu();
	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
	bool save( FeConfigContext &ctx );
	void set_mapping( FeMapping * );
};

class FeInputSelMenu : public FeBaseConfigMenu
{
private:
	std::vector<FeMapping> m_mappings;
	FeInputEditMenu m_edit_menu;

public:
	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
	bool save( FeConfigContext &ctx );
};

class FeSoundMenu : public FeBaseConfigMenu
{
public:
	void get_options( FeConfigContext &ctx );
	bool save( FeConfigContext &ctx );
};

class FeMiscMenu : public FeBaseConfigMenu
{
	std::vector<FeLanguage> m_languages;

public:
	void get_options( FeConfigContext &ctx );
	bool save( FeConfigContext &ctx );
};

class FeScraperMenu : public FeBaseConfigMenu
{
public:
	void get_options( FeConfigContext &ctx );
	bool save( FeConfigContext &ctx );
};

class FePluginEditMenu : public FeScriptConfigMenu
{
private:
	FePlugInfo *m_plugin;

public:
	FePluginEditMenu();
	void get_options( FeConfigContext &ctx );

	bool save( FeConfigContext &ctx );
	void set_plugin( FePlugInfo *plugin, int index );
};

class FePluginSelMenu : public FeBaseConfigMenu
{
private:
	FePluginEditMenu m_edit_menu;

public:
	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
};

class FeConfigMenu : public FeBaseConfigMenu
{
private:
	FeEmulatorSelMenu m_emu_menu;
	FeDisplaySelMenu m_list_menu;
	FeInputSelMenu m_input_menu;
	FeSoundMenu m_sound_menu;
	FeIntroEditMenu m_intro_menu;
	FeSaverEditMenu m_saver_menu;
	FePluginSelMenu m_plugin_menu;
	FeScraperMenu m_scraper_menu;
	FeMiscMenu m_misc_menu;

public:
	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );
	bool save( FeConfigContext &ctx );
};

class FeEditGameMenu : public FeBaseConfigMenu
{
private:
	FeRomInfo m_rom_original;
	bool m_update_rl;
	bool m_update_stats;
	bool m_update_extras;

public:
	void get_options( FeConfigContext &ctx );
	bool on_option_select( FeConfigContext &ctx,
		FeBaseConfigMenu *& submenu );

	bool save( FeConfigContext &ctx );
};


#endif
