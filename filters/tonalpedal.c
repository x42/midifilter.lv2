MFD_FILTER(tonalpedal)

#ifdef MX_TTF

	mflt:tonalpedal
	TTF_DEFAULTDEF("MIDI Piano Pedal", "MIDI Piano Pedal")
	, TTF_IPORT( 0, "pedal",  "Pedal CC", 0, 1, 0,
			lv2:scalePoint [ rdfs:label "CC64" ; rdf:value 0 ] ;
			lv2:scalePoint [ rdfs:label "CC66" ; rdf:value 1 ] ;
			lv2:portProperty lv2:integer;  lv2:portProperty lv2:enumeration;)
	, TTF_IPORTTOGGLE( 1, "forward_cc",   "Forward CC", 0)
	, TTF_IPORT( 2, "pedal_style",  "Pedal style", 0, 1, 1,
			lv2:scalePoint [ rdfs:label "sustain" ; rdf:value 0 ] ;
			lv2:scalePoint [ rdfs:label "sostenuto" ; rdf:value 1 ] ;
			lv2:portProperty lv2:integer;  lv2:portProperty lv2:enumeration;)
	; rdfs:comment "This filter holds any notes played when the pedal is pressed for as long as the pedal remains pressed. Releasing the pedal sends Note-Off events. When pedal style is set to 'sostenuto', new notes played after pressing the pedal are not affected."
	.

#elif defined MX_CODE

void
filter_midi_tonalpedal(MidiFilter* self,
		const uint32_t tme,
		const uint8_t* const buffer,
		const uint32_t size)
{
	const int pedal = RAIL(*self->cfg[0], 0, 1);
	const int fwdcc = RAIL(*self->cfg[1], 0, 1);
	const int pedalstyle = RAIL(*self->cfg[2], 0, 1);

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
		/* pedal release, send note-off unless manually held */
		for (int k=0; k < 127; ++k) {
			if (self->memCS[chn][k] && !self->memCM[chn][k]) {
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

	if (mst == MIDI_NOTEON && vel == 0) {
		mst = MIDI_NOTEOFF;
	}

	/* record key state */
	self->memCM[chn][key] = (mst == MIDI_NOTEON);

	if (self->memI[chn]) {
		/* pedal is pressed, filter note-off of held notes */
		if (mst == MIDI_NOTEOFF && self->memCS[chn][key]) {
			return;
		}
	}

	/* record sustain state */
	if ((pedalstyle && !self->memI[chn]) || !pedalstyle) {
		self->memCS[chn][key] = (mst == MIDI_NOTEON);
	}

	forge_midimessage(self, tme, buffer, size);

}

static void filter_init_tonalpedal(MidiFilter* self) {
	for (uint32_t c = 16; c < 16; ++c) {
		self->memI[c] = 0; // per channel pedal (CC64)
		for (int k=0; k < 127; ++k) {
			self->memCS[c][k] = 0; // notes sustained by pedal
			self->memCM[c][k] = 0; // notes with key pressed
		}
	}
}

#endif
