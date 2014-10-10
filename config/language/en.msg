#
# Languages
#
en;English
fr;Français

#
# General
#
Exit Attract-Mode;Exit Attract-Mode
Exit Attract-Mode?;Exit Attract-Mode?
Yes;Yes
No;No

#
# Help text
#
_help_back;Exit menu
_help_emulators;Configure available emulators
_help_art_label;The label that identifies this artwork resource (used by layouts)
_help_art_path;The path to the files for this artwork.  Multiple paths can be entered if separated by a semicolon.
_help_art_add;Add a new artwork resource (marquee images, screen shots, etc.)
_help_emu_executable;The full path and name of the emulator executable. $HOME gets replaced with the user's home dir
_help_emu_args;The command line arguments for this emulator.  The following get substituted appropriately: [name], [romext], [rompath], [emulator], [romfilename]
_help_emu_rompath;The path to the roms for this emulator.  Multiple paths can be entered if separated by a semicolon.
_help_emu_romext;The rom extension(s) for this emulator (i.e. .zip).  Multiple extensions can be entered if separated by a semicolon.  Specify "<DIR>" as an entry if games are stored in separate subdirectories
_help_emu_import_extras;Path to any additional files that need to be read when generating a romlist for this emulator.  Supported files are: catver.ini (for game categories) and nplayers.ini (for number of players).  Multiple file paths can be entered if separated by a semicolon
_help_emu_system;The system identifier(s) for this emulator.  Multiple values can be entered if separated by a semicolon
_help_emu_info_source;Use "mame" if the emulator provides mame -listxml output, "mess" for mess -listsoftware output, or "thegamesdb.net" to scrape data from thegamesdb.net.  "mess" and "thegamesdb.net" also require that a System Identifier recognized by those sources be configured
_help_emu_gen_romlist;Generate a romlist by searching the configured rom path for roms with the configured extension, incorporating XML and additional import file information into the generated list (if available)
_help_emu_delete;Delete the entry for this emulator
_help_emu_sel;Configure emulator
_help_emu_add;Add a new emulator
_help_filter_name;The name for this filter
_help_filter_rule;Configure filter rule
_help_filter_add_rule;Add a rule to this filter
_help_filter_sort_by;The game information attribute that you wish to sort this list by.  Set to "No Sort" to keep the ordering from the source romlist
_help_filter_list_limit;Set this to a non-zero value if you wish to limit the size of a sorted list.  Use a positive value for the first X entries (i.e. "10" gives the first 10 entries), a negative value for the last X entries (i.e. "-25" gives the last 25 entries), or "0" for no limit
_help_filter_delete;Delete this filter
_help_rule_target;The game information attribute that you wish to filter based on
_help_rule_comp;Comparison type for the filter
_help_rule_value;The regular expression value to filter to.  For example, if the filter target is "Year" and comparison is "equals", a value of "1984" will match the year of 1984, "198." will match any year in the 80s, and "(198.)|(199.)" will match any year in the 80s or 90s
_help_rule_delete;Delete this filter rule
_help_list_name;The title of this display list
_help_list_layout;The layout to use for this display list
_help_list_romlist;The romlist to use for this display list
_help_list_filter;Configure filter
_help_list_add_filter;Add a filter to this display list
_help_list_layout_options;Configure any layout-specific options
_help_list_delete;Delete this display list
_help_list_sel;Configure display list
_help_list_add;Add a new display list
_help_input_action;
_help_input_delete;Remove this control mapping
_help_input_add;Add a new control mapping for this action
_help_control_sel;Configure control mappings
_help_joystick_threshold;The threshold for registering joystick moves, from 1 to 100 (100=least sensitive)
_help_mouse_threshold;The threshold for registering mouse moves, from 1 to 100 (100=least sensitive)
_help_plugin_command;The executable associated with this plug-in
_help_plugin_enabled;Set whether this plug-in is enabled
_help_plugin_sel;Configure plug-in
_help_volume;Valid volume settings are from 0 (mute) to 100 
_help_sound_sel;Assign a sound to this event/action
_help_exit_command;Command to execute when Exit (Confirm) is selected [i.e. "sudo shutdown"]
_help_default_font;The name of the default font to use
_help_font_path;The directories containing fonts.  Multiple directories can be entered if separated by a semicolon.  Attract-Mode will recursively search these directories to find a specified font
_help_screen_saver_timeout;The amount of time in seconds to wait before switching to the screen saver.  0 to disable
_help_lists_menu_exit;Configure whether an Exit option should be provided in the"Lists Menu"
_help_hide_brackets;Hide text in brackets when showing game titles
_help_autolaunch_last_game;Automatically launch the last game played on startup
_help_confirm_favs;Confirm the addition/removal of Favourites
_help_window_mode;Set whether Attract-Mode fills the screen or runs in a window
_help_filter_wrap_mode;Set what happens when the "Next Filter" or "Previous Filter" actions reach the end of the filter list
_help_emulators;Configure emulator settings
_help_lists;Configure display lists
_help_controls;Configure controls
_help_sound;Configure sound volumes and sound mappings
_help_screen_saver;Configure screen saver
_help_plugins;Configure plug-ins
_help_misc;Configure Miscellaneous settings
_sort_regexp;^(Vs\. The |The |Vs\. )
