MFD_FILTER(randvelocity)

#ifdef MX_TTF

	mflt:randvelocity
	TTF_DEFAULTDEF("MIDI Velocity Randomization")
	, TTF_IPORT(0, "channel", "Channel",  0.0, 16.0,  0.0, \
			lv2:portProperty lv2:integer; lv2:scalePoint [ rdfs:label "Any" ; rdf:value 0.0 ])
	, TTF_IPORTFLOAT(1, "randfact", "Velocity Randomization",  0.0, 64.0,  8.0)
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
	const int msg = buffer[0] & 0xf0;

	if (size == 3
			&& (msg == 0x90 || msg == 0x80) // note on, off
			&& (floor(*self->cfg[0]) == 0 || chs == chn)
		 )
	{
		uint8_t buf[3];
		const float rf = *(self->cfg[1]);
		const float rnd = -rf + 2.0 * rf * random() / (float)RAND_MAX;
		buf[0] = buffer[0];
		buf[1] = buffer[1];
		buf[2] = midi_limit_val(rintf(buffer[2] + rnd));
		forge_midimessage(self, tme, buf, size);
	} else {
		forge_midimessage(self, tme, buffer, size);
	}
}

#endif
