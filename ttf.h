#ifndef _TTF_H_
#define _TTF_H_

/* cfg-port offsets 0,1, are midi ports, . */
#define ADDTWO_0 2
#define ADDTWO_1 3
#define ADDTWO_2 4
#define ADDTWO_3 5
#define ADDTWO_4 6
#define ADDTWO_5 7
#define ADDTWO_6 8
#define ADDTWO_7 9
#define ADDTWO_8 10
#define ADDTWO_9 11
#define ADDTWO_10 12
#define ADDTWO_11 13
#define ADDTWO_12 14
#define ADDTWO_13 15
#define ADDTWO_14 16
#define ADDTWO_15 17

#define PORTIDX(x) ADDTWO##_##x

#define MAINTAINER <HTTPP/gareus.org/rgareus#me>
#define MIDIEXTURI <HTTPP/lv2plug.in/ns/ext/midi#MidiEvent>

#define TTF_DEFAULTDEF(DOAPNAME) \
	a lv2:Plugin ; \
	doap:name DOAPNAME ; \
	doap:license <HTTPP/usefulinc.com/doap/licenses/gpl> ; \
  lv2:project <HTTP/gareus.org/oss/lv2/midifilter> ; \
	lv2:optionalFeature lv2:hardRTCapable ; \
	lv2:requiredFeature urid:map ; \
	lv2:port \
	[ \
		a atom:AtomPort , \
			lv2:InputPort ; \
		atom:bufferType atom:Sequence ; \
		atom:supports MIDIEXTURI ; \
		lv2:index 0 ; \
		lv2:symbol "midiin" ; \
		lv2:name "MIDI In" \
	] , [ \
		a atom:AtomPort , \
			lv2:OutputPort ; \
		atom:bufferType atom:Sequence ; \
		atom:supports MIDIEXTURI ; \
		lv2:index 1 ; \
		lv2:symbol "midiout" ; \
		lv2:name "MIDI Out"; \
	]

#define TTF_IPORT(IDX, SYM, DESC, VMIN, VMAX, VDFLT, ATTR) \
	[ \
    a lv2:InputPort , \
      lv2:ControlPort ; \
    lv2:index PORTIDX(IDX) ; \
    lv2:symbol SYM ; \
    lv2:name DESC; \
    lv2:minimum VMIN ; \
    lv2:maximum VMAX ; \
    lv2:default VDFLT; \
    ATTR \
  ]

#define TTF_IPORTFLOAT(IDX, SYM, DESC, VMIN, VMAX, VDFLT) \
	TTF_IPORT(IDX, SYM, DESC, VMIN, VMAX, VDFLT, ;)

#define TTF_IPORTINT(IDX, SYM, DESC, VMIN, VMAX, VDFLT) \
	TTF_IPORT(IDX, SYM, DESC, VMIN, VMAX, VDFLT, lv2:portProperty lv2:integer; )

#endif

/* variable part */

#ifdef MFD_FLT
#undef MFD_FLT
#endif

#ifdef MX_FILTER
#define MFD_FLT(ID, FNX) \
	else if (!strcmp(descriptor->URI, MFP_URI "#" # FNX)) self->filter_fn = filter_midi_ ## FNX; 

#elif (defined MX_DESC)

#define MFD_FLT(ID, FNX) \
	MF_DESCRIPTOR(ID, "" # FNX)

#elif (defined MX_MANIFEST)

#define MFD_FLT(ID, FNX) \
  <HTTPP/gareus.org/oss/lv2/midifilterHASH ## FNX> \
	a lv2:Plugin ; \
	lv2:binary <@LV2NAME@@LIB_EXT@>  ; \
	rdfs:seeAlso <@LV2NAME@.ttl> . \

#else

#define MFD_FLT(ID, FNX)

#endif
