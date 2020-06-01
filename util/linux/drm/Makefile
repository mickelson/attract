CC=gcc
PKG_CONFIG=pkg-config
RM=rm -f

prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin

EXE=attract-drm-helper

LIBS=$(shell $(PKG_CONFIG) --libs libdrm)
CFLAGS=$(shell $(PKG_CONFIG) --cflags libdrm)

OBJ = attract-drm-helper.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJ) $(LIBS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
.PHONY: install

clean:
	-$(RM) *.o $(EXE)

install: $(EXE)
	install -D $(EXE) $(bindir)/$(EXE)
	chown root $(bindir)/$(EXE)
	chgrp root $(bindir)/$(EXE)
	chmod u+s $(bindir)/$(EXE)

uninstall:
	-$(RM) $(bindir)/$(EXE)
