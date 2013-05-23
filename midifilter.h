#ifndef _MIDIFILTER_H_
#define _MIDIFILTER_H_

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"

#define MFP_URI "http://gareus.org/oss/lv2/midifilter"

#define MAXCFG 16

#define LOOP_CFG(FN) \
	FN(0)  FN(1)  FN(2)  FN(3) \
	FN(4)  FN(5)  FN(6)  FN(7) \
	FN(8)  FN(9)  FN(10) FN(11) \
	FN(12) FN(13) FN(14) FN(15) \

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
	float lcfg[MAXCFG];

	int     memI[127];
	int     memCI[16][127];
	uint8_t memCM[16][127];

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

static int midi_limit_val(const int);
static int midi_limit_chn(const int);
static int midi_valid(const int);
static int midi_14bit(const uint8_t * const);

#define MIDI_NOTEOFF         0x80
#define MIDI_NOTEON          0x90
#define MIDI_POLYKEYPRESSURE 0xA0
#define MIDI_CONTROLCHANGE   0xB0
#define MIDI_PROGRAMCHANGE   0xC0
#define MIDI_CHANNELPRESSURE 0xD0
#define MIDI_PITCHBEND       0xE0

#define MIDI_SYSEX           0xF0
#define MIDI_QUARTERFRAME    0xF1
#define MIDI_SONGPOSITION    0xF2
#define MIDI_SONGSELECT      0xF3
#define MIDI_F4              0xF4
#define MIDI_F5              0xF5
#define MIDI_TUNEREQUEST     0xF6
#define MIDI_EOX             0xF7
#define MIDI_TIMINGCLOCK     0xF8
#define MIDI_F9              0xF9
#define MIDI_START           0xFA
#define MIDI_CONTINUE        0xFB
#define MIDI_STOP            0xFC
#define MIDI_FD              0xFD
#define MIDI_ACTIVESENSING   0xFE
#define MIDI_SYSTEMRESET     0xFF

#endif
