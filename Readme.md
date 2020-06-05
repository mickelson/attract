Attract-Mode Frontend
---------------------

Attract-Mode is a graphical frontend for command line emulators such as
MAME, MESS, and Nestopia.  It hides the underlying operating system and is
intended to be controlled with a joystick, gamepad or spin dial, making it
ideal for use in arcade cabinet setups.

Attract-Mode was originally developed for Linux.  It is known to work on
Linux (x86, x86-64, ARM, Raspberry Pi), Mac OS X and Windows based-systems.

Attract-Mode is licensed under the terms of the GNU General Public License,
version 3 or later.

Please visit <http://attractmode.org> for more information.

See [Compile.md][] if you intend to compile Attract-Mode from source.

Quick Start
-----------

1. Run Attract-Mode.  By default, Attract-Mode will look for its configuration
files in the "$HOME/.attract" directory on Linux and Mac OS X, and in the
current working directory on Windows.  If you want to use a different location
for the Attract-Mode configuration then you need to specify it at the command
line as follows:

		attract --config /my/config/location

In the (hopefully unlikely) event that Attract-Mode has difficulty finding a
display font to use on your system, you can specify one at the command line as
follows:

		attract --font <font_name>

2. If you are running Attract-Mode for the first time, you will be prompted to
select the language to use and then the frontend will load directly to its
configuration mode.  If you do not start in config mode, pressing the "TAB" key
will get you there.  By default, config mode can be navigated using the up/
down arrows, enter to select an option, and escape to go back.

3. Select the "Emulators" option, where you will be presented with the option
of editting the configuration for the emulators Attract-Mode auto-detected (if
any).  Edit an existing emulator or select 'Add' to add a new emulator
configuration.  Default configuration templates are provided for various
popular emulators to help you get started, however some settings will likely
have to be customized for your system (file locations etc).

4. Once you have an emulator configured correctly for your system, select
the "Generate Collection/Rom List" option from the emulator's configuration
menu.  Attract-Mode will use the configured emulator settings to generate a
list of available games for the emulator.  Next select the "Scrape Artwork"
option if you want to have Attract-Mode go and automatically download artwork
images for the emulator from the web.

5.  Exit config mode by selecting the "Back" option a few times.  You should
now have a usable front-end!

Basic Organization
---------------

At its most basic level, Attract-Mode is organized into "Displays", of which
you can configure one or more.  Each "Display" is a grouping of the following:

- a Collection/Rom List (the listing of available games/roms to show).  Romlists
  are text files located in the "romlists" directory.  They are created by
  Attract-Mode using the "Generate Collection/Rom List" option available when
  configuring an emulator, or by using the "--build-romlist" or
  "--import-romlist" command line options.
- a Layout (the visual theme/skin to use for the Display).  Layouts are located
  in the "layouts" directory, with each layout either in its own subdirectory
  or contained in a .zip file in the "layouts" directory.
- a Global Filter.  The global filter is a set of filter rules that always get
  applied to a Collection/Rom List.  This filter can be used to remove entries
  you will never want to see
- zero or more available filters.  Filters are a set of rules for which games
  to display and how to sort them.  Filters can be created to categorize games
  based attributes such as their orientation, category, manufacturer, year,
  times played, favourite status, file availability, etc.  Filters can be
  cycled through using the "Previous Filter" and "Next Filter" controls, and
  can also be selected from the "Filters Menu".

Further Customization
---------------------

**CONTROLS:** The inputs used to control Attract-Mode can be configured from
from the "Controls" menu in config mode.  Attract-Mode actions can be mapped
to most keyboard, mouse and joystick inputs, including combinations.  Here is
a list of the default control mappings:

