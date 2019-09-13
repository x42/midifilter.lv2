MFD_FILTER(velocitygamma)

#ifdef MX_TTF

	mflt:velocitygamma
	TTF_DEFAULTDEF("MIDI Velocity Gamma", "MIDI Vel. Gamma")
	, TTF_IPORT(0, "channel", "Channel", 0, 16, 0,
			PORTENUMZ("Any")
			DOC_CHANF)
	, TTF_IPORTFLOAT(1, "ongamma",  "Note-on Gamma",     0.0,   5.0,   1.0)
	, TTF_IPORTFLOAT(2, "offgamma", "Note-off Gamma",    0.0,   5.0,   1.0)
	; rdfs:comment "Change the velocity of note events with separate controls for Note-on and Note-off. "
                   "Velocities are first normalized to the range 0..1, then the gamma is applied as an "
                   "exponent, and then the result is scaled back onto the 0..127 range. "
                   "Higher gamma values produce a 'softer' velocity curve, lower gamma values make "
                   "the low end harder. Gamma = 0 effectively produces a constant velocity."
	.

#elif defined MX_CODE

static void filter_init_velocitygamma(MidiFilter* self) { }

static void
filter_midi_velocitygamma(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const int chs = midi_limit_chn(floorf(*self->cfg[0]) -1);
	const uint8_t chn = buffer[0] & 0x0f;
	uint8_t mst = buffer[0] & 0xf0;

	if (size != 3
			|| !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF)
			|| !(floorf(*self->cfg[0]) == 0 || chs == chn)
		 )
	{
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	const uint8_t vel  = (buffer[2] & 0x7f);

	if (mst == MIDI_NOTEON && vel == 0 ) {
		mst = MIDI_NOTEOFF;
	}

	uint8_t buf[3];
	buf[0] = buffer[0];
	buf[1] = buffer[1];


	switch(mst) {
		case MIDI_NOTEON:
			{
				const float gamma = *(self->cfg[1]);
				buf[2] = RAIL(rintf(powf((float)vel / 127.0f, gamma) * 127.0f), 1, 127);
			}
			break;
		case MIDI_NOTEOFF:
			{
				const float gamma = *(self->cfg[2]);
				buf[2] = RAIL(rintf(powf((float)vel / 127.0f, gamma) * 127.0f), 1, 127);
			}
			break;
	}
	forge_midimessage(self, tme, buf, size);
}

#endif

