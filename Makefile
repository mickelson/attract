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
# By default, fontconfig is enabled on Linux/FreeBSD and disabled on Mac OS-X.
# Uncomment next line to disable fontconfig on Linux/FreeBSD ...
#DISABLE_FONTCONFIG=1
# ... or uncomment next line to enable fontconfig on Mac OS-X.
#ENABLE_FONTCONFIG=1
#
# By default, if fontconfig is enabled we link against the system's expat 
# library (because fontconfig uses expat too).  If fontconfig is disabled 
# then Attract-Mode is statically linked to its own version of expat.
# Uncomment next line to always link to Attract-Mode's version of expat.
#BUILD_EXPAT=1
###############################

#FE_DEBUG=1

CC=gcc
CPP=g++
CFLAGS=
AR=ar
ARFLAGS=rc
OBJ_DIR=obj
SRC_DIR=src
INSTALL_DIR=/usr/local/bin

_DEP =\
	fe_base.hpp \
	fe_util.hpp \
	fe_info.hpp \
	fe_input.hpp \
	fe_settings.hpp \
	fe_present.hpp \
	fe_image.hpp \
	fe_config.hpp \
	fe_overlay.hpp \
	tp.hpp \
	fe_text.hpp \
	fe_listxml.hpp \
	fe_icon.hpp

_OBJ =\
	fe_base.o \
	fe_util.o \
	fe_info.o \
	fe_input.o \
	fe_settings.o \
	fe_build.o \
	fe_present.o \
	fe_image.o \
	fe_config.o \
	fe_overlay.o \
	tp.o \
	fe_text.o \
	fe_listxml.o \
	main.o

LIBS =\
	-lsfml-window \
	-lsfml-graphics \
	-lsfml-system \
	-lsfml-audio

#
# Test for Mac OS-X, so we can set default disable for fontconfig
#
UNAME = $(shell uname -a)
ifeq ($(firstword $(filter Darwin,$(UNAME))),Darwin)
ifneq ($(ENABLE_FONTCONFIG),1)
DISABLE_FONTCONFIG=1
endif
endif

#
# Now process the various settings...
#
ifeq ($(FE_DEBUG),1)
CFLAGS += -g -Wall -DFE_DEBUG
else
CFLAGS += -O2
endif

ifeq ($(DISABLE_MOVIE),1)
CFLAGS += -DNO_MOVIE
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
CFLAGS += -DNO_FONTCONFIG
BUILD_EXPAT=1
else
LIBS += -lfontconfig
endif

ifeq ($(BUILD_EXPAT),1)
CFLAGS += -Iextlibs/expat
EXPAT = $(OBJ_DIR)/libexpat.a
else
LIBS += -lexpat
EXPAT =
endif

OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))
DEP = $(patsubst %,$(SRC_DIR)/%,$(_DEP))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP)
	$(CPP) -c -o $@ $< $(CFLAGS)

attract: $(OBJ) $(EXPAT)
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

$(OBJ_DIR)/libexpat.a:
	cd extlibs/expat; $(CC) -c xmlparse.c xmlrole.c xmltok.c -DHAVE_MEMMOVE $(CFLAGS); \
	$(AR) $(ARFLAGS) ../../obj/libexpat.a *.o; rm *.o

install: attract
	cp attract $(INSTALL_DIR)

clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.a *~ core