| **Keyboard**      | **Joystick**      | **Action**              |
| ----------------- | ----------------- | ----------------------- |
| Up                | Up                | Previous Entry          |
| Down              | Down              | Next Entry              |
| Left              | Left              | Previous Display        |
| Right             | Right             | Next Display            |
| Enter or LControl | Button A          | Select                  |
| Escape            | Button B          | Back/Exit               |
| LControl+Up       | Button A+Up       | Jump to Previous Letter |
| LControl+Down     | Button A+Down     | Jump to Next Letter     |
| LControl+Left     | Button A+Left     | Show Filters Menu       |
| LControl+Right    | Button A+Right    | Next Filter             |
| LControl+Escape   | Button A+Button B | Toggle Favourite        |
| Tab or Escape+Up  | Button B+Up       | Configure               |
| Escape+Down       | Button B+Down     | Edit Game               |

**FILTERS:** Filters can be added to a Display in config mode.  Filters are a
list of "rules" and "exceptions" that the frontend steps through, in order, to
determine whether or not to list a game.  If a game does not match a "rule",
then it is not shown.  If a game matches to an "exception", then it gets listed
no matter what (ignoring the rest of the rules in the filter).  In other words,
in  order to be listed, a game has to match *all* the rules or *just one* of
the exceptions configured for the filter.

For example, you might want to have a filter that only shows 1980's
multiplayer sports games.  This would be achieved by creating a filter with
three rules: (1) that the year be in the 1980s (Year equals "198."), (2) that
the number of players in not 1 (Players not_equals "1"), and (3) that the
category contains "Sports" (Category contains "Sports").  Filters use regular
expressions, which allow for powerful text matching capabilities.  From the
example above, the "198." will match any four letter word that starts with
"198".

Filters also allow you to do some other stuff as well, such as controlling
how the gamelist gets sorted, and how many entries are listed.

**SOUND:** To configure sounds, place the sound file in the "sounds"
subdirectory of your Attract-Mode config directory.  The sound file can then
be selected from the "Sound" menu when in config mode and mapped to an action
or event.  Attract-Mode should support any sound format supported by FFmpeg
(MP3, etc).

**ARTWORK:** Attract-Mode supports PNG, JPEG, GIF, BMP and TGA image formats.
For video formats, Attract-Mode should support any video format supported by
FFmpeg (MP4, FLV, AVI, etc).  When deciding what file to use for a particular
artwork type, Attract-Mode will use the artwork selection order set out below.

The location of artwork resources is configured on a per-emulator basis in the
"Emulators" configuration menu.  Attract-Mode's default artworks are: "marquee"
(for cabinet marquee images), "snap" (for attract-mode videos and game screen
shots), "flyer" (for game flyer/box art) and "wheel" (for Hyperspin wheel art).
You can add others as needed in the "Emulators" configuration menu.  Multiple
paths can be specified for each artwork, in which case Attract-Mode will check
each path in the order they are specified before moving to the next check in
the selection order.

**ARTWORK SELECTION ORDER:**

   * From the artwork paths configured for the emulator (if any) and the
     previously scraped artworks (if any):

      - [Name].*      (video, i.e. "pacman.mp4")
      - [CloneOf].*   (video, i.e. "puckman.mp4")
      - [Name].*      (image, i.e. "pacman.jpg")
      - [CloneOf].*   (image, i.e. "puckman.png")
      - [Emulator].*  (video, i.e. "mame.avi")
      - [Emulator].*  (image, i.e. "mame.gif")

   * From the layout path for the current layout (layouts are located in
   the "layouts" subdirectory):

      - [Emulator]-[ArtLabel].*   (video, i.e. "mame-marquee.mp4")
      - [Emulator]-[ArtLabel].*   (image, i.e. "mame-marquee.png")
      - [ArtLabel].*              (video, i.e. "marquee.avi")
      - [ArtLabel].*              (image, i.e. "marquee.png")

   * When looking for artwork for the 'Displays Menu', artwork is loaded
   from the "menu-art" subdirectory.  Artwork matching the Display's name or
   the Display's romlist name are matched from the corresponding artwork
   directories located there.

   * If no files are found matching the above rules, then the artwork
   is not drawn.

   If no file match is found, Attract-Mode will check for a subdirectory
   that meets the match criteria.  If a subdirectory is found (for example
   a "pacman" directory in the configured artwork path) then Attract-Mode
   will then pick a random video or image from that directory.

