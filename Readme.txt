Attract-Mode Frontend
=====================

Attract-Mode is a graphical front-end for emulators such as MAME, 
MESS, nestopia, etc.  It is designed to be run in an arcade cabinet 
and controlled with a joystick or spin dial.  It is written in C++ 
and uses SFML 2.0 (for graphics, sound and input). It uses ffmpeg / 
libav libraries for movie support.

Attract-Mode was developed for use in Linux.  It is known to work 
on Ubuntu Linux, Mac OS-X 10.6, and Windows 7.

Features include:

- Fully customizable display, sounds, and input (joystick, keyboard, 
mouse).

- Display videos, artwork (marquees, screenshots, etc.) and 
information related to selected game.  

- Supports screen rotation (including auto-rotation to match the 
rotation of the last game played).  Layouts can further rotate, scale 
and position images, videos and text.

- Generates lists from directory contents, imports game info from 
MAME and MESS -xmlinfo commands and from catver.ini files.

- Supports Unicode (UTF-8)

- Configuration mode

Download 
========
You can get the current version of Attract-Mode from the git 
repository (https://github.com/mickelson/attract.git)

Compile
=======

1. Install development versions of the the following:

   Required:
      - SFML version 2.0 
      - expat XML parser

   Optional:
      - ffmpeg/libav's avformat, avcodec, swscale and avutil libs.
     	(for movie support).
      - fontconfig (for font configuration).

2. Download the Attract-Mode source, extract it to your system.

3. On Linux/OS-X: Run "make".  Edit the Makefile first if you 
don't want to use ffmpeg and/or libfontconfig (see Makefile comments).  

On Windows: Load the Codeblocks project file located in the "win32" 
subdirectory and compile in Codeblocks.  The Windows version of Attract
Mode is very experimental at this point.

Basic Installation
==================

1. Copy the contents of the "config" directory from the Attract-Mode 
source to the directory that you will use for your Attract-Mode 
configuration.  By default, the configuration is located in 
"~/.attract" on Linux/FreeBSD, "~/Library/Application Support/Attract" 
on Mac OS-X, and "~/attract" on Windows systems.

2. Run Attract-Mode.  If you are not using the default config location 
then you need to specify your config location at the command line as 
follows:

   attract -config /my/config/location

If you have compiled Attract-Mode without libfontconfig support, it may
have difficulty finding a display font to use on your system.  If this
occurs you can specify a font file to use at the command line as follows: 

	attract -font <font_name>

3. With Attract-Mode running, press "TAB" to enter configuration mode.

4. Select the "Emulators" option and edit/create a configuration for 
an emulator that you wish to use.  Default configs are provided for 
some popular emulators, however some settings will likely have to be 
customized for your system (file locations etc).

5. Once you have an Emulator configured correctly for your system, 
select the "Generate Romlist" option from the Emulator's configuration 
menu.  Attract-Mode will use the configured emulator settings to 
generate a list of available games for the emulator.

6. Now select the "Lists" configuration option from the main config menu
and create a new list using the Romlist generated above in step 5.  
Select one of the included layouts for the layout to use.

7.  Exit configuration mode by selecting the "Back" option a few times. 
You should now have a usable front-end! 
