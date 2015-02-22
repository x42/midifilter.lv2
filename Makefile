#!/usr/bin/make -f

OPTIMIZATIONS ?= -msse -msse2 -mfpmath=sse -ffast-math -fomit-frame-pointer -O3 -fno-finite-math-only
PREFIX ?= /usr/local
CFLAGS ?= $(OPTIMIZATIONS) -Wall
LIBDIR ?= lib

STRIP=strip
STRIPFLAGS=-s

###############################################################################

LV2DIR ?= $(PREFIX)/$(LIBDIR)/lv2
LOADLIBES=-lm
LV2NAME=midifilter
BUNDLE=midifilter.lv2
targets=

UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
  LV2LDFLAGS=-dynamiclib
  LIB_EXT=.dylib
  STRIPFLAGS=-u -r -arch all -s lv2syms
  targets+=lv2syms
else
  LV2LDFLAGS=-Wl,-Bstatic -Wl,-Bdynamic
  LIB_EXT=.so
endif

ifneq ($(XWIN),)
  CC=$(XWIN)-gcc
  STRIP=$(XWIN)-strip
  LV2LDFLAGS=-Wl,-Bstatic -Wl,-Bdynamic -Wl,--as-needed
  LIB_EXT=.dll
  override LDFLAGS += -static-libgcc -static-libstdc++
endif

targets+=$(LV2NAME)$(LIB_EXT)

# check for build-dependencies
ifeq ($(shell pkg-config --exists lv2 || echo no), no)
  $(error "LV2 SDK was not found")
endif

override CFLAGS += -fPIC -std=c99
override CFLAGS += `pkg-config --cflags lv2`

# build target definitions
default: all

all: manifest.ttl presets.ttl $(LV2NAME).ttl $(targets)

FILTERS := $(wildcard filters/*.c)

lv2syms:
	echo "_lv2_descriptor" > lv2syms

filters.c: $(FILTERS)
	echo "#include \"ttf.h\"" > filters.c
	i=0; for file in $(FILTERS); do \
		echo "#define MFD_FILTER(FNX) MFD_FLT($$i, FNX)" >> filters.c; \
		echo "#include \"$${file}\"" >> filters.c; \
		echo "#undef MFD_FILTER" >> filters.c; \
		i=`expr $$i + 1`; \
		done;
	echo "#define LOOP_DESC(FN) \\" >> filters.c;
	i=0; for file in $(FILTERS); do \
		echo "FN($$i) \\" >> filters.c; \
		i=`expr $$i + 1`; \
		done;
	echo >> filters.c;

manifest.ttl: manifest.ttl.in ttf.h filters.c
	cat manifest.ttl.in > manifest.ttl
	gcc -E -I. -DMX_MANIFEST filters.c \
		| grep -v '^\#' \
		| sed "s/HTTPP/http:\//g;s/HASH/#/g;s/@LV2NAME@/$(LV2NAME)/g;s/@LIB_EXT@/$(LIB_EXT)/g" \
		| uniq \
		>> manifest.ttl
	for file in presets/*.ttl; do head -n 3 $$file >> manifest.ttl; echo "rdfs:seeAlso <presets.ttl> ." >> manifest.ttl; done

presets.ttl: presets.ttl.in presets/*.ttl
	cat presets.ttl.in > presets.ttl
	cat presets/*.ttl >> presets.ttl

$(LV2NAME).ttl: $(LV2NAME).ttl.in ttf.h filters.c
	cat $(LV2NAME).ttl.in > $(LV2NAME).ttl
	gcc -E -I. -DMX_TTF filters.c \
		| grep -v '^\#' \
		| sed 's/HTTPP/http:\//g' \
		| uniq \
		>> $(LV2NAME).ttl

$(LV2NAME)$(LIB_EXT): $(LV2NAME).c midifilter.h filters.c
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	  -o $(LV2NAME)$(LIB_EXT) $(LV2NAME).c \
		-shared $(LV2LDFLAGS) $(LDFLAGS) $(LOADLIBES)
	$(STRIP) $(STRIPFLAGS) $(LV2NAME)$(LIB_EXT)

# install/uninstall/clean target definitions

install: all
	install -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	install -m755 $(LV2NAME)$(LIB_EXT) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	install -m644 manifest.ttl $(LV2NAME).ttl presets.ttl $(DESTDIR)$(LV2DIR)/$(BUNDLE)

uninstall:
	rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/manifest.ttl
	rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/presets.ttl
	rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(LV2NAME).ttl
	rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(LV2NAME)$(LIB_EXT)
	-rmdir $(DESTDIR)$(LV2DIR)/$(BUNDLE)

clean:
	rm -f manifest.ttl presets.ttl $(LV2NAME).ttl $(LV2NAME)$(LIB_EXT) lv2syms

.PHONY: clean all install uninstall
