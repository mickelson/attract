Compiling Attract-Mode
----------------------

Before you begin: These instructions require that you have the GNU
C/C++ compilers and basic build utilities (make, pkg-config, ar) on your
system.  This means:

* the "build-essential" and "pkg-config" packages on Debian or Ubuntu-based
  Linux distributions.  Other distributions should have similar packages
  available.

* "MinGW" (<http://mingw.org>) and "pkg-config" on Windows 
  (see <http://sourceforge.net/projects/pkgconfiglite/> for what looks to be
  the least painful way of getting pkg-config on Windows).

* "X-Code" on OS X.

Build instructions are set out by operating system below:

Linux, FreeBSD and Windows:
---------------------------

1. Install the following libraries and related headers on your system:
   * Required:
      - SFML SDK version 2.x (<http://sfml-dev.org>)
      - OpenAL (this is used by SFML as well)
   * Optional:
      - The following FFmpeg libraries (required for movies): avformat,
      avcodec, swscale and either swresample or avresample.
      - Fontconfig (Linux/FreeBSD only - to assist with font configuration).

2. Extract the Attract-Mode source to your system.

3. From the directory you extracted the source into, run:

           make

   or, to build without movie support: 

           make NO_MOVIE=1

   This step will create the "attract" executable file.

   On Windows: Remember to set the MinGW environment variables first and 
   to use the MINGW make command.

4. The final step is to actually copy the Attract-Mode executable and data
   to a location where they can be used.  On Linux and FreeBSD, you can run:

           sudo make install

   to install on a system-wide basis, available to all users.  This will copy
   the 'attract' executable to /usr/local/bin/ and data to
   /usr/local/share/attract/

   On Windows systems (or if you prefer to do a single user install on Linux or
   FreeBSD) you can complete this step by copying the contents of the "config"
   directory from the Attract-Mode source directory to the location that you
   will use as your Attract-Mode config directory.  By default, this config
   directory is located in "$HOME/.attract" on Linux/FreeBSD and in the current
   working directory on Windows.

OS X:
-----

1. Install Homebrew (<http://brew.sh>).  This can be done by running the
   following command at a terminal command prompt:

           ruby -e "$(curl -fsSL https://raw.github.com/Homebrew/homebrew/go/install)"

2.  Install the "pkg-config", "ffmpeg" and "sfml" homebrew recipes:

           brew update
           brew install pkg-config ffmpeg sfml

3. Extract the Attract-Mode source to your system.

4. From the directory you extracted the Attract-Mode source into, run:

           make

   This step will create the "attract" executable file.

5. The final step is to actually copy the Attract-Mode executable and data to
   the location where they will be used.  You can run:

           sudo make install

   to install on a system-wide basis.  This will copy the 'attract' executable
   to /usr/local/bin/ and data to /usr/local/share/attract/

   If you prefer to do a single user install, you can complete this step by
   copying the contents of the "config" directory from the Attract-Mode
   source directory to the location that you will use as your Attract-Mode
   config directory.  By default, this config directory is "$HOME/.attract" on
   OS X.

