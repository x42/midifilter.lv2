MFD_FILTER(miditranspose)

#ifdef MX_TTF

	mflt:miditranspose
	TTF_DEFAULTDEF("MIDI Chromatic Transpose")
	, TTF_IPORT(0, "channelf", "Filter Channel",  0.0, 16.0,  0.0, \
			lv2:portProperty lv2:integer; lv2:scalePoint [ rdfs:label "all channels" ; rdf:value 0.0 ])
	, TTF_IPORTINT(1, "transpose", "Transpose",  -72.0, 72.0, 0.0)
	.

#elif defined MX_CODE

static void filter_init_miditranspose(MidiFilter* self) {
	int c,k;

	for (c=0; c < 16; ++c) for (k=0; c < 127; ++k) {
		self->memCI[c][k] = -1000; // current key transpose
		self->memCM[c][k] = 0; // velocity
	}
}

static void
filter_midi_miditranspose(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const int chs = floor(*self->cfg[0]);
	const int transp = rint(*(self->cfg[1]));

	/* config changed */
	// TODO option what to do:
	if (self->lcfg[1] != *(self->cfg[1])) {
		int c,k;
		uint8_t buf[3];
		buf[2] = 0;
		for (c=0; c < 16; ++c) {
#if 0 /* send "all notes off" */
			buf[0] = MIDI_CONTROLCHANGE | c;
			buf[1] = 123;
			forge_midimessage(self, tme, buf, 3);

			// send "all sound off"
			buf[0] = 0xb0 | i;
			buf[1] = 120;
			forge_midimessage(self, tme, buf, 3);
#else /* re-transpose playing notes */
			for (k=0; c < 127; ++k) {
				if (!self->memCM[c][k]) continue;

				buf[0] = MIDI_NOTEOFF | c;
				buf[1] = midi_limit(k + self->memCI[c][k]);
				buf[2] = 0;
				forge_midimessage(self, tme, buf, 3);

				buf[0] = MIDI_NOTEON | c;
				buf[1] = midi_limit(k + transp);
				buf[2] = self->memCM[c][k];
				self->memCI[c][k] = transp;
				forge_midimessage(self, tme, buf, 3);
			}
#endif
		}
	}
	self->lcfg[1] = *(self->cfg[1]);


	const uint8_t chn = buffer[0] & 0x0f;
	const uint8_t key = buffer[1] & 0x7f;
	const uint8_t vel = buffer[2] & 0x7f;
	uint8_t mst = buffer[0] & 0xf0;


	if (size != 3
			|| !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF || mst == MIDI_POLYKEYPRESSURE)
			|| !(chs == 0 || chs == chn)
		 )
	{
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	if (mst == MIDI_NOTEON && vel ==0 ) {
		mst = MIDI_NOTEOFF;
	}

	int note;
	uint8_t buf[3];

	buf[0] = buffer[0];
	buf[1] = buffer[1];
	buf[2] = buffer[2];

	switch (mst) {
		case MIDI_NOTEON:
			note = key + transp;
			if (midi_valid(note)) {
				buf[1] = note;
				forge_midimessage(self, tme, buf, size);
			}
			self->memCM[chn][key] = vel;
			self->memCI[chn][key] = transp;
			break;
		case MIDI_NOTEOFF:
			note = key + self->memCI[chn][key];
			if (midi_valid(note)) {
				buf[1] = note;
				forge_midimessage(self, tme, buf, size);
			}
			self->memCM[chn][key] = 0;
			self->memCI[chn][key] = -1000;
			break;
		case MIDI_POLYKEYPRESSURE:
			note = key + transp;
			if (midi_valid(note)) {
				buf[1] = note;
				forge_midimessage(self, tme, buf, size);
			}
			break;
	}
}

#endif
