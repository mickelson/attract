Compiling Attract-Mode
----------------------

Before you begin: These instructions require that you have the GNU
C/C++ compilers and basic build utilities (make, pkg-config, ar) on your
system.  This means:

* the "build-essential" and "pkg-config" packages on Debian/Ubuntu.  Other
  Linux distributions should have similar packages available.

* "MinGW" (<http://mingw.org>) and "pkg-config" on Windows 
  (see <http://sourceforge.net/projects/pkgconfiglite/> for what looks to be
  the least painful way of getting pkg-config on Windows).

* "X-Code" on OS X.

Build instructions are set out by operating system below:

Linux, FreeBSD and Windows:
---------------------------

1. Install the following libraries and headers on your system:
   * Required:
      - SFML SDK version 2.x (<http://sfml-dev.org>)
   * Optional:
      - The following FFmpeg development libraries (required for movies):
        avformat, avcodec, swscale and either swresample or avresample.
      - Fontconfig (Linux/FreeBSD only - to assist with font configuration).

2. Extract the Attract-Mode source to your system.

3. From the directory you extracted the source into, run:

           make

   or, to build without movie support: 

           make NO_MOVIE=1

   This step will create the "attract" executable file.

   On Windows: Remember to set the MinGW environment variables first and 
to use the MINGW make command.

4. Copy the contents of the "config" directory from the Attract-Mode 
source directory to the location that you will use as your Attract-Mode 
config directory.  By default, this config directory is located in 
"$HOME/.attract" on Linux and in the current working directory on Windows.

OS X:
-----

1. Install Homebrew by pasting the following at the command prompt:

           ruby -e "$(curl -fsSL https://raw.github.com/Homebrew/homebrew/go/install)"

2.  Install the pkg-config and ffmpeg homebrew recipes:

           brew install pkg-config
           brew install ffmpeg

3. Download and install SFML on your system.  This can be done by following
"Installing SFML" instructions at: <http://sfml-dev.org/tutorials/2.1/start-osx.php>

4. Extract the Attract-Mode source to your system.

5. From the directory you extracted the Attract-Mode source into, run:

           make

   This step will create the "attract" executable file.

6. Copy the contents of the "config" directory from the Attract-Mode 
source to the location that you will use as your Attract-Mode config
directory.  By default, this config directory is located in "$HOME/.attract"
on OS X.
