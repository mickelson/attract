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
# Uncomment next line to disable movie support (i.e. don't use ffmpeg/libav):
#DISABLE_MOVIE=1
#
# By default, fontconfig is enabled on Linux/FreeBSD and disabled on Mac OS-X
# Uncomment next line to disable fontconfig on Linux/FreeBSD:
#DISABLE_FONTCONFIG=1
#
# Uncomment next line to enable fontconfig on Mac OS-X:
#ENABLE_FONTCONFIG=1
#
###############################

#FE_DEBUG=1

#
# Test for Mac OS-X, so we can handle fontconfig differently
#
UNAME = $(shell uname -a)
ifeq ($(firstword $(filter Darwin,$(UNAME))),Darwin)
ifneq ($(ENABLE_FONTCONFIG),1)
DISABLE_FONTCONFIG=1
endif
endif

CPP=g++
CFLAGS=
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
	-lsfml-audio \
	-lexpat

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
else
LIBS += -lfontconfig
endif

OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))
DEP = $(patsubst %,$(SRC_DIR)/%,$(_DEP))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP)
	$(CPP) -c -o $@ $< $(CFLAGS)

attract: $(OBJ)
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

install: attract
	cp attract $(INSTALL_DIR)

clean:
	rm -f $(OBJ_DIR)/*.o *~ core
