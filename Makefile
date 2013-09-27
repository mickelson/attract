##
##
##  Attract-Mode frontend
##  Copyright (C) 2013 Andrew Mickelson
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
# Uncomment next line to disable movie support (i.e. don't use ffmpeg).
#DISABLE_MOVIE=1
#
# By default, fontconfig is enabled on Linux & FreeBSD and disabled on Mac OS-X
# & Windows.  Uncomment next line to always disable fontconfig...
#DISABLE_FONTCONFIG=1
# ...or uncomment next line to always enable fontconfig.
#ENABLE_FONTCONFIG=1
#
# By default, if fontconfig is enabled we link against the system's expat 
# library (because fontconfig uses expat too).  If fontconfig is disabled 
# then Attract-Mode is statically linked to its own version of expat.
# Uncomment next line to always link to Attract-Mode's version of expat.
#BUILD_EXPAT=1
#
# By default, layout scripts do not get access to the Squirrel std io or 
# system libraries.  Uncomment the following to allow such access.  This 
# could be useful for creating scripts to integrate with other programs...
#ENABLE_SCRIPT_SYSTEM_ACCESS=1
###############################

#FE_DEBUG=1

CC=gcc
CPP=g++
CFLAGS=
AR=ar
ARFLAGS=rc
RM=rm -f
MD=mkdir
OBJ_DIR=obj
SRC_DIR=src
EXTLIBS_DIR=extlibs
INSTALL_DIR=/usr/local/bin
FE_FLAGS=

_DEP =\
	fe_base.hpp \
	fe_util.hpp \
	fe_info.hpp \
	fe_input.hpp \
	fe_listxml.hpp \
	fe_settings.hpp \
	fe_config.hpp \
	fe_presentable.hpp \
	fe_present.hpp \
	fe_image.hpp \
	fe_sound.hpp \
	fe_overlay.hpp \
	tp.hpp \
	fe_text.hpp \
	fe_icon.hpp

_OBJ =\
	fe_base.o \
	fe_util.o \
	fe_info.o \
	fe_input.o \
	fe_listxml.o \
	fe_settings.o \
	fe_build.o \
	fe_config.o \
	fe_presentable.o \
	fe_present.o \
	fe_image.o \
	fe_sound.o \
	fe_overlay.o \
	tp.o \
	fe_text.o \
	main.o

LIBS =\
	-lsfml-window \
	-lsfml-graphics \
	-lsfml-system \
	-lsfml-audio

#
# Test OS to set defaults
#
ifeq ($(OS),Windows_NT)
ifneq ($(ENABLE_FONTCONFIG),1)
DISABLE_FONTCONFIG=1
endif
else
UNAME = $(shell uname -a)
ifeq ($(firstword $(filter Darwin,$(UNAME))),Darwin)
ifneq ($(ENABLE_FONTCONFIG),1)
DISABLE_FONTCONFIG=1
endif
endif
endif

#
# Now process the various settings...
#
ifeq ($(FE_DEBUG),1)
CFLAGS += -g -Wall
FE_FLAGS += -DFE_DEBUG
else
CFLAGS += -O2
endif

ifeq ($(DISABLE_MOVIE),1)
FE_FLAGS += -DNO_MOVIE
else
LIBS +=\
	-lavformat \
	-lavcodec \
	-lavutil \
	-lswscale
_DEP += media.hpp
_OBJ += media.o
endif

ifeq ($(DISABLE_FONTCONFIG),1)
FE_FLAGS += -DNO_FONTCONFIG
BUILD_EXPAT=1
else
LIBS += -lfontconfig
endif

ifeq ($(BUILD_EXPAT),1)
CFLAGS += -I$(EXTLIBS_DIR)/expat
EXPAT = $(OBJ_DIR)/libexpat.a
else
LIBS += -lexpat
EXPAT =
endif

ifeq ($(ENABLE_SCRIPT_SYSTEM_ACCESS),1)
FE_FLAGS += -DENABLE_SCRIPT_SYSTEM_ACCESS
endif

CFLAGS += -I$(EXTLIBS_DIR)/squirrel/include -I$(EXTLIBS_DIR)/sqrat/include
SQUIRREL = $(OBJ_DIR)/libsquirrel.a $(OBJ_DIR)/libsqstdlib.a

OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))
DEP = $(patsubst %,$(SRC_DIR)/%,$(_DEP))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP) | $(OBJ_DIR)
	$(CPP) -c -o $@ $< $(CFLAGS) $(FE_FLAGS)

attract: $(OBJ) $(EXPAT) $(SQUIRREL)
	$(CPP) -o $@ $^ $(CFLAGS) $(FE_FLAGS) $(LIBS)

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
	$(EXPAT_OBJ_DIR)/xmltok.o

$(OBJ_DIR)/libexpat.a: $(EXPATOBJS) | $(OBJ_DIR)
	$(AR) $(ARFLAGS) $@ $(EXPATOBJS)

$(EXPAT_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/expat/%.c | $(EXPAT_OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS) -DHAVE_MEMMOVE

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
	$(AR) $(ARFLAGS) $@ $(SQUIRRELOBJS)

$(SQUIRREL_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/squirrel/squirrel/%.cpp | $(SQUIRREL_OBJ_DIR)
	$(CPP) -c $< -o $@ $(CFLAGS) $(SQUIRREL_FLAGS) 

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
	$(SQSTDLIB_OBJ_DIR)/sqstdsystem.o \
	$(SQSTDLIB_OBJ_DIR)/sqstdaux.o \
	$(SQSTDLIB_OBJ_DIR)/sqstdrex.o

$(OBJ_DIR)/libsqstdlib.a: $(SQSTDLIBOBJS) | $(SQSTDLIB_OBJ_DIR)
	$(AR) $(ARFLAGS) $@ $(SQSTDLIBOBJS)

$(SQSTDLIB_OBJ_DIR)/%.o: $(EXTLIBS_DIR)/squirrel/sqstdlib/%.cpp | $(SQSTDLIB_OBJ_DIR)
	$(CPP) -c $< -o $@ $(CFLAGS) $(SQUIRREL_FLAGS) 

$(SQSTDLIB_OBJ_DIR):
	$(MD) $@

install: attract
	cp attract $(INSTALL_DIR)

smallclean:
	-$(RM) $(OBJ_DIR)/*.o *~ core

clean:
	-$(RM) $(OBJ_DIR)/*.o $(EXPAT_OBJ_DIR)/*.o $(SQUIRREL_OBJ_DIR)/*.o $(SQSTDLIB_OBJ_DIR)/*.o $(OBJ_DIR)/*.a *~ core
