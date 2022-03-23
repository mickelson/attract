##
##
##  Attract-Mode frontend
##  Copyright (C) 2013-2018 Andrew Mickelson
##
##  This file is part of Attract-Mode.
##
##  Attract-Mode is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  Attract-Mode is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with Attract-Mode.  If not, see <http://www.gnu.org/licenses/>.
##
##
###############################
#
# BUILD CONFIGURATION OPTIONS:
#
# Uncomment next line to build GLES version (embedded systems)
#USE_GLES=1
#
# Uncomment the next line to build the DRM/KMS version (alternative to X11)
#USE_DRM=1
#
# Uncomment next line to disable movie support (i.e. no FFmpeg).
#NO_MOVIE=1
#
# Uncomment the next line to enable the use of MMAL video decoder
#USE_MMAL=1
#
# By default, if FontConfig gets enabled we link against the system's expat
# library (because FontConfig uses expat too).  If FontConfig is not used
# then Attract-Mode is statically linked to its own version of expat.
# Uncomment next line to always link to Attract-Mode's version of expat.
#BUILD_EXPAT=1
#
# Uncomment next line for Windows static cross-compile build (mxe)
#WINDOWS_STATIC=1
#
# By default, Attract-Mode on Windows is built as a GUI application, which
# does not allow for command line interactions at the Windows console.
# Uncomment the next line to build a console version of Attract-Mode
# instead (Windows only)
#WINDOWS_CONSOLE=1
#
# Uncomment the next line to disable SWF support (i.e. no game_swf)
#NO_SWF=1
#
# Uncomment the next line(s) as appropriate to enable hardware video
# decoding on Linux (req FFmpeg >= 3.4)
#FE_HWACCEL_VAAPI=1
#FE_HWACCEL_VDPAU=1
#
# Uncomment the next line to build attract.debug file with debug symbols
#ENABLE_DEBUG_SYMBOLS=1
#
###############################

#FE_DEBUG=1
#VERBOSE=1
#WINDOWS_XP=1

FE_VERSION=v2.6.2

CC=gcc
CXX=g++
CFLAGS=
CXXFLAGS=-std=c++11
CPPFLAGS=
STRIP=strip
OBJCOPY=objcopy
PKG_CONFIG=pkg-config
AR=ar
ARFLAGS=rc
RM=rm -f
MD=mkdir -p
WINDRES=windres

-include attract.mk

OPTIMIZE ?= 2

ifndef VERBOSE
 SILENT = @
 CC_MSG = @echo Compiling $@...
 AR_MSG = @echo Archiving $@...
 EXE_MSG = @echo Creating executable: $@
 DEBUG_MSG = @echo Writing debug symbols: $@.debug
endif

ifneq ($(origin TOOLCHAIN),undefined)
override CC := $(TOOLCHAIN)-$(CC)
override CXX := $(TOOLCHAIN)-$(CXX)
override AR := $(TOOLCHAIN)-$(AR)
endif

ifneq ($(origin CROSS),undefined)
override STRIP := $(TOOLCHAIN)-$(STRIP)
override PKG_CONFIG := $(TOOLCHAIN)-$(PKG_CONFIG)
override WINDRES := $(TOOLCHAIN)-$(WINDRES)
endif

prefix ?= /usr/local
datarootdir ?= $(prefix)/share
datadir ?= $(datarootdir)
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin

DATA_PATH:=$(datadir)/attract/
EXE_BASE=attract
EXE_EXT=
OBJ_DIR=obj
SRC_DIR=src
EXTLIBS_DIR=extlibs
FE_FLAGS :=

