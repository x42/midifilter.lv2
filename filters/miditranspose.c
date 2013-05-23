MFD_FILTER(miditranspose)

#ifdef MX_TTF

	mflt:miditranspose
	TTF_DEFAULTDEF("MIDI Chromatic Transpose")
	, TTF_IPORTINT(0, "transpose", "Transpose",  -72.0, 72.0, 0.0)
	.

#elif defined MX_CODE

void filter_init_miditranspose(MidiFilter* self) {
	int c,k;

	for (c=0; c < 16; ++c) for (k=0; c < 127; ++k) {
		self->memCI[c][k] = -1000;
		self->memCM[c][k] = 0;
	}
}

void
filter_midi_miditranspose(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	// TODO, keep track of notes and pitch-bends... etc -> don't mute all
	// TODO allow to select channel to mod
	// TODO option mute note <0 || > 127 

	/* config changed */
	if (self->lcfg[0] != *(self->cfg[0])) {
		int i;
		uint8_t buf[3];
		buf[2] = 0;
		for (i=0; i < 16; ++i) {
			// send "all notes off"
			buf[0] = 0xb0 | i;
			buf[1] = 123;
			forge_midimessage(self, tme, buf, 3);
#if 0
			// send "all sound off"
			buf[0] = 0xb0 | i;
			buf[1] = 120;
			forge_midimessage(self, tme, buf, 3);
#endif
		}
	}
	self->lcfg[0] = *(self->cfg[0]);


	if (size == 3 && (
				((buffer[0] & 0xf0) != 0x90) // Note on
			||
				((buffer[0] & 0xf0) != 0x80) // Note off
			)
		 )
	{
		uint8_t buf[3];
		buf[0] = buffer[0];
		buf[1] = midi_limit(rintf(buffer[1] + self->lcfg[0]));
		buf[2] = buffer[2];
		forge_midimessage(self, tme, buf, size);
	} else {
		forge_midimessage(self, tme, buffer, size);
	}
}

#endif
