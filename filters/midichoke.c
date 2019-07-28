MFD_FILTER(chokefilter)

#ifdef MX_TTF

	mflt:chokefilter
	TTF_DEFAULTDEF("MIDI Choke", "MIDI Choke")
	, TTF_IPORT(0, "channelf", "Filter Channel",  0, 16,  0, PORTENUMZ("Any") DOC_CHANF)
	, TTF_IPORT(1, "triggerlo", "Low Trigger Note",  0, 127,  42,
			lv2:portProperty lv2:integer; units:unit units:midiNote ;
			rdfs:comment "Lowest note of the note range, which triggers the choke."
			)
	, TTF_IPORT(2, "triggerhi", "High Trigger Note",  0, 127,  42,
			lv2:portProperty lv2:integer; units:unit units:midiNote ;
			rdfs:comment "Highest note of the note range, which triggers the choke."
			)
	, TTF_IPORT(3, "choke", "Choke Note",  0, 127,  46,
			lv2:portProperty lv2:integer; units:unit units:midiNote ;
			rdfs:comment "Note, which is choked when a trigger note is received."
			)
	, TTF_IPORT(4, "rvmode", "Release Velocity",  0, 2, 2,
			lv2:portProperty lv2:integer; lv2:portProperty lv2:enumeration;
			lv2:scalePoint [ rdfs:label "Set to zero" ; rdf:value 0 ] ;
			lv2:scalePoint [ rdfs:label "From choke note-on velocity" ; rdf:value 1 ] ;
			lv2:scalePoint [ rdfs:label "From trigger note-on velocity" ; rdf:value 2 ] ;
			rdfs:comment "What to set the release velocity of the choke note-off to."
			)
	; rdfs:comment "MIDI Choke filter. Send note-off message for choke note (if it is on), when any note within the trigger note range is received." ;
	.
#elif defined MX_CODE

void filter_init_chokefilter(MidiFilter* self)
{
	for (uint8_t c=0; c < 16; ++c) {
		self->memF[c] = 0.0;
	}
}

void filter_midi_chokefilter(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	if (size != 3) {
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	bool block = false;
	const int8_t chf = RAIL(floorf(*self->cfg[0]) -1, -1, 15);
	const uint8_t chn = buffer[0] & 0x0f;
	const uint8_t mst = buffer[0] & 0xf0;

	if ((chf != -1 && chf != chn) || (mst != MIDI_NOTEON && mst != MIDI_NOTEOFF)) {
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	const uint8_t low = midi_limit_val(floorf(*self->cfg[1]));
	const uint8_t high = midi_limit_val(floorf(*self->cfg[2]));
	const uint8_t choke = midi_limit_val(floorf(*self->cfg[3]));
	const uint8_t key = buffer[1] & 0x7f;
	const uint8_t vel = buffer[2] & 0x7f;
	const uint8_t rvmode = RAIL(*self->cfg[4], 0, 2);
	uint8_t relvel;

	switch(mst) {
		case MIDI_NOTEON:
			if (vel > 0) {
				if (key == choke)
					self->memF[chn] = (float)vel;
				else if ((key >= low && key <= high) && self->memF[chn] != 0.0) {
					switch(rvmode) {
						case 0:
							relvel = 0;
							break;
						case 1:  // rel. vel. = choke note vel.
							relvel = (uint8_t)self->memF[chn];
							break;
						case 2:
						default:  // rel. vel. = trigger note vel.
							relvel = vel;
					}
					uint8_t msg[3] = {MIDI_NOTEOFF | chn, choke, relvel};
					forge_midimessage(self, tme, msg, 3);
					self->memF[chn] = 0.0;
				}
				break;
			}
			// fall-through if vel == 0 (note-off)
		case MIDI_NOTEOFF:
			if (key == choke) {
				if (self->memF[chn] == 0.0)
					// note-off for choke note already sent
					block = true;
				else
					self->memF[chn] = 0.0;
			}
	}

	if (!block) forge_midimessage(self, tme, buffer, 3);
}

#endif
