Attract-Mode Frontend
---------------------

Attract-Mode is a graphical front-end for command line emulators such 
as MAME, MESS, and Nestopia.  It hides the underlying operating system and 
is intended to be controlled with a joystick, gamepad or spin dial, making 
it ideal for use in arcade cabinet setups.  Attract-Mode is written in C++ 
and uses [SFML][].  

Attract-Mode was developed for use in Linux.  It is known to work on Ubuntu 
Linux (x86), Mac OS-X 10.6, and Windows 7.

Attract-Mode's layouts are actually scripts written in the [Squirrel][]
programming language. [FFmpeg][] is used for (optional) movie support.

Please visit <http://attractmode.org> for more information.

Download 
--------

You can get the current version of Attract-Mode from the git 
repository: <https://github.com/mickelson/attract.git>

Compile
-------

1. These instructions assume that you have the GNU C/C++ compilers and 
basic build utilities (make, ar) on your system.  This means the "build-
essential" package on Debian/Ubuntu, X-Code on OS-X, or MinGW on Windows. 

2. Install the following libraries and headers on your system:

   * Required:
      - SFML SDK version 2.x 

   * Optional:
      - FFmpeg's avformat, avcodec, swscale and avutil libs.
     	(for movie support).
      - Fontconfig (for font configuration on Linux/FreeBSD).

3. Download the Attract-Mode source, extract it to your system.

4. On Linux/OS-X: Run the "make" command.  Edit the Makefile first if you 
wish to change any build options  (i.e. to disable FFmpeg or Fontconfig).  

	On Windows: Either use the Makefile with the MINGW make command or load
the Codeblocks project file (attract.cbp) and compile in Codeblocks.

Initial Setup
-------------

1. Copy the contents of the "config" directory from the Attract-Mode 
source to the location that you will use as your Attract-Mode config
directory.  By default, this config directory is located in "$HOME/.attract" 
on Linux and Mac OS-X, and in the current working directory on Windows-based
systems.

2. Run Attract-Mode.  If you are not using a default config directory
location (see above) then you need to specify a config directory location at
the command line as follows:

		attract --config /my/config/location

	If you have compiled Attract-Mode without Fontconfig support, it might
have difficulty finding a display font to use on your system.  If this
occurs you can specify a font to use at the command line as follows: 

		attract --font <font_name>

3. If you are running Attract-Mode for the first time, it will load directly 
to its configuration mode.  If you are not in configuration mode, press "TAB" 
to enter configuration mode.  By default, configuration mode can be navigated
using the up/down arrows, enter to select an option, and escape to go back a 
menu.

4. Select the "Emulators" menu and edit/create a configuration for an 
emulator that you wish to use.  Default configurations are provided for some 
popular emulators, however some settings will likely have to be customized 
for your system (file locations etc).

5. Once you have an emulator configured correctly for your system, select 
the "Generate Romlist" option from the emulator's configuration menu.  
Attract-Mode will use the configured emulator settings to generate a list of 
available games for the emulator.

6.  Exit configuration mode by selecting the "Back" option a few times. 
You should now have a usable front-end!

Further Customization
---------------------

**INPUT:** The inputs used to control Attract-Mode can be configured from
from the "Input" menu in configuration mode.  Attract-Mode actions can be
mapped to most keyboard, mouse and joystick inputs.

**LIST FILTERS:** List filters can be configured from the "Lists" menu 
in configuration mode. Lists can be created that filter to specific 
categories, years, manufacturers, etc.

**SOUND:** To play sounds in your setup, place the sound file in the "sounds" 
subdirectory of your Attract-Mode config directory.  The sound file can 
then be selected from the "Sound" menu when in configuration mode and mapped
to an input or event.

**ARTWORK:** Attract-Mode supports PNG, JPEG, GIF, BMP and TGA image formats.
When deciding what image file to use for a particular artwork type
(default artworks are "marquee" and "screen"), Attract-Mode will use the
artwork/movie selection order set out below.

**MOVIE:** Attract-Mode should support any movie format supported by FFmpeg.
The movie to play is decided in the order set out below.

**ARTWORK/MOVIE SELECTION ORDER:**

   * From the artwork path configured in the emulator setting (if any):

      - [Name].xxx  (i.e. "pacman.png")  
      - [CloneOf].xxx  (i.e. "puckman.gif")  
      - [Emulator].xxx (i.e. "mame.jpg")  

   * From the layout path for the current layout (layouts are located in
   the "layouts" subdirectory of your Attract-Mode config directory):

      - [Emulator]-[ArtLabel].xxx  (i.e. "mame-marquee.png")  
      - [ArtLabel].xxx  (i.e. "marquee.png")  

   * If no files are found matching the above rules, then the artwork/movie 
   is left blank.

**LAYOUTS:** See: [Layouts.md][]

**ROMLISTS:** Romlists are saved in the "romlist" subdirectory of your 
Attract-Mode config directory.  Each list is a semi-colon delimited text 
file that can be edited by most common spreadsheet programs (be sure to
load it as "Text CSV").  The file has one game entry per line, with the 
first line describing what each column represents.

In addition to the Romlist generation function available in configuration 
mode, Attract-Mode can generate a romlist for multiple emulators from the 
command line using the following command: 

		attract --build-rom-list [emulator names...]

[Layouts.md]: Layouts.md
[SFML]: http://www.sfml-dev.org
[Squirrel]: http://www.squirrel-lang.org
[FFmpeg]: http://www.ffmpeg.org
