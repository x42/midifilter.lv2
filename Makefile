#!/usr/bin/make -f

OPTIMIZATIONS ?= -msse -msse2 -mfpmath=sse -ffast-math -fomit-frame-pointer -O3 -fno-finite-math-only
PREFIX ?= /usr/local
CFLAGS ?= $(OPTIMIZATIONS) -Wall
LIBDIR ?= lib

###############################################################################

LV2DIR ?= $(PREFIX)/$(LIBDIR)/lv2
LOADLIBES=-lm
LV2NAME=midifilter
BUNDLE=midifilter.lv2

CFLAGS+=-fPIC -std=c99

IS_OSX=
UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
  IS_OSX=yes
  LV2LDFLAGS=-dynamiclib
  LIB_EXT=.dylib
else
  CFLAGS+= -DHAVE_MEMSTREAM
  LV2LDFLAGS=-Wl,-Bstatic -Wl,-Bdynamic
  LIB_EXT=.so
endif

targets=$(LV2NAME)$(LIB_EXT)

# check for build-dependencies
ifeq ($(shell pkg-config --exists lv2 lv2core || echo no), no)
  $(error "LV2 SDK was not found")
else
  CFLAGS+=`pkg-config --cflags lv2 lv2core`
endif

# build target definitions
default: all

all: manifest.ttl $(LV2NAME).ttl $(targets)

manifest.ttl: manifest.ttl.in
	sed "s/@LV2NAME@/$(LV2NAME)/;s/@LIB_EXT@/$(LIB_EXT)/" \
	  manifest.ttl.in > manifest.ttl

$(LV2NAME).ttl: $(LV2NAME).ttl.in
	cat $(LV2NAME).ttl.in > $(LV2NAME).ttl

$(LV2NAME)$(LIB_EXT): $(LV2NAME).c
	$(CC) $(CFLAGS) \
	  -o $(LV2NAME)$(LIB_EXT) $(LV2NAME).c \
	  $(LDFLAGS) $(LOADLIBES) -shared $(LV2LDFLAGS)

# install/uninstall/clean target definitions

install: all
	install -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	install -m755 $(LV2NAME)$(LIB_EXT) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	install -m644 manifest.ttl $(LV2NAME).ttl $(DESTDIR)$(LV2DIR)/$(BUNDLE)

uninstall:
	rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/*.ttl
	rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(LV2NAME)$(LIB_EXT)
	-rmdir $(DESTDIR)$(LV2DIR)/$(BUNDLE)

clean:
	rm -f manifest.ttl $(LV2NAME).ttl $(LV2NAME)$(LIB_EXT)

.PHONY: clean all install uninstall
