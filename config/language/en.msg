#@English
#
# Interface strings
#

#
# Help messages
#
_help_art;An artwork resource available for use by layouts
_help_art_add;Add a new artwork resource (marquee images, screen shots, etc.)
_help_art_label;The label that identifies this artwork resource (used by layouts)
_help_art_path;The path to the files for this artwork.  Multiple paths can be entered if separated by a semicolon.
_help_back;Exit menu
_help_confirm_exit;Confirm the exiting of Attact-Mode
_help_confirm_favs;Confirm the addition/removal of Favourites
_help_control_add_favourite;Set the control(s) that mark/unmark the currently selected game as a favourite
_help_control_add_tags;Set the control(s) that add/remove tags from the currently selected game
_help_control_back;Set the control(s) that go back
_help_control_configure;Set the control(s) that show the "Configuration Menu"
_help_control_custom1;Set the "Custom1" control(s)
_help_control_custom2;Set the "Custom2" control(s)
_help_control_custom3;Set the "Custom3" control(s)
_help_control_custom4;Set the "Custom4" control(s)
_help_control_custom5;Set the "Custom5" control(s)
_help_control_custom6;Set the "Custom6" control(s)
_help_control_displays_menu;Set the control(s) that show the "Displays Menu"
_help_control_down;Set the control(s) that move the selection down
_help_control_edit_game;Set the control(s) to edit the current selection
_help_control_exit;Set the control(s) that exit the frontend
_help_control_exit_to_desktop;Set the control(s) that exit the frontend immediately to the desktop
_help_control_filters_menu;Set the control(s) that show the "Filters Menu"
_help_control_intro;Set the control(s) that show the intro video
_help_control_left;Set the control(s) that move the selection left
_help_control_next_display;Set the control(s) that move to the next display
_help_control_next_favourite;Set the control(s) that move to the next favourite
_help_control_next_filter;Set the control(s) that move to the next filter
_help_control_next_game;Set the control(s) that move to the next game
_help_control_next_letter;Set the control(s) that move to the next letter
_help_control_next_page;Set the control(s) that move the selection up one page
_help_control_prev_display;Set the control(s) that move to the previous display
_help_control_prev_favourite;Set the control(s) that move to the previous favourite
_help_control_prev_filter;Set the control(s) that move to the previous filter
_help_control_prev_game;Set the control(s) that move to the previous game
_help_control_prev_letter;Set the control(s) that move to the previous letter
_help_control_prev_page;Set the control(s) that move the selection down one page
_help_control_random_game;Set the control(s) that select a random game from the current list
_help_control_replay_last_game;Set the control(s) that relaunch the last game played
_help_control_right;Set the control(s) that move the selection right
_help_control_screen_saver;Set the control(s) that launch screen saver mode
_help_control_screenshot;Set the control(s) that take a screenshot of the frontend
_help_control_select;Set the control(s) that launch the current selection
_help_control_toggle_flip;Set the control(s) that toggle whether the display should be rotated 180 degrees
_help_control_toggle_layout;Set the control(s) that toggle the layout script to use (It depends on the layout you are using whether this does anything)
_help_control_toggle_movie;Set the control(s) that toggle whether or not videos should be played
_help_control_toggle_mute;Set the control(s) that mute/unmute frontend sound
_help_control_toggle_rotate_left;Set the control(s) that toggle whether the display should be rotated 90 degrees to the left
_help_control_toggle_rotate_right;Set the control(s) that toggle whether the display should be rotated 90 degrees to the right
_help_control_up;Set the control(s) that move the selection up
_help_controls;Configure the joystick, keyboard and mouse inputs that control Attract-Mode
_help_default_font;The name of the default font to use
_help_display_add;Add a new display
_help_display_add_filter;Add a filter to this display's filter set
_help_display_delete;Delete this display
_help_display_filter;Configure this filter from the display's filter set
_help_display_global_filter;Configure global filter rules that are applied to every entry in this display's collection/rom list
_help_display_in_cycle;Include this display in the "Previous Display"/"Next Display" cycle?
_help_display_in_menu;Include this display in the "Displays Menu"?
_help_display_layout;The layout to use for this display
_help_display_layout_options;Configure any layout-specific options
_help_display_name;The title for this display
_help_display_romlist;The collection/rom list to use for this display
_help_display_sel;Configure display
_help_displays;Configure the romlist/layout groupings (a.k.a. "Displays") that are available to be cycled through in Attract-Mode
_help_displays_menu;Configure options for the "Displays Menu"
_help_displays_menu_exit;Configure whether an Exit option should be provided in the"Displays Menu"
_help_displays_menu_layout;The layout to use for the 'Displays Menu'. Set to 'Default' to use the frontend's built-in menu
_help_displays_menu_prompt;The 'Displays Menu' prompt (i.e. "Select System")
_help_emu_add;Add a new emulator
_help_emu_args;The command line arguments for this emulator.  The following get substituted appropriately: [name], [romext], [rompath], [emulator], [romfilename], [system], [systemn], [workdir], [nothing], as well as any valid 'magic tokens' (see layout documentation)
_help_emu_delete;Delete the entry for this emulator
_help_emu_executable;The full path and name of the emulator executable. Possible variables: $HOME = User's home dir, $PROGDIR = Attract-Mode program dir. Windows OS additionally have %SYSTEMROOT% and %PROGRAMFILES% matching system environment variables.
_help_emu_exit_hotkey;Set a global hotkey to exit the emulator.  Attract-Mode will terminate the emulator process if this hotkey gets pressed
_help_emu_gen_romlist;Generate a romlist by searching the configured rom path for roms with the configured extension, incorporating XML and additional import file information into the generated list (if available)
_help_emu_import_extras;Path to any additional files that need to be read when generating a romlist for this emulator.  Supported files are: catver.ini (for game categories), nplayers.ini (for number of players) and mame.xml.  Multiple file paths can be entered if separated by a semicolon
_help_emu_info_source;Use "listxml" for mame -listxml output, "listsoftware" for mame/mess -listsoftware output, "thegamesdb.net" to scrape data from thegamesdb.net, "steam" to read from a steam installation, or "scummvm" if the emulator is scummvm.  "listsoftware" and "thegamesdb.net" also require that a System Identifier recognized by those sources be configured
_help_emu_nb_mode_wait;If this is non-zero, the emulator will be run in 'non-blocking mode'.  The frontend will wait up to this number of seconds to lose focus to the emulator's window, and will wake up again when focus returns to the frontend's window.  This is necessary for launching programs like Steam correctly.  Should be 0 unless you know you need it
_help_emu_pause_hotkey;Set a global hotkey to pause the emulator.  When pressed, Attract-Mode will pause the emulator process and return to the frontend. The emulator process can be resumed by selecting the same game again in the frontend.
_help_emu_romext;The rom extension(s) for this emulator (i.e. .zip).  Multiple extensions can be entered if separated by a semicolon.  Specify "<DIR>" as an entry if games are stored in separate subdirectories
_help_emu_rompath;The path to the roms for this emulator.  Multiple paths can be entered if separated by a semicolon.
_help_emu_scrape_artwork;Scrape missing artwork images for this emulator from the net
_help_emu_sel;Configure emulator
_help_emu_sel_gen_romlist;Generate a Collection/Romlist containing games for multiple emulators
_help_emu_system;The system identifier(s) for this emulator.  Multiple values can be entered if separated by a semicolon
_help_emu_workdir;The working directory for this emulator.  This is the directory that the frontend runs the emulator from.  Also, any relative rom paths, artwork paths, or additional import file paths are treated as being relative to this directory
_help_emulators;Configure emulator settings
_help_exit_command;Command to execute when exiting the frontend [i.e. "sudo shutdown"]
_help_exit_message;Alternate message to exit the frontend [i.e. "Shut Down System"]
_help_filter_add_exception;Add an exception to this filter.  If a game matches an exception (and wasn't filtered out by an earlier rule) it will be listed in the filter
_help_filter_add_rule;Add a rule to this filter.  A game must match all filter rules (or a single exception) to be listed in a filter
_help_filter_delete;Delete this filter
_help_filter_exception;Configure this filter exception.  If a game matches an exception it will be listed in the filter.  Rules and exceptions get applied in the order that they are listed
_help_filter_list_limit;Set this to a non-zero value if you wish to limit the size of a sorted list.  Use a positive value for the first X entries (i.e. "10" gives the first 10 entries), a negative value for the last X entries (i.e. "-25" gives the last 25 entries), or "0" for no limit
_help_filter_name;The name for this filter
_help_filter_rule;Configure this filter rule.  A game must match all filter rules (or a single exception) to be listed in a filter
_help_filter_sort_by;The game information attribute that you wish to sort the collection/rom list by.  Set to "No Sort" to keep the ordering from the source collection/rom list
_help_filter_wrap_mode;Set what happens when the "Next Filter" or "Previous Filter" actions reach the end of the filter set
_help_font_path;The directories containing fonts.  Multiple directories can be entered if separated by a semicolon.  Attract-Mode will recursively search these directories to find a specified font
_help_game_custom_args;Set this to override the default command line arguments for this game.  Set this to [nothing] if you want to override with no parameters.  The following get substituted appropriately: [name], [romext], [rompath], [emulator], [romfilename], [system], [systemn]
_help_game_custom_executable;Set this to override the default executable for this game
_help_game_delete;Delete this game from the Romlist
_help_game_edit;Edit this game information
_help_game_overview;Edit game overview
_help_generator_build;Generate romlist with the selected emulator(s)
_help_generator_opt;Set whether this emulator is to be included in the generated romlist
_help_hide_brackets;Hide text in brackets when showing game titles
_help_hide_console;Hide console on startup. Note that this only works if starting Attract-Mode from a Windows GUI. If started from an existing console window (e.g. a batch file) then the process will always "attach" to the existing console and output to it.
_help_input_action;
_help_input_add;Add a new input mapping for this control
_help_input_default_action;Set the default behaviour for this control
_help_input_delete;Remove this input mapping
_help_intro;Configure intro
_help_joystick_map;Map the frontend's joystick slots to specifically named joystick/gamepad devices attached to your computer (You can use this to force the frontend to always use 'Microsoft X-Box 360 pad' as Joystick 0, for example.)
_help_joystick_threshold;The threshold for registering joystick moves, from 1 to 100 (100=least sensitive)
_help_language;Select the language to use in Attract-Mode's user interface
_help_layout_file;Some layouts provide multiple layout*.nut files.  Select which layout file to use
_help_misc;Configure miscellaneous settings
_help_mouse_threshold;The threshold for registering mouse moves, from 1 to 100 (100=least sensitive)
_help_multiple_monitors;Enable the use of multiple monitors by Attract-Mode.  Setting this to 'No' may reduce screen flicker when launching games
_help_plugin_command;The executable associated with this plug-in
_help_plugin_enabled;Set whether this plug-in is enabled
_help_plugin_sel;Configure plug-in
_help_plugins;Configure available plug-ins
_help_rule_comp;Comparison type for the filter
_help_rule_delete;Delete this filter rule
_help_rule_target;The game information attribute that you wish to filter based on
_help_rule_value;The regular expression value to filter to.  For example, if the filter target is "Year" and comparison is "equals", a value of "1984" will match the year of 1984, "198." will match any year in the 80s, and "(198.)|(199.)" will match any year in the 80s or 90s
_help_scrape_fanart;Set whether game fanart should be downloaded when scraping
_help_scrape_flyers;Set whether game flyers and box covers should be downloaded when scraping
_help_scrape_marquees;Set whether game marquees and banners should be downloaded when scraping
_help_scrape_snaps;Set whether game snapshots should be downloaded when scraping
_help_scrape_vids;Set whether game videos should be downloaded when scraping
_help_scrape_wheels;Set whether game logos should be downloaded when scraping
_help_scraper;Configure artwork scraper settings
_help_screen_saver;Configure screen saver
_help_screen_saver_timeout;The amount of time in seconds to wait before switching to the screen saver.  0 to disable
_help_shortcut_artwork_name_edit;Optionally provide an artwork name to be associated with this shortcut.  The frontend will try to load artworks with this name when displaying the shortcut
_help_shortcut_delete;Delete this shortcut from the Romlist
_help_shortcut_label_edit;Edit this shortcut's label
_help_shortcut_target_edit;Edit this shortcut's target
_help_sound;Configure sound volumes, sound event mappings and the ambient soundtrack
_help_sound_sel;Assign a sound to this event/action
_help_startup_mode;Set what should happen when Attract-Mode first starts up
_help_track_usage;Configure whether Attract-Mode should track usage (played time and play count for each game)
_help_video_decoder;Configure the decoder to use for video playback (if multiple decoders are available)
_help_volume;Valid volume settings are from 0 (mute) to 100
_help_window_mode;Set whether Attract-Mode fills the screen or runs in a window
_sort_regexp;^(Vs\. The |The |Vs\. )

#
# Extra strings provided by translation
#
cn;中文 (Chinese)
de;Deutsch
en;English
es;Español
fr;Français
it;Italiano
jp;日本語 (Japanese)
kr;한국어 (Korean)

#@DONE_TRANSLATED