**LAYOUTS:** Attract-Mode's layouts are located in the "layouts" directory.
Each layout has to be in its own subdirectory or contained in a .zip file.
Attract-Mode's native layouts are made up of a squirrel script (a .nut file)
and related resources.  See [Layouts.md][] for more information on
Attract-Mode layouts.

Attract-Mode can also display layouts made for other frontends, including
MaLa and Hyperspin.  This feature is experimental, and certain features from
these other frontends might not be fully implemented.

For MaLa layouts, just copy the layout and related resources into a new
subdirectory of the "layouts" directory, or place a zip, 7z or rar file
containing these things into the "layouts" directory.  After doing this, you
hould be able to select the layout when configuring a Display in
Attract-Mode, just as you would for a native Attract- Mode layout.

For Hyperspin, copy the Hyperspin "Media" directory into its own directory
in the Attract-Mode layouts directory.  Create a Display in Attract-Mode
and configure the layout to be the directory you copied "Media" into.  The
Display's name needs to match the name of one of the system subdirectories
in the Hyperspin "Media" directory.  This will allow Attract-Mode to find the
Hyperspin themes and graphics to use.  So for example, naming the Display
"MAME" will cause it to match Hyperspin's MAME/Themes/* for themes,
MAME/Images/Artwork1/* for artwork1, etc.  Note that the wheel images are
located using Attract-Mode's built-in wheel artwork.  Wheel images located in
the Hyperspin directories are ignored.

**PLUG-INS:** Plug-ins are squirrel scripts that need to be placed in the
"plugins" subdirectory of your Attract-Mode config directory.  Available
plugins can be enabled/disabled and configured from the "Plug-Ins" menu when
in config mode.  See [Layouts.md][] for a description of Attract-Mode's
Plug-in API.

**ROMLISTS:** Collection/Rom lists are saved in the "romlist" subdirectory of
your Attract-Mode config directory.  Each list is a semi-colon delimited text
file that can be edited by most common spreadsheet programs (be sure to
load it as "Text CSV").  The file has one game entry per line, with the very
first line of the file specifying what each column represents.

In addition to the romlist generation function available in config mode,
Attract-Mode can generate a single romlist containing roms for multiple
emulators from the command line using the following command:

		attract --build-romlist <emulator names...>

You can also import romlists from mame listxml files as well as gamelists for
other frontends.  Supported source files include: *.lst (MameWah lists), *.txt
(Attract-Mode lists) and *.xml (Mame listxml, listsoftware and HyperSpin lists):

		attract --import-romlist <source_file> [emulator name]

The --build-romlist and --import-romlist options can be chained together in
all sorts of strange and wonderful ways to generate combined Attract-Mode
romlists. So:

`attract --import-romlist mame.lst --import-romlist nintendo.lst nestopia`

will combine the entries from the mame.lst and nintendo.lst files (located
in the current directory) into a single Attract-Mode romlist.  The "mame"
emulator will be used for the mame.lst games, while the "nestopia" emulator
will be used for the nintendo.lst games.

One or more filter rules can also be applied when importing or building a
romlist from the command line using the "--filter <FILTER RULE>" option.  So
for example the following command line:

`attract --build-romlist mame --filter "Rotation equals 90|270"`

will build a romlist that contains only the vertical games in your collection.
The <FILTER RULE> can be specified in exactly the same format as how filter
rules are specified in Attract-Mode's attract.cfg file.

If you wish to specify the name of the created romlist at the command
line, you can do so with the `--output <name>` option.  Beware that this will
overwrite any existing Attract-Mode romlist with the specified name.

For a full description of the command lines options available, run:

`attract --help`

Attract-Mode by default will print log messages to the console window (stdout).
To suppress these messages, run with the following command line:

`attract --loglevel silent`

Alternatively, more verbose debug log messages can be enabled by running:

`attract --loglevel debug`

[Compile.md]: Compile.md
[Layouts.md]: Layouts.md