_DEP =\
	fe_base.hpp \
	fe_file.hpp \
	fe_util.hpp \
	fe_util_sq.hpp \
	fe_info.hpp \
	fe_input.hpp \
	fe_romlist.hpp \
	scraper_base.hpp \
	scraper_xml.hpp \
	fe_settings.hpp \
	fe_config.hpp \
	fe_presentable.hpp \
	fe_present.hpp \
	sprite.hpp \
	fe_image.hpp \
	fe_sound.hpp \
	fe_shader.hpp \
	fe_overlay.hpp \
	fe_window.hpp \
	tp.hpp \
	fe_text.hpp \
	fe_listbox.hpp \
	fe_vm.hpp \
	fe_blend.hpp \
	path_cache.hpp \
	image_loader.hpp \
	zip.hpp

_OBJ =\
	fe_base.o \
	fe_file.o \
	fe_util.o \
	fe_util_sq.o \
	fe_cmdline.o \
	fe_info.o \
	fe_input.o \
	fe_romlist.o \
	fe_settings.o \
	scraper_base.o \
	scraper_xml.o \
	scraper_general.o \
	scraper_net.o \
	scraper_gamesdb.o \
	fe_config.o \
	fe_presentable.o \
	fe_present.o \
	sprite.o \
	fe_image.o \
	fe_sound.o \
	fe_shader.o \
	fe_overlay.o \
	fe_window.o \
	tp.o \
	fe_text.o \
	fe_listbox.o \
	fe_vm.o \
	fe_blend.o \
	zip.o \
	path_cache.o \
	image_loader.o \
	main.o

ifneq ($(FE_WINDOWS_COMPILE),1)
 #
 # Test OS to set some defaults
 #
 ifeq ($(OS),Windows_NT)
  #
  # Windows
  #
  FE_WINDOWS_COMPILE=1
 else
  UNAME = $(shell uname -a)
  ifeq ($(firstword $(filter Darwin,$(UNAME))),Darwin)
   FE_MACOSX_COMPILE=1
  endif
  ifeq ($(FE_MACOSX_COMPILE),1)
   #
   # Mac OS X
   #
   _DEP += fe_util_osx.hpp
   _OBJ += fe_util_osx.o
   LIBS += -framework Cocoa -framework Carbon -framework IOKit -framework CoreVideo
  else
   ifeq ($(USE_GLES),1)
   else
    ifeq ($(USE_DRM),1)
    else
     #
     # Test for Xlib and Xinerama...
     #
     ifeq ($(shell $(PKG_CONFIG) --exists x11 && echo "1" || echo "0"), 1)
      USE_XLIB=1
      ifeq ($(shell $(PKG_CONFIG) --exists xinerama && echo "1" || echo "0"), 1)
       USE_XINERAMA=1
      endif
     endif
    endif
   endif
  endif
 endif
endif

#
# Deal with SFML
#
ifeq ($(WINDOWS_STATIC),1)
 ifeq ($(shell $(PKG_CONFIG) --exists sfml && echo "1" || echo "0"), 1)
  SFML_PC="sfml"
 else
  SFML_PC="sfml-system sfml-window sfml-graphics"
 endif
 LIBS += $(shell $(PKG_CONFIG) --static --libs $(SFML_PC))
 CPPFLAGS += -DSFML_STATIC $(shell $(PKG_CONFIG) --static --cflags $(SFML_PC))
 FE_WINDOWS_COMPILE=1

else

 LIBS += -lsfml-graphics \
	-lsfml-window \
	-lsfml-system
endif

ifneq ($(NO_SWF),1)
 _DEP += swf.hpp
 _OBJ += swf.o
 LIBS += -ljpeg -lz -lpthread

 ifneq ($(FE_WINDOWS_COMPILE),1)
  ifneq ($(FE_MACOSX_COMPILE),1)
   CPPFLAGS += -Wl,--export-dynamic
   LIBS += -ldl
   TEMP_LIBS += freetype2
   CPPFLAGS += $(shell $(PKG_CONFIG) --cflags --silence-errors freetype2)
  endif
 else
  TEMP_LIBS += freetype2
 endif

endif

