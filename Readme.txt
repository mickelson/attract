Attract-Mode Frontend
=====================

Attract-Mode is a graphical front-end for command line emulators such 
as MAME, MESS, and Nestopia.  It is designed to be run in an arcade 
cabinet setup and controlled with a joystick or spin dial.  Attract-
Mode is written in C++ and requires SFML 2.x to run.  It can use the
ffmpeg/libav libraries for movie support.

Attract-Mode was developed for use in Linux.  It is known to work 
on Ubuntu Linux, Mac OS-X 10.6, and Windows 7.

Please visit http://attractmode.org for more information.

Download 
========
You can get the current version of Attract-Mode from the git 
repository (https://github.com/mickelson/attract.git)

Compile
=======

1. These instructions assume that you have the GNU C/C++ compilers and 
basic build utilities (make, ar) on your system.  This means the "build-
essential" package on Debian/Ubuntu, X-Code on OS-X, or MinGW on Windows. 

2. Install the following libraries and headers on your system:

   Required:
      - SFML SDK version 2.x 

   Optional:
      - FFMPEG's avformat, avcodec, swscale and avutil libs.
     	(for movie support).
      - FontConfig (for font configuration on Linux/FreeBSD).

3. Download the Attract-Mode source, extract it to your system.

4. On Linux/OS-X: Run the "make" command.  Edit the Makefile first if you 
wish to change any build options  (i.e. to disable ffmpeg or fontconfig).  

On Windows: Either use the Makefile with the MINGW make command or load
the Codeblocks project file located in the "win32" subdirectory and 
compile in Codeblocks.

Basic Installation
==================

1. Copy the contents of the "config" directory from the Attract-Mode 
source to the directory that you will use for your Attract-Mode 
configuration.  By default, the configuration is located in 
"~/.attract" on Linux/FreeBSD, "~/Library/Application Support/Attract" 
on Mac OS-X, and "./" on Windows systems.

2. Run Attract-Mode.  If you are not using the default config location 
then you need to specify your config location at the command line as 
follows:

   attract --config /my/config/location

If you have compiled Attract-Mode without fontconfig support, it may
have difficulty finding a display font to use on your system.  If this
occurs you can specify a font file to use at the command line as follows: 

   attract --font <font_name>

3. With Attract-Mode running, press "TAB" to enter configuration mode.
By default, configuration mode can be navigated using the up/down arrows,
enter to select an option, and escape to go back a menu.

4. Select the "Emulators" option and edit/create a configuration for 
an emulator that you wish to use.  Default configs are provided for 
some popular emulators, however some settings will likely have to be 
customized for your system (file locations etc).

5. Once you have an emulator configured correctly for your system, 
select the "Generate Romlist" option from the emulator's configuration 
menu.  Attract-Mode will use the configured emulator settings to 
generate a list of available games for the emulator.

6. Now select the "Lists" configuration option from the main config menu
and create a new list using the romlist generated above in step 5.  
Select one of the included layouts for the layout to use.

7.  Exit configuration mode by selecting the "Back" option a few times. 
You should now have a usable front-end! 

Further Customization
=====================

SOUND: To play sounds in your setup, place the sound file in the "sounds" 
subdirectory of your Attract-Mode config directory.  The sound file can 
then be selected in the Sound menu when in configuration mode and mapped
to an input or event.

ARTWORK: Attract-Mode supports PNG, JPEG, GIF, BMP and TGA image formats.
When deciding what image file to use for a particular named artwork
(default artwork names are "marquee" and "screen"), Attract-Mode will 
use the artwork/movie selection order set out below.

MOVIE: Attract-Mode supports any movie format supported by FFMPEG.  The 
movie to play is decided as set out below:

ARTWORK/MOVIE SELECTION ORDER:

A. From the artwork path configured in the emulator setting (if any):
  1. [romname].xxx  (i.e. "pacman.png")
  2. [cloneof].xxx  (i.e. "puckman.gif")
  3. [emulator].xxx (i.e. "mame.jpg")

B. From the layout path for the current layout (layouts are located in
the "layouts" subdirectory of your Attract-Mode config directory):
  4. [emulator]-[artlabel].xxx  (i.e. "mame-marquee.png")
  5. [artlabel].xxx  (i.e. "marquee.png")

C. If no files are found matching the above rules, then the artwork/movie 
is left blank.

ROMLISTS: Romlists are saved in the "romlist" subdirectory of your 
Attract-Mode config directory.  Each list is a semi-colon delimited text 
file with one game/rom entry per line. The first line of each file 
describes what each column represents.  In addition to the Romlist 
generation function available in configuration mode, Attract-Mode can 
generate a romlist for multiple emulators from the command line using the 
following command: 

   attract --build-rom-list [emulator names...]
