MFD_FILTER(velocityscale)

#ifdef MX_TTF

	mflt:velocityscale
	TTF_DEFAULTDEF("MIDI Velocity Adjust")
	, TTF_IPORT(0, "channel", "Channel",  0.0, 16.0,  0.0, PORTENUMZ("Any"))
	, TTF_IPORTFLOAT(1, "gain", "Note-on Gain (mult)",  0.0, 4.0,  1.0)
	, TTF_IPORTFLOAT(2, "offset", "Note-on Level (add)",  -126.0, 126.0,  0.0)
	, TTF_IPORTFLOAT(3, "gain", "Note-off Gain (mult)",  0.0, 4.0,  1.0)
	, TTF_IPORTFLOAT(4, "offset", "Note-off Level (add)",  -127.0, 127.0,  0.0)
	.

#elif defined MX_CODE

static void filter_init_velocityscale(MidiFilter* self) { }

static void
filter_midi_velocityscale(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const int chs = midi_limit_chn(floor(*self->cfg[0]) -1);
	const uint8_t chn = buffer[0] & 0x0f;
	uint8_t mst = buffer[0] & 0xf0;

	if (size != 3
			|| !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF)
			|| !(floor(*self->cfg[0]) == 0 || chs == chn)
		 )
	{
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	const uint8_t vel = (buffer[2] & 0x7f);

	if (mst == MIDI_NOTEON && vel == 0 ) {
		mst = MIDI_NOTEOFF;
	}

	uint8_t buf[3];
	buf[0] = buffer[0];
	buf[1] = buffer[1];

	switch(mst) {
		case MIDI_NOTEON:
			{
				const float gain = *(self->cfg[1]);
				const float off = *(self->cfg[2]);
				buf[2] = RAIL(rintf((float)vel * gain +off), 1, 127);
			}
			break;
		case MIDI_NOTEOFF:
			{
				const float gain = *(self->cfg[3]);
				const float off = *(self->cfg[4]);
				buf[2] = RAIL(rintf((float)vel * gain +off), 1, 127);
			}
			break;
	}
	forge_midimessage(self, tme, buf, size);
}

#endif