ifeq ($(FE_WINDOWS_COMPILE),1)
 _DEP += attract.rc
 _OBJ += attract.res
 ifeq ($(WINDOWS_XP),1)
  FE_FLAGS += -DWINDOWS_XP
 else
  LIBS += -ldwmapi
 endif
 ifeq ($(WINDOWS_CONSOLE),1)
  CPPFLAGS += -mconsole
  FE_FLAGS += -DWINDOWS_CONSOLE
 else
  CPPFLAGS += -Wl,--subsystem,windows
 endif

 EXE_EXT = .exe
else
 CPPFLAGS += -DDATA_PATH=\"$(DATA_PATH)\"
endif

#
# Check whether optional libs should be enabled
#
ifneq ($(FE_WINDOWS_COMPILE),1)
 ifeq ($(shell $(PKG_CONFIG) --exists fontconfig && echo "1" || echo "0"), 1)
 USE_FONTCONFIG=1
 endif
endif

ifeq ($(shell $(PKG_CONFIG) --exists libswresample && echo "1" || echo "0"), 1)
 USE_SWRESAMPLE=1
endif

ifeq ($(shell $(PKG_CONFIG) --exists libavresample && echo "1" || echo "0"), 1)
 USE_AVRESAMPLE=1
endif

ifeq ($(shell $(PKG_CONFIG) --exists libarchive && echo "1" || echo "0"), 1)
 USE_LIBARCHIVE=1
endif

ifeq ($(shell $(PKG_CONFIG) --exists libcurl && echo "1" || echo "0"), 1)
 USE_LIBCURL=1
endif

#
# Now process the various settings...
#
ifeq ($(FE_DEBUG),1)
 CPPFLAGS += -Wall
 FE_FLAGS += -DFE_DEBUG
 ENABLE_DEBUG_SYMBOLS=1
else
 CPPFLAGS += -O$(OPTIMIZE) -DNDEBUG
endif

ifeq ($(USE_GLES),1)
 FE_FLAGS += -DUSE_GLES

 #
 # Hack for Raspberry Pi includes...
 #
 ifneq ($(USE_VC4),1)
  ifneq ("$(wildcard /opt/vc/include/bcm_host.h)","")
   CPPFLAGS += -I/opt/vc/include -L/opt/vc/lib
   ifneq ("$(wildcard /opt/vc/lib/libbrcmGLESv2.so)","")
    FE_FLAGS += -DUSE_BCM
    GLES_LIB := -lbrcmGLESv2
    LIBS += -lbcm_host
   else
    GLES_LIB := -lGLESv2
   endif
  endif
 endif
endif

ifeq ($(FE_MACOSX_COMPILE),1)
  LIBS += -framework OpenGL -ljpeg
else
 ifeq ($(FE_WINDOWS_COMPILE),1)
  LIBS += -lopengl32
 else
  LIBS += -lpthread
  ifeq ($(USE_GLES),1)
   GLES_LIB ?= -lGLESv1_CM
   LIBS += $(GLES_LIB)
  else
   LIBS += -lGL
  endif
 endif
endif


ifeq ($(USE_MMAL),1)
 FE_FLAGS += -DUSE_MMAL
endif

ifeq ($(USE_XLIB),1)
 FE_FLAGS += -DUSE_XLIB
 LIBS += -lX11 -lXrandr

 ifeq ($(USE_XINERAMA),1)
  FE_FLAGS += -DUSE_XINERAMA
  LIBS += -lXinerama
 endif

endif

ifeq ($(USE_DRM),1)
 TEMP_LIBS += libdrm gbm
 FE_FLAGS += -DUSE_DRM
endif

ifeq ($(FE_HWACCEL_VAAPI),1)
 FE_FLAGS += -DFE_HWACCEL_VAAPI
endif

ifeq ($(FE_HWACCEL_VDPAU),1)
 FE_FLAGS += -DFE_HWACCEL_VDPAU
endif

