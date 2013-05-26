MFD_FILTER(randvelocity)

#ifdef MX_TTF

	mflt:randvelocity
	TTF_DEFAULTDEF("MIDI Velocity Randomization")
	, TTF_IPORT(0, "channel", "Filter Channel",  0.0, 16.0,  0.0, PORTENUMZ("Any"))
	, TTF_IPORTFLOAT(1, "randfact", "Velocity Randomization",  0.0, 127.0,  8.0)
	.

#elif defined MX_CODE

static void filter_init_randvelocity(MidiFilter* self) {
	srandom ((unsigned int) time (NULL));
}

static void
filter_midi_randvelocity(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const int chs = midi_limit_chn(floor(*self->cfg[0]) -1);
	const int chn = buffer[0] & 0x0f;

	const uint8_t vel = buffer[2] & 0x7f;
	int mst = buffer[0] & 0xf0;

	if (size != 3
			|| !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF)
			|| !(floor(*self->cfg[0]) == 0 || chs == chn)
		 )
	{
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	if (mst == MIDI_NOTEON && vel ==0 ) {
		mst = MIDI_NOTEOFF;
	}

	uint8_t buf[3];
	const float rf = *(self->cfg[1]);
	const float rnd = -rf + 2.0 * rf * random() / (float)RAND_MAX;
	buf[0] = buffer[0];
	buf[1] = buffer[1];

	switch (mst) {
		case MIDI_NOTEON:
			buf[2] = RAIL(rintf(buffer[2] + rnd), 1, 127);
			break;
		case MIDI_NOTEOFF:
			buf[2] = RAIL(rintf(buffer[2] + rnd), 0, 127);
			break;
	}
	forge_midimessage(self, tme, buf, size);
}

#endif
