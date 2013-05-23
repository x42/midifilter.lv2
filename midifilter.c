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
#include <math.h>

#include "midifilter.h"

/******************************************************************************
 * common 'helper' functions
 */
static int midi_limit_val(const int d) {
	if (d < 0) return 0;
	if (d > 127) return 127;
	return d;
}

static int midi_limit_chn(const int c) {
	if (c < 0) return 0;
	if (c > 15) return 15;
	return c;
}

static int midi_valid(const int d) {
	if (d >=0 && d < 128) return 1;
	return 0;
}

static int midi_14bit(const uint8_t * const b) {
	return ((b[1]) | (b[2]<<7));
}

/**
 * add a midi message to the output port
 */
void
forge_midimessage(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	LV2_Atom midiatom;
	midiatom.type = self->uris.midi_MidiEvent;
	midiatom.size = size;

	lv2_atom_forge_frame_time(&self->forge, tme);
	lv2_atom_forge_raw(&self->forge, &midiatom, sizeof(LV2_Atom));
	lv2_atom_forge_raw(&self->forge, buffer, size);
	lv2_atom_forge_pad(&self->forge, sizeof(LV2_Atom) + size);
}

/******************************************************************************
 * include vairant code
 */

#define MX_CODE
#include "filters.c"
#undef MX_CODE

/******************************************************************************
 * LV2
 */

static void
run(LV2_Handle instance, uint32_t n_samples)
{
	int i;
	MidiFilter* self = (MidiFilter*)instance;

	/* prepare midiout port */
	const uint32_t capacity = self->midiout->atom.size;
	lv2_atom_forge_set_buffer(&self->forge, (uint8_t*)self->midiout, capacity);
	lv2_atom_forge_sequence_head(&self->forge, &self->frame, 0);

	/* process events on the midiin port */
	LV2_Atom_Event* ev = lv2_atom_sequence_begin(&(self->midiin)->body);
	while(!lv2_atom_sequence_is_end(&(self->midiin)->body, (self->midiin)->atom.size, ev)) {
		if (ev->body.type == self->uris.midi_MidiEvent) {
			self->filter_fn(self, ev->time.frames, (uint8_t*)(ev+1), ev->body.size);
		}
		ev = lv2_atom_sequence_next(ev);
	}

	for (i = 0 ; i < MAXCFG ; ++i) {
		if (!self->cfg[i]) continue;
		self->lcfg[i] = *self->cfg[i];
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
instantiate(const LV2_Descriptor*         descriptor,
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

	if (0) ;
#define MX_FILTER
#include "filters.c"
#undef MX_FILTER
	else {
		fprintf(stderr, "midifilter.lv2 error: unsupported plugin function.\n");
		free(self);
		return NULL;
	}

	for (i=0; i < MAXCFG; ++i) {
		self->lcfg[i] = 0;
	}

	return (LV2_Handle)self;
}

#define CFG_PORT(n) \
	case (n+2): \
		self->cfg[n] = (float*)data; \
		break;

static void
connect_port(LV2_Handle    instance,
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
		LOOP_CFG(CFG_PORT)
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

#define MX_DESC
#include "filters.c"
#undef MX_DESC

#define LV2DESC(ID) \
	case ID: return &(descriptor ## ID);

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index) {
	LOOP_DESC(LV2DESC)
	default: return NULL;
	}
}
/* vi:set ts=8 sts=8 sw=8: */