ifeq ($(USE_FONTCONFIG),1)
 FE_FLAGS += -DUSE_FONTCONFIG
 TEMP_LIBS += fontconfig
else
 BUILD_EXPAT=1
endif

ifeq ($(USE_LIBARCHIVE),1)
 FE_FLAGS += -DUSE_LIBARCHIVE
 TEMP_LIBS += libarchive
 LIBS += -lz
else
 CPPFLAGS += -I$(EXTLIBS_DIR)/miniz
endif

ifeq ($(USE_LIBCURL),1)
 FE_FLAGS += -DUSE_LIBCURL
 TEMP_LIBS += libcurl
 _DEP += fe_net.hpp
 _OBJ += fe_net.o
endif

ifeq ($(ENABLE_DEBUG_SYMBOLS),1)
 CPPFLAGS += -g
 #
 # libs for pretty output from backward-cpp
 #
 ifeq ($(shell $(PKG_CONFIG) --exists libdw && echo "1" || echo "0"), 1)
  TEMP_LIBS += libdw
  CPPFLAGS +=-DBACKWARD_HAS_DW=1
  _OBJ += backward.o
 else
  ifeq ($(shell $(PKG_CONFIG) --exists libbfd && echo "1" || echo "0"), 1)
   TEMP_LIBS += libbfd
   CPPFLAGS +=-g -DBACKWARD_HAS_BFD=1
   _OBJ += backward.o
  else
   ifeq ($(shell $(PKG_CONFIG) --exists libdwarf && echo "1" || echo "0"), 1)
    TEMP_LIBS += libdwarf
    CPPFLAGS += -DBACKWARD_HAS_DWARF=1
    _OBJ += backward.o
   endif
  endif
 endif
endif

ifeq ($(NO_MOVIE),1)
 FE_FLAGS += -DNO_MOVIE
 ifeq ($(WINDOWS_STATIC),1)
  LIBS += -lsfml-audio-s
 else
  LIBS += -lsfml-audio
 endif
 AUDIO =
else
 TEMP_LIBS += libavformat libavcodec libavutil libswscale

 ifeq ($(FE_MACOSX_COMPILE),1)
  LIBS += -framework OpenAL
 else
  TEMP_LIBS += openal
 endif

 ifeq ($(USE_SWRESAMPLE),1)
  TEMP_LIBS += libswresample
  FE_FLAGS += -DUSE_SWRESAMPLE
 else
  ifeq ($(USE_AVRESAMPLE),1)
  TEMP_LIBS += libavresample
  FE_FLAGS += -DUSE_AVRESAMPLE
  endif
 endif

 _DEP += media.hpp
 _OBJ += media.o

 CPPFLAGS += -I$(EXTLIBS_DIR)/audio/include
 AUDIO = $(OBJ_DIR)/libaudio.a
endif

CPPFLAGS += -D__STDC_CONSTANT_MACROS

LIBS := $(LIBS) $(shell $(PKG_CONFIG) --libs $(TEMP_LIBS))
CPPFLAGS := $(CPPFLAGS) $(shell $(PKG_CONFIG) --cflags $(TEMP_LIBS))

EXE = $(EXE_BASE)$(EXE_EXT)

ifeq ($(BUILD_EXPAT),1)
 CPPFLAGS += -I$(EXTLIBS_DIR)/expat
 EXPAT = $(OBJ_DIR)/libexpat.a
 ifneq ($(FE_WINDOWS_COMPILE),1)
  EXPAT_FLAGS = -DXML_DEV_URANDOM -DHAVE_MEMMOVE
 endif
else
 LIBS += -lexpat
 EXPAT =
endif

CPPFLAGS += -I$(EXTLIBS_DIR)/squirrel/include -I$(EXTLIBS_DIR)/sqrat/include -I$(EXTLIBS_DIR)/nowide -I$(EXTLIBS_DIR)/nvapi -I$(EXTLIBS_DIR)/rapidjson/include
SQUIRREL = $(OBJ_DIR)/libsquirrel.a $(OBJ_DIR)/libsqstdlib.a

