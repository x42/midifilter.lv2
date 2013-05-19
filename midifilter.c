/* midifilter.lv2
 *
 * Copyright (C) 2013 Robin Gareus <robin@gareus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"

#define MFP_URI "http://gareus.org/oss/lv2/midifilter"

typedef struct {
	LV2_URID atom_Blank;
	LV2_URID midi_MidiEvent;
	LV2_URID atom_Sequence;
} MidiFilterURIs;


typedef struct {
  LV2_Atom_Forge forge;
  LV2_Atom_Forge_Frame frame;

  LV2_URID_Map* map;
  MidiFilterURIs uris;

  const LV2_Atom_Sequence* midiin;
  LV2_Atom_Sequence* midiout;
} MidiFilter;


static inline void
forge_midimessage(LV2_Atom_Forge* forge,
		const MidiFilterURIs* uris,
		uint32_t tme,
		const uint8_t* const msg, uint32_t size)
{
	LV2_Atom midiatom;
	midiatom.type = uris->midi_MidiEvent;
	midiatom.size = size;

	lv2_atom_forge_frame_time(forge, tme);
	lv2_atom_forge_raw(forge, &midiatom, sizeof(LV2_Atom));
	lv2_atom_forge_raw(forge, msg, size);
	lv2_atom_forge_pad(forge, sizeof(LV2_Atom) + size);
}

static void
run(LV2_Handle instance, uint32_t n_samples)
{
	MidiFilter* self = (MidiFilter*)instance;

  const uint32_t capacity = self->midiout->atom.size;
  lv2_atom_forge_set_buffer(&self->forge, (uint8_t*)self->midiout, capacity);
  lv2_atom_forge_sequence_head(&self->forge, &self->frame, 0);

	LV2_Atom_Event* ev = lv2_atom_sequence_begin(&(self->midiin)->body);
	while(!lv2_atom_sequence_is_end(&(self->midiin)->body, (self->midiin)->atom.size, ev)) {
		if (ev->body.type == self->uris.midi_MidiEvent) {
#if 0
      uint8_t msg[3];
      msg[0] = 0xb0; // control change
      msg[1] = 0x00; // param
      msg[2] = 0x00; // val
      forge_midimessage(&self->forge, &self->uris, 0, msg, 3);
#else
      forge_midimessage(&self->forge, &self->uris, ev->time.frames, (uint8_t*)(ev+1), ev->body.size);
#endif
		}
		ev = lv2_atom_sequence_next(ev);
	}
}


static inline void
map_mf_uris(LV2_URID_Map* map, MidiFilterURIs* uris)
{
	uris->atom_Blank         = map->map(map->handle, LV2_ATOM__Blank);
	uris->midi_MidiEvent     = map->map(map->handle, LV2_MIDI__MidiEvent);
  uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
}

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	int i;
	MidiFilter* self = (MidiFilter*)calloc(1, sizeof(MidiFilter));
	if (!self) return NULL;

  for (i=0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_URID__map)) {
      self->map = (LV2_URID_Map*)features[i]->data;
    }
  }

	if (!self->map) {
    fprintf(stderr, "midifilter.lv2 error: Host does not support urid:map\n");
		free(self);
		return NULL;
	}

	map_mf_uris(self->map, &self->uris);
  lv2_atom_forge_init(&self->forge, self->map);

	return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	MidiFilter* self = (MidiFilter*)instance;

	switch (port) {
		case 0:
      self->midiin = (const LV2_Atom_Sequence*)data;
			break;
		case 1:
      self->midiout = (LV2_Atom_Sequence*)data;
			break;
		default:
			break;
	}
}

static void
cleanup(LV2_Handle instance)
{
	free(instance);
}

const void*
extension_data(const char* uri)
{
	return NULL;
}

static const LV2_Descriptor descriptor = {
	MFP_URI,
	instantiate,
	connect_port,
	NULL,
	run,
	NULL,
	cleanup,
	extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}
