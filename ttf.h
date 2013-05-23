#ifndef _TTF_H_
#define _TTF_H_

#ifndef MFP_URIX
#define MFP_URIX HTTPP/gareus.org/oss/lv2/midifilter
#endif

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

#define TTF_INTPORT(IDX, SYM, DESC, VMIN, VMAX, VDFLT) \
	[ \
    a lv2:InputPort , \
      lv2:ControlPort ; \
    lv2:index IDX ; \
    lv2:symbol SYM ; \
    lv2:name DESC; \
    lv2:minimum VMIN ; \
    lv2:maximum VMAX ; \
    lv2:default VDFLT; \
    lv2:portProperty lv2:integer; \
  ]

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
