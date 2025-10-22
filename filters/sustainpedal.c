MFD_FILTER(sustainpedal)

#ifdef MX_TTF

	mflt:sustainpedal
	TTF_DEFAULTDEF("MIDI Sustain Pedal", "MIDI Sustain Pedal")
	, TTF_IPORT( 0, "pedal",  "Pedal CC", 0, 1, 0,
			lv2:scalePoint [ rdfs:label "CC64" ; rdf:value 0 ] ;
			lv2:scalePoint [ rdfs:label "CC66" ; rdf:value 1 ] ;
			lv2:portProperty lv2:integer;  lv2:portProperty lv2:enumeration;)
	, TTF_IPORTTOGGLE( 1, "forward_cc",   "Forward CC", 0)
	; rdfs:comment "This filter holds any notes that are currently, or subsequently, played when the sustain pedal is pressed for as long as the pedal remains pressed. Releasing the pedal sends Note-Off events."
	.

#elif defined MX_CODE

void
filter_midi_sustainpedal(MidiFilter* self,
		const uint32_t tme,
		const uint8_t* const buffer,
		const uint32_t size)
{
	const int pedal = RAIL(*self->cfg[0], 0, 1);
	const int fwdcc = RAIL(*self->cfg[1], 0, 1);

	const uint8_t chn = buffer[0] & 0x0f;
	const uint8_t key = buffer[1] & 0x7f;
	const uint8_t vel = buffer[2] & 0x7f;

	uint8_t mst = buffer[0] & 0xf0;
	bool    fwd = true;

	int oldstate = self->memI[chn];

	if (size == 3 && mst == MIDI_CONTROLCHANGE && (buffer[1]) == 64 && pedal == 0) {
		self->memI[chn] = vel > 63 ? 1 : 0;
		if (!fwdcc) {
			fwd = false;
		}
	}
	if (size == 3 && mst == MIDI_CONTROLCHANGE && (buffer[1]) == 66 && pedal == 1) {
		self->memI[chn] = vel > 63 ? 1 : 0;
		if (!fwdcc) {
			fwd = false;
		}
	}

	if (midi_is_panic(buffer, size)) {
		for (int k=0; k < 127; ++k) {
			if (self->memCS[chn][k]) {
				uint8_t buf[3];
				buf[0] = MIDI_NOTEOFF | chn;
				buf[1] = k;
				buf[2] = 0;
				forge_midimessage(self, tme, buf, 3);
				self->memCS[chn][k] = 0;
			}
		}
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	if (oldstate && !self->memI[chn]) {
		/* pedal release, send note-off */
		for (int k=0; k < 127; ++k) {
			if (self->memCS[chn][k]) {
				uint8_t buf[3];
				buf[0] = MIDI_NOTEOFF | chn;
				buf[1] = k;
				buf[2] = 0;
				forge_midimessage(self, tme, buf, 3);
				self->memCS[chn][k] = 0;
			}
		}
	}

	if (size != 3 || !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF))
	{
		if (fwd) {
			forge_midimessage(self, tme, buffer, size);
		}
		return;
	}

	if (mst == MIDI_NOTEON && vel ==0 ) {
		mst = MIDI_NOTEOFF;
	}

	if (self->memI[chn]) {
		/* pedal is pressed, filter note-off of held notes */
		if (mst == MIDI_NOTEOFF && self->memCS[chn][key]) {
			return;
		}
		forge_midimessage(self, tme, buffer, size);
		// return;
	}

	/* pedal is not pressed, collect potential notes */
	if (mst == MIDI_NOTEON) {
		self->memCS[chn][key] = 1;
	} else if (mst == MIDI_NOTEOFF) {
		self->memCS[chn][key] = 0;
	}
	forge_midimessage(self, tme, buffer, size);
}

static void filter_init_sustainpedal(MidiFilter* self) {
	for (uint32_t c = 16; c < 16; ++c) {
		self->memI[c] = 0; // per channel pedal (CC64)
		for (int k=0; k < 127; ++k) {
			self->memCS[c][k] = 0;
		}
	}
}

#endif