# Our nowide "lib" is only needed on Windows systems
ifeq ($(FE_WINDOWS_COMPILE),1)
 SQUIRREL += $(OBJ_DIR)/libnowide.a
endif

ifeq ($(NO_SWF),1)
 FE_FLAGS += -DNO_SWF
else
 CPPFLAGS += -I$(EXTLIBS_DIR)/gameswf
 SQUIRREL += $(OBJ_DIR)/libgameswf.a
endif

$(info flags: $(CXXFLAGS) $(CPPFLAGS) $(FE_FLAGS))

OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))
DEP = $(patsubst %,$(SRC_DIR)/%,$(_DEP))

VER_TEMP  = $(subst -, ,$(FE_VERSION))
VER_PARTS = $(subst ., ,$(word 1,$(VER_TEMP)))
VER_MAJOR = $(subst v,,$(word 1,$(VER_PARTS)))
VER_MINOR = $(word 2,$(VER_PARTS))
ifneq ($(word 3,$(VER_PARTS)),)
 VER_POINT = $(word 3,$(VER_PARTS))
else
 VER_POINT = 0
endif

# version macros
FE_FLAGS += -DFE_VERSION_MAJOR=$(VER_MAJOR) -DFE_VERSION_MINOR=$(VER_MINOR) -DFE_VERSION_POINT=$(VER_POINT)
FE_FLAGS += -DFE_VERSION_D='"$(FE_VERSION)"' -DFE_VERSION_NUM=$(VER_MAJOR)$(VER_MINOR)$(VER_POINT)

