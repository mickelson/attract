Compiling Attract-Mode
----------------------

Build instructions are set out by operating system below:

Linux, FreeBSD:
---------------

These instructions assume that you have the GNU C/C++ compilers and basic
build utilities (make, pkg-config, ar) on your system.  This means the
"build-essential" and "pkg-config" packages on Debian or Ubuntu-based
distributions.  Other distributions should have similar packages available.

1. Install the following libraries and related headers on your system:
   * Required:
      - SFML SDK version 2.x (<http://sfml-dev.org>)
      - OpenAL (this is used by SFML as well)
   * Optional:
      - The following FFmpeg libraries (required for movies): avformat,
      avcodec, swscale and either swresample or avresample.
      - Fontconfig (to assist with finding fonts).

2. Extract the Attract-Mode source to your system.

3. From the directory you extracted the source into, run:

           make

   or, to build without movie support:

           make NO_MOVIE=1

   This step will create the "attract" executable file.

4. The final step is to actually copy the Attract-Mode executable and data
   to a location where they can be used.  To install on a system-wide basis
   you should run:

           sudo make install

   This will copy the "attract" executable to `/usr/local/bin/` and default
   data to `/usr/local/share/attract/`

   For a single user install on Linux or FreeBSD, you can complete this step
   by copying the contents of the "config" directory from the Attract-Mode
   source directory to the location that you will use as your Attract-Mode
   config directory.  By default, this config directory is located in
   `$HOME/.attract` on Linux/FreeBSD systems.

   NOTE: The Attract-Mode makefile tries to follow the GNU standards for
   specifying installation directories: <https://www.gnu.org/prep/standards/html_node/Directory-Variables.html>.
   If you want to change the location where Attract-Mode looks for its default
   data from `/usr/local/share/attract` you should change these values
   appropriately before running the `make` and `make install` commands.

OS X:
-----

These instructions assume that you have X Code installed.

1. Install Homebrew (<http://brew.sh>).  This can be done by running the
   following command at a terminal command prompt:

           ruby -e "$(curl -fsSL https://raw.github.com/Homebrew/homebrew/go/install)"

2.  Install the "pkg-config", "ffmpeg" and "sfml" homebrew recipes:

           brew update
           brew install pkg-config ffmpeg sfml

   NOTE (XCODE 5 or LATER): The homebrew version of SFML seems to produce
   linker errors with Xcode 5 and clang.  If you have Xcode 5 or later then
   it is recommended that you install the SFML SDK using the package from
   the SFML website instead of using the homebrew recipe:
   <http://www.sfml-dev.org/download/sfml/2.1/SFML-2.1-osx-clang-universal.tar.gz>

3. Extract the Attract-Mode source to your system.

4. From the directory you extracted the Attract-Mode source into, run:

           make

   This step will create the "attract" executable file.

5. The final step is to actually copy the Attract-Mode executable and data to
   the location where they will be used.  You can run:

           sudo make install

   to install on a system-wide basis.  This will copy the 'attract' executable
   to `/usr/local/bin/` and data to `/usr/local/share/attract/`

   If you prefer to do a single user install, you can complete this step by
   copying the contents of the "config" directory from the Attract-Mode
   source directory to the location that you will use as your Attract-Mode
   config directory.  By default, this config directory is `$HOME/.attract` on
   OS X.

Windows (cross-compile):
------------------------

The recommended way to build Windows binaries for Attract-Mode is to cross
compile on an OS that supports MXE (<http://mxe.cc>) such as Linux, FreeBSD or
OS X.

1. Follow the steps in the mxe tutorial to set up mxe on your system: 
<http://mxe.cc/#tutorial>

2. Make mxe's sfml and ffmpeg packages:

           make ffmpeg sfml

   the above command will make 32-bit versions of ffmpeg and sfml (and anything
   else that they depend on). To make the 64-bit version use the following:

           make MXE_TARGETS='x86_64-w64-mingw32' ffmpeg sfml

3. Extract the Attract-Mode source to your system.

4. From the directory you extracted the source into, run the following:

           make WINDOWS_STATIC=1 CROSS=i686-pc-mingw32-

   to build the 32-bit version of Attract-Mode. To build 64-bit, run:

           make WINDOWS_STATIC=1 CROSS=x86_64-w64-mingw32-

   This step will create the "attract.exe" executable file.

5. Copy the contents of the config directory from the Attract-Mode source
   directory and the executable you just built into the same directory on your
   Windows-based system, and you should be ready to go!

Windows (native compile):
-------------------------

It is possible (but not recommended or supported) to compile Attract-Mode
natively on Windows-based systems.  If you can, you really should cross-compile
following the instructions above.

To build natively on Windows you might have luck following the instructions
for compiling on Linux and FreeBSD with the following modifications:

-  Before starting, install MinGW (<http://mingw.org>) and "pkg-config" on your
   system.  (see <http://sourceforge.net/projects/pkgconfiglite/> for what looks
   to be the least painful way of getting pkg-config on Windows).  This will
   get you a gcc compiler and the required build tools.

-  Before running the make command, remember to set the required MinGW
   environment variables first (see the MinGW documentation) and be sure to use
   the MinGW make command.  Path variables may have to be set in order for
   the make script to be able to find the pkg-config command.

-  Once you have built the executable, you should copy the contents of the
   "config" directory from the Attract-Mode source directory and the
   Attract-Mode executable into the same directory before running.

