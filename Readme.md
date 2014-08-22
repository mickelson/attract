Attract-Mode Frontend
---------------------

Attract-Mode is a graphical frontend for command line emulators such as
MAME, MESS, and Nestopia.  It hides the underlying operating system and is
intended to be controlled with a joystick, gamepad or spin dial, making it
ideal for use in arcade cabinet setups.

Attract-Mode was originally developed for use in Linux.  It is known to work
on Ubuntu Linux (x86, x86-64), Mac OS X (10.6.8), and Windows (x86 and
x86-64, XP, 7).

Attract-Mode is licensed under the terms of the GNU General Public License,
version 3 or later.

Please visit <http://attractmode.org> for more information.

See [Compile.md][] if you intend to compile Attract-Mode from source.

Quick Start
-----------

1. Run Attract-Mode.  By default, Attract-Mode will look for its configuration
files in "$HOME/.attract" on Linux and Mac OS X, and in the current working
directory on Windows-based systems.  If you want to use a different location
for the Attract-Mode configuration then you need to specify it at the command
line as follows:

		attract --config /my/config/location

	In the (hopefully unlikely) event that Attract-Mode has difficulty
finding a display font to use on your system, you can specify one at the
command line as follows:

		attract --font <font_name>

2. If you are running Attract-Mode for the first time, it will load directly
to its configuration mode.  If you do not start in config mode, pressing "TAB"
will get you there.  By default, config mode can be navigated using the up/
down arrows, enter to select an option, and escape to go back.

3. Select the "Emulators" menu and edit/create a configuration for an
emulator that you wish to use.  Default configurations are provided for some
popular emulators, however some settings will likely have to be customized
for your system (file locations etc).

4. Once you have an emulator configured correctly for your system, select
the "Generate Romlist" option from the emulator's configuration menu.
Attract-Mode will use the configured emulator settings to generate a list of
available games for the emulator.

5.  Exit config mode by selecting the "Back" option a few times.  You should
now have a usable front-end!

Further Customization
---------------------

**INPUT:** The inputs used to control Attract-Mode can be configured from
from the "Input" menu in config mode.  Attract-Mode actions can be mapped to
most keyboard, mouse and joystick inputs.

**LIST FILTERS:** List filters can be added from the "Lists" menu in config
mode.  Each filter can have multiple rules associated with it (i.e. a "1980's
Multiplayer Sports" filter would have 3 rules: (1) that the year be in the
1980's, (2) that the number of players is not 1, and (3) that the category
contain "Sports").  You can create multiple filters per List and cycle/switch
between them using the "Previous Filter", "Next Filter" and "Filters List"
actions.

**SOUND:** To play sounds in your setup, place the sound file in the "sounds"
subdirectory of your Attract-Mode config directory.  The sound file can then
be selected from the "Sound" menu when in config mode and mapped to an action
or event.

**ARTWORK:** Attract-Mode supports PNG, JPEG, GIF, BMP and TGA image formats.
For video formats, Attract-Mode should support any video format supported by
FFmpeg.  When deciding what file to use for a particular artwork type,
Attract-Mode will use the artwork selection order set out below.

The location of artwork resources is configured on a per-emulator basis in the
emulator configuration menu.  Attract-Mode's default artworks are: "marquee"
(for cabinet marquee images), "snap" (for attract-mode videos and game screen
shots), "flyer" (for game flyer/box art) and "wheel" (for Hyperspin wheel art).
You can add others as needed in the emulator configuration menu.  Multiple
paths can be specified for each artwork, in which case Attract-Mode will check
each path in the order they are specified before moving to the next check in
the selection order.

**ARTWORK SELECTION ORDER:**

   * From the artwork path configured for the emulator (if any):

      - [Name].*      (video, i.e. "pacman.mp4")
      - [CloneOf].*   (video, i.e. "puckman.mp4")
      - [Name].*      (image, i.e. "pacman.jpg")
      - [CloneOf].*   (image, i.e. "puckman.png")
      - [Emulator].*  (video, i.e. "mame.avi")
      - [Emulator].*  (image, i.e. "mame.gif")

   * From the layout path for the current layout (layouts are located in
   the "layouts" subdirectory of your Attract-Mode config directory):

      - [Emulator]-[ArtLabel].*   (video, i.e. "mame-marquee.mp4")
      - [Emulator]-[ArtLabel].*   (image, i.e. "mame-marquee.png")
      - [ArtLabel].*              (video, i.e. "marquee.avi")
      - [ArtLabel].*              (image, i.e. "marquee.png")

   * If no files are found matching the above rules, then the artwork
   is not drawn.

   If no file match is found, Attract-Mode will check for a subdirectory
   that meets the match criteria.  If a subdirectory is found (for example
   a "pacman" directory in the configured artwork path) then Attract-Mode
   will then pick a random video or image from that directory.

**LAYOUTS:** See: [Layouts.md][]

**PLUG-INS:** Plug-ins are scripts that need to be placed in the "plugins"
subdirectory of your Attract-Mode config directory.  Available plugins
can be enabled/disabled and configured from the "Plug-Ins" menu when in
config mode.

**ROMLISTS:** Romlists are saved in the "romlist" subdirectory of your
Attract-Mode config directory.  Each list is a semi-colon delimited text
file that can be edited by most common spreadsheet programs (be sure to
load it as "Text CSV").  The file has one game entry per line, with the very
first line of the file specifying what each column represents.

In addition to the romlist generation function available in config mode,
Attract-Mode can generate a romlist for multiple emulators from the command
line using the following command:

		attract --build-romlist <emulator names...>

You can also import romlists from MameWah/Wahcade! (.lst), Attract-Mode
(.txt) and HyperSpin (.xml) using the following command:

		attract --import-romlist <file> [emulator name]

The --build-romlist and --import-romlist options can be chained together in
all sorts of strange and wonderful ways to generate combined Attract-Mode
romlists. So:

`attract --import-romlist mame.lst --import-romlist nintendo.lst nestopia`

will combine the entries from the mame.lst and nintendo.lst files (located
in the current directory) into a single Attract-Mode romlist.  The "mame"
emulator will be used for the mame.lst games, while the "nestopia" emulator
will be used for the nintendo.lst games.

If you wish to specify the name of the created romlist at the command
line, you can do so with the `--output <name>` option.  Beware that this will
overwrite any existing Attract-Mode romlist with the specified name.

[Compile.md]: Compile.md
[Layouts.md]: Layouts.md