$(OBJ_DIR)/%.res: $(SRC_DIR)/%.rc | $(OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(WINDRES) $(FE_FLAGS) $< -O coff -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP) | $(OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CXX) -c -o $@ $< $(CXXFLAGS) $(CPPFLAGS) $(FE_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.mm $(DEP) | $(OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS) $(FE_FLAGS)

$(EXE): $(OBJ) $(EXPAT) $(SQUIRREL) $(AUDIO)
	$(EXE_MSG)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(CPPFLAGS) $(FE_FLAGS) $(LIBS)
ifeq ($(ENABLE_DEBUG_SYMBOLS),1)
	$(DEBUG_MSG)
	$(SILENT)$(OBJCOPY) --only-keep-debug $@ $@.debug
	$(SILENT)$(STRIP) --strip-debug --strip-unneeded $@
	$(SILENT)$(OBJCOPY) --add-gnu-debuglink=$@.debug $@
else
	$(SILENT)$(STRIP) $@
endif

.PHONY: clean
.PHONY: install

$(OBJ_DIR):
	$(MD) $@

#
# Expat Library
#
EXPAT_OBJ_DIR = $(OBJ_DIR)/expat

EXPATOBJS = \
	$(EXPAT_OBJ_DIR)/xmlparse.o \
	$(EXPAT_OBJ_DIR)/xmlrole.o \
	$(EXPAT_OBJ_DIR)/xmltok.o \
	$(EXPAT_OBJ_DIR)/loadlibrary.o

$(OBJ_DIR)/libexpat.a: $(EXPATOBJS) | $(EXPAT_OBJ_DIR)
	$(AR_MSG)
	$(SILENT)$(AR) $(ARFLAGS) $@ $(EXPATOBJS)

$(EXPAT_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/expat/%.c | $(EXPAT_OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS) $(EXPAT_FLAGS)

$(EXPAT_OBJ_DIR):
	$(MD) $@

#
# Squirrel Library
#
SQUIRREL_FLAGS = -fno-exceptions -fno-rtti -fno-strict-aliasing
SQUIRREL_OBJ_DIR = $(OBJ_DIR)/squirrel

SQUIRRELOBJS= \
	$(SQUIRREL_OBJ_DIR)/sqapi.o \
	$(SQUIRREL_OBJ_DIR)/sqbaselib.o \
	$(SQUIRREL_OBJ_DIR)/sqfuncstate.o \
	$(SQUIRREL_OBJ_DIR)/sqdebug.o \
	$(SQUIRREL_OBJ_DIR)/sqlexer.o \
	$(SQUIRREL_OBJ_DIR)/sqobject.o \
	$(SQUIRREL_OBJ_DIR)/sqcompiler.o \
	$(SQUIRREL_OBJ_DIR)/sqstate.o \
	$(SQUIRREL_OBJ_DIR)/sqtable.o \
	$(SQUIRREL_OBJ_DIR)/sqmem.o \
	$(SQUIRREL_OBJ_DIR)/sqvm.o \
	$(SQUIRREL_OBJ_DIR)/sqclass.o

$(OBJ_DIR)/libsquirrel.a: $(SQUIRRELOBJS) | $(SQUIRREL_OBJ_DIR)
	$(AR_MSG)
	$(SILENT)$(AR) $(ARFLAGS) $@ $(SQUIRRELOBJS)

$(SQUIRREL_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/squirrel/squirrel/%.cpp | $(SQUIRREL_OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS) $(SQUIRREL_FLAGS)

$(SQUIRREL_OBJ_DIR):
	$(MD) $@

#
# Squirrel libsqstdlib
#
SQSTDLIB_OBJ_DIR = $(OBJ_DIR)/sqstdlib

SQSTDLIBOBJS= \
	$(SQSTDLIB_OBJ_DIR)/sqstdblob.o \
	$(SQSTDLIB_OBJ_DIR)/sqstdio.o \
	$(SQSTDLIB_OBJ_DIR)/sqstdstream.o \
	$(SQSTDLIB_OBJ_DIR)/sqstdmath.o \
	$(SQSTDLIB_OBJ_DIR)/sqstdstring.o \
	$(SQSTDLIB_OBJ_DIR)/sqstdaux.o \
	$(SQSTDLIB_OBJ_DIR)/sqstdsystem.o \
	$(SQSTDLIB_OBJ_DIR)/sqstdrex.o

$(OBJ_DIR)/libsqstdlib.a: $(SQSTDLIBOBJS) | $(SQSTDLIB_OBJ_DIR)
	$(AR_MSG)
	$(SILENT)$(AR) $(ARFLAGS) $@ $(SQSTDLIBOBJS)

$(SQSTDLIB_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/squirrel/sqstdlib/%.cpp | $(SQSTDLIB_OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS) $(SQUIRREL_FLAGS)

$(SQSTDLIB_OBJ_DIR):
	$(MD) $@

#
# Audio
#
AUDIO_OBJ_DIR = $(OBJ_DIR)/audiolib

AUDIOOBJS= \
	$(AUDIO_OBJ_DIR)/ALCheck.o \
	$(AUDIO_OBJ_DIR)/AudioDevice.o \
	$(AUDIO_OBJ_DIR)/Listener.o \
	$(AUDIO_OBJ_DIR)/SoundSource.o \
	$(AUDIO_OBJ_DIR)/SoundStream.o

$(OBJ_DIR)/libaudio.a: $(AUDIOOBJS) | $(AUDIO_OBJ_DIR)
	$(AR_MSG)
	$(SILENT)$(AR) $(ARFLAGS) $@ $(AUDIOOBJS)

$(AUDIO_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/audio/Audio/%.cpp | $(AUDIO_OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS)

$(AUDIO_OBJ_DIR):
	$(MD) $@

#
# Nowide
#
NOWIDE_OBJ_DIR = $(OBJ_DIR)/nowidelib

NOWIDEOBJS= \
	$(NOWIDE_OBJ_DIR)/iostream.o \
	$(NOWIDE_OBJ_DIR)/filebuf.o \
	$(NOWIDE_OBJ_DIR)/cstdio.o \
	$(NOWIDE_OBJ_DIR)/cstdlib.o \
	$(NOWIDE_OBJ_DIR)/console_buffer.o \
	$(NOWIDE_OBJ_DIR)/stat.o

$(OBJ_DIR)/libnowide.a: $(NOWIDEOBJS) | $(NOWIDE_OBJ_DIR)
	$(AR_MSG)
	$(SILENT)$(AR) $(ARFLAGS) $@ $(NOWIDEOBJS)

$(NOWIDE_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/nowide/%.cpp | $(NOWIDE_OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS)

$(NOWIDE_OBJ_DIR):
	$(MD) $@

#
# gameswf
#
GAMESWF_OBJ_DIR = $(OBJ_DIR)/gameswflib
GSBASE_OBJ_DIR = $(OBJ_DIR)/gsbaselib

GAMESWFOBJS= \
	$(GSBASE_OBJ_DIR)/configvars.o                          \
	$(GSBASE_OBJ_DIR)/ear_clip_triangulate_float.o          \
	$(GSBASE_OBJ_DIR)/ear_clip_triangulate_sint16.o         \
	$(GSBASE_OBJ_DIR)/container.o                           \
	$(GSBASE_OBJ_DIR)/file_util.o                           \
	$(GSBASE_OBJ_DIR)/image.o                               \
	$(GSBASE_OBJ_DIR)/image_filters.o                       \
	$(GSBASE_OBJ_DIR)/jpeg.o                                \
	$(GSBASE_OBJ_DIR)/logger.o                              \
	$(GSBASE_OBJ_DIR)/membuf.o                              \
	$(GSBASE_OBJ_DIR)/postscript.o                          \
	$(GSBASE_OBJ_DIR)/triangulate_float.o                   \
	$(GSBASE_OBJ_DIR)/triangulate_sint32.o                  \
	$(GSBASE_OBJ_DIR)/tu_file.o                             \
	$(GSBASE_OBJ_DIR)/tu_gc_singlethreaded_marksweep.o      \
	$(GSBASE_OBJ_DIR)/tu_loadlib.o                          \
	$(GSBASE_OBJ_DIR)/tu_random.o                           \
	$(GSBASE_OBJ_DIR)/tu_timer.o                            \
	$(GSBASE_OBJ_DIR)/tu_types.o                            \
	$(GSBASE_OBJ_DIR)/utf8.o                                \
	$(GSBASE_OBJ_DIR)/utility.o                             \
	$(GSBASE_OBJ_DIR)/zlib_adapter.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_array.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_boolean.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_broadcaster.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_class.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_color.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_color_transform.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_date.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_event.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_flash.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_geom.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_global.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_key.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_loadvars.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_math.o   \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_matrix.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_mcloader.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_mouse.o    \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_mouse_event.o    \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_netconnection.o  \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_netstream.o  \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_number.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_point.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_selection.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_sharedobject.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_sound.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_string.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_textformat.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_transform.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_xml.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_classes/as_xmlsocket.o \
	$(GAMESWF_OBJ_DIR)/gameswf_abc.o  \
	$(GAMESWF_OBJ_DIR)/gameswf_action.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_avm2.o \
	$(GAMESWF_OBJ_DIR)/gameswf_as_sprite.o    \
	$(GAMESWF_OBJ_DIR)/gameswf_button.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_canvas.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_character.o    \
	$(GAMESWF_OBJ_DIR)/gameswf_disasm.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_dlist.o        \
	$(GAMESWF_OBJ_DIR)/gameswf_environment.o  \
	$(GAMESWF_OBJ_DIR)/gameswf_filters.o              \
	$(GAMESWF_OBJ_DIR)/gameswf_font.o         \
	$(GAMESWF_OBJ_DIR)/gameswf_function.o     \
	$(GAMESWF_OBJ_DIR)/gameswf_impl.o         \
	$(GAMESWF_OBJ_DIR)/gameswf_listener.o     \
	$(GAMESWF_OBJ_DIR)/gameswf_log.o          \
	$(GAMESWF_OBJ_DIR)/gameswf_morph2.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_movie_def.o    \
	$(GAMESWF_OBJ_DIR)/gameswf_object.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_player.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_render.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_root.o         \
	$(GAMESWF_OBJ_DIR)/gameswf_shape.o        \
	$(GAMESWF_OBJ_DIR)/gameswf_sound.o        \
	$(GAMESWF_OBJ_DIR)/gameswf_sprite.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_sprite_def.o   \
	$(GAMESWF_OBJ_DIR)/gameswf_stream.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_styles.o       \
	$(GAMESWF_OBJ_DIR)/gameswf_tesselate.o    \
	$(GAMESWF_OBJ_DIR)/gameswf_text.o         \
	$(GAMESWF_OBJ_DIR)/gameswf_tools.o        \
	$(GAMESWF_OBJ_DIR)/gameswf_types.o        \
	$(GAMESWF_OBJ_DIR)/gameswf_value.o        \
	$(GAMESWF_OBJ_DIR)/gameswf_video_impl.o   \
	$(GAMESWF_OBJ_DIR)/gameswf_mutex.o   \
	$(GAMESWF_OBJ_DIR)/gameswf_sound_handler_openal.o

ifeq ($(FE_MACOSX_COMPILE),1)
	GAMESWFOBJS += $(GAMESWF_OBJ_DIR)/gameswf_fontlib.o
else
	GAMESWFOBJS += $(GAMESWF_OBJ_DIR)/gameswf_freetype.o
endif

ifeq ($(USE_GLES),1)
	GAMESWFOBJS += $(GAMESWF_OBJ_DIR)/gameswf_render_handler_ogles.o
else
	GAMESWFOBJS += $(GAMESWF_OBJ_DIR)/gameswf_render_handler_ogl.o
endif

$(OBJ_DIR)/libgameswf.a: $(GAMESWFOBJS) | $(GAMESWF_OBJ_DIR) $(GSBASE_OBJ_DIR)
	$(AR_MSG)
	$(SILENT)$(AR) $(ARFLAGS) $@ $(GAMESWFOBJS)

$(GAMESWF_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/gameswf/gameswf/%.cpp | $(GAMESWF_OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS) -Wno-deprecated

$(GAMESWF_OBJ_DIR):
	$(MD) $@
	$(MD) $@/gameswf_as_classes

$(GSBASE_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/gameswf/base/%.cpp | $(GSBASE_OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS) -Wno-deprecated

$(GSBASE_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/gameswf/base/%.c | $(GSBASE_OBJ_DIR)
	$(CC_MSG)
	$(SILENT)$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

$(GSBASE_OBJ_DIR):
	$(MD) $@

$(DATA_PATH):
	$(MD) -p $(DESTDIR)$@

install: $(EXE) $(DATA_PATH)
	install -D $(EXE) $(DESTDIR)$(bindir)/$(EXE)
	mkdir -p $(DESTDIR)$(DATA_PATH)
	cp -r config/* $(DESTDIR)$(DATA_PATH)

smallclean:
	-$(RM) $(OBJ_DIR)/*.o *~ core

clean:
	-$(RM) $(OBJ_DIR)/*.o $(EXPAT_OBJ_DIR)/*.o $(SQUIRREL_OBJ_DIR)/*.o $(SQSTDLIB_OBJ_DIR)/*.o $(AUDIO_OBJ_DIR)/*.o $(NOWIDE_OBJ_DIR)/*.o $(GSBASE_OBJ_DIR)/*.o $(GAMESWF_OBJ_DIR)/*.o $(GAMESWF_OBJ_DIR)/gameswf_as_classes/*.o $(OBJ_DIR)/*.a $(OBJ_DIR)/*.res *~ core attract attract.debug
