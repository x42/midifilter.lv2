#ifndef _MIDIFILTER_H_
#define _MIDIFILTER_H_

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"

#define MAXCFG 16
#define MFP_URI "http://gareus.org/oss/lv2/midifilter"

typedef struct {
	LV2_URID atom_Blank;
	LV2_URID midi_MidiEvent;
	LV2_URID atom_Sequence;
} MidiFilterURIs;


typedef struct _MidiFilter{
	LV2_Atom_Forge forge;
	LV2_Atom_Forge_Frame frame;

	LV2_URID_Map* map;
	MidiFilterURIs uris;

	const LV2_Atom_Sequence* midiin;
	LV2_Atom_Sequence* midiout;

	float *cfg[MAXCFG];
	void (*filter_fn) (struct _MidiFilter*, uint32_t, const uint8_t* const, uint32_t);
} MidiFilter;

void forge_midimessage(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size);

#define MF_DESCRIPTOR(ID, URLSUFFIX) \
	static const LV2_Descriptor descriptor ## ID = { \
		MFP_URI "#" URLSUFFIX, \
		instantiate, \
		connect_port, \
		NULL, \
		run, \
		NULL, \
		cleanup, \
		extension_data \
};

#endif
