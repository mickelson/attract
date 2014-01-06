Compiling Attract-Mode
----------------------

These instructions assume that you have the GNU C/C++ compilers and basic
build utilities (make, ar) on your system.  This means the "build-essential"
package on Debian/Ubuntu, X-Code on OS X, or MinGW on Windows. 

Steps:
------

1. Install the following libraries and headers on your system:
   * Required:
      - SFML SDK version 2.x
   * Optional:
      - FFmpeg's avformat, avcodec, swscale, avresample and avutil libs.
      (for movies).
      - Fontconfig (Linux/FreeBSD only - to assist with font configuration).

2. Extract the Attract-Mode source to your system.

3. On Linux/OS X: Run the "make" command.  You can edit the Makefile first 
if you wish to change build options  (i.e. to disable FFmpeg or Fontconfig).  

	On Windows: Either use the Makefile with the MINGW make command or load
the Codeblocks project file (attract.cbp) and compile in Codeblocks.

4. Copy the contents of the "config" directory from the Attract-Mode 
source directory to the location that you will use as your Attract-Mode 
config directory.  By default, this config directory is located in 
"$HOME/.attract" on Linux and Mac OS X, and in the current working directory 
on Windows-based systems.
